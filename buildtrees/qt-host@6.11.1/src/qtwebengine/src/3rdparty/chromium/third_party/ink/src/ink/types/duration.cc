// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ink/types/duration.h"

#include <cmath>
#include <limits>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/time/time.h"

namespace ink {

Duration32 operator*(Duration32 d, float scale) {
  if (std::isnan(scale)) {
    return d >= Duration32::Zero() ? Duration32::Infinite()
                                   : -Duration32::Infinite();
  }
  if (!d.IsFinite()) {
    return (d >= Duration32::Zero()) == (scale >= 0) ? Duration32::Infinite()
                                                     : -Duration32::Infinite();
  }
  return Duration32(d.value_seconds_ * scale);
}

Duration32 operator/(Duration32 d, float divisor) {
  if (std::isnan(divisor)) {
    return d >= Duration32::Zero() ? Duration32::Infinite()
                                   : -Duration32::Infinite();
  }
  if (!d.IsFinite()) {
    return (d >= Duration32::Zero()) == (divisor >= 0)
               ? Duration32::Infinite()
               : -Duration32::Infinite();
  }
  return Duration32(d.value_seconds_ / divisor);
}

float operator/(Duration32 a, Duration32 b) {
  if (b == Duration32::Zero()) {
    return (a >= Duration32::Zero()) ? std::numeric_limits<float>::infinity()
                                     : -std::numeric_limits<float>::infinity();
  }
  if (!a.IsFinite()) {
    return (a >= Duration32::Zero()) == (b >= Duration32::Zero())
               ? std::numeric_limits<float>::infinity()
               : -std::numeric_limits<float>::infinity();
  }
  return a.value_seconds_ / b.value_seconds_;
}

std::string Duration32::ToFormattedString() const {
  absl::Duration abseil_duration = ToAbseil();
  // Duration32 has a greater range than absl::Duration.  If converting a finite
  // Duration32 overflows to an infinite absl::Duration, fall back to printing
  // the value in seconds, so that it doesn't print the same as an actually
  // infinite Duration32.
  if ((abseil_duration == absl::InfiniteDuration() ||
       abseil_duration == -absl::InfiniteDuration()) &&
      IsFinite()) {
    return absl::StrCat(value_seconds_, "s");
  }
  // In all other cases, use absl::Duration's nicer formatting (which chooses
  // units based on the duration value).
  return absl::FormatDuration(abseil_duration);
}

}  // namespace ink
