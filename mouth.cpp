/// @file mouth.cpp
/// @brief Jaw animation implementation.

#include "mouth.hpp"

#include <avr/pgmspace.h>

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{
namespace
{

constexpr uint8_t SHUT = 0U;

static_assert(SPEECH_COUNT > 0U, "at least one jaw script is required");

constexpr uint8_t PERCENT_MAX = 100U;

}  // namespace

Mouth::Mouth(uint8_t servo_pin, uint8_t closed_deg, uint8_t open_deg)
    : servo_(),
      servo_pin_(servo_pin),
      closed_deg_(closed_deg),
      open_deg_(open_deg),
      speech_(&SPEECHES[0]),
      frame_{SHUT, 0U},
      frame_index_(0U),
      frame_start_ms_(0U),
      is_speaking_(false)
{
}

void Mouth::begin()
{
  // attach() only reports the timer channel it took; the Uno has 12, far
  // more than the three servos used here.
  static_cast<void>(servo_.attach(servo_pin_));
  setOpenness(SHUT);
  frame_start_ms_ = millis();
}

void Mouth::startSpeech(uint8_t speech_index, uint32_t now_ms)
{
  if ((speech_index >= SPEECH_COUNT) ||
      (SPEECHES[speech_index].length == 0U))
  {
    return;
  }
  if (!servo_.attached())
  {
    static_cast<void>(servo_.attach(servo_pin_));
  }
  speech_ = &SPEECHES[speech_index];
  frame_index_ = 0U;
  frame_start_ms_ = now_ms;
  is_speaking_ = true;
  loadFrame();
}

void Mouth::update(uint32_t now_ms)
{
  if (!is_speaking_)
  {
    // frame_start_ms_ marks when the jaw was last told to shut.
    if (servo_.attached() &&
        ((now_ms - frame_start_ms_) >= config::SERVO_DETACH_DELAY_MS))
    {
      servo_.detach();
    }
    return;
  }
  if ((now_ms - frame_start_ms_) < frame_.hold_ms)
  {
    return;
  }

  ++frame_index_;
  frame_start_ms_ = now_ms;
  if (frame_index_ >= speech_->length)
  {
    is_speaking_ = false;
    setOpenness(SHUT);
    return;
  }
  loadFrame();
}

bool Mouth::isSpeaking() const
{
  return is_speaking_;
}

void Mouth::loadFrame()
{
  // Plain reads would fetch RAM at the flash address, so copy it out.
  static_cast<void>(memcpy_P(&frame_, &speech_->frames[frame_index_],
                             sizeof(frame_)));
  setOpenness(frame_.open_pct);
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
