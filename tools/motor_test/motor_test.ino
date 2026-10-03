/// @file motor_test.ino
/// @brief Bench test for the arm motors on the Bot'n Roll ONE A+.
///
/// 1. Shows the battery voltage and the motor-board firmware version.
/// 2. After a button press, drives both motors briefly up and back down,
///    first open-loop with move() and then PID-controlled with moveRpm(),
///    printing the encoder counts of each step on the LCD:
///      line 1: "PWR <power>" or "RPM <speed>"
///      line 2: "<left counts> <right counts>"
///    Counts of 0 mean that command/speed did not move the motor.
///
/// Upload this sketch on its own (open tools/motor_test in the IDE). Hold
/// the arms clear of their end stops: the moves are short but unbounded.

#include <SPI.h>

#include <BnrOneAPlus.h>

namespace
{

constexpr byte SSPIN = 2U;
constexpr float MIN_BATTERY_V = 10.5F;
constexpr int TEST_POWER[] = {20, 35, 50};
constexpr int TEST_RPM[] = {20, 40, 80, 160};
constexpr unsigned long INFO_SHOW_MS = 3000UL;
constexpr unsigned long MOVE_MS = 400UL;
constexpr unsigned long RESULT_SHOW_MS = 1500UL;

BnrOneAPlus one;

/// @brief How a test step drives the motors.
enum class Drive : uint8_t
{
  POWER,  ///< Open-loop move(), percent power.
  RPM,    ///< PID moveRpm(), motor RPM.
};

/// @brief Runs both motors briefly and shows the counts it produced.
/// @param [in] drive Command used.
/// @param [in] value Signed power or RPM for both motors.
void runStep(Drive drive, int value)
{
  one.resetEncoders();
  switch (drive)
  {
    case Drive::POWER:
      one.move(value, value);
      break;
    case Drive::RPM:
      one.moveRpm(value, value);
      break;
  }
  delay(MOVE_MS);
  one.stop();
  int left_counts = 0;
  int right_counts = 0;
  one.readAndResetEncoders(left_counts, right_counts);
  one.lcd1((drive == Drive::POWER) ? "PWR " : "RPM ", value);
  one.lcd2(left_counts, right_counts);
  delay(RESULT_SHOW_MS);
}

/// @brief Waits until any button on the robot is pressed.
void waitForButton()
{
  while (one.readButton() == 0U)
  {
  }
}

}  // namespace

void setup()
{
  one.spiConnect(SSPIN);
  one.stop();
  one.setMinBatteryV(MIN_BATTERY_V);

  // Motors are cut below MIN_BATTERY_V; check this first.
  one.lcd1("Battery V:");
  one.lcd2(static_cast<double>(one.readBattery()));
  delay(INFO_SHOW_MS);

  // moveRpm() needs recent firmware; older boards ignore it silently.
  byte major = 0U;
  byte minor = 0U;
  byte patch = 0U;
  one.readFirmware(&major, &minor, &patch);
  one.lcd1("Firmware:");
  one.lcd2(static_cast<unsigned int>(major),
           static_cast<unsigned int>(minor),
           static_cast<unsigned int>(patch));
  delay(INFO_SHOW_MS);

  one.lcd1("Motor test");
  one.lcd2("Press a button");
  waitForButton();
}

void loop()
{
  for (const int power : TEST_POWER)
  {
    runStep(Drive::POWER, power);   // up
    runStep(Drive::POWER, -power);  // back down
  }
  for (const int rpm : TEST_RPM)
  {
    runStep(Drive::RPM, rpm);
    runStep(Drive::RPM, -rpm);
  }
  one.lcd1("Done. Press a");
  one.lcd2("button to repeat");
  waitForButton();
}
