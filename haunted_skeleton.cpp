/// @file haunted_skeleton.cpp
/// @brief Top-level behaviour implementation.

#include "haunted_skeleton.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

/// voice_index_ value before the first greeting: no clip to avoid yet.
constexpr uint8_t NO_VOICE = Mouth::SPEECH_COUNT;

}  // namespace

HauntedSkeleton::HauntedSkeleton()
    : mp3_(config::MP3_RX_PIN, config::MP3_TX_PIN),
      sensor_(),
      mouth_(config::MOUTH_SERVO_PIN,
             config::MOUTH_CLOSED_DEG,
             config::MOUTH_OPEN_DEG),
      arms_(),
      eyes_(config::EYES_LED_PIN),
      mode_(Mode::WAITING),
      cooldown_start_ms_(0U),
      voice_start_ms_(0U),
      voice_index_(NO_VOICE),
      song_index_(0U),
      song_start_ms_(0U),
      last_play_ms_(0U),
      song_queued_ms_(0U),
      is_song_queued_(false),
      song_retry_count_(0U),
      last_print_ms_(0U)
{
}

void HauntedSkeleton::begin()
{
  eyes_.begin();
  mouth_.begin();
  arms_.begin();
  if (config::SERIAL_DEBUG)
  {
    Serial.begin(config::SERIAL_BAUD);
  }

  if (!sensor_.begin())
  {
    if (config::SERIAL_DEBUG)
    {
      Serial.println(F("Distance sensor not answering"));
    }
    signalError(config::SENSOR_ERROR_BLINK_COUNT);
  }
  if (!mp3_.begin(config::MP3_VOLUME))
  {
    if (config::SERIAL_DEBUG)
    {
      Serial.println(F("MP3 player not answering"));
    }
    signalError(config::MP3_ERROR_BLINK_COUNT);
  }
  queueSong(0U, millis());
}

void HauntedSkeleton::update(uint32_t now_ms)
{
  sensor_.update(now_ms);
  printSensor(now_ms);
  mouth_.update(now_ms);
  arms_.update(now_ms);
  updateMusic(now_ms);
  updateMode(now_ms);
}

void HauntedSkeleton::signalError(uint8_t blink_count)
{
  // Blocking is fine here: it only runs from begin().
  for (uint8_t i = 0U; i < blink_count; ++i)
  {
    eyes_.turnOn();
    delay(config::ERROR_BLINK_MS);
    eyes_.turnOff();
    delay(config::ERROR_BLINK_MS);
  }
  delay(config::ERROR_GAP_MS);
}

void HauntedSkeleton::printSensor(uint32_t now_ms)
{
  if (!config::SERIAL_DEBUG || !config::SERIAL_DEBUG_SENSOR ||
      ((now_ms - last_print_ms_) < config::SERIAL_PRINT_PERIOD_MS))
  {
    return;
  }
  last_print_ms_ = now_ms;
  const int16_t distance_cm = sensor_.getLastDistanceCm();
  if (distance_cm == PresenceSensor::NO_TARGET)
  {
    Serial.print(F("-- cm"));
  }
  else
  {
    Serial.print(distance_cm);
    Serial.print(F(" cm"));
  }
  Serial.println(sensor_.isTargetPresent() ? F("  VISITOR") : F(""));
}

uint16_t HauntedSkeleton::nextSongIndex() const
{
  const uint16_t next = static_cast<uint16_t>(song_index_ + 1U);
  return (next >= config::MP3_SONG_COUNT) ? 0U : next;
}

void HauntedSkeleton::queueSong(uint16_t song_index, uint32_t now_ms)
{
  song_index_ = song_index;
  song_queued_ms_ = now_ms;
  is_song_queued_ = true;
}

void HauntedSkeleton::playQueuedSong(uint32_t now_ms)
{
  if (!is_song_queued_ ||
      ((now_ms - song_queued_ms_) < config::MP3_NEXT_SONG_DELAY_MS))
  {
    return;
  }
  is_song_queued_ = false;
  const uint16_t track =
      static_cast<uint16_t>(config::MP3_FIRST_SONG_TRACK + song_index_);
  mp3_.play(track);
  song_start_ms_ = now_ms;
  last_play_ms_ = now_ms;
  if (config::SERIAL_DEBUG && config::SERIAL_DEBUG_MP3)
  {
    Serial.print(F("MP3 playing track "));
    Serial.println(track);
  }
}

