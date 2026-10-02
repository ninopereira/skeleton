/// @file arms.cpp
/// @brief Two-arm motor controller implementation.

#include "arms.hpp"

#include "config.hpp"

namespace skeleton
{

Arms::Arms(BnrOneAPlus& robot)
    : robot_(robot),
      left_(config::LEFT_ARM_UP_SIGN),
      right_(config::RIGHT_ARM_UP_SIGN),
      last_control_ms_(0U)
{
}

void Arms::begin()
{
  robot_.stop();
  robot_.resetEncoders();
}

void Arms::start(uint32_t now_ms)
{
  left_.start(now_ms);
  right_.start(now_ms);
}

void Arms::goHome(uint32_t now_ms)
{
  left_.goHome(now_ms);
  right_.goHome(now_ms);
}

void Arms::update(uint32_t now_ms)
{
  if ((now_ms - last_control_ms_) < config::ARM_CONTROL_PERIOD_MS)
  {
    return;
  }
  last_control_ms_ = now_ms;

  // One SPI request returns both counts since the last call and clears them.
  int left_counts = 0;
  int right_counts = 0;
  robot_.readAndResetEncoders(left_counts, right_counts);
  const int16_t left_rpm =
      left_.update(now_ms, static_cast<int16_t>(left_counts));
  const int16_t right_rpm =
      right_.update(now_ms, static_cast<int16_t>(right_counts));
  // PID speed control: 0 RPM actively holds the arm instead of letting it
  // drop, and low speeds still get enough power to move.
  robot_.moveRpm(left_rpm, right_rpm);
}

}  // namespace skeleton
