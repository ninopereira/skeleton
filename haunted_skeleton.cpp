/// @file haunted_skeleton.cpp
/// @brief Top-level behaviour implementation.

#include "haunted_skeleton.hpp"

#include <Arduino.h>
#include <EEPROM.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

/// voice_index_ value before the first greeting: no clip to avoid yet.
constexpr uint8_t NO_VOICE = Mouth::SPEECH_COUNT;

}  // namespace

HauntedSkeleton::HauntedSkeleton()
    : mp3_(Serial, config::MP3_BUSY_PIN),
      sensor_(),
      mouth_(config::MOUTH_SERVO_PIN,
             config::MOUTH_CLOSED_DEG,
             config::MOUTH_OPEN_DEG),
      arms_(),
      eyes_(config::EYES_LED_PIN),
      playlist_(),
      mode_(Mode::WAITING),
      cooldown_start_ms_(0U),
      voice_start_ms_(0U),
      voice_index_(NO_VOICE),
      song_start_ms_(0U),
      last_play_ms_(0U),
      song_queued_ms_(0U),
      is_song_queued_(false),
      song_retry_count_(0U),
      last_busy_ms_(0U)
{
}

void HauntedSkeleton::begin()
{
  eyes_.begin();
  mouth_.begin();
  arms_.begin();

  if (!sensor_.begin())
  {
    signalError(config::SENSOR_ERROR_BLINK_COUNT);
  }
  if (!mp3_.begin(config::MP3_VOLUME))
  {
    signalError(config::MP3_ERROR_BLINK_COUNT);
  }
  seedFromBootCount();
  playlist_.begin();
  queueSong(millis());
}

void HauntedSkeleton::update(uint32_t now_ms)
{
  sensor_.update(now_ms);
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

void HauntedSkeleton::seedFromBootCount()
{
  uint32_t boot_count = 0U;
  EEPROM.get(config::BOOT_COUNT_EEPROM_ADDR, boot_count);
  ++boot_count;
  // put() only rewrites bytes that changed, which spares the EEPROM.
  EEPROM.put(config::BOOT_COUNT_EEPROM_ADDR, boot_count);
  // Mix in the time since reset too, in case the count was ever reset.
  randomSeed(boot_count ^ micros());
}

void HauntedSkeleton::queueSong(uint32_t now_ms)
{
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
  const uint16_t track = playlist_.getTrack();
  mp3_.play(track);
  song_start_ms_ = now_ms;
  last_play_ms_ = now_ms;
  // BUSY takes a moment to go active; do not count that as silence.
  last_busy_ms_ = now_ms;
}

bool HauntedSkeleton::hasSongStopped(uint32_t now_ms)
{
  if (mp3_.isPlaying())
  {
    last_busy_ms_ = now_ms;
    return false;
  }
  return (now_ms - last_busy_ms_) >= config::MP3_STOPPED_MS;
}

void HauntedSkeleton::handleMp3Events(const Mp3Player::Events& events,
                                      uint32_t now_ms)
{
  if (is_song_queued_)
  {
    // Late reports about the previous song; the queued one will replace it.
    return;
  }
  // This module never reports the end of a track, so BUSY is what
  // normally ends a song; a report, if one comes, is just as good.
  const bool is_song_over = events.is_track_finished || hasSongStopped(now_ms);
  const bool is_play_recent =
      (now_ms - last_play_ms_) < config::MP3_FINISH_GUARD_MS;
  const bool is_song_settled =
      (now_ms - song_start_ms_) >= config::MP3_FINISH_GUARD_MS;
  if (events.has_error && is_play_recent)
  {
    if (song_retry_count_ < config::MP3_PLAY_RETRY_COUNT)
    {
      ++song_retry_count_;
      queueSong(now_ms);
    }
    else
    {
      // The song itself is unplayable (e.g. missing): move on.
      song_retry_count_ = 0U;
      playlist_.advance();
      queueSong(now_ms);
    }
  }
  else if (is_song_over && is_song_settled)
  {
    song_retry_count_ = 0U;
    playlist_.advance();
    queueSong(now_ms);
  }
  else
  {
    // Nothing to act on.
  }
}

void HauntedSkeleton::updateMusic(uint32_t now_ms)
{
  // Poll in every mode so the receive buffer is drained during the voice.
  const Mp3Player::Events events = mp3_.poll();
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
        // BUSY was not watched during the voice; start afresh.
        last_busy_ms_ = now_ms;
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
