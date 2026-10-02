/// @file buzzer.hpp
/// @brief Simple beeper on the Bot'n Roll ONE A+ on-board buzzer.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Plays short beeps with tone().
///
/// Blocking by design: intended for start-up signalling only, before the
/// non-blocking main loop runs.
class Buzzer
{
 public:
  /// @brief Creates a buzzer on the given pin.
  /// @param [in] pin Digital pin driving the buzzer.
  explicit Buzzer(uint8_t pin);

  Buzzer(const Buzzer&) = delete;
  Buzzer& operator=(const Buzzer&) = delete;
  Buzzer(Buzzer&&) = delete;
  Buzzer& operator=(Buzzer&&) = delete;

  /// @brief Configures the pin and keeps the buzzer silent.
  void begin();

  /// @brief Plays a series of identical beeps, then returns.
  /// @param [in] count Number of beeps.
  void beep(uint8_t count);

 private:
  uint8_t pin_;
};

}  // namespace skeleton
