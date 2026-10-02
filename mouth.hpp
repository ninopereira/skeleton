/// @file mouth.hpp
/// @brief Non-blocking jaw animation driven by the Bot'n Roll SER1 servo.

#pragma once

#include <stdint.h>

#include <Servo.h>

namespace skeleton
{

/// @brief Plays a scripted jaw motion that mimics a spoken phrase.
///
/// The script is a list of (openness, hold time) keyframes generated from
/// the voice clip (see mouth_script.hpp), so the jaw follows the recorded
/// speech. Call update() every loop iteration; it never blocks.
///
/// @code{.cpp}
/// Mouth mouth(3U, 90U, 130U);
/// mouth.begin();
/// mouth.startSpeech(millis());
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

  /// @brief Starts the phrase from the first keyframe.
  /// @param [in] now_ms Current time from millis().
  void startSpeech(uint32_t now_ms);

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

  Servo servo_;
  uint8_t servo_pin_;
  uint8_t closed_deg_;
  uint8_t open_deg_;
  uint8_t frame_index_;
  uint32_t frame_start_ms_;
  bool is_speaking_;
};

}  // namespace skeleton
