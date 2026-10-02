/// @file arms.hpp
/// @brief Drives both arm motors from their encoders via the Bot'n Roll.

#pragma once

#include <stdint.h>

#include <BnrOneAPlus.h>

#include "arm.hpp"

namespace skeleton
{

/// @brief Owns the left and right Arm planners and talks to the motors.
///
/// Both motors are set with a single BnrOneAPlus::moveRpm() call, so the arms
/// share one control period but plan their motion independently.
class Arms
{
 public:
  /// @brief Creates the controller.
  /// @param [in] robot Bot'n Roll interface (must outlive this object).
  explicit Arms(BnrOneAPlus& robot);

  Arms(const Arms&) = delete;
  Arms& operator=(const Arms&) = delete;
  Arms(Arms&&) = delete;
  Arms& operator=(Arms&&) = delete;

  /// @brief Stops the motors and zeroes the encoders. Arms must be down.
  void begin();

  /// @brief Starts random flailing on both arms.
  /// @param [in] now_ms Current time from millis().
  void start(uint32_t now_ms);

  /// @brief Returns both arms to the rest position.
  /// @param [in] now_ms Current time from millis().
  void goHome(uint32_t now_ms);

  /// @brief Reads encoders and updates motors every control period.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

 private:
  BnrOneAPlus& robot_;
  Arm left_;
  Arm right_;
  uint32_t last_control_ms_;
};

}  // namespace skeleton
