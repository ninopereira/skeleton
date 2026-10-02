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
/// - When the sonar sees a visitor the music pauses for
///   config::MUSIC_PAUSE_MS and the jaw "says" its phrase.
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
    TALKING,   ///< Jaw animation in progress.
    COOLDOWN,  ///< Jaw resting before it may talk again.
  };

  /// @brief Pauses the music and starts the spoken phrase.
  /// @param [in] now_ms Current time from millis().
  void greetVisitor(uint32_t now_ms);

  /// @brief Advances the WAITING -> TALKING -> COOLDOWN cycle.
  /// @param [in] now_ms Current time from millis().
  void updateMode(uint32_t now_ms);

  /// @brief Resumes the music once its pause period has elapsed.
  /// @param [in] now_ms Current time from millis().
  void updateMusic(uint32_t now_ms);

  BnrOneAPlus robot_;
  Mp3Player mp3_;
  Sonar sonar_;
  Mouth mouth_;
  Arms arms_;
  Buzzer buzzer_;
  Mode mode_;
  uint32_t cooldown_start_ms_;
  uint32_t music_paused_at_ms_;
  bool is_music_paused_;
};

}  // namespace skeleton
