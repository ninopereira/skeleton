/// @file arm.hpp
/// @brief Up-then-down gesture for one encoder-driven arm.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Raises one arm by ARM_MAX_COUNTS, holds briefly, then lowers it
///        back home.
///
/// Hardware-free: the caller feeds in encoder counts and applies the
/// returned motor speed (negative = reverse). The encoders count up in both
/// directions, so instead of a signed position the arm measures the
/// distance travelled in each phase: it lowers until it has covered at
/// least the distance it rose (including any drift while holding). If the
/// encoder registered nothing, it lowers by time instead.
class Arm
{
 public:
  /// @brief Creates an arm resting at home.
  /// @param [in] up_sign +1 if positive motor speed lifts the arm, else -1.
  explicit Arm(int8_t up_sign);

  Arm(const Arm&) = delete;
  Arm& operator=(const Arm&) = delete;
  Arm(Arm&&) = delete;
  Arm& operator=(Arm&&) = delete;

  /// @brief Starts one up-then-down gesture after a delay.
  ///
  /// Ignored while a gesture is still running: the arm always completes
  /// its way back home before it can start again.
  ///
  /// @param [in] now_ms   Current time from millis().
  /// @param [in] delay_ms Wait before the arm starts rising.
  void start(uint32_t now_ms, uint32_t delay_ms);

  /// @retval true  The arm is home and idle (or waiting to start), so its
  ///               motor should be switched off rather than held.
  /// @retval false The arm is moving or holding at the top.
  bool isResting() const;

  /// @brief Runs one control step.
  /// @param [in] now_ms       Current time from millis().
  /// @param [in] counts_moved Encoder counts since the previous call.
  /// @return Motor speed to apply in RPM (negative = reverse, 0 = hold).
  int16_t update(uint32_t now_ms, int16_t counts_moved);

 private:
  /// @brief Gesture phase.
  enum class Phase : uint8_t
  {
    IDLE,      ///< Home and still.
    WAITING,   ///< Delay before rising.
    RAISING,   ///< Rising ARM_MAX_COUNTS.
    HOLDING,   ///< Pausing at the top.
    LOWERING,  ///< Returning home.
  };

  /// @brief Switches phase and restarts the phase timer.
  /// @param [in] phase  New phase.
  /// @param [in] now_ms Current time from millis().
  void enter(Phase phase, uint32_t now_ms);

  /// @brief Updates phase from the elapsed time and distances.
  /// @param [in] now_ms Current time from millis().
  void advance(uint32_t now_ms);

  /// @brief Plans the descent from what happened on the way up.
  /// @param [in] raise_ms How long the arm rose.
  void planDescent(uint32_t raise_ms);

  /// @retval true  The arm is back home.
  /// @retval false Keep lowering.
  /// @param [in] elapsed_ms Time spent lowering so far.
  bool isHome(uint32_t elapsed_ms) const;

  /// @brief Speed magnitude for the distance still to go.
  /// @param [in] remaining  Counts still to travel.
  /// @param [in] cruise_rpm Speed used outside the slow zone.
  /// @return RPM, or 0 when within tolerance.
  static int16_t rpmFor(int32_t remaining, int16_t cruise_rpm);

  int8_t up_sign_;
  Phase phase_;
  uint32_t phase_start_ms_;
  uint32_t delay_ms_;
  int32_t raised_counts_;    ///< Distance up, including drift at the top.
  int32_t lowered_counts_;   ///< Distance back down so far.
  uint32_t lower_time_ms_;   ///< Descent time when lowering by time.
  bool is_lowering_by_time_;
};

}  // namespace skeleton
