/// @file mp3_test.ino
/// @brief Bench test for the DFPlayer Mini.
///
/// Plays the SD card tracks one after the other, starting with track 1.
/// The module is on the hardware serial port, as in the main sketch, so
/// the Serial Monitor cannot be used; the eye LEDs show what happens:
/// - lit while the module plays (its BUSY output is active);
/// - five blinks, repeated, when the module or its SD card does not
///   answer.
///
/// Wiring: module VCC -> 3V3 (not 5V), module TX -> D0 (RX), D1 (TX) ->
/// 680 Ohm -> module RX, 1 kOhm from module RX to GND, module BUSY -> A0;
/// eye LEDs on D8. Upload this sketch on its own (open tools/mp3_test in
/// the IDE); if the upload fails, unplug the module's TX from D0 while
/// uploading.

#include <stdint.h>

#include <DFRobotDFPlayerMini.h>

namespace
{

constexpr uint32_t MP3_BAUD = 9600U;
/// 14 is A0 on the Uno.
constexpr uint8_t MP3_BUSY_PIN = 14U;
constexpr uint8_t EYES_LED_PIN = 8U;
/// Volume range is 0..30.
constexpr uint8_t MP3_VOLUME = 25U;
constexpr uint16_t FIRST_TRACK = 1U;
/// BUSY is 0 V while playing and the module's 3.3 V when idle, too close
/// to the Uno's 3.0 V digital threshold, so it is split at ~1.65 V on the
/// ADC (10 bits over 5 V).
constexpr int BUSY_IDLE_MIN_COUNTS = 338;
/// Silence and "finished" reports this soon after a play command are
/// ignored: BUSY takes a moment to go active, and the module repeats the
/// previous report.
constexpr uint32_t FINISH_GUARD_MS = 2000U;
/// BUSY must stay idle this long before the track counts as over.
constexpr uint32_t STOPPED_MS = 1000U;
/// Same pattern as the main sketch's "MP3 player not answering".
constexpr uint8_t ERROR_BLINK_COUNT = 5U;
constexpr uint32_t ERROR_BLINK_MS = 150U;
constexpr uint32_t ERROR_GAP_MS = 1000U;

DFRobotDFPlayerMini mp3;

bool is_online = false;
uint16_t track = FIRST_TRACK;
uint16_t track_count = 0U;
uint32_t track_start_ms = 0U;
uint32_t last_busy_ms = 0U;

/// @brief Plays a track.
/// @param [in] new_track 1-based track index on the SD card.
void playTrack(uint16_t new_track)
{
  track = new_track;
  mp3.play(static_cast<int>(track));
  track_start_ms = millis();
  last_busy_ms = track_start_ms;
}

/// @brief Plays the track after the current one, wrapping to the first.
void playNextTrack()
{
  const bool is_last = (track_count > 0U) && (track >= track_count);
  playTrack(is_last ? FIRST_TRACK : static_cast<uint16_t>(track + 1U));
}

/// @brief Blinks the error pattern once, then pauses.
void blinkError()
{
  for (uint8_t i = 0U; i < ERROR_BLINK_COUNT; ++i)
  {
    digitalWrite(EYES_LED_PIN, HIGH);
    delay(ERROR_BLINK_MS);
    digitalWrite(EYES_LED_PIN, LOW);
    delay(ERROR_BLINK_MS);
  }
  delay(ERROR_GAP_MS);
}

/// @brief Reads pending messages from the module.
/// @retval true  The module reported the end of a track.
/// @retval false Nothing, or only other messages.
bool readFinishReport()
{
  bool is_finished = false;
  while (mp3.available())
  {
    if (mp3.readType() == DFPlayerPlayFinished)
    {
      is_finished = true;
    }
  }
  return is_finished;
}

}  // namespace

void setup()
{
  pinMode(EYES_LED_PIN, OUTPUT);
  digitalWrite(EYES_LED_PIN, LOW);
  // No pull-up: it would drive 5 V into the module's 3.3 V output.
  pinMode(MP3_BUSY_PIN, INPUT);
  Serial.begin(MP3_BAUD);

  is_online = mp3.begin(Serial, /*isACK=*/true, /*doReset=*/true);
  if (!is_online)
  {
    return;
  }
  mp3.volume(MP3_VOLUME);
  const int file_count = mp3.readFileCounts();
  track_count = (file_count > 0) ? static_cast<uint16_t>(file_count) : 0U;
  playTrack(FIRST_TRACK);
}

void loop()
{
  if (!is_online)
  {
    blinkError();
    return;
  }

  const uint32_t now_ms = millis();
  const bool is_playing = analogRead(MP3_BUSY_PIN) < BUSY_IDLE_MIN_COUNTS;
  digitalWrite(EYES_LED_PIN, is_playing ? HIGH : LOW);
  if (is_playing)
  {
    last_busy_ms = now_ms;
  }

  // Clone modules often never report the end of a track, so a silent BUSY
  // ends it too; a track that will not play is skipped the same way.
  const bool is_finished = readFinishReport();
  const bool is_silent = (now_ms - last_busy_ms) >= STOPPED_MS;
  const bool is_settled = (now_ms - track_start_ms) >= FINISH_GUARD_MS;
  if ((is_finished || is_silent) && is_settled)
  {
    playNextTrack();
  }
}
