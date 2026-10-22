/// @file mp3_player.hpp
/// @brief Driver for the DFPlayer Mini MP3 module.

#pragma once

#include <stdint.h>

#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

namespace skeleton
{

/// @brief Plays tracks on a DFPlayer Mini over a software serial port.
///
/// Tracks are addressed by index in the order they were copied to the SD
/// card. The module reports the natural end of a track on its TX line.
///
/// @code{.cpp}
/// Mp3Player mp3(3U, 2U);
/// mp3.begin(25U);
/// mp3.play(1U);
/// // in loop():
/// if (mp3.poll().is_track_finished)
/// {
///   mp3.play(2U);
/// }
/// @endcode
class Mp3Player
{
 public:
  /// @brief Binds the player to two digital pins.
  /// @param [in] rx_pin Board pin wired to the module's TX.
  /// @param [in] tx_pin Board pin wired to the module's RX.
  Mp3Player(uint8_t rx_pin, uint8_t tx_pin);

  /// @brief What the module reported since the last poll().
  struct Events
  {
    bool is_track_finished;  ///< A track reached its natural end.
    bool has_error;          ///< The module rejected a command.
    uint16_t error_code;     ///< Module error code, when has_error is set.
  };

  Mp3Player(const Mp3Player&) = delete;
  Mp3Player& operator=(const Mp3Player&) = delete;
  Mp3Player(Mp3Player&&) = delete;
  Mp3Player& operator=(Mp3Player&&) = delete;

  /// @brief Resets the module and sets the volume. Blocks up to ~2 s.
  /// @param [in] volume Level 0..30.
  /// @retval true  The module answered and the SD card is online.
  /// @retval false No answer, or no card.
  bool begin(uint8_t volume);

  /// @brief Plays a track once, interrupting whatever is playing.
  /// @param [in] track 1-based track index on the SD card.
  void play(uint16_t track);

  /// @brief Plays /ADVERT/NNNN.mp3 over the current track.
  ///
  /// The current track pauses and resumes from the same position when the
  /// advert ends. Nothing plays if no track is playing.
  ///
  /// @param [in] file_number NNNN in the advert file name (1..3000).
  void playAdvert(uint16_t file_number);

  /// @brief Reads pending messages from the module without blocking.
  ///
  /// Call every loop(): the next command waits for the previous one's
  /// acknowledgement, which is consumed here.
  ///
  /// @return Track-finished and error reports received since the last call.
  Events poll();

 private:
  SoftwareSerial serial_;
  DFRobotDFPlayerMini player_;
};

}  // namespace skeleton
