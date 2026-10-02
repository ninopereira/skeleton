/// @file mouth.cpp
/// @brief Jaw animation implementation.

#include "mouth.hpp"

#include "mouth_script.hpp"

namespace skeleton
{
namespace
{

constexpr uint8_t SHUT = 0U;

static_assert((sizeof(SPEECH) / sizeof(SPEECH[0])) < 255U,
              "jaw script too long for an 8-bit frame index");
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
