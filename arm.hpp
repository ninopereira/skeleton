/// @file arm.hpp
/// @brief Random up/down motion planner for one encoder-driven arm.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Moves one arm to random positions within [0, ARM_MAX_COUNTS].
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

  /// @brief Starts random motion after a random delay.
  /// @param [in] now_ms Current time from millis().
  void start(uint32_t now_ms);

  /// @brief Stops random motion and returns the arm to position 0.
  /// @param [in] now_ms Current time from millis().
  void goHome(uint32_t now_ms);

  /// @brief Runs one control step.
  /// @param [in] now_ms       Current time from millis().
  /// @param [in] counts_moved Encoder counts since the previous call.
  /// @return Motor speed to apply in RPM (0 = hold still).
  int16_t update(uint32_t now_ms, int16_t counts_moved);

 private:
  /// @brief Motion state.
  enum class State : uint8_t
  {
    IDLE,     ///< Stopped at home.
    PAUSING,  ///< Resting until next_move_ms_.
    MOVING,   ///< Heading to a random target.
    HOMING,   ///< Heading back to 0.
  };

  /// @brief Picks a random target at least ARM_MIN_STEP_COUNTS away.
  void pickTarget();

  /// @brief Sets a target and starts moving towards it.
  /// @param [in] target Target position in counts.
  /// @param [in] state  MOVING or HOMING.
  /// @param [in] now_ms Current time from millis().
  void beginMove(int16_t target, State state, uint32_t now_ms);

  /// @brief Speed towards target_, or 0 when arrived.
  /// @return Signed motor speed in RPM.
  int16_t computeSpeed() const;

  /// @brief Ends the current move (arrived or timed out).
  /// @param [in] now_ms Current time from millis().
  void finishMove(uint32_t now_ms);

  int8_t up_sign_;
  State state_;
  int16_t position_;
  int16_t target_;
  int16_t cruise_rpm_;
  int8_t last_direction_;
  uint32_t move_start_ms_;
  uint32_t next_move_ms_;
};

}  // namespace skeleton
