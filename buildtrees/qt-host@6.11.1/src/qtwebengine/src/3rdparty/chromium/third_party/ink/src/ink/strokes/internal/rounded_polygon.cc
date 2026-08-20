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

#include "ink/strokes/internal/rounded_polygon.h"

#include <cmath>
#include <cstddef>
#include <optional>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink::strokes_internal {

using ::ink::geometry_internal::Circle;

RoundedPolygon::RoundedPolygon(absl::Span<const Circle> circles) {
  ABSL_CHECK_GT(circles.size(), 1u);

  ABSL_CHECK(!circles.front().Contains(circles.back()));
  ABSL_CHECK(!circles.back().Contains(circles.front()));

  Angle last_to_first_tangent_angle =
      circles.back().GuaranteedRightTangentAngle(circles.front());

  Angle incoming_tangent_angle = last_to_first_tangent_angle;
  for (size_t i = 0; i < circles.size() - 1; ++i) {
    ABSL_CHECK(!circles[i].Contains(circles[i + 1]));
    ABSL_CHECK(!circles[i + 1].Contains(circles[i]));

    Angle outgoing_tangent_angle =
        circles[i].GuaranteedRightTangentAngle(circles[i + 1]);

    arcs_.push_back({.circle = circles[i],
                     .start_unit_vector = Vec::FromDirectionAndMagnitude(
                         incoming_tangent_angle, 1),
                     .end_unit_vector = Vec::FromDirectionAndMagnitude(
                         outgoing_tangent_angle, 1)});

    incoming_tangent_angle = outgoing_tangent_angle;
  }

  arcs_.push_back({.circle = circles.back(),
                   .start_unit_vector = Vec::FromDirectionAndMagnitude(
                       incoming_tangent_angle, 1),
                   .end_unit_vector = Vec::FromDirectionAndMagnitude(
                       last_to_first_tangent_angle, 1)});
}

namespace {

// Returns the signed distance from `point` to `arc` if `point` is between the
// rays from `arc`'s center in the direction of `start_unit_vector` and
// `end_unit_vector`; otherwise, the arc does not contribute to the distance to
// the `RoundedPolygon` boundary, and so this returns `std::nullopt`.
//
// To account for floating-point precision loss in the calculation of
// `start_unit_vector` and `end_unit_vector`, we allow for a small tolerance in
// which they are considered to point in the same direction; in that case,
// nothing is between the arcs and this returns `std::nullopt`.
std::optional<float> SignedDistanceToArc(const RoundedPolygon::Arc& arc,
                                         Point point) {
  // Check if `point` is between the two aforementioned rays.
  Vec circle_center_to_point_vector = point - arc.circle.Center();
  float arc_vector_det =
      Vec::Determinant(arc.start_unit_vector, arc.end_unit_vector);
  if (std::abs(arc_vector_det) < 5e-6) {
    // The start and end vector are sufficiently close that we consider there to
    // be no arc. This includes the case where the arc nominally loops all the
    // way around, which can occur due to floating-point precision loss.
    return std::nullopt;
  } else if (arc_vector_det < 0) {
    // This is a major arc (i.e. it winds around the long way); `point` is
    // inside if it is left of `arc.start_unit_vector` *or* right of
    // `arc.end_unit_vector`.
    if (Vec::Determinant(arc.start_unit_vector, circle_center_to_point_vector) <
            0 &&
        Vec::Determinant(arc.end_unit_vector, circle_center_to_point_vector) >
            0) {
      return std::nullopt;
    }
  } else {
    // This is a minor arc (i.e. it winds around the short way); `point` is
    // inside if it is left of `arc.start_unit_vector` *and* right of
    // `arc.end_unit_vector`.
    if (Vec::Determinant(arc.start_unit_vector, circle_center_to_point_vector) <
            0 ||
        Vec::Determinant(arc.end_unit_vector, circle_center_to_point_vector) >
            0) {
      return std::nullopt;
    }
  }

  // `point` is inside the circular sector; the distance from `point` to the arc
  // is equal to the distance from `point` to the circle's center minus the
  // circle's radius.
  return circle_center_to_point_vector.Magnitude() - arc.circle.Radius();
}

// Returns the signed distance from `point` to `segment` if the projection of
// `point` lies in the interval [0, 1]; otherwise, the segment does not
// contribute to the distance to the `RoundedPolygon` boundary, and so this
// returns `std::nullopt`.
std::optional<float> SignedDistanceToSegment(const Segment& segment,
                                             Point point) {
  std::optional<float> projection = segment.Project(point);
  if (!projection.has_value() || *projection < 0 || *projection > 1)
    return std::nullopt;

  return -Vec::Determinant(segment.Vector(), point - segment.start) /
         segment.Length();
}

}  // namespace

bool RoundedPolygon::ContainsCircle(const Circle& circle) const {
  // `circle` is contained in the `RoundedPolygon` iff its radius plus the
  // signed distance from its center to the boundary is less than or equal to
  // zero.
  for (size_t i = 0; i < arcs_.size(); ++i) {
    if (SignedDistanceToArc(arcs_[i], circle.Center()) > -circle.Radius() ||
        SignedDistanceToSegment(GetSegment(i), circle.Center()) >
            -circle.Radius())
      return false;
  }
  return true;
}

}  // namespace ink::strokes_internal
