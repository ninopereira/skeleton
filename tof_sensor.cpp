/// @file tof_sensor.cpp
/// @brief VL53L0X time-of-flight sensor implementation.

#include "tof_sensor.hpp"

#include <Arduino.h>
#include <Wire.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

/// Low three bits of RESULT_INTERRUPT_STATUS: a new measurement is ready.
constexpr uint8_t RANGE_READY_MASK = 0x07U;
constexpr uint16_t MM_PER_CM = 10U;

}  // namespace

TofSensor::TofSensor()
    : sensor_(),
      filter_(),
      is_ready_(false),
      last_reading_ms_(0U),
      last_distance_cm_(NO_TARGET)
{
}

bool TofSensor::begin()
{
  Wire.begin();
  // Wire.begin() turns on the internal pull-ups, which pull SDA and SCL
  // towards 5 V. The sensor runs on 3.3 V and is not 5 V tolerant, so
  // only its board's pull-ups to 3.3 V may drive the lines high.
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Wire.setClock(config::TOF_I2C_CLOCK_HZ);
  // Without a timeout a loose wire can hang the I2C bus, and the whole
  // sketch with it.
  Wire.setWireTimeout(config::TOF_I2C_TIMEOUT_US, true);

  sensor_.setTimeout(config::TOF_IO_TIMEOUT_MS);
  if (!sensor_.init())
  {
    return false;
  }
  if (!sensor_.setMeasurementTimingBudget(config::TOF_TIMING_BUDGET_US))
  {
    return false;
  }
  sensor_.startContinuous(config::TOF_PERIOD_MS);
  last_reading_ms_ = millis();
  is_ready_ = true;
  return true;
}

void TofSensor::update(uint32_t now_ms)
{
  if (!is_ready_)
  {
    return;
  }
  const uint8_t status =
      sensor_.readReg(VL53L0X::RESULT_INTERRUPT_STATUS);
  if ((status & RANGE_READY_MASK) != 0U)
  {
    last_reading_ms_ = now_ms;
    last_distance_cm_ = readCm();
    filter_.addReading(last_distance_cm_);
  }
  else if ((now_ms - last_reading_ms_) >= config::TOF_STALE_MS)
  {
    // The sensor stopped answering; never leave a visitor latched in.
    last_reading_ms_ = now_ms;
    last_distance_cm_ = NO_TARGET;
    filter_.addReading(NO_TARGET);
  }
  else
  {
    // Measurement still in progress.
  }
}

bool TofSensor::isTargetPresent() const
{
  return filter_.isTargetPresent();
}

int16_t TofSensor::getLastDistanceCm() const
{
  return last_distance_cm_;
}

int16_t TofSensor::readCm()
{
  // Returns at once: the ready flag was checked by the caller.
  const uint16_t range_mm = sensor_.readRangeContinuousMillimeters();
  if (sensor_.timeoutOccurred() || (range_mm > config::TOF_MAX_RANGE_MM))
  {
    return NO_TARGET;
  }
  // TOF_MAX_RANGE_MM bounds range_mm, so the result fits in int16_t.
  return static_cast<int16_t>(range_mm / MM_PER_CM);
}

}  // namespace skeleton
