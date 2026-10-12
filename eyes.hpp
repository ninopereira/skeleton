/// @file eyes.hpp
/// @brief LEDs that light up the skeleton's eyes.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Switches the eye LEDs on and off.
class Eyes
{
 public:
  /// @brief Creates the eyes on the given pin.
  /// @param [in] pin Digital pin driving the LEDs (active high).
  explicit Eyes(uint8_t pin);

  Eyes(const Eyes&) = delete;
  Eyes& operator=(const Eyes&) = delete;
  Eyes(Eyes&&) = delete;
  Eyes& operator=(Eyes&&) = delete;

  /// @brief Configures the pin and starts with the eyes off.
  void begin();

  /// @brief Lights the eyes.
  void turnOn();

  /// @brief Darkens the eyes.
  void turnOff();

 private:
  uint8_t pin_;
};

}  // namespace skeleton
