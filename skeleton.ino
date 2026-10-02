/// @file skeleton.ino
/// @brief Entry point for the haunted skeleton (Bot'n Roll ONE A+ / Uno).
///
/// Hardware Serial (pins 0/1) belongs to the MP3 player, so there is no
/// debug output on the serial monitor. Disconnect the MP3 module's TX wire
/// (board RX, pin 0) while uploading.

#include <SPI.h>

#include "haunted_skeleton.hpp"

namespace
{

skeleton::HauntedSkeleton haunted_skeleton;

}  // namespace

void setup()
{
  haunted_skeleton.begin();
}

void loop()
{
  haunted_skeleton.update(millis());
}
