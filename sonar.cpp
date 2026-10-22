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

Sonar::Sonar() : filter_(), last_ping_ms_(0U), last_distance_cm_(NO_ECHO)
{
}

bool Sonar::begin()
{
  pinMode(config::SONAR_TRIG_PIN, OUTPUT);
  digitalWrite(config::SONAR_TRIG_PIN, LOW);
  pinMode(config::SONAR_ECHO_PIN, INPUT);
  return true;
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
  filter_.addReading(last_distance_cm_);
}

bool Sonar::isTargetPresent() const
{
  return filter_.isTargetPresent();
}

int16_t Sonar::getLastDistanceCm() const
{
  return last_distance_cm_;
}

int16_t Sonar::measureCm() const
{
  digitalWrite(config::SONAR_TRIG_PIN, LOW);
  delayMicroseconds(TRIG_SETTLE_US);
  digitalWrite(config::SONAR_TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_US);
  digitalWrite(config::SONAR_TRIG_PIN, LOW);

  const uint32_t echo_us = static_cast<uint32_t>(
      pulseIn(config::SONAR_ECHO_PIN, HIGH, config::SONAR_TIMEOUT_US));
  if (echo_us == 0U)
  {
    return NO_ECHO;
  }
  // Timeout bounds echo_us, so the result always fits in int16_t.
  return static_cast<int16_t>(echo_us / US_PER_CM);
}

}  // namespace skeleton
