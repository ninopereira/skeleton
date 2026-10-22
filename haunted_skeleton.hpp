/// @file haunted_skeleton.hpp
/// @brief Top-level behaviour: background music, visitor trigger, talking
///        jaw.

#pragma once

#include <stdint.h>

#include "arms.hpp"
#include "eyes.hpp"
#include "mouth.hpp"
#include "mp3_player.hpp"
#include "presence_sensor.hpp"

namespace skeleton
{

/// @brief Coordinates the MP3 player, visitor sensor and mouth.
///
/// - On begin() the playlist starts with its first song. Each song is
///   followed by the next one, wrapping back to the first after the last.
/// - When the sensor (config::DISTANCE_SENSOR) sees a visitor the song pauses, a randomly chosen voice
///   clip (never the same one twice in a row) plays with the jaw in sync, and the song then resumes where it
///   left off.
/// - When the jaw starts talking each arm goes up and back down once, one
///   slightly after the other.
/// - The eyes light up on detection and stay lit until the cooldown ends.
/// - After the phrase the jaw rests for config::MOUTH_COOLDOWN_MS before a
///   new detection can trigger it again.
class HauntedSkeleton
{
 public:
  HauntedSkeleton();

  HauntedSkeleton(const HauntedSkeleton&) = delete;
  HauntedSkeleton& operator=(const HauntedSkeleton&) = delete;
  HauntedSkeleton(HauntedSkeleton&&) = delete;
  HauntedSkeleton& operator=(HauntedSkeleton&&) = delete;

  /// @brief Initialises all hardware and starts the music. Call in setup().
  void begin();

  /// @brief Runs one non-blocking control step. Call in loop().
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

 private:
  /// @brief Behaviour state.
  enum class Mode : uint8_t
  {
    WAITING,   ///< Listening for a visitor.
    STARTING,  ///< Voice requested; waiting for the player to start sound.
    TALKING,   ///< Jaw animation in progress.
    COOLDOWN,  ///< Jaw resting before it may talk again.
  };

  /// @brief Starts a random voice clip; the jaw and arms follow after
  ///        latency.
  /// @param [in] now_ms Current time from millis().
  void greetVisitor(uint32_t now_ms);

  /// @brief Picks a random voice clip other than the previous one.
  /// @return 0-based phrase index, below Mouth::SPEECH_COUNT.
  uint8_t pickVoice() const;

  /// @brief Blinks the eyes to report a missing module, then pauses.
  /// @param [in] blink_count Number of blinks; tells the errors apart.
  void signalError(uint8_t blink_count);

  /// @brief Prints the sensor reading every config::SERIAL_PRINT_PERIOD_MS
  ///        when config::SERIAL_DEBUG is set.
  /// @param [in] now_ms Current time from millis().
  void printSensor(uint32_t now_ms);

  /// @brief Moves to the next song in the playlist and plays it.
  /// @param [in] now_ms Current time from millis().
  void playNextSong(uint32_t now_ms);

  /// @brief Starts the next song when the current one ends on its own.
  /// @param [in] now_ms Current time from millis().
  void updateMusic(uint32_t now_ms);

  /// @brief Advances the WAITING -> TALKING -> COOLDOWN cycle.
  /// @param [in] now_ms Current time from millis().
  void updateMode(uint32_t now_ms);

  Mp3Player mp3_;
  PresenceSensor sensor_;
  Mouth mouth_;
  Arms arms_;
  Eyes eyes_;
  Mode mode_;
  uint32_t cooldown_start_ms_;
  uint32_t voice_start_ms_;
  /// 0-based phrase chosen for the current (or last) greeting.
  uint8_t voice_index_;
  /// 0-based playlist position of the current song.
  uint16_t song_index_;
  /// When the current song started or last resumed after the voice.
  uint32_t song_start_ms_;
  /// When the sensor reading was last printed.
  uint32_t last_print_ms_;
};

}  // namespace skeleton
