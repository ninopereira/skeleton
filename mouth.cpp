/// @file mouth.cpp
/// @brief Jaw animation implementation.

#include "mouth.hpp"

namespace skeleton
{
namespace
{

/// @brief One step of the jaw script.
struct Keyframe
{
  uint8_t open_pct;   ///< 0 = closed, 100 = fully open.
  uint16_t hold_ms;   ///< How long to stay here before the next frame.
};

constexpr uint8_t SHUT = 0U;
constexpr uint8_t HALF = 55U;
constexpr uint8_t WIDE = 100U;
constexpr uint16_t GAP_MS = 70U;  // brief close between syllables

/// "Who are you? What are you doing here?" -- open on each vowel, close
/// between syllables, longer pause at the question mark.
constexpr Keyframe SPEECH[] = {
    {WIDE, 220U}, {SHUT, GAP_MS},  // Who
    {HALF, 150U}, {SHUT, GAP_MS},  // are
    {WIDE, 260U}, {SHUT, 400U},    // you?
    {WIDE, 200U}, {SHUT, GAP_MS},  // What
    {HALF, 140U}, {SHUT, GAP_MS},  // are
    {HALF, 160U}, {SHUT, GAP_MS},  // you
    {WIDE, 180U}, {SHUT, GAP_MS},  // do-
    {HALF, 140U}, {SHUT, GAP_MS},  // -ing
    {WIDE, 380U}, {SHUT, 150U},    // here?
};

constexpr uint8_t SPEECH_LENGTH =
    static_cast<uint8_t>(sizeof(SPEECH) / sizeof(SPEECH[0]));

constexpr uint8_t PERCENT_MAX = 100U;

}  // namespace

Mouth::Mouth(uint8_t servo_pin, uint8_t closed_deg, uint8_t open_deg)
    : servo_(),
      servo_pin_(servo_pin),
      closed_deg_(closed_deg),
      open_deg_(open_deg),
      frame_index_(0U),
      frame_start_ms_(0U),
      is_speaking_(false)
{
}

void Mouth::begin()
{
  // attach() only reports the timer channel it took; with a single servo
  // it cannot run out of channels.
  static_cast<void>(servo_.attach(servo_pin_));
  setOpenness(SHUT);
}

void Mouth::startSpeech(uint32_t now_ms)
{
  frame_index_ = 0U;
  frame_start_ms_ = now_ms;
  is_speaking_ = true;
  setOpenness(SPEECH[0].open_pct);
}

void Mouth::update(uint32_t now_ms)
{
  if (!is_speaking_ || (frame_index_ >= SPEECH_LENGTH))
  {
    return;
  }
  if ((now_ms - frame_start_ms_) < SPEECH[frame_index_].hold_ms)
  {
    return;
  }

  ++frame_index_;
  frame_start_ms_ = now_ms;
  if (frame_index_ >= SPEECH_LENGTH)
  {
    is_speaking_ = false;
    setOpenness(SHUT);
    return;
  }
  setOpenness(SPEECH[frame_index_].open_pct);
}

bool Mouth::isSpeaking() const
{
  return is_speaking_;
}

void Mouth::setOpenness(uint8_t open_pct)
{
  const int16_t pct = (open_pct > PERCENT_MAX) ? PERCENT_MAX : open_pct;
  // Signed span so a reversed servo (open < closed) also works.
  const int16_t span =
      static_cast<int16_t>(open_deg_) - static_cast<int16_t>(closed_deg_);
  const int16_t angle =
      static_cast<int16_t>(closed_deg_) + ((span * pct) / PERCENT_MAX);
  servo_.write(angle);
}

}  // namespace skeleton
