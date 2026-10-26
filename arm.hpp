/// @file arm.hpp
/// @brief Up-then-down gesture for one servo-driven arm.

#pragma once

#include <stdint.h>

#include <Servo.h>

namespace skeleton
{

/// @brief Sweeps one arm servo from its down angle to its up angle, holds
///        briefly, then sweeps it back down.
///
/// Each sweep eases in and out so the arm never starts or stops abruptly.
/// The servo holds every angle by itself, so the arm stays up during the
/// pause at the top without any braking. Once the arm is back down the
/// servo is detached so it cannot twitch at rest.
class Arm
{
 public:
  /// @brief Creates an arm resting down.
  /// @param [in] servo_pin Pin the servo signal wire is on.
  /// @param [in] down_deg  Servo angle with the arm resting down.
  /// @param [in] up_deg    Servo angle with the arm raised.
  Arm(uint8_t servo_pin, uint8_t down_deg, uint8_t up_deg);

  Arm(const Arm&) = delete;
  Arm& operator=(const Arm&) = delete;
  Arm(Arm&&) = delete;
  Arm& operator=(Arm&&) = delete;

  /// @brief Attaches the servo and moves the arm down. Call in setup().
  void begin();

  /// @brief Starts one up-then-down gesture after a delay.
  ///
  /// Ignored while a gesture is still running: the arm always completes
  /// its way back down before it can start again.
  ///
  /// @param [in] now_ms   Current time from millis().
  /// @param [in] delay_ms Wait before the arm starts rising.
  void start(uint32_t now_ms, uint32_t delay_ms);

  /// @brief Moves the servo along the gesture.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

 private:
  /// @brief Gesture phase.
  enum class Phase : uint8_t
  {
    IDLE,      ///< Down and still.
    WAITING,   ///< Delay before rising.
    RAISING,   ///< Sweeping up.
    HOLDING,   ///< Pausing at the top.
    LOWERING,  ///< Sweeping down.
  };

  /// @brief Switches phase and restarts the phase timer.
  /// @param [in] phase  New phase.
  /// @param [in] now_ms Current time from millis().
  void enter(Phase phase, uint32_t now_ms);

  /// @brief Servo angle a fraction of the way from down to up.
  /// @param [in] elapsed_ms  Time into the sweep.
  /// @param [in] duration_ms Length of the whole sweep (non-zero).
  /// @param [in] is_rising   true sweeps down to up, false up to down.
  /// @return Angle in degrees, clamped to the end of the sweep.
  int16_t sweepAngle(uint32_t elapsed_ms,
                     uint32_t duration_ms,
                     bool is_rising) const;

  /// @brief Writes an angle to the servo, only if it changed.
  /// @param [in] angle Angle in degrees.
  void setAngle(int16_t angle);

  /// @brief Detaches the servo once it has had time to settle at rest.
  /// @param [in] elapsed_ms Time since the arm went idle.
  void releaseWhenSettled(uint32_t elapsed_ms);

  Servo servo_;
  uint8_t servo_pin_;
  uint8_t down_deg_;
  uint8_t up_deg_;
  Phase phase_;
  uint32_t phase_start_ms_;
  uint32_t delay_ms_;
  int16_t last_angle_;  ///< Last angle written, to skip repeats.
};

}  // namespace skeleton
