/// @file arm.cpp
/// @brief Up-then-down servo arm gesture implementation.

#include "arm.hpp"

#include "config.hpp"

namespace skeleton
{
namespace
{

/// No angle has been written yet; outside the servo's 0..180 range.
constexpr int16_t NO_ANGLE = -1;

}  // namespace

Arm::Arm(uint8_t servo_pin, uint8_t down_deg, uint8_t up_deg)
    : servo_(),
      servo_pin_(servo_pin),
      down_deg_(down_deg),
      up_deg_(up_deg),
      phase_(Phase::IDLE),
      phase_start_ms_(0U),
      delay_ms_(0U),
      last_angle_(NO_ANGLE)
{
}

void Arm::begin()
{
  // attach() only reports the timer channel it took; the Uno has 12, far
  // more than the three servos used here.
  static_cast<void>(servo_.attach(servo_pin_));
  setAngle(static_cast<int16_t>(down_deg_));
}

void Arm::start(uint32_t now_ms, uint32_t delay_ms)
{
  if (phase_ != Phase::IDLE)
  {
    return;
  }
  delay_ms_ = delay_ms;
  enter(Phase::WAITING, now_ms);
}

void Arm::update(uint32_t now_ms)
{
  const uint32_t elapsed_ms = now_ms - phase_start_ms_;
  switch (phase_)
  {
    case Phase::IDLE:
      break;
    case Phase::WAITING:
      if (elapsed_ms >= delay_ms_)
      {
        enter(Phase::RAISING, now_ms);
      }
      break;
    case Phase::RAISING:
      setAngle(sweepAngle(elapsed_ms, config::ARM_RAISE_MS, true));
      if (elapsed_ms >= config::ARM_RAISE_MS)
      {
        enter(Phase::HOLDING, now_ms);
      }
      break;
    case Phase::HOLDING:
      if (elapsed_ms >= config::ARM_HOLD_TOP_MS)
      {
        enter(Phase::LOWERING, now_ms);
      }
      break;
    case Phase::LOWERING:
      setAngle(sweepAngle(elapsed_ms, config::ARM_LOWER_MS, false));
      if (elapsed_ms >= config::ARM_LOWER_MS)
      {
        enter(Phase::IDLE, now_ms);
      }
      break;
  }
}

void Arm::enter(Phase phase, uint32_t now_ms)
{
  phase_ = phase;
  phase_start_ms_ = now_ms;
}

int16_t Arm::sweepAngle(uint32_t elapsed_ms,
                        uint32_t duration_ms,
                        bool is_rising) const
{
  const uint32_t clamped_ms =
      (elapsed_ms > duration_ms) ? duration_ms : elapsed_ms;
  const uint32_t remaining_ms = duration_ms - clamped_ms;
  const int32_t progress_ms =
      static_cast<int32_t>(is_rising ? clamped_ms : remaining_ms);
  // Signed span so a mirrored servo (up < down) also works.
  const int32_t span =
      static_cast<int32_t>(up_deg_) - static_cast<int32_t>(down_deg_);
  const int32_t angle =
      static_cast<int32_t>(down_deg_) +
      ((span * progress_ms) / static_cast<int32_t>(duration_ms));
  return static_cast<int16_t>(angle);
}

void Arm::setAngle(int16_t angle)
{
  if (angle == last_angle_)
  {
    return;
  }
  servo_.write(angle);
  last_angle_ = angle;
}

}  // namespace skeleton
