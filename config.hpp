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
/// co-processor (which drives the motor outputs, unused here).
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

/// @name Arm servos
/// @{
/// The Servo library drives any digital pin (no hardware PWM needed), up
/// to 12 servos on the Uno. SER2 is pin 5 (see the library's PanTilt
/// example); the right arm needs another free digital pin.
constexpr uint8_t LEFT_ARM_SERVO_PIN = 5U;
constexpr uint8_t RIGHT_ARM_SERVO_PIN = 4U;
/// Calibrate these on the real arms. The servos face opposite ways, so
/// the right arm's angles are mirrored (up may be smaller than down).
constexpr uint8_t LEFT_ARM_DOWN_DEG = 20U;
constexpr uint8_t LEFT_ARM_UP_DEG = 110U;
constexpr uint8_t RIGHT_ARM_DOWN_DEG = 160U;
constexpr uint8_t RIGHT_ARM_UP_DEG = 70U;
/// Sweep times; longer is slower and smoother.
constexpr uint32_t ARM_RAISE_MS = 400U;
constexpr uint32_t ARM_LOWER_MS = 600U;
/// Pause at the top before coming back down.
constexpr uint32_t ARM_HOLD_TOP_MS = 200U;
/// The second arm starts this long after the first (random in range), so
/// they never move in lockstep.
constexpr uint32_t ARM_STAGGER_MIN_MS = 250U;
constexpr uint32_t ARM_STAGGER_MAX_MS = 500U;
/// Servo angle update period; matches the 20 ms servo pulse frame.
constexpr uint32_t ARM_UPDATE_PERIOD_MS = 20U;

static_assert((LEFT_ARM_DOWN_DEG <= 180U) && (LEFT_ARM_UP_DEG <= 180U) &&
                  (RIGHT_ARM_DOWN_DEG <= 180U) && (RIGHT_ARM_UP_DEG <= 180U),
              "servo angles must be within 0..180");
static_assert((ARM_RAISE_MS > 0U) && (ARM_LOWER_MS > 0U),
              "sweep times must be non-zero");
static_assert(ARM_STAGGER_MIN_MS <= ARM_STAGGER_MAX_MS,
              "minimum stagger must not exceed maximum stagger");
static_assert((LEFT_ARM_SERVO_PIN != RIGHT_ARM_SERVO_PIN) &&
                  (LEFT_ARM_SERVO_PIN != MOUTH_SERVO_PIN) &&
                  (RIGHT_ARM_SERVO_PIN != MOUTH_SERVO_PIN),
              "each servo needs its own pin");
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

/// @name Eye LEDs
/// @{
/// On from the moment a visitor is detected until the cooldown ends.
constexpr uint8_t EYES_LED_PIN = 8U;
/// @}

/// @name Behaviour timing
/// @{
/// Minimum quiet time after the mouth finishes before it may talk again.
constexpr uint32_t MOUTH_COOLDOWN_MS = 5000U;
/// @}

}  // namespace config
}  // namespace skeleton
