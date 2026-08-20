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

#include "ink/geometry/internal/circle.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {

void Circle::AppendArcToPolyline(Angle start, Angle arc_angle,
                                 float max_chord_height,
                                 std::vector<Point>& polyline) const {
  ABSL_CHECK_GT(max_chord_height, 0);

  if (radius_ == 0) {
    polyline.insert(polyline.end(), {center_, center_});
    return;
  }

  Angle max_step_angle = GetArcAngleForChordHeight(max_chord_height);

  int32_t steps =
      std::clamp<float>(std::ceil(std::abs(arc_angle / max_step_angle)), 1,
                        std::numeric_limits<int16_t>::max());
  Angle step_angle = arc_angle / steps;

  // We do not call `vector::reserve` because we expect cases with multiple
  // "small" arcs strung together.
  polyline.push_back(GetPoint(start));
  for (int32_t i = 1; i < steps; ++i) {
    polyline.push_back(GetPoint(start + i * step_angle));
  }
  polyline.push_back(GetPoint(start + arc_angle));
}

std::optional<Circle::TangentAngles> Circle::GetTangentAngles(
    const Circle& other) const {
  Vec center_offset = other.center_ - center_;
  float distance = center_offset.Magnitude();
  float delta_radius = radius_ - other.radius_;

  if (center_ == other.center_ || std::abs(delta_radius) >= distance) {
    // If the centers coincide or the difference in radii is greater than or
    // equal to the distance between the centers, then this is either an
    // indeterminate or degenerate case.
    return std::nullopt;
  }

  Angle offset_angle = Acos(delta_radius / distance);
  Angle reference_angle = center_offset.Direction();
  return TangentAngles{
      .left = (reference_angle + offset_angle).NormalizedAboutZero(),
      .right = (reference_angle - offset_angle).NormalizedAboutZero()};
}

Angle Circle::GetArcAngleForChordHeight(float chord_height) const {
  if (radius_ == 0) return Angle();
  return 2 * Acos(std::clamp(1 - chord_height / radius_, -1.f, 1.f));
}

}  // namespace ink::geometry_internal
