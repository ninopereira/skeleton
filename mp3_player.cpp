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
/// @}

/// @name Command codes
/// @{
constexpr uint8_t CMD_SET_VOLUME = 0x06U;
constexpr uint8_t CMD_LOOP_TRACK = 0x08U;
constexpr uint8_t CMD_SELECT_DEVICE = 0x09U;
constexpr uint8_t CMD_PLAY = 0x0DU;
constexpr uint8_t CMD_PAUSE = 0x0EU;
/// @}

constexpr uint16_t DEVICE_TF_CARD = 0x0002U;
constexpr uint8_t MAX_VOLUME = 30U;

/// Module needs ~500 ms after power-up before it accepts commands.
constexpr uint32_t BOOT_DELAY_MS = 500U;
/// Card mount time after selecting the TF device.
constexpr uint32_t DEVICE_SELECT_DELAY_MS = 200U;

}  // namespace

Mp3Player::Mp3Player(HardwareSerial& serial) : serial_(serial)
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

void Mp3Player::playLooped(uint16_t track)
{
  sendCommand(CMD_LOOP_TRACK, track);
}

void Mp3Player::pause()
{
  sendCommand(CMD_PAUSE, 0U);
}

void Mp3Player::resume()
{
  sendCommand(CMD_PLAY, 0U);
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
