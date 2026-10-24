/// @file playlist.cpp
/// @brief Shuffled playlist implementation.

#include "playlist.hpp"

#include <Arduino.h>

namespace skeleton
{

Playlist::Playlist() : order_(), position_(0U)
{
  for (uint16_t i = 0U; i < config::MP3_SONG_COUNT; ++i)
  {
    order_[i] = static_cast<uint8_t>(i);
  }
}

void Playlist::begin()
{
  shuffle();
  position_ = 0U;
}

void Playlist::advance()
{
  ++position_;
  if (position_ < config::MP3_SONG_COUNT)
  {
    return;
  }
  const uint8_t last_song = order_[config::MP3_SONG_COUNT - 1U];
  shuffle();
  if ((config::MP3_SONG_COUNT > 1U) && (order_[0] == last_song))
  {
    // Swap with a random later slot so the round's start stays random.
    const uint16_t other = static_cast<uint16_t>(
        random(1L, static_cast<long>(config::MP3_SONG_COUNT)));
    order_[0] = order_[other];
    order_[other] = last_song;
  }
  position_ = 0U;
}

uint16_t Playlist::getTrack() const
{
  return static_cast<uint16_t>(config::MP3_FIRST_SONG_TRACK +
                               order_[position_]);
}

void Playlist::shuffle()
{
  for (uint16_t i = config::MP3_SONG_COUNT - 1U; i > 0U; --i)
  {
    const uint16_t j =
        static_cast<uint16_t>(random(static_cast<long>(i) + 1L));
    const uint8_t swapped = order_[i];
    order_[i] = order_[j];
    order_[j] = swapped;
  }
}

}  // namespace skeleton
