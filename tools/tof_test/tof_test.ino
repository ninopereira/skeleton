/// @file tof_test.ino
/// @brief Bench test for the VL53L0X time-of-flight sensor.
///
/// Scans the I2C bus, checks the sensor's model ID, then prints every
/// measurement on the Serial Monitor (115200 baud). The eye LEDs light up
/// while something is within the skeleton's detection range, so the test
/// also works without a computer.
///
/// Wiring: VIN -> 5V, GND -> GND, SDA -> A4, SCL -> A5; eye LEDs on D8.
/// Upload this sketch on its own (open tools/tof_test in the IDE).

#include <stdint.h>

#include <VL53L0X.h>
#include <Wire.h>

namespace
{

constexpr uint32_t MONITOR_BAUD = 115200U;
constexpr uint8_t EYES_LED_PIN = 8U;
/// Same values as config.hpp, so the LED shows what would trigger.
constexpr uint16_t DETECT_MIN_MM = 30U;
constexpr uint16_t DETECT_MAX_MM = 2000U;
constexpr uint32_t TIMING_BUDGET_US = 33000U;
constexpr uint32_t PERIOD_MS = 50U;
constexpr uint16_t IO_TIMEOUT_MS = 500U;
constexpr uint32_t I2C_TIMEOUT_US = 3000U;
constexpr uint32_t I2C_CLOCK_HZ = 400000U;
/// Readings beyond this mean nothing was seen (the sensor reports ~8 m).
constexpr uint16_t MAX_RANGE_MM = 2000U;
/// The VL53L0X answers at this address unless it was changed.
constexpr uint8_t SENSOR_ADDRESS = 0x29U;
constexpr uint8_t EXPECTED_MODEL_ID = 0xEEU;
constexpr uint8_t FIRST_I2C_ADDRESS = 0x08U;
constexpr uint8_t LAST_I2C_ADDRESS = 0x77U;
/// Delay between retries while the sensor is not answering.
constexpr uint32_t RETRY_MS = 2000U;

VL53L0X sensor;

/// @brief Prints every address that acknowledges on the I2C bus.
/// @retval true  The sensor's address answered.
/// @retval false It did not.
bool scanBus()
{
  bool is_sensor_found = false;
  Serial.print(F("I2C devices:"));
  for (uint8_t address = FIRST_I2C_ADDRESS; address <= LAST_I2C_ADDRESS;
       ++address)
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0U)
    {
      Serial.print(F(" 0x"));
      Serial.print(address, HEX);
      is_sensor_found = is_sensor_found || (address == SENSOR_ADDRESS);
    }
  }
  Serial.println();
  return is_sensor_found;
}

/// @brief Checks, initialises and starts the sensor.
/// @retval true  The sensor is ranging.
/// @retval false A step failed; the reason has been printed.
bool startSensor()
{
  if (!scanBus())
  {
    Serial.println(F("Nothing at 0x29: check VIN, GND, SDA->A4, SCL->A5"));
    return false;
  }
  const uint8_t model_id = sensor.readReg(VL53L0X::IDENTIFICATION_MODEL_ID);
  Serial.print(F("Model ID 0x"));
  Serial.print(model_id, HEX);
  Serial.println((model_id == EXPECTED_MODEL_ID) ? F(" (VL53L0X)")
                                                 : F(" (not a VL53L0X?)"));
  if (!sensor.init())
  {
    Serial.println(F("init() failed"));
    return false;
  }
  if (!sensor.setMeasurementTimingBudget(TIMING_BUDGET_US))
  {
    Serial.println(F("Timing budget rejected"));
    return false;
  }
  sensor.startContinuous(PERIOD_MS);
  Serial.println(F("Ranging. Columns: mm, then IN RANGE when it would "
                   "trigger the skeleton"));
  return true;
}

/// @brief Prints one measurement and drives the LED from it.
/// @param [in] range_mm Reading from the sensor.
void reportRange(uint16_t range_mm)
{
  if (range_mm > MAX_RANGE_MM)
  {
    Serial.println(F("--- nothing in range"));
    digitalWrite(EYES_LED_PIN, LOW);
    return;
  }
  const bool in_range =
      (range_mm >= DETECT_MIN_MM) && (range_mm <= DETECT_MAX_MM);
  Serial.print(range_mm);
  Serial.println(in_range ? F("  IN RANGE") : F(""));
  digitalWrite(EYES_LED_PIN, in_range ? HIGH : LOW);
}

bool is_ranging = false;

}  // namespace

void setup()
{
  Serial.begin(MONITOR_BAUD);
  pinMode(EYES_LED_PIN, OUTPUT);
  digitalWrite(EYES_LED_PIN, LOW);
  Serial.println(F("VL53L0X test"));

  Wire.begin();
  Wire.setClock(I2C_CLOCK_HZ);
  Wire.setWireTimeout(I2C_TIMEOUT_US, true);
  sensor.setTimeout(IO_TIMEOUT_MS);
  is_ranging = startSensor();
}

void loop()
{
  if (!is_ranging)
  {
    // Keep retrying so the wiring can be fixed without re-uploading.
    delay(RETRY_MS);
    is_ranging = startSensor();
    return;
  }

  // Blocking is fine in a bench test: it waits at most one period.
  const uint16_t range_mm = sensor.readRangeContinuousMillimeters();
  if (sensor.timeoutOccurred())
  {
    Serial.println(F("Timeout: sensor stopped answering, restarting"));
    digitalWrite(EYES_LED_PIN, LOW);
    is_ranging = false;
    return;
  }
  reportRange(range_mm);
}
