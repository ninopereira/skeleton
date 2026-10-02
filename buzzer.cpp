/// @file buzzer.cpp
/// @brief Buzzer implementation.

#include "buzzer.hpp"

#include <Arduino.h>

#include "config.hpp"

namespace skeleton
{

Buzzer::Buzzer(uint8_t pin) : pin_(pin)
{
}

void Buzzer::begin()
{
  pinMode(pin_, OUTPUT);
  noTone(pin_);
}

void Buzzer::beep(uint8_t count)
{
  for (uint8_t i = 0U; i < count; ++i)
  {
    tone(pin_, config::BEEP_FREQUENCY_HZ);
    delay(config::BEEP_ON_MS);
    noTone(pin_);
    delay(config::BEEP_OFF_MS);
  }
}

}  // namespace skeleton
