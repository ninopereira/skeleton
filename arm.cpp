/// @file arm.cpp
/// @brief Up-then-down arm gesture implementation.

#include "arm.hpp"

#include "config.hpp"

namespace skeleton
{

Arm::Arm(int8_t up_sign)
    : up_sign_((up_sign < 0) ? static_cast<int8_t>(-1)
                             : static_cast<int8_t>(1)),
      phase_(Phase::IDLE),
      phase_start_ms_(0U),
      delay_ms_(0U),
      raised_counts_(0),
      lowered_counts_(0),
      lower_time_ms_(0U),
      is_lowering_by_time_(false)
{
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

bool Arm::isResting() const
{
  return (phase_ == Phase::IDLE) || (phase_ == Phase::WAITING);
}

int16_t Arm::update(uint32_t now_ms, int16_t counts_moved)
{
  // The encoders count up in both directions; the phase says which way.
  const int32_t magnitude =
      (counts_moved < 0) ? -static_cast<int32_t>(counts_moved)
                         : static_cast<int32_t>(counts_moved);
  switch (phase_)
  {
    case Phase::RAISING:  // drift while holding still counts as height
    case Phase::HOLDING:
      raised_counts_ += magnitude;
      break;
    case Phase::LOWERING:
      lowered_counts_ += magnitude;
      break;
    case Phase::IDLE:  // resting on the stop: no distance to track
    case Phase::WAITING:
      break;
  }

  advance(now_ms);

  int16_t rpm = 0;
  if (phase_ == Phase::RAISING)
  {
    rpm = rpmFor(config::ARM_MAX_COUNTS - raised_counts_, config::ARM_UP_RPM);
  }
  else if (phase_ == Phase::LOWERING)
  {
    // Negative RPM drives the motor in reverse.
    const int16_t down_rpm =
        is_lowering_by_time_
            ? config::ARM_DOWN_RPM
            : rpmFor(raised_counts_ - lowered_counts_, config::ARM_DOWN_RPM);
    rpm = static_cast<int16_t>(-down_rpm);
  }
  else
  {
    // IDLE, WAITING and HOLDING keep the motor still.
  }
  return static_cast<int16_t>(rpm * up_sign_);
}

void Arm::advance(uint32_t now_ms)
{
  const uint32_t elapsed_ms = now_ms - phase_start_ms_;
  // A move also ends on timeout so a blocked arm is not driven forever.
  const bool is_move_timed_out = elapsed_ms >= config::ARM_MOVE_TIMEOUT_MS;

  switch (phase_)
  {
    case Phase::IDLE:
      break;
    case Phase::WAITING:
      if (elapsed_ms >= delay_ms_)
      {
        raised_counts_ = 0;
        lowered_counts_ = 0;
        enter(Phase::RAISING, now_ms);
      }
      break;
    case Phase::RAISING:
      if ((rpmFor(config::ARM_MAX_COUNTS - raised_counts_,
                  config::ARM_UP_RPM) == 0) ||
          is_move_timed_out)
      {
        planDescent(elapsed_ms);
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
      // A timed descent is already bounded; the timeout guards the
      // encoder-driven one (the arm is then resting on its bottom stop).
      if (isHome(elapsed_ms) || (!is_lowering_by_time_ && is_move_timed_out))
      {
        enter(Phase::IDLE, now_ms);
      }
      break;
  }
}

void Arm::planDescent(uint32_t raise_ms)
{
  // Without encoder feedback no distance was measured, so lower for as
  // long as the arm rose, stretched by the speed ratio. Erring long only
  // presses the arm on its bottom stop; erring short leaves it up.
  is_lowering_by_time_ = raised_counts_ <= config::ARM_TOLERANCE_COUNTS;
  lower_time_ms_ = (raise_ms * static_cast<uint32_t>(config::ARM_UP_RPM)) /
                   static_cast<uint32_t>(config::ARM_DOWN_RPM);
}

bool Arm::isHome(uint32_t elapsed_ms) const
{
  if (is_lowering_by_time_)
  {
    return elapsed_ms >= lower_time_ms_;
  }
  return rpmFor(raised_counts_ - lowered_counts_, config::ARM_DOWN_RPM) == 0;
}

void Arm::enter(Phase phase, uint32_t now_ms)
{
  phase_ = phase;
  phase_start_ms_ = now_ms;
}

int16_t Arm::rpmFor(int32_t remaining, int16_t cruise_rpm)
{
  if (remaining <= config::ARM_TOLERANCE_COUNTS)
  {
    return 0;
  }
  return (remaining <= config::ARM_SLOW_ZONE_COUNTS) ? config::ARM_SLOW_RPM
                                                     : cruise_rpm;
}

}  // namespace skeleton
