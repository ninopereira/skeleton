/// @file arm.cpp
/// @brief Arm motion planner implementation.

#include "arm.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

constexpr int16_t HOME_COUNTS = 0;

/// @brief Uniform random integer in [low, high].
/// @param [in] low  Inclusive lower bound.
/// @param [in] high Inclusive upper bound (>= low).
/// @return Random value.
int16_t randomBetween(int16_t low, int16_t high)
{
  return static_cast<int16_t>(
      random(static_cast<long>(low), static_cast<long>(high) + 1L));
}

/// @brief Uniform random delay in [0, max_ms].
/// @param [in] max_ms Inclusive upper bound.
/// @return Random delay in ms.
uint32_t randomDelayMs(uint32_t max_ms)
{
  return static_cast<uint32_t>(
      random(0L, static_cast<long>(max_ms) + 1L));
}

}  // namespace

Arm::Arm(int8_t up_sign)
    : up_sign_((up_sign < 0) ? static_cast<int8_t>(-1)
                             : static_cast<int8_t>(1)),
      state_(State::IDLE),
      position_(HOME_COUNTS),
      target_(HOME_COUNTS),
      cruise_rpm_(config::ARM_RPM_MIN),
      last_direction_(1),
      move_start_ms_(0U),
      next_move_ms_(0U)
{
}

void Arm::start(uint32_t now_ms)
{
  state_ = State::PAUSING;
  next_move_ms_ = now_ms + randomDelayMs(config::ARM_MAX_START_DELAY_MS);
}

void Arm::goHome(uint32_t now_ms)
{
  beginMove(HOME_COUNTS, State::HOMING, now_ms);
}

int16_t Arm::update(uint32_t now_ms, int16_t counts_moved)
{
  // Counts that arrive after a stop come from coasting in the last
  // direction, hence last_direction_ rather than the current command.
  const int16_t magnitude = (counts_moved < 0) ? -counts_moved : counts_moved;
  position_ = static_cast<int16_t>(position_ + (last_direction_ * magnitude));

  switch (state_)
  {
    case State::IDLE:
      break;
    case State::PAUSING:
      // Signed difference so the comparison survives the millis() wrap.
      if (static_cast<int32_t>(now_ms - next_move_ms_) >= 0)
      {
        pickTarget();
        beginMove(target_, State::MOVING, now_ms);
      }
      break;
    case State::MOVING:  // shares the arrival check with HOMING
    case State::HOMING:
      if ((computeSpeed() == 0) ||
          ((now_ms - move_start_ms_) >= config::ARM_MOVE_TIMEOUT_MS))
      {
        finishMove(now_ms);
      }
      break;
  }

  const bool is_moving =
      (state_ == State::MOVING) || (state_ == State::HOMING);
  const int16_t speed = is_moving ? computeSpeed() : static_cast<int16_t>(0);
  if (speed != 0)
  {
    last_direction_ = (speed > 0) ? static_cast<int8_t>(1)
                                  : static_cast<int8_t>(-1);
  }
  return static_cast<int16_t>(speed * up_sign_);
}

void Arm::pickTarget()
{
  const bool can_go_up =
      (position_ + config::ARM_MIN_STEP_COUNTS) <= config::ARM_MAX_COUNTS;
  const bool can_go_down =
      (position_ - config::ARM_MIN_STEP_COUNTS) >= HOME_COUNTS;
  bool go_up = can_go_up;
  if (can_go_up && can_go_down)
  {
    go_up = (random(2L) == 0L);
  }

  if (go_up)
  {
    const int16_t far = static_cast<int16_t>(
        position_ + config::ARM_MAX_STEP_COUNTS);
    target_ = randomBetween(
        static_cast<int16_t>(position_ + config::ARM_MIN_STEP_COUNTS),
        (far < config::ARM_MAX_COUNTS) ? far : config::ARM_MAX_COUNTS);
  }
  else if (can_go_down)
  {
    const int16_t far = static_cast<int16_t>(
        position_ - config::ARM_MAX_STEP_COUNTS);
    target_ = randomBetween(
        (far > HOME_COUNTS) ? far : HOME_COUNTS,
        static_cast<int16_t>(position_ - config::ARM_MIN_STEP_COUNTS));
  }
  else
  {
    // Unreachable while the config static_asserts hold; stay safe anyway.
    target_ = HOME_COUNTS;
  }
}

void Arm::beginMove(int16_t target, State state, uint32_t now_ms)
{
  target_ = target;
  state_ = state;
  move_start_ms_ = now_ms;
  cruise_rpm_ = randomBetween(config::ARM_RPM_MIN, config::ARM_RPM_MAX);
}

int16_t Arm::computeSpeed() const
{
  const int16_t remaining = static_cast<int16_t>(target_ - position_);
  const int16_t distance = (remaining < 0) ? -remaining : remaining;
  if (distance <= config::ARM_TOLERANCE_COUNTS)
  {
    return 0;
  }
  const int16_t magnitude = (distance <= config::ARM_SLOW_ZONE_COUNTS)
                                ? config::ARM_SLOW_RPM
                                : cruise_rpm_;
  return (remaining > 0) ? magnitude : static_cast<int16_t>(-magnitude);
}

void Arm::finishMove(uint32_t now_ms)
{
  if (state_ == State::HOMING)
  {
    state_ = State::IDLE;
    return;
  }
  state_ = State::PAUSING;
  next_move_ms_ = now_ms + config::ARM_MIN_PAUSE_MS +
                  randomDelayMs(config::ARM_MAX_PAUSE_MS -
                                config::ARM_MIN_PAUSE_MS);
}

}  // namespace skeleton
