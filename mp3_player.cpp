/// @file mp3_player.cpp
/// @brief DFPlayer Mini driver implementation.

#include "mp3_player.hpp"

#include <Arduino.h>

namespace skeleton
{
namespace
{

/// Fixed by the module.
constexpr uint16_t BAUD = 9600U;
/// BUSY swings between 0 V (playing) and the module's 3.3 V (idle). That
/// "high" is too close to the Uno's 3.0 V digital threshold to trust, so
/// it is read with the ADC and split at ~1.65 V (10 bits over 5 V).
constexpr int BUSY_IDLE_MIN_COUNTS = 338;

}  // namespace

Mp3Player::Mp3Player(uint8_t rx_pin, uint8_t tx_pin, uint8_t busy_pin)
    : serial_(rx_pin, tx_pin), player_(), busy_pin_(busy_pin)
{
}

bool Mp3Player::begin(uint8_t volume)
{
  // No pull-up: it would drive 5 V into the module's 3.3 V output.
  pinMode(busy_pin_, INPUT);
  serial_.begin(BAUD);
  const bool is_online =
      player_.begin(serial_, /*isACK=*/true, /*doReset=*/true);
  player_.volume(volume);
  return is_online;
}

void Mp3Player::play(uint16_t track)
{
  player_.play(static_cast<int>(track));
}

void Mp3Player::playAdvert(uint16_t file_number)
{
  player_.advertise(static_cast<int>(file_number));
}

Mp3Player::Events Mp3Player::poll()
{
  Events events = {false, false, 0U, false, 0U, 0U, 0U};
  // available() returns after each complete message; readType() clears it.
  while (player_.available())
  {
    const uint8_t type = player_.readType();
    const uint8_t command = player_.readCommand();
    if (type == DFPlayerPlayFinished)
    {
      events.is_track_finished = true;
    }
    else if (type == DFPlayerError)
    {
      events.has_error = true;
      events.error_code = player_.read();
    }
    else
    {
      events.has_other = true;
      events.other_type = type;
      events.other_command = command;
      events.other_value = player_.read();
    }
  }
  return events;
}

bool Mp3Player::isPlaying() const
{
  return analogRead(busy_pin_) < BUSY_IDLE_MIN_COUNTS;
}

}  // namespace skeleton
