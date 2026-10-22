/// @file mp3_player.cpp
/// @brief DFPlayer Mini driver implementation.

#include "mp3_player.hpp"

namespace skeleton
{
namespace
{

/// Fixed by the module.
constexpr uint32_t BAUD = 9600U;

}  // namespace

Mp3Player::Mp3Player(uint8_t rx_pin, uint8_t tx_pin)
    : serial_(rx_pin, tx_pin), player_()
{
}

bool Mp3Player::begin(uint8_t volume)
{
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
  Events events = {false, false, 0U};
  // available() returns after each complete message; readType() clears it.
  while (player_.available())
  {
    const uint8_t type = player_.readType();
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
      // Acknowledgements and card reports need no action.
    }
  }
  return events;
}

}  // namespace skeleton
