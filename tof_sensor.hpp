/// @file tof_sensor.hpp
/// @brief VL53L0X time-of-flight sensor with debounced presence detection.

#pragma once

#include <stdint.h>

#include <VL53L0X.h>

#include "presence_filter.hpp"

namespace skeleton
{

/// @brief Runs a VL53L0X in continuous mode on the I2C bus (A4/A5 on the
///        Uno) and reports whether someone is near.
///
/// Settings come from config::TOF_*. Readings are debounced by a
/// PresenceFilter. Unlike the sonar, update() never waits for a
/// measurement: it only collects one when the sensor has it ready.
class TofSensor
{
 public:
  /// Value returned by getLastDistanceCm() when nothing is in range.
  static constexpr int16_t NO_TARGET = -1;

  TofSensor();

  TofSensor(const TofSensor&) = delete;
  TofSensor& operator=(const TofSensor&) = delete;
  TofSensor(TofSensor&&) = delete;
  TofSensor& operator=(TofSensor&&) = delete;

  /// @brief Starts the I2C bus, initialises the sensor and starts ranging.
  /// @retval true  The sensor answered and is ranging.
  /// @retval false No answer; isTargetPresent() then stays false.
  bool begin();

  /// @brief Collects a new measurement if one is ready.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

  /// @retval true  A target has been in range for enough consecutive
  ///               measurements.
  /// @retval false Nothing in range.
  bool isTargetPresent() const;

  /// @return Last measured distance in cm, or NO_TARGET.
  int16_t getLastDistanceCm() const;

 private:
  /// @brief Reads the measurement the sensor has ready.
  /// @return Distance in cm, or NO_TARGET when out of range.
  int16_t readCm();

  VL53L0X sensor_;
  PresenceFilter filter_;
  bool is_ready_;
  uint32_t last_reading_ms_;
  int16_t last_distance_cm_;
};

}  // namespace skeleton
