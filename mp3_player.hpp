/// @file mp3_player.hpp
/// @brief Driver for the Catalex Serial MP3 Player v1.0 (YX5300 chip).

#pragma once

#include <stdint.h>

#include <HardwareSerial.h>

namespace skeleton
{

/// @brief Sends commands to a Catalex serial MP3 player.
///
/// The module cannot play files by name; tracks are addressed by index in
/// the order they were copied to the SD card.
///
/// @code{.cpp}
/// Mp3Player mp3(Serial);
/// mp3.begin(9600U);
/// mp3.setVolume(25U);
/// mp3.playLooped(1U);  // background music
/// mp3.playOnce(2U);    // interrupts it; music restarts via playLooped()
/// @endcode
class Mp3Player
{
 public:
  /// @brief Binds the player to a serial port.
  /// @param [in] serial Port wired to the module (not yet started).
  explicit Mp3Player(HardwareSerial& serial);

  Mp3Player(const Mp3Player&) = delete;
  Mp3Player& operator=(const Mp3Player&) = delete;
  Mp3Player(Mp3Player&&) = delete;
  Mp3Player& operator=(Mp3Player&&) = delete;

  /// @brief Starts the serial link and selects the SD card as source.
  ///
  /// Blocks for roughly 0.7 s while the module boots; call from setup().
  ///
  /// @param [in] baud Serial baud rate (the module uses 9600).
  void begin(uint32_t baud);

  /// @brief Sets the output volume.
  /// @param [in] volume Level 0..30; larger values are clamped.
  void setVolume(uint8_t volume);

  /// @brief Plays a track and repeats it forever.
  /// @param [in] track 1-based track index on the SD card.
  void playLooped(uint16_t track);

  /// @brief Plays a track once, interrupting whatever is playing.
  /// @param [in] track 1-based track index on the SD card.
  void playOnce(uint16_t track);

 private:
  /// @brief Writes one 8-byte command frame.
  /// @param [in] command Command byte.
  /// @param [in] data    16-bit argument.
  void sendCommand(uint8_t command, uint16_t data);

  HardwareSerial& serial_;
};

}  // namespace skeleton
