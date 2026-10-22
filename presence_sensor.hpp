/// @file presence_sensor.hpp
/// @brief Selects the visitor sensor chosen by config::DISTANCE_SENSOR.

#pragma once

#include "config.hpp"
#include "sonar.hpp"
#include "tof_sensor.hpp"

namespace skeleton
{

/// @brief Maps a sensor choice to the class that drives it.
///
/// Every class provides begin(), update(now_ms), isTargetPresent() and
/// getLastDistanceCm(), so the rest of the sketch does not depend on which
/// one is fitted.
///
/// @tparam TYPE Sensor fitted to the skeleton.
template <config::DistanceSensorType TYPE>
struct PresenceSensorFor;

template <>
struct PresenceSensorFor<config::DistanceSensorType::SONAR>
{
  using Type = Sonar;
};

template <>
struct PresenceSensorFor<config::DistanceSensorType::TIME_OF_FLIGHT>
{
  using Type = TofSensor;
};

/// The sensor selected in config.hpp.
using PresenceSensor = PresenceSensorFor<config::DISTANCE_SENSOR>::Type;

}  // namespace skeleton
