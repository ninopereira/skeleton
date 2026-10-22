/// @file presence_filter.hpp
/// @brief Debounces distance readings into a visitor present/absent flag.

#pragma once

#include <stdint.h>

namespace skeleton
{

/// @brief Reports a target only after several consecutive in-range
///        readings, which filters out single spurious readings.
///
/// The range and the number of readings come from config::DETECT_*.
class PresenceFilter
{
 public:
  PresenceFilter();

  PresenceFilter(const PresenceFilter&) = delete;
  PresenceFilter& operator=(const PresenceFilter&) = delete;
  PresenceFilter(PresenceFilter&&) = delete;
  PresenceFilter& operator=(PresenceFilter&&) = delete;

  /// @brief Feeds one distance reading.
  /// @param [in] distance_cm Measured distance; any negative value means
  ///             nothing was detected.
  void addReading(int16_t distance_cm);

  /// @retval true  Enough consecutive readings were in range.
  /// @retval false Nothing in range.
  bool isTargetPresent() const;

 private:
  uint8_t hit_count_;
};

}  // namespace skeleton