void HauntedSkeleton::handleMp3Events(const Mp3Player::Events& events,
                                      uint32_t now_ms)
{
  if (is_song_queued_)
  {
    // Late reports about the previous song; the queued one will replace it.
    return;
  }
  const bool is_play_recent =
      (now_ms - last_play_ms_) < config::MP3_FINISH_GUARD_MS;
  const bool is_song_settled =
      (now_ms - song_start_ms_) >= config::MP3_FINISH_GUARD_MS;
  if (events.has_error && is_play_recent)
  {
    if (song_retry_count_ < config::MP3_PLAY_RETRY_COUNT)
    {
      ++song_retry_count_;
      queueSong(song_index_, now_ms);
    }
    else
    {
      // The song itself is unplayable (e.g. missing): move on.
      song_retry_count_ = 0U;
      queueSong(nextSongIndex(), now_ms);
    }
  }
  else if (events.is_track_finished && is_song_settled)
  {
    song_retry_count_ = 0U;
    queueSong(nextSongIndex(), now_ms);
  }
  else
  {
    // Nothing to act on.
  }
}

void HauntedSkeleton::printMp3Events(const Mp3Player::Events& events) const
{
  if (!config::SERIAL_DEBUG || !config::SERIAL_DEBUG_MP3)
  {
    return;
  }
  if (events.is_track_finished)
  {
    Serial.println(F("MP3 track finished"));
  }
  if (events.has_error)
  {
    // 1 busy, 2 sleeping, 3 bad frame, 4 bad checksum, 5 track out of
    // range, 6 track not found.
    Serial.print(F("MP3 error "));
    Serial.println(events.error_code);
  }
  if (events.has_other)
  {
    // Library types: 0 no reply (timeout), 1 garbled message, 2 card in,
    // 3 card out, 4 card online (the module restarted), 11 query reply.
    Serial.print(F("MP3 message "));
    Serial.print(events.other_type);
    Serial.print(F(" cmd 0x"));
    Serial.print(events.other_command, HEX);
    Serial.print(F(" "));
    Serial.println(events.other_value);
  }
}

void HauntedSkeleton::updateMusic(uint32_t now_ms)
{
  // Poll in every mode so the receive buffer is drained during the voice.
  const Mp3Player::Events events = mp3_.poll();
  printMp3Events(events);
  // While talking, the song is paused under the voice, so reports belong
  // to the voice, and a new song must not cut it off.
  const bool is_voice_active =
      (mode_ == Mode::STARTING) || (mode_ == Mode::TALKING);
  if (!is_voice_active)
  {
    handleMp3Events(events, now_ms);
    playQueuedSong(now_ms);
  }
}

uint8_t HauntedSkeleton::pickVoice() const
{
  if ((voice_index_ == NO_VOICE) || (Mouth::SPEECH_COUNT < 2U))
  {
    return static_cast<uint8_t>(random(Mouth::SPEECH_COUNT));
  }
  // Draw from the other clips and step over the last one, so each of them
  // is equally likely.
  uint8_t pick =
      static_cast<uint8_t>(random(Mouth::SPEECH_COUNT - 1U));
  if (pick >= voice_index_)
  {
    ++pick;
  }
  return pick;
}

void HauntedSkeleton::greetVisitor(uint32_t now_ms)
{
  eyes_.turnOn();
  // The visitor's arrival time is unpredictable, which makes it a good
  // seed: the phrase and which arm leads differ on every greeting.
  randomSeed(micros());
  voice_index_ = pickVoice();
  // An advert, not play(): the module pauses the song and resumes it from
  // the same position once the voice ends. Advert files are 1-based.
  mp3_.playAdvert(static_cast<uint16_t>(voice_index_ + 1U));
  voice_start_ms_ = now_ms;
  mode_ = Mode::STARTING;
}

void HauntedSkeleton::updateMode(uint32_t now_ms)
{
  switch (mode_)
  {
    case Mode::WAITING:
      if (sensor_.isTargetPresent())
      {
        greetVisitor(now_ms);
      }
      break;
    case Mode::STARTING:
      if ((now_ms - voice_start_ms_) >= config::VOICE_LATENCY_MS)
      {
        mouth_.startSpeech(voice_index_, now_ms);
        arms_.start(now_ms);
        mode_ = Mode::TALKING;
      }
      break;
    case Mode::TALKING:
      // The jaw script lasts exactly as long as the clip, so its end is
      // the end of the voice.
      if (!mouth_.isSpeaking())
      {
        // The module resumes the interrupted song by itself. Restart the
        // guard so a late finish report from the voice does not skip it.
        song_start_ms_ = now_ms;
        cooldown_start_ms_ = now_ms;
        mode_ = Mode::COOLDOWN;
      }
      break;
    case Mode::COOLDOWN:
      // Unsigned subtraction stays correct across the millis() wrap.
      if ((now_ms - cooldown_start_ms_) >= config::MOUTH_COOLDOWN_MS)
      {
        eyes_.turnOff();
        mode_ = Mode::WAITING;
      }
      break;
  }
}

}  // namespace skeleton
