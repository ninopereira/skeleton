/// @file sonar.hpp
/// @brief HC-SR04 ultrasonic sensor with debounced presence detection.

#pragma once

#include <stdint.h>

#include "presence_filter.hpp"

namespace skeleton
{

/// @brief Periodically pings an HC-SR04 and reports whether someone is near.
///
/// Pins come from config::SONAR_*. Readings are debounced by a
/// PresenceFilter.
class Sonar
{
 public:
  /// Value returned by getLastDistanceCm() when no echo was received.
  static constexpr int16_t NO_ECHO = -1;

  Sonar();

  Sonar(const Sonar&) = delete;
  Sonar& operator=(const Sonar&) = delete;
  Sonar(Sonar&&) = delete;
  Sonar& operator=(Sonar&&) = delete;

  /// @brief Configures the pins.
  /// @retval true Always: the HC-SR04 gives no sign of being connected.
  bool begin();

  /// @brief Pings the sensor if the sampling period has elapsed.
  ///
  /// Blocks for at most config::SONAR_TIMEOUT_US while waiting for the echo.
  ///
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

  /// @retval true  A target has been in range for enough consecutive pings.
  /// @retval false Nothing in range.
  bool isTargetPresent() const;

  /// @return Last measured distance in cm, or NO_ECHO.
  int16_t getLastDistanceCm() const;

 private:
  /// @brief Fires one ping and measures the echo.
  /// @return Distance in cm, or NO_ECHO on timeout.
  int16_t measureCm() const;

  PresenceFilter filter_;
  uint32_t last_ping_ms_;
  int16_t last_distance_cm_;
};

}  // namespace skeleton
