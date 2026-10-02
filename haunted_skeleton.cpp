/// @file haunted_skeleton.cpp
/// @brief Top-level behaviour implementation.

#include "haunted_skeleton.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{

HauntedSkeleton::HauntedSkeleton()
    : robot_(),
      mp3_(Serial),
      sonar_(config::SONAR_TRIG_PIN, config::SONAR_ECHO_PIN),
      mouth_(config::MOUTH_SERVO_PIN,
             config::MOUTH_CLOSED_DEG,
             config::MOUTH_OPEN_DEG),
      arms_(robot_),
      buzzer_(config::BUZZER_PIN),
      mode_(Mode::WAITING),
      cooldown_start_ms_(0U),
      voice_start_ms_(0U)
{
}

void HauntedSkeleton::begin()
{
  robot_.spiConnect(config::BNR_SPI_SS_PIN);
  robot_.setMinBatteryV(config::MIN_BATTERY_V);
  arms_.begin();

  buzzer_.begin();
  sonar_.begin();
  mouth_.begin();

  mp3_.begin(config::MP3_BAUD);
  mp3_.setVolume(config::MP3_VOLUME);
  // Boot signal; placed last so the music follows the beeps immediately.
  buzzer_.beep(config::BOOT_BEEP_COUNT);
  mp3_.playLooped(config::MP3_MUSIC_TRACK);
}

void HauntedSkeleton::update(uint32_t now_ms)
{
  sonar_.update(now_ms);
  mouth_.update(now_ms);
  arms_.update(now_ms);
  updateMode(now_ms);
}

void HauntedSkeleton::greetVisitor(uint32_t now_ms)
{
  mp3_.playOnce(config::MP3_VOICE_TRACK);
  voice_start_ms_ = now_ms;
  // The visitor's arrival time is unpredictable, which makes it a good
  // seed: the arm pattern differs on every greeting.
  randomSeed(micros());
  arms_.start(now_ms);
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
        mode_ = Mode::TALKING;
      }
      break;
    case Mode::TALKING:
      // The jaw script lasts exactly as long as the clip, so its end is
      // the end of the voice.
      if (!mouth_.isSpeaking())
      {
        mp3_.playLooped(config::MP3_MUSIC_TRACK);
        arms_.goHome(now_ms);
        cooldown_start_ms_ = now_ms;
        mode_ = Mode::COOLDOWN;
      }
      break;
    case Mode::COOLDOWN:
      // Unsigned subtraction stays correct across the millis() wrap.
      if ((now_ms - cooldown_start_ms_) >= config::MOUTH_COOLDOWN_MS)
      {
        mode_ = Mode::WAITING;
      }
      break;
  }
}

}  // namespace skeleton
