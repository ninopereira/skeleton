/// @file mp3_test.ino
/// @brief Bench test for the DFPlayer Mini.
///
/// Plays the SD card tracks one after the other, starting with track 1,
/// and prints what the module reports on the Serial Monitor (115200 baud).
/// Type a track number and press Enter to play that track.
///
/// Wiring: module VCC -> 3V3 (not 5V), module TX -> D3, D2 -> 680 Ohm ->
/// module RX, 1 kOhm from module RX to GND. Upload this sketch
/// on its own (open tools/mp3_test in the IDE).

#include <stdint.h>

#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

namespace
{

constexpr uint32_t MONITOR_BAUD = 115200U;
constexpr uint32_t MP3_BAUD = 9600U;
constexpr uint8_t MP3_RX_PIN = 3U;
constexpr uint8_t MP3_TX_PIN = 2U;
/// Volume range is 0..30.
constexpr uint8_t MP3_VOLUME = 25U;
constexpr uint16_t FIRST_TRACK = 1U;
/// "Finished" reports this soon after a play command are the module's
/// duplicate of the previous report.
constexpr uint32_t FINISH_GUARD_MS = 2000U;

SoftwareSerial mp3_serial(MP3_RX_PIN, MP3_TX_PIN);
DFRobotDFPlayerMini mp3;

uint16_t track = FIRST_TRACK;
uint16_t track_count = 0U;
uint32_t track_start_ms = 0U;

/// @brief Plays a track and prints its number.
/// @param [in] new_track 1-based track index on the SD card.
void playTrack(uint16_t new_track)
{
  track = new_track;
  mp3.play(static_cast<int>(track));
  track_start_ms = millis();
  Serial.print(F("Playing "));
  Serial.println(track);
}

/// @brief Plays the track after the current one, wrapping to the first.
void playNextTrack()
{
  const bool is_last = (track_count > 0U) && (track >= track_count);
  playTrack(is_last ? FIRST_TRACK : static_cast<uint16_t>(track + 1U));
}

/// @brief Prints one message from the module and reacts to it.
/// @param [in] type  Message type from readType().
/// @param [in] value Message parameter from read().
void handleMessage(uint8_t type, uint16_t value)
{
  switch (type)
  {
    case DFPlayerPlayFinished:
      Serial.print(F("Finished "));
      Serial.println(value);
      if ((millis() - track_start_ms) >= FINISH_GUARD_MS)
      {
        playNextTrack();
      }
      break;
    case DFPlayerError:
      // 1 busy, 2 sleeping, 3 bad frame, 4 bad checksum, 5 track out of
      // range, 6 track not found.
      Serial.print(F("Error "));
      Serial.println(value);
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card in"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card out"));
      break;
    case TimeOut:
      Serial.println(F("Timeout: no reply from the module"));
      break;
    default:
      Serial.print(F("Message "));
      Serial.print(type);
      Serial.print(F(" "));
      Serial.println(value);
      break;
  }
}

}  // namespace

void setup()
{
  Serial.begin(MONITOR_BAUD);
  mp3_serial.begin(MP3_BAUD);
  Serial.println(F("DFPlayer test"));

  if (!mp3.begin(mp3_serial, /*isACK=*/true, /*doReset=*/true))
  {
    Serial.println(F("No module or no SD card: check wiring and card"));
  }
  mp3.volume(MP3_VOLUME);
  const int file_count = mp3.readFileCounts();
  track_count = (file_count > 0) ? static_cast<uint16_t>(file_count) : 0U;
  Serial.print(F("Files on card: "));
  Serial.println(file_count);
  playTrack(FIRST_TRACK);
}

void loop()
{
  while (mp3.available())
  {
    const uint8_t type = mp3.readType();
    handleMessage(type, mp3.read());
  }

  if (Serial.available() > 0)
  {
    const long requested = Serial.parseInt();
    if ((requested > 0L) && (requested <= 0xFFFFL))
    {
      playTrack(static_cast<uint16_t>(requested));
    }
  }
}
