/// @file mp3_player.cpp
/// @brief Catalex serial MP3 player driver implementation.

#include "mp3_player.hpp"

#include <Arduino.h>

namespace skeleton
{
namespace
{

/// @name Frame layout: 7E FF 06 CMD FB DH DL EF
/// @{
constexpr uint8_t FRAME_START = 0x7EU;
constexpr uint8_t FRAME_VERSION = 0xFFU;
constexpr uint8_t FRAME_LENGTH = 0x06U;
constexpr uint8_t FRAME_NO_FEEDBACK = 0x00U;
constexpr uint8_t FRAME_END = 0xEFU;
constexpr uint8_t FRAME_SIZE = 8U;
/// Offset of the command byte, the same in commands and replies.
constexpr uint8_t FRAME_COMMAND_INDEX = 3U;
/// @}

/// @name Command codes
/// @{
constexpr uint8_t CMD_PLAY_TRACK = 0x03U;
constexpr uint8_t CMD_SET_VOLUME = 0x06U;
constexpr uint8_t CMD_SELECT_DEVICE = 0x09U;
/// Sent by the module when a TF-card track ends on its own (not when it
/// is interrupted by another play command).
constexpr uint8_t EVT_TF_TRACK_FINISHED = 0x3DU;
/// @}

constexpr uint16_t DEVICE_TF_CARD = 0x0002U;
constexpr uint8_t MAX_VOLUME = 30U;

/// Module needs ~500 ms after power-up before it accepts commands.
constexpr uint32_t BOOT_DELAY_MS = 500U;
/// Card mount time after selecting the TF device.
constexpr uint32_t DEVICE_SELECT_DELAY_MS = 200U;

}  // namespace

Mp3Player::Mp3Player(HardwareSerial& serial)
    : serial_(serial), rx_frame_{}, rx_count_(0U)
{
}

void Mp3Player::begin(uint32_t baud)
{
  serial_.begin(baud);
  delay(BOOT_DELAY_MS);
  sendCommand(CMD_SELECT_DEVICE, DEVICE_TF_CARD);
  delay(DEVICE_SELECT_DELAY_MS);
}

void Mp3Player::setVolume(uint8_t volume)
{
  const uint8_t clamped = (volume > MAX_VOLUME) ? MAX_VOLUME : volume;
  sendCommand(CMD_SET_VOLUME, clamped);
}

void Mp3Player::playOnce(uint16_t track)
{
  sendCommand(CMD_PLAY_TRACK, track);
}

bool Mp3Player::pollTrackFinished()
{
  bool is_finished = false;
  while (serial_.available() > 0)
  {
    const int value = serial_.read();
    if (value < 0)
    {
      break;
    }
    if (consumeByte(static_cast<uint8_t>(value)))
    {
      is_finished = true;
    }
  }
  return is_finished;
}

bool Mp3Player::consumeByte(uint8_t value)
{
  // Skip bytes until a start byte, so the parser resynchronises after a
  // dropped or corrupted byte.
  if ((rx_count_ == 0U) && (value != FRAME_START))
  {
    return false;
  }
  rx_frame_[rx_count_] = value;
  ++rx_count_;

  // The checksum's high byte is always 0xFD..0xFF, so an end byte at
  // offset 7 can only belong to a frame without checksum.
  const bool is_complete =
      (value == FRAME_END) &&
      ((rx_count_ == FRAME_SIZE) || (rx_count_ == RX_FRAME_CAPACITY));
  if (is_complete)
  {
    rx_count_ = 0U;
    return (rx_frame_[1] == FRAME_VERSION) &&
           (rx_frame_[FRAME_COMMAND_INDEX] == EVT_TF_TRACK_FINISHED);
  }
  if (rx_count_ >= RX_FRAME_CAPACITY)
  {
    rx_count_ = 0U;  // no end byte where expected: drop the frame
  }
  return false;
}

void Mp3Player::sendCommand(uint8_t command, uint16_t data)
{
  const uint8_t frame[FRAME_SIZE] = {
      FRAME_START,
      FRAME_VERSION,
      FRAME_LENGTH,
      command,
      FRAME_NO_FEEDBACK,
      static_cast<uint8_t>(data >> 8U),
      static_cast<uint8_t>(data & 0xFFU),
      FRAME_END,
  };
  const size_t written = serial_.write(frame, FRAME_SIZE);
  // Hardware Serial queues into its TX buffer, so a short write means the
  // buffer is broken; there is nothing useful to recover at runtime.
  static_cast<void>(written);
}

}  // namespace skeleton
