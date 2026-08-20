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

#include "ink/geometry/internal/legacy_vector_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace geometry_internal {

namespace {

inline float PreviousFloat(float f) {
  return std::nextafter(f, std::numeric_limits<float>::lowest());
}
inline float NextFloat(float f) {
  return std::nextafter(f, std::numeric_limits<float>::max());
}

}  // namespace

RelativePos PositionRelativeToLine(Point p1, Point p2, Point test_point) {
  if (p1 == p2)
    return RelativePos::kIndeterminate;
  else if (p1 == test_point || p2 == test_point)
    return RelativePos::kCollinear;

  Vec line_vector = p2 - p1;
  float det = Vec::Determinant(line_vector, test_point - p1);
  if (det == 0) return RelativePos::kCollinear;

  // Checking whether the next representable point lies on the opposite side of
  // the line is relatively expensive, so we first check whether the test point
  // is close enough to the line. We use twice the machine epsilon of the
  // largest component-value as an approximation of "close enough" (this is
  // actually slightly larger than actual maximum "close enough" distance, which
  // is fine -- it still allows us to prune the vast majority of uninteresting
  // cases).
  //
  // We can't actually call geometry::Distance() here, as that would introduce a
  // circular dependency. However, recalling that det(a, b) = ‖a‖‖b‖sinθ, and
  // that a⋅a = ‖a‖², we can see that det(a, b)² / a⋅a = ‖b‖²sin²θ, which is the
  // squared height of the triangle formed by vectors a and b, i.e. the distance
  // from a point to a line.
  float max_distance =
      2 * std::numeric_limits<float>::epsilon() *
      std::max(std::max(std::max(std::abs(p1.x), std::abs(p1.y)),
                        std::max(std::abs(p2.x), std::abs(p2.y))),
               std::max(std::abs(test_point.x), std::abs(test_point.y)));
  if (det * det <=
      max_distance * max_distance * Vec::DotProduct(line_vector, line_vector)) {
    auto adjust_for_error = [](Point v, Vec dir) {
      auto adjusted = v;
      if (dir.x > 0) {
        adjusted.x = NextFloat(v.x);
      } else if (dir.x < 0) {
        adjusted.x = PreviousFloat(v.x);
      }
      if (dir.y > 0) {
        adjusted.y = NextFloat(v.y);
      } else if (dir.y < 0) {
        adjusted.y = PreviousFloat(v.y);
      }
      return adjusted;
    };

    Vec error_vector =
        det > 0 ? line_vector.Orthogonal() : -line_vector.Orthogonal();
    if (Vec::DotProduct(test_point.Offset(), test_point.Offset()) >
        std::max(Vec::DotProduct(p1.Offset(), p1.Offset()),
                 Vec::DotProduct(p2.Offset(), p2.Offset()))) {
      test_point = adjust_for_error(test_point, -error_vector);
    } else {
      p1 = adjust_for_error(p1, error_vector);
      p2 = adjust_for_error(p2, error_vector);
    }
    if (det * Vec::Determinant(p2 - p1, test_point - p1) <= 0)
      return RelativePos::kCollinear;
  }

  return det > 0 ? RelativePos::kLeft : RelativePos::kRight;
}

}  // namespace geometry_internal
}  // namespace ink
