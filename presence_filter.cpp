/// @file presence_filter.cpp
/// @brief Presence debounce implementation.

#include "presence_filter.hpp"

#include "config.hpp"

namespace skeleton
{

PresenceFilter::PresenceFilter() : hit_count_(0U)
{
}

void PresenceFilter::addReading(int16_t distance_cm)
{
  const bool in_range = (distance_cm >= config::DETECT_MIN_CM) &&
                        (distance_cm <= config::DETECT_MAX_CM);
  if (!in_range)
  {
    hit_count_ = 0U;
  }
  else if (hit_count_ < config::DETECT_CONFIRM_COUNT)
  {
    ++hit_count_;
  }
  else
  {
    // Already confirmed; saturate instead of overflowing.
  }
}

bool PresenceFilter::isTargetPresent() const
{
  return hit_count_ >= config::DETECT_CONFIRM_COUNT;
}

}  // namespace skeleton
