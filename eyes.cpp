/// @file eyes.cpp
/// @brief Eyes implementation.

#include "eyes.hpp"

#include <Arduino.h>

namespace skeleton
{

Eyes::Eyes(uint8_t pin) : pin_(pin)
{
}

void Eyes::begin()
{
  pinMode(pin_, OUTPUT);
  turnOff();
}

void Eyes::turnOn()
{
  digitalWrite(pin_, HIGH);
}

void Eyes::turnOff()
{
  digitalWrite(pin_, LOW);
}

}  // namespace skeleton
