/// @file config.hpp
/// @brief Pin assignments and tunable parameters for the haunted skeleton.

#pragma once

#include <stdint.h>

namespace skeleton
{
namespace config
{

/// @name Bot'n Roll ONE A+
/// @{
/// SPI slave-select pin used by the BnrOneAPlus library to talk to the
/// co-processor (which drives the motors and reads the encoders).
constexpr uint8_t BNR_SPI_SS_PIN = 2U;
/// Motors are cut below this battery voltage (value from the library
/// examples).
constexpr float MIN_BATTERY_V = 10.5F;
/// @}

/// @name HC-SR04 sonar
/// @{
constexpr uint8_t SONAR_TRIG_PIN = 7U;
constexpr uint8_t SONAR_ECHO_PIN = 6U;

/// Time between pings; HC-SR04 datasheet recommends >= 60 ms.
constexpr uint32_t SONAR_PERIOD_MS = 60U;
/// Echo timeout (~4 m round trip) so pulseIn() never blocks for long.
constexpr uint32_t SONAR_TIMEOUT_US = 25000U;
/// Readings below this are treated as noise.
constexpr int16_t DETECT_MIN_CM = 3;
/// Anything closer than this counts as a visitor.
constexpr int16_t DETECT_MAX_CM = 80;
/// Consecutive in-range readings required before reporting presence.
constexpr uint8_t DETECT_CONFIRM_COUNT = 2U;
/// @}

/// @name Catalex serial MP3 player (on hardware Serial, pins 0/1)
/// @{
constexpr uint32_t MP3_BAUD = 9600U;
/// Volume range is 0..30.
constexpr uint8_t MP3_VOLUME = 25U;
/// Tracks are numbered in the order files were copied to the card: copy
/// haunted.mp3 first, then voice.mp3.
constexpr uint16_t MP3_MUSIC_TRACK = 1U;
constexpr uint16_t MP3_VOICE_TRACK = 2U;
/// Time from the play command until sound comes out (file seek/decode).
/// The jaw starts this much later so it stays in sync; tune by eye.
constexpr uint32_t VOICE_LATENCY_MS = 100U;
/// @}

/// @name Mouth servo (Bot'n Roll SER1 connector)
/// @{
/// SER1 is wired straight to an Arduino pin on the ONE A+ (see the
/// library's PanTilt example).
constexpr uint8_t MOUTH_SERVO_PIN = 3U;
/// Calibrate these on the real jaw.
constexpr uint8_t MOUTH_CLOSED_DEG = 90U;
constexpr uint8_t MOUTH_OPEN_DEG = 130U;
/// @}

/// @name Arms (left motor = left arm, right motor = right arm)
/// @{
/// Position 0 is the arm resting down at power-up; this is the top of the
/// gesture.
constexpr int16_t ARM_MAX_COUNTS = 30;
/// Close enough to the target to stop.
constexpr int16_t ARM_TOLERANCE_COUNTS = 1;
/// Within this distance of the target the motor slows down to limit
/// overshoot.
constexpr int16_t ARM_SLOW_ZONE_COUNTS = 5;
/// Speeds in motor RPM, sent with moveRpm() so the board's PID holds them.
constexpr int16_t ARM_UP_RPM = 70;
constexpr int16_t ARM_DOWN_RPM = 60;
constexpr int16_t ARM_SLOW_RPM = 25;
/// Library default maximum motor speed (RobotParams::max_speed_rpm).
constexpr int16_t MOTOR_MAX_RPM = 300;
/// Pause at the top before coming back down.
constexpr uint32_t ARM_HOLD_TOP_MS = 200U;
/// The second arm starts this long after the first (random in range), so
/// they never move in lockstep.
constexpr uint32_t ARM_STAGGER_MIN_MS = 250U;
constexpr uint32_t ARM_STAGGER_MAX_MS = 500U;
/// Give up on a move after this long (blocked arm / missed encoder).
constexpr uint32_t ARM_MOVE_TIMEOUT_MS = 1500U;
/// Encoder read and motor command period.
constexpr uint32_t ARM_CONTROL_PERIOD_MS = 10U;
/// Set to -1 if positive motor speed moves that arm down.
constexpr int8_t LEFT_ARM_UP_SIGN = 1;
constexpr int8_t RIGHT_ARM_UP_SIGN = 1;

static_assert(ARM_TOLERANCE_COUNTS < ARM_SLOW_ZONE_COUNTS,
              "tolerance must be inside the slow zone");
static_assert(ARM_SLOW_ZONE_COUNTS < ARM_MAX_COUNTS,
              "slow zone must be smaller than the travel");
static_assert((ARM_SLOW_RPM > 0) && (ARM_SLOW_RPM <= ARM_UP_RPM) &&
                  (ARM_SLOW_RPM <= ARM_DOWN_RPM) &&
                  (ARM_UP_RPM <= MOTOR_MAX_RPM) &&
                  (ARM_DOWN_RPM <= MOTOR_MAX_RPM),
              "arm speeds must be within ARM_SLOW_RPM..MOTOR_MAX_RPM");
static_assert(ARM_STAGGER_MIN_MS <= ARM_STAGGER_MAX_MS,
              "minimum stagger must not exceed maximum stagger");
/// @}

/// @name Buzzer (on-board, see the library's Buzzer example)
/// @{
constexpr uint8_t BUZZER_PIN = 9U;
constexpr uint16_t BEEP_FREQUENCY_HZ = 2000U;
constexpr uint32_t BEEP_ON_MS = 120U;
constexpr uint32_t BEEP_OFF_MS = 120U;
/// Beeps played once at boot, right before the music starts.
constexpr uint8_t BOOT_BEEP_COUNT = 2U;
/// @}

/// @name Behaviour timing
/// @{
/// Minimum quiet time after the mouth finishes before it may talk again.
constexpr uint32_t MOUTH_COOLDOWN_MS = 5000U;
/// @}

}  // namespace config
}  // namespace skeleton
