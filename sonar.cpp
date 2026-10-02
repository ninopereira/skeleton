/// @file sonar.cpp
/// @brief HC-SR04 sonar implementation.

#include "sonar.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

constexpr uint32_t TRIG_SETTLE_US = 2U;
constexpr uint32_t TRIG_PULSE_US = 10U;
/// Sound travels 1 cm and back in ~58 us.
constexpr uint32_t US_PER_CM = 58U;

}  // namespace

Sonar::Sonar(uint8_t trig_pin, uint8_t echo_pin)
    : trig_pin_(trig_pin),
      echo_pin_(echo_pin),
      last_ping_ms_(0U),
      last_distance_cm_(NO_ECHO),
      hit_count_(0U)
{
}

void Sonar::begin()
{
  pinMode(trig_pin_, OUTPUT);
  digitalWrite(trig_pin_, LOW);
  pinMode(echo_pin_, INPUT);
}

void Sonar::update(uint32_t now_ms)
{
  // Unsigned subtraction stays correct across the millis() wrap.
  if ((now_ms - last_ping_ms_) < config::SONAR_PERIOD_MS)
  {
    return;
  }
  last_ping_ms_ = now_ms;
  last_distance_cm_ = measureCm();

  const bool in_range = (last_distance_cm_ >= config::DETECT_MIN_CM) &&
                        (last_distance_cm_ <= config::DETECT_MAX_CM);
  if (!in_range)
  {
    hit_count_ = 0U;
  }
  else if (hit_count_ < config::DETECT_CONFIRM_COUNT)
  {
    ++hit_count_;
  }
  else
  {
    // Already confirmed; saturate instead of overflowing.
  }
}

bool Sonar::isTargetPresent() const
{
  return hit_count_ >= config::DETECT_CONFIRM_COUNT;
}

int16_t Sonar::getLastDistanceCm() const
{
  return last_distance_cm_;
}

int16_t Sonar::measureCm() const
{
  digitalWrite(trig_pin_, LOW);
  delayMicroseconds(TRIG_SETTLE_US);
  digitalWrite(trig_pin_, HIGH);
  delayMicroseconds(TRIG_PULSE_US);
  digitalWrite(trig_pin_, LOW);

  const uint32_t echo_us = static_cast<uint32_t>(
      pulseIn(echo_pin_, HIGH, config::SONAR_TIMEOUT_US));
  if (echo_us == 0U)
  {
    return NO_ECHO;
  }
  // Timeout bounds echo_us, so the result always fits in int16_t.
  return static_cast<int16_t>(echo_us / US_PER_CM);
}

}  // namespace skeleton
