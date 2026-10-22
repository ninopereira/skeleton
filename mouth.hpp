/// @file mouth.hpp
/// @brief Non-blocking jaw animation driven by a servo.

#pragma once

#include <stdint.h>

#include <Servo.h>

#include "mouth_script.hpp"

namespace skeleton
{

/// @brief Plays a scripted jaw motion that mimics a spoken phrase.
///
/// Each script is a list of (openness, hold time) keyframes generated from
/// one voice clip (see mouth_script.hpp), so the jaw follows the recorded
/// speech. Call update() every loop iteration; it never blocks.
///
/// @code{.cpp}
/// Mouth mouth(9U, 90U, 130U);
/// mouth.begin();
/// mouth.startSpeech(0U, millis());
/// while (mouth.isSpeaking()) { mouth.update(millis()); }
/// @endcode
class Mouth
{
 public:
  /// @brief Creates the mouth controller.
  /// @param [in] servo_pin  Arduino pin wired to the servo signal.
  /// @param [in] closed_deg Servo angle with the jaw shut.
  /// @param [in] open_deg   Servo angle with the jaw fully open.
  Mouth(uint8_t servo_pin, uint8_t closed_deg, uint8_t open_deg);

  Mouth(const Mouth&) = delete;
  Mouth& operator=(const Mouth&) = delete;
  Mouth(Mouth&&) = delete;
  Mouth& operator=(Mouth&&) = delete;

  /// @brief Attaches the servo and closes the jaw.
  void begin();

  /// @brief Number of phrases, one per voice clip.
  static constexpr uint8_t SPEECH_COUNT = skeleton::SPEECH_COUNT;

  /// @brief Starts a phrase from its first keyframe.
  ///
  /// An out-of-range index is ignored and the jaw stays shut.
  ///
  /// @param [in] speech_index 0-based phrase, below SPEECH_COUNT; phrase i
  ///                          belongs to voice clip /ADVERT/000(i+1).mp3.
  /// @param [in] now_ms       Current time from millis().
  void startSpeech(uint8_t speech_index, uint32_t now_ms);

  /// @brief Advances the animation; call every loop iteration.
  /// @param [in] now_ms Current time from millis().
  void update(uint32_t now_ms);

  /// @retval true  The phrase is still being animated.
  /// @retval false The jaw is closed and idle.
  bool isSpeaking() const;

 private:
  /// @brief Commands the servo to a given openness.
  /// @param [in] open_pct 0 = closed, 100 = fully open.
  void setOpenness(uint8_t open_pct);

  /// @brief Copies the current keyframe out of flash and applies it.
  void loadFrame();

  Servo servo_;
  uint8_t servo_pin_;
  uint8_t closed_deg_;
  uint8_t open_deg_;
  /// Phrase being animated; points into SPEECHES.
  const Speech* speech_;
  /// Copy of speech_->frames[frame_index_], which lives in flash.
  Keyframe frame_;
  uint8_t frame_index_;
  uint32_t frame_start_ms_;
  bool is_speaking_;
};

}  // namespace skeleton
