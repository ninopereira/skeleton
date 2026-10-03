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

  /// @brief Starts one up-then-down gesture on each arm.
  ///
  /// A random arm goes first; the other follows after a random stagger.
  /// @param [in] now_ms Current time from millis().
  void start(uint32_t now_ms);

  /// @brief Reads encoders and updates motors every control period.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

 private:
  /// @brief What a motor was last told to do.
  struct MotorCommand
  {
    bool is_stopped;  ///< Power cut (free stop), no PID.
    int16_t rpm;      ///< PID speed; only meaningful when not stopped.
  };

  /// @brief Builds the command for one arm.
  /// @param [in] arm Arm planner.
  /// @param [in] rpm Speed the planner asked for.
  /// @return Stopped when the arm rests, otherwise the PID speed.
  static MotorCommand commandFor(const Arm& arm, int16_t rpm);

  /// @brief Sends the commands to the board if either one changed.
  /// @param [in] left  Command for the left motor.
  /// @param [in] right Command for the right motor.
  void apply(const MotorCommand& left, const MotorCommand& right);

  BnrOneAPlus& robot_;
  Arm left_;
  Arm right_;
  uint32_t last_control_ms_;
  MotorCommand last_left_;   ///< Last command sent, to skip repeats.
  MotorCommand last_right_;
};

}  // namespace skeleton
