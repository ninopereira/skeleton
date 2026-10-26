/// @file mp3_player.hpp
/// @brief Driver for the DFPlayer Mini MP3 module.

#pragma once

#include <stdint.h>

#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

namespace skeleton
{

/// @brief Plays tracks on a DFPlayer Mini over a hardware serial port.
///
/// Software serial ports block interrupts while each byte is sent or
/// received, which stretches the servo pulses and makes the arms jerk past
/// their limits; the hardware port does not.
///
/// Tracks are addressed by index in the order they were copied to the SD
/// card. Clone modules do not reliably report the end of a track on their
/// TX line, so the module's BUSY output (low while playing) is read too.
///
/// @code{.cpp}
/// Mp3Player mp3(Serial, 14U);
/// mp3.begin(25U);
/// mp3.play(1U);
/// // in loop():
/// static_cast<void>(mp3.poll());
/// if (!mp3.isPlaying())
/// {
///   mp3.play(2U);
/// }
/// @endcode
class Mp3Player
{
 public:
  /// @brief Binds the player to its serial port and BUSY pin.
  /// @param [in] serial   Port wired to the module; begin() opens it.
  /// @param [in] busy_pin Analog-capable board pin wired to the module's
  ///                      BUSY output.
  Mp3Player(HardwareSerial& serial, uint8_t busy_pin);

  /// @brief What the module reported since the last poll().
  struct Events
  {
    bool is_track_finished;  ///< A track reached its natural end.
    bool has_error;          ///< The module rejected a command.
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

  /// @brief Reads the module's BUSY output.
  ///
  /// It takes a few tens of milliseconds after a play command to go
  /// active, and blips inactive when an advert starts and ends.
  ///
  /// @retval true  The module is producing sound.
  /// @retval false The module is stopped or paused.
  bool isPlaying() const;

 private:
  HardwareSerial& serial_;
  DFRobotDFPlayerMini player_;
  uint8_t busy_pin_;
};

}  // namespace skeleton
