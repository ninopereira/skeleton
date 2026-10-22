/// @file skeleton.ino
/// @brief Entry point for the haunted skeleton (Arduino Uno).

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
