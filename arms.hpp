/// @file arms.hpp
/// @brief Drives both arm servos with a staggered start.

#pragma once

#include <stdint.h>

#include "arm.hpp"

namespace skeleton
{

/// @brief Owns the left and right Arm servos and staggers their gestures.
///
/// Both arms share one update period but move independently, so they never
/// go up in lockstep.
class Arms
{
 public:
  /// @brief Creates the controller with both arms resting down.
  Arms();

  Arms(const Arms&) = delete;
  Arms& operator=(const Arms&) = delete;
  Arms(Arms&&) = delete;
  Arms& operator=(Arms&&) = delete;

  /// @brief Attaches both servos and moves the arms down.
  void begin();

  /// @brief Starts one up-then-down gesture on each arm.
  ///
  /// A random arm goes first; the other follows after a random stagger.
  /// @param [in] now_ms Current time from millis().
  void start(uint32_t now_ms);

  /// @brief Moves the servos every update period.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

 private:
  Arm left_;
  Arm right_;
  uint32_t last_update_ms_;
};

}  // namespace skeleton
