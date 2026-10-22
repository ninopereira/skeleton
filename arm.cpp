/// @file arm.cpp
/// @brief Up-then-down servo arm gesture implementation.

#include "arm.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

/// No angle has been written yet; outside the servo's 0..180 range.
constexpr int16_t NO_ANGLE = -1;

/// Keeps t * t * (3 * duration - 2 * t) in easeInOut() within uint32_t.
constexpr uint32_t MAX_SWEEP_MS = 1000U;

static_assert((config::ARM_RAISE_MS <= MAX_SWEEP_MS) &&
                  (config::ARM_LOWER_MS <= MAX_SWEEP_MS),
              "sweep times are limited by the easing arithmetic");

/// @brief Smoothstep easing: slow start, fast middle, slow end.
///
/// Starting and stopping the arm at full speed jolts it, which can make
/// the horn slip on the servo spline a little on every gesture.
///
/// @param [in] elapsed_ms  Time into the sweep, at most duration_ms.
/// @param [in] duration_ms Length of the sweep, 1..MAX_SWEEP_MS.
/// @return Eased time into the sweep, 0..duration_ms.
uint32_t easeInOut(uint32_t elapsed_ms, uint32_t duration_ms)
{
  // duration * s(t / duration) with s(x) = 3x^2 - 2x^3.
  const uint32_t t = elapsed_ms;
  const uint32_t d = duration_ms;
  return (t * t * ((3U * d) - (2U * t))) / (d * d);
}

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
  enter(Phase::IDLE, millis());
}

void Arm::start(uint32_t now_ms, uint32_t delay_ms)
{
  if (phase_ != Phase::IDLE)
  {
    return;
  }
  if (!servo_.attached())
  {
    // The library kept the last angle, so it resumes at the down angle.
    static_cast<void>(servo_.attach(servo_pin_));
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
      releaseWhenSettled(elapsed_ms);
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
  const uint32_t eased_ms = easeInOut(clamped_ms, duration_ms);
  const uint32_t remaining_ms = duration_ms - eased_ms;
  const int32_t progress_ms =
      static_cast<int32_t>(is_rising ? eased_ms : remaining_ms);
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

void Arm::releaseWhenSettled(uint32_t elapsed_ms)
{
  if (servo_.attached() && (elapsed_ms >= config::SERVO_DETACH_DELAY_MS))
  {
    servo_.detach();
  }
}

}  // namespace skeleton
