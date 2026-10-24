/// @file playlist.hpp
/// @brief Shuffled song order that plays every song before repeating one.

#pragma once

#include <stdint.h>

#include "config.hpp"

namespace skeleton
{

/// @brief Plays the songs in a random order, each once per round.
///
/// When a round ends a new order is drawn, so no song repeats until all of
/// them have played. The first song of a round is never the last song of
/// the round before, so no song plays twice in a row either. Draws come
/// from Arduino's random(); seed it before begin().
///
/// @code{.cpp}
/// Playlist playlist;
/// randomSeed(seed);
/// playlist.begin();
/// mp3.play(playlist.getTrack());
/// // when the song ends:
/// playlist.advance();
/// mp3.play(playlist.getTrack());
/// @endcode
class Playlist
{
 public:
  Playlist();

  Playlist(const Playlist&) = delete;
  Playlist& operator=(const Playlist&) = delete;
  Playlist(Playlist&&) = delete;
  Playlist& operator=(Playlist&&) = delete;

  /// @brief Draws the first round's order and moves to its first song.
  void begin();

  /// @brief Moves to the next song, drawing a new order after the last.
  void advance();

  /// @brief The current song.
  /// @return 1-based track index on the SD card.
  uint16_t getTrack() const;

 private:
  /// The song indices fit in a byte; keeps the order small in RAM.
  static_assert(config::MP3_SONG_COUNT <= 256U,
                "song indices must fit in uint8_t");

  /// @brief Shuffles order_ (Fisher-Yates).
  void shuffle();

  /// 0-based song indices in the order they play this round.
  uint8_t order_[config::MP3_SONG_COUNT];
  /// Position in order_ of the current song.
  uint16_t position_;
};

}  // namespace skeleton
