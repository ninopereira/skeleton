/// @file arms.cpp
/// @brief Two-arm servo controller implementation.

#include "arms.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{

Arms::Arms()
    : left_(config::LEFT_ARM_SERVO_PIN,
            config::LEFT_ARM_DOWN_DEG,
            config::LEFT_ARM_UP_DEG),
      right_(config::RIGHT_ARM_SERVO_PIN,
             config::RIGHT_ARM_DOWN_DEG,
             config::RIGHT_ARM_UP_DEG),
      last_update_ms_(0U)
{
}

void Arms::begin()
{
  left_.begin();
  right_.begin();
}

void Arms::start(uint32_t now_ms)
{
  const uint32_t stagger_ms = static_cast<uint32_t>(
      random(static_cast<long>(config::ARM_STAGGER_MIN_MS),
             static_cast<long>(config::ARM_STAGGER_MAX_MS) + 1L));
  const bool is_left_first = (random(2L) == 0L);
  left_.start(now_ms, is_left_first ? 0U : stagger_ms);
  right_.start(now_ms, is_left_first ? stagger_ms : 0U);
}

void Arms::update(uint32_t now_ms)
{
  if ((now_ms - last_update_ms_) < config::ARM_UPDATE_PERIOD_MS)
  {
    return;
  }
  last_update_ms_ = now_ms;
  left_.update(now_ms);
  right_.update(now_ms);
}

bool Arms::isAtRest()
{
  return left_.isAtRest() && right_.isAtRest();
}

}  // namespace skeleton
