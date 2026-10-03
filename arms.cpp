/// @file arms.cpp
/// @brief Two-arm motor controller implementation.

#include "arms.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

/// Motor ids used by BnrOneAPlus::stop1m().
constexpr uint8_t LEFT_MOTOR_ID = 0U;
constexpr uint8_t RIGHT_MOTOR_ID = 1U;

}  // namespace

Arms::Arms(BnrOneAPlus& robot)
    : robot_(robot),
      left_(config::LEFT_ARM_UP_SIGN),
      right_(config::RIGHT_ARM_UP_SIGN),
      last_control_ms_(0U),
      last_left_{true, 0},
      last_right_{true, 0}
{
}

void Arms::begin()
{
  robot_.stop();
  robot_.resetEncoders();
  last_left_ = MotorCommand{true, 0};
  last_right_ = MotorCommand{true, 0};
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
  apply(commandFor(left_, left_rpm), commandFor(right_, right_rpm));
}

Arms::MotorCommand Arms::commandFor(const Arm& arm, int16_t rpm)
{
  return arm.isResting() ? MotorCommand{true, 0} : MotorCommand{false, rpm};
}

void Arms::apply(const MotorCommand& left, const MotorCommand& right)
{
  const bool is_left_same = (left.is_stopped == last_left_.is_stopped) &&
                            (left.rpm == last_left_.rpm);
  const bool is_right_same = (right.is_stopped == last_right_.is_stopped) &&
                             (right.rpm == last_right_.rpm);
  // Only send on change: every new command restarts the board's PID speed
  // loop, so re-sending each period keeps it from ever building up power.
  if (is_left_same && is_right_same)
  {
    return;
  }

  if (left.is_stopped && right.is_stopped)
  {
    // Free stop: no PWM at all while both arms rest.
    robot_.stop();
  }
  else
  {
    // PID speed control (negative RPM = reverse). moveRpm() always drives
    // both motors, so switch a resting one back off straight after it.
    robot_.moveRpm(left.rpm, right.rpm);
    if (left.is_stopped)
    {
      robot_.stop1m(LEFT_MOTOR_ID);
    }
    if (right.is_stopped)
    {
      robot_.stop1m(RIGHT_MOTOR_ID);
    }
  }
  last_left_ = left;
  last_right_ = right;
}

}  // namespace skeleton
