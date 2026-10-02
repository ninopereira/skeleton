/// @file haunted_skeleton.hpp
/// @brief Top-level behaviour: background music, sonar trigger, talking jaw.

#pragma once

#include <stdint.h>

#include <BnrOneAPlus.h>

#include "arms.hpp"
#include "buzzer.hpp"
#include "mouth.hpp"
#include "mp3_player.hpp"
#include "sonar.hpp"

namespace skeleton
{

/// @brief Coordinates the MP3 player, sonar and mouth.
///
/// - On begin() the music (haunted.mp3) starts looping.
/// - When the sonar sees a visitor the voice clip replaces the music and
///   the jaw moves in sync with it; the music then restarts from the top
///   (the player cannot resume a track after playing another one).
/// - While the jaw talks the arms flail randomly; afterwards they go down.
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

  /// @brief Starts the voice clip and the arms.
  /// @param [in] now_ms Current time from millis().
  void greetVisitor(uint32_t now_ms);

  /// @brief Advances the WAITING -> TALKING -> COOLDOWN cycle.
  /// @param [in] now_ms Current time from millis().
  void updateMode(uint32_t now_ms);

  BnrOneAPlus robot_;
  Mp3Player mp3_;
  Sonar sonar_;
  Mouth mouth_;
  Arms arms_;
  Buzzer buzzer_;
  Mode mode_;
  uint32_t cooldown_start_ms_;
  uint32_t voice_start_ms_;
};

}  // namespace skeleton
