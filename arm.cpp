/// @file arm.cpp
/// @brief Up-then-down arm gesture implementation.

#include "arm.hpp"

#include "config.hpp"

namespace skeleton
{
namespace
{

constexpr int16_t HOME_COUNTS = 0;

}  // namespace

Arm::Arm(int8_t up_sign)
    : up_sign_((up_sign < 0) ? static_cast<int8_t>(-1)
                             : static_cast<int8_t>(1)),
      phase_(Phase::IDLE),
      position_(HOME_COUNTS),
      last_direction_(1),
      phase_start_ms_(0U),
      delay_ms_(0U)
{
}

void Arm::start(uint32_t now_ms, uint32_t delay_ms)
{
  delay_ms_ = delay_ms;
  enter(Phase::WAITING, now_ms);
}

void Arm::goHome(uint32_t now_ms)
{
  if (phase_ != Phase::IDLE)
  {
    enter(Phase::LOWERING, now_ms);
  }
}

int16_t Arm::update(uint32_t now_ms, int16_t counts_moved)
{
  // Counts that arrive after a stop come from coasting in the last
  // direction, hence last_direction_ rather than the current command.
  const int16_t magnitude = (counts_moved < 0) ? -counts_moved : counts_moved;
  position_ = static_cast<int16_t>(position_ + (last_direction_ * magnitude));

  advance(now_ms);

  int16_t speed = 0;
  if (phase_ == Phase::RAISING)
  {
    speed = speedTowards(config::ARM_MAX_COUNTS, config::ARM_UP_RPM);
  }
  else if (phase_ == Phase::LOWERING)
  {
    speed = speedTowards(HOME_COUNTS, config::ARM_DOWN_RPM);
  }
  else
  {
    // IDLE, WAITING and HOLDING keep the motor still.
  }

  if (speed != 0)
  {
    last_direction_ = (speed > 0) ? static_cast<int8_t>(1)
                                  : static_cast<int8_t>(-1);
  }
  return static_cast<int16_t>(speed * up_sign_);
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
        enter(Phase::RAISING, now_ms);
      }
      break;
    case Phase::RAISING:
      if ((speedTowards(config::ARM_MAX_COUNTS, config::ARM_UP_RPM) == 0) ||
          is_move_timed_out)
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
      if ((speedTowards(HOME_COUNTS, config::ARM_DOWN_RPM) == 0) ||
          is_move_timed_out)
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

int16_t Arm::speedTowards(int16_t target, int16_t cruise_rpm) const
{
  const int16_t remaining = static_cast<int16_t>(target - position_);
  const int16_t distance = (remaining < 0) ? -remaining : remaining;
  if (distance <= config::ARM_TOLERANCE_COUNTS)
  {
    return 0;
  }
  const int16_t magnitude = (distance <= config::ARM_SLOW_ZONE_COUNTS)
                                ? config::ARM_SLOW_RPM
                                : cruise_rpm;
  return (remaining > 0) ? magnitude : static_cast<int16_t>(-magnitude);
}

}  // namespace skeleton
