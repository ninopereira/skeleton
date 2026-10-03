/// @file arm.hpp
/// @brief Up-then-down gesture for one encoder-driven arm.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Raises one arm to ARM_MAX_COUNTS, holds briefly, then lowers it.
///
/// Hardware-free: the caller feeds in encoder counts and applies the
/// returned motor speed. Position is tracked from encoder magnitude and the
/// last commanded direction, so it works whether the encoder is signed or
/// not.
class Arm
{
 public:
  /// @brief Creates an arm resting at position 0.
  /// @param [in] up_sign +1 if positive motor speed lifts the arm, else -1.
  explicit Arm(int8_t up_sign);

  Arm(const Arm&) = delete;
  Arm& operator=(const Arm&) = delete;
  Arm(Arm&&) = delete;
  Arm& operator=(Arm&&) = delete;

  /// @brief Starts one up-then-down gesture after a delay.
  /// @param [in] now_ms   Current time from millis().
  /// @param [in] delay_ms Wait before the arm starts rising.
  void start(uint32_t now_ms, uint32_t delay_ms);

  /// @brief Abandons the gesture and lowers the arm straight away.
  /// @param [in] now_ms Current time from millis().
  void goHome(uint32_t now_ms);

  /// @brief Runs one control step.
  /// @param [in] now_ms       Current time from millis().
  /// @param [in] counts_moved Encoder counts since the previous call.
  /// @return Motor speed to apply in RPM (0 = hold still).
  int16_t update(uint32_t now_ms, int16_t counts_moved);

 private:
  /// @brief Gesture phase.
  enum class Phase : uint8_t
  {
    IDLE,      ///< Down and still.
    WAITING,   ///< Delay before rising.
    RAISING,   ///< Heading to ARM_MAX_COUNTS.
    HOLDING,   ///< Pausing at the top.
    LOWERING,  ///< Heading back to 0.
  };

  /// @brief Switches phase and restarts the phase timer.
  /// @param [in] phase  New phase.
  /// @param [in] now_ms Current time from millis().
  void enter(Phase phase, uint32_t now_ms);

  /// @brief Signed speed towards a target, or 0 when there.
  /// @param [in] target     Target position in counts.
  /// @param [in] cruise_rpm Speed used outside the slow zone.
  /// @return Signed motor speed in RPM.
  int16_t speedTowards(int16_t target, int16_t cruise_rpm) const;

  /// @brief Updates phase from the elapsed time and position.
  /// @param [in] now_ms Current time from millis().
  void advance(uint32_t now_ms);

  int8_t up_sign_;
  Phase phase_;
  int16_t position_;
  int8_t last_direction_;
  uint32_t phase_start_ms_;
  uint32_t delay_ms_;
};

}  // namespace skeleton
