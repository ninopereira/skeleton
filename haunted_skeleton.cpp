/// @file haunted_skeleton.cpp
/// @brief Top-level behaviour implementation.

#include "haunted_skeleton.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{

HauntedSkeleton::HauntedSkeleton()
    : mp3_(config::MP3_RX_PIN, config::MP3_TX_PIN),
      sonar_(config::SONAR_TRIG_PIN, config::SONAR_ECHO_PIN),
      mouth_(config::MOUTH_SERVO_PIN,
             config::MOUTH_CLOSED_DEG,
             config::MOUTH_OPEN_DEG),
      arms_(),
      eyes_(config::EYES_LED_PIN),
      mode_(Mode::WAITING),
      cooldown_start_ms_(0U),
      voice_start_ms_(0U),
      song_index_(0U),
      song_start_ms_(0U)
{
}

void HauntedSkeleton::begin()
{
  eyes_.begin();
  sonar_.begin();
  mouth_.begin();
  arms_.begin();

  if (!mp3_.begin(config::MP3_VOLUME))
  {
    signalMp3Error();
  }
  mp3_.play(config::MP3_FIRST_SONG_TRACK);
  song_start_ms_ = millis();
}

void HauntedSkeleton::update(uint32_t now_ms)
{
  sonar_.update(now_ms);
  mouth_.update(now_ms);
  arms_.update(now_ms);
  updateMusic(now_ms);
  updateMode(now_ms);
}

void HauntedSkeleton::signalMp3Error()
{
  // Blocking is fine here: it only runs once, from begin().
  for (uint8_t i = 0U; i < config::MP3_ERROR_BLINK_COUNT; ++i)
  {
    eyes_.turnOn();
    delay(config::MP3_ERROR_BLINK_MS);
    eyes_.turnOff();
    delay(config::MP3_ERROR_BLINK_MS);
  }
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
  // While talking, a finish report is the voice's; the TALKING state
  // restarts the music itself when the jaw script ends.
  const bool is_voice_active =
      (mode_ == Mode::STARTING) || (mode_ == Mode::TALKING);
  const bool is_song_settled =
      (now_ms - song_start_ms_) >= config::MP3_FINISH_GUARD_MS;
  if (is_track_finished && !is_voice_active && is_song_settled)
  {
    playNextSong(now_ms);
  }
}

void HauntedSkeleton::greetVisitor(uint32_t now_ms)
{
  eyes_.turnOn();
  mp3_.play(config::MP3_VOICE_TRACK);
  voice_start_ms_ = now_ms;
  // The visitor's arrival time is unpredictable, which makes it a good
  // seed: which arm leads differs on every greeting.
  randomSeed(micros());
  mode_ = Mode::STARTING;
}

void HauntedSkeleton::updateMode(uint32_t now_ms)
{
  switch (mode_)
  {
    case Mode::WAITING:
      if (sonar_.isTargetPresent())
      {
        greetVisitor(now_ms);
      }
      break;
    case Mode::STARTING:
      if ((now_ms - voice_start_ms_) >= config::VOICE_LATENCY_MS)
      {
        mouth_.startSpeech(now_ms);
        arms_.start(now_ms);
        mode_ = Mode::TALKING;
      }
      break;
    case Mode::TALKING:
      // The jaw script lasts exactly as long as the clip, so its end is
      // the end of the voice.
      if (!mouth_.isSpeaking())
      {
        playNextSong(now_ms);
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
