/// @file mp3_player.hpp
/// @brief Driver for the Catalex Serial MP3 Player v1.0 (YX5300 chip).

#pragma once

#include <stdint.h>

#include <HardwareSerial.h>

namespace skeleton
{

/// @brief Sends commands to, and reads events from, a Catalex serial MP3
/// player.
///
/// The module cannot play files by name; tracks are addressed by index in
/// the order they were copied to the SD card. It reports the natural end
/// of a track on its TX line, which must be wired to the board's RX pin.
///
/// @code{.cpp}
/// Mp3Player mp3(Serial);
/// mp3.begin(9600U);
/// mp3.setVolume(25U);
/// mp3.playOnce(2U);
/// // in loop():
/// if (mp3.pollTrackFinished())
/// {
///   mp3.playOnce(3U);
/// }
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

  /// @brief Plays a track once, interrupting whatever is playing.
  /// @param [in] track 1-based track index on the SD card.
  void playOnce(uint16_t track);

  /// @brief Reads pending bytes from the module without blocking.
  ///
  /// Call every loop() so the receive buffer never overflows.
  ///
  /// @retval true  A track reached its natural end since the last call.
  /// @retval false No track finished.
  bool pollTrackFinished();

 private:
  /// Longest reply frame: 8 bytes, or 10 when the module adds a checksum.
  static constexpr uint8_t RX_FRAME_CAPACITY = 10U;

  /// @brief Feeds one received byte into the frame parser.
  /// @param [in] value Received byte.
  /// @retval true  The byte completed a "track finished" frame.
  /// @retval false Otherwise.
  bool consumeByte(uint8_t value);

  /// @brief Writes one 8-byte command frame.
  /// @param [in] command Command byte.
  /// @param [in] data    16-bit argument.
  void sendCommand(uint8_t command, uint16_t data);

  HardwareSerial& serial_;
  uint8_t rx_frame_[RX_FRAME_CAPACITY];
  uint8_t rx_count_;
};

}  // namespace skeleton
