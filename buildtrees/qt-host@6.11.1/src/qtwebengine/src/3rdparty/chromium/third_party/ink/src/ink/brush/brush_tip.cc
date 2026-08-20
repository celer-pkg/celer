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

#include "ink/brush/brush_tip.h"

#include <cmath>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "ink/brush/brush_behavior.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"
#include "ink/types/duration.h"

namespace ink {

bool BrushTip::operator==(const BrushTip& other) const {
  return scale == other.scale && corner_rounding == other.corner_rounding &&
         slant == other.slant && pinch == other.pinch &&
         rotation == other.rotation &&
         opacity_multiplier == other.opacity_multiplier &&
         particle_gap_distance_scale == other.particle_gap_distance_scale &&
         particle_gap_duration == other.particle_gap_duration &&
         behaviors == other.behaviors;
}

bool BrushTip::operator!=(const BrushTip& other) const {
  return !(*this == other);
}

namespace brush_internal {

absl::Status ValidateBrushTipTopLevel(const BrushTip& tip) {
  if (!std::isfinite(tip.scale.x) || !std::isfinite(tip.scale.y) ||
      tip.scale.x < 0 || tip.scale.y < 0 || tip.scale == Vec{0, 0}) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "Both values of `BrushTip::scale` must be finite and non-negative, and "
        "at least one value must be positive. Got %v",
        tip.scale));
  }
  if (!(tip.corner_rounding >= 0 && tip.corner_rounding <= 1)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`BrushTip::corner_rounding` must be a value in the "
                        "interval [0, 1]. Got %f",
                        tip.corner_rounding));
  }

  if (!std::isfinite(tip.slant.ValueInRadians()) ||
      !(tip.slant >= -kQuarterTurn && tip.slant <= kQuarterTurn)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`BrushTip::slant` must be a finite value in the "
                        "interval [-π/2, π/2] radians. Got %v",
                        tip.slant));
  }
  if (!(tip.pinch >= 0 && tip.pinch <= 1)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`BrushTip::pinch` must be a value in the interval [0, 1]. Got %f",
        tip.pinch));
  }
  if (!std::isfinite(tip.rotation.ValueInRadians())) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`BrushTip::rotation` must be finite. Got %v", tip.rotation));
  }
  if (!(tip.opacity_multiplier >= 0 && tip.opacity_multiplier <= 2)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`BrushTip::opacity_multiplier` must be a value in the "
                        "interval [0, 2]. Got %f",
                        tip.opacity_multiplier));
  }
  if (!std::isfinite(tip.particle_gap_distance_scale) ||
      tip.particle_gap_distance_scale < 0) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`BrushTip::particle_gap_distance_scale` must be "
                        "finite and non-negative. Got %v",
                        tip.particle_gap_distance_scale));
  }
  if (!tip.particle_gap_duration.IsFinite() ||
      tip.particle_gap_duration < Duration32::Zero()) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`BrushTip::particle_gap_duration` must be finite and "
                        "non-negative. Got %v",
                        tip.particle_gap_duration));
  }
  return absl::OkStatus();
}

absl::Status ValidateBrushTip(const BrushTip& tip) {
  if (absl::Status status = ValidateBrushTipTopLevel(tip); !status.ok()) {
    return status;
  }
  for (const BrushBehavior& behavior : tip.behaviors) {
    if (absl::Status status = ValidateBrushBehavior(behavior); !status.ok()) {
      return status;
    }
  }
  return absl::OkStatus();
}

std::string ToFormattedString(const BrushTip& tip) {
  std::string formatted = absl::StrCat(
      "BrushTip{scale=", tip.scale, ", corner_rounding=", tip.corner_rounding);
  if (tip.slant != Angle()) {
    absl::StrAppend(&formatted, ", slant=", tip.slant);
  }
  if (tip.pinch != 0.f) {
    absl::StrAppend(&formatted, ", pinch=", tip.pinch);
  }
  if (tip.rotation != Angle()) {
    absl::StrAppend(&formatted, ", rotation=", tip.rotation);
  }
  if (tip.opacity_multiplier != 1.f) {
    absl::StrAppend(&formatted,
                    ", opacity_multiplier=", tip.opacity_multiplier);
  }
  if (tip.particle_gap_distance_scale != 0) {
    absl::StrAppend(&formatted, ", particle_gap_distance_scale=",
                    tip.particle_gap_distance_scale);
  }
  if (tip.particle_gap_duration != Duration32::Zero()) {
    absl::StrAppend(&formatted,
                    ", particle_gap_duration=", tip.particle_gap_duration);
  }
  if (!tip.behaviors.empty()) {
    absl::StrAppend(&formatted, ", behaviors={",
                    absl::StrJoin(tip.behaviors, ", "), "}");
  }
  formatted.push_back('}');
  return formatted;
}

}  // namespace brush_internal
}  // namespace ink
