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
    signalError(config::MP3_ERROR_BLINK_COUNT);
  }
  mp3_.play(config::MP3_FIRST_SONG_TRACK);
  song_start_ms_ = millis();
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
  if (!config::SERIAL_DEBUG ||
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

void HauntedSkeleton::playNextSong(uint32_t now_ms)
{
  ++song_index_;
  if (song_index_ >= config::MP3_SONG_COUNT)
  {
    song_index_ = 0U;
  }
  mp3_.play(
      static_cast<uint16_t>(config::MP3_FIRST_SONG_TRACK + song_index_));
  song_start_ms_ = now_ms;
}

void HauntedSkeleton::updateMusic(uint32_t now_ms)
{
  // Poll in every mode so the receive buffer is drained during the voice.
  const bool is_track_finished = mp3_.pollTrackFinished();
  // While talking, the song is paused under the voice, so a finish report
  // belongs to the voice.
  const bool is_voice_active =
      (mode_ == Mode::STARTING) || (mode_ == Mode::TALKING);
  const bool is_song_settled =
      (now_ms - song_start_ms_) >= config::MP3_FINISH_GUARD_MS;
  if (is_track_finished && !is_voice_active && is_song_settled)
  {
    playNextSong(now_ms);
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
