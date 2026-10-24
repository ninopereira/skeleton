/// @file config.hpp
/// @brief Pin assignments and tunable parameters for the haunted skeleton.

#pragma once

#include <stdint.h>

namespace skeleton
{
namespace config
{

/// @name Visitor detection
/// @{
/// @brief Sensors that can detect a visitor.
enum class DistanceSensorType : uint8_t
{
  SONAR,           ///< HC-SR04 ultrasonic, on SONAR_TRIG_PIN / ECHO_PIN.
  TIME_OF_FLIGHT,  ///< VL53L0X laser, on the I2C pins (SDA A4, SCL A5).
};
/// The sensor fitted to the skeleton.
constexpr DistanceSensorType DISTANCE_SENSOR =
    DistanceSensorType::TIME_OF_FLIGHT;

/// Readings below this are treated as noise.
constexpr int16_t DETECT_MIN_CM = 3;
/// Anything closer than this counts as a visitor.
constexpr int16_t DETECT_MAX_CM = 90;
/// Consecutive in-range readings required before reporting presence.
constexpr uint8_t DETECT_CONFIRM_COUNT = 2U;
/// @}

/// @name HC-SR04 sonar
/// @{
constexpr uint8_t SONAR_TRIG_PIN = 7U;
constexpr uint8_t SONAR_ECHO_PIN = 6U;

/// Time between pings; HC-SR04 datasheet recommends >= 60 ms.
constexpr uint32_t SONAR_PERIOD_MS = 60U;
/// Echo timeout (~4 m round trip) so pulseIn() never blocks for long.
constexpr uint32_t SONAR_TIMEOUT_US = 25000U;
/// @}

/// @name VL53L0X time-of-flight sensor
/// @{
/// Time spent on each measurement; longer is more accurate. 33 ms is the
/// sensor's default and reaches about 1.2 m.
constexpr uint32_t TOF_TIMING_BUDGET_US = 33000U;
/// Time between measurement starts; must cover the timing budget.
constexpr uint32_t TOF_PERIOD_MS = 50U;
/// Readings beyond this are "nothing there" (the sensor reports ~8 m).
constexpr uint16_t TOF_MAX_RANGE_MM = 1000U;
/// With no new measurement for this long the sensor is assumed lost and
/// any visitor is cleared.
constexpr uint32_t TOF_STALE_MS = 500U;
/// Limits on the library's own waits and on each I2C transfer.
constexpr uint16_t TOF_IO_TIMEOUT_MS = 500U;
constexpr uint32_t TOF_I2C_TIMEOUT_US = 3000U;
/// Standard mode: the 3.3 V pull-ups only just clear the Uno's 3.0 V
/// "high" threshold, so the lines need time to rise.
constexpr uint32_t TOF_I2C_CLOCK_HZ = 100000U;

static_assert((TOF_PERIOD_MS * 1000U) >= TOF_TIMING_BUDGET_US,
              "the period must leave time for the whole measurement");
static_assert(TOF_STALE_MS > TOF_PERIOD_MS,
              "the sensor must get a chance to report before it is stale");
static_assert(static_cast<uint32_t>(DETECT_MAX_CM) * 10U < TOF_MAX_RANGE_MM,
              "the detection range must be within the sensor's range");
/// @}

/// @name DFPlayer Mini MP3 player (software serial)
/// @{
/// Board pin wired to the module's TX.
constexpr uint8_t MP3_RX_PIN = 3U;
/// Board pin wired to the module's RX through a 680 Ohm + 1 kOhm divider
/// (5 V to ~3 V; the module runs on 3.3 V).
constexpr uint8_t MP3_TX_PIN = 2U;
/// Board pin wired to the module's BUSY output (14 is A0 on the Uno). It
/// is read with analogRead(), so it must be one of A0..A5.
constexpr uint8_t MP3_BUSY_PIN = 14U;
constexpr uint8_t MP3_VOLUME = 25U;
static_assert(MP3_VOLUME <= 30U, "DFPlayer volume range is 0..30");
/// The voice clips are adverts, /ADVERT/0001.mp3 onwards, one per jaw
/// script in mouth_script.hpp: an advert pauses the song and the module
/// resumes the song where it left off when the voice ends.
/// Songs are numbered in the order they were copied to the card, so copy
/// them first, in the order they should play, then the ADVERT folder.
constexpr uint16_t MP3_FIRST_SONG_TRACK = 1U;
constexpr uint16_t MP3_SONG_COUNT = 42U;
/// "Track finished" reports arriving this soon after a song starts or
/// resumes are ignored: they belong to the voice clip or are duplicates.
constexpr uint32_t MP3_FINISH_GUARD_MS = 2000U;
/// BUSY must stay idle this long before the song counts as over; it blips
/// idle while an advert starts or ends.
constexpr uint32_t MP3_STOPPED_MS = 1000U;
/// Wait between a "track finished" report and the next play command. The
/// module sends the report twice and may drop a command sent in between.
constexpr uint32_t MP3_NEXT_SONG_DELAY_MS = 500U;
/// Times a play command the module rejects is resent before the song is
/// skipped.
constexpr uint8_t MP3_PLAY_RETRY_COUNT = 3U;

static_assert(MP3_NEXT_SONG_DELAY_MS < MP3_FINISH_GUARD_MS,
              "a rejected play must be reported within the guard");

static_assert(MP3_SONG_COUNT > 0U, "the playlist needs at least one song");
/// Time from the play command until sound comes out (file seek/decode).
/// The jaw starts this much later so it stays in sync; tune by eye.
constexpr uint32_t VOICE_LATENCY_MS = 100U;
/// @}

/// @name Servo rest
/// @{
/// After a servo comes to rest it is detached (no more pulses), which
/// stops the twitching caused by pulse jitter. This is how long it gets
/// to reach the rest angle first.
constexpr uint32_t SERVO_DETACH_DELAY_MS = 500U;
/// @}

/// @name Mouth servo
/// @{
constexpr uint8_t MOUTH_SERVO_PIN = 9U;
/// Calibrate these on the real jaw.
constexpr uint8_t MOUTH_CLOSED_DEG = 90U;
constexpr uint8_t MOUTH_OPEN_DEG = 130U;
/// @}

/// @name Arm servos
/// @{
constexpr uint8_t LEFT_ARM_SERVO_PIN = 10U;
constexpr uint8_t RIGHT_ARM_SERVO_PIN = 11U;
/// Calibrate these on the real arms. The servos face opposite ways, so
/// the right arm's angles are mirrored (up may be smaller than down).
constexpr uint8_t LEFT_ARM_DOWN_DEG = 20U;
constexpr uint8_t LEFT_ARM_UP_DEG = 70U;
constexpr uint8_t RIGHT_ARM_DOWN_DEG = 160U;
constexpr uint8_t RIGHT_ARM_UP_DEG = 100U;
/// Sweep times; longer is slower and smoother.
constexpr uint32_t ARM_RAISE_MS = 1500U;
constexpr uint32_t ARM_LOWER_MS = 1500U;
/// Pause at the top before coming back down.
constexpr uint32_t ARM_HOLD_TOP_MS = 700U;
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

/// @name Eye LEDs
/// @{
/// On from the moment a visitor is detected until the cooldown ends.
constexpr uint8_t EYES_LED_PIN = 8U;
/// Blinks at boot when the MP3 player or its SD card does not answer.
constexpr uint8_t MP3_ERROR_BLINK_COUNT = 5U;
/// Blinks at boot when the time-of-flight sensor does not answer.
constexpr uint8_t SENSOR_ERROR_BLINK_COUNT = 3U;
constexpr uint32_t ERROR_BLINK_MS = 150U;
/// Dark pause after each error pattern, so two patterns can be told apart.
constexpr uint32_t ERROR_GAP_MS = 1000U;
/// @}

/// @name Serial Monitor diagnostics
/// @{
/// Enables the Serial Monitor (pins 0 and 1) and the start-up messages.
constexpr bool SERIAL_DEBUG = true;
/// Prints every distance sensor reading (needs SERIAL_DEBUG).
constexpr bool SERIAL_DEBUG_SENSOR = false;
/// Prints the MP3 player's commands and reports (needs SERIAL_DEBUG).
constexpr bool SERIAL_DEBUG_MP3 = true;
constexpr uint32_t SERIAL_BAUD = 115200U;
/// Time between printed sensor readings.
constexpr uint32_t SERIAL_PRINT_PERIOD_MS = 100U;
/// @}

/// @name Behaviour timing
/// @{
/// Minimum quiet time after the mouth finishes before it may talk again.
constexpr uint32_t MOUTH_COOLDOWN_MS = 5000U;
/// @}

}  // namespace config
}  // namespace skeleton
