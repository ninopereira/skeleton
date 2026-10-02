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
      music_paused_at_ms_(0U),
      is_music_paused_(false)
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
  updateMusic(now_ms);
}

void HauntedSkeleton::greetVisitor(uint32_t now_ms)
{
  mp3_.pause();
  is_music_paused_ = true;
  music_paused_at_ms_ = now_ms;
  // The visitor's arrival time is unpredictable, which makes it a good
  // seed: the arm pattern differs on every greeting.
  randomSeed(micros());
  mouth_.startSpeech(now_ms);
  arms_.start(now_ms);
  mode_ = Mode::TALKING;
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
    case Mode::TALKING:
      if (!mouth_.isSpeaking())
      {
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

void HauntedSkeleton::updateMusic(uint32_t now_ms)
{
  if (is_music_paused_ &&
      ((now_ms - music_paused_at_ms_) >= config::MUSIC_PAUSE_MS))
  {
    mp3_.resume();
    is_music_paused_ = false;
  }
}

}  // namespace skeleton
