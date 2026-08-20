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

#include "ink/geometry/triangle.h"

#include <algorithm>
#include <optional>
#include <string>

#include "absl/log/absl_check.h"
#include "absl/strings/str_format.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {

bool Triangle::Contains(Point point) const {
  float d0 = Vec::Determinant(point - p0, p1 - p0);
  float d1 = Vec::Determinant(point - p1, p2 - p1);
  if (d0 * d1 < 0) return false;

  if (d0 == 0 && d1 == 0) {
    // The test point is on the same line as two segments of the triangle, which
    // means that the triangle must be degenerate. We still need to check
    // containment, though, since the triangle's boundary is still considered to
    // be contained by the triangle.

    // We project the corners of the triangle and the test point to the common
    // line, and check if the test point lies within the interval covered by the
    // triangle. Note that, since we're projecting to one of the edges of the
    // triangle, we know that the corners on either side of that edge are at
    // parameters 0 and 1, and so we only need to project the third corner.
    std::optional<float> third_corner_projection;
    std::optional<float> test_point_projection;

    // We have to choose a non-degenerate edge to project to, try the first
    // if the endpoints are not the same.
    if (p0 != p1) {
      Segment seg{p0, p1};
      third_corner_projection = seg.Project(p2);
      test_point_projection = seg.Project(point);
    }

    if ((!third_corner_projection.has_value() ||
         !test_point_projection.has_value()) &&
        p1 != p2) {
      // The first segment was degenerate, try the second. Note that this can
      // occur when the segments endpoints are not equal, but close enough that
      // the projection can't be found without underflowing.
      Segment seg{p1, p2};
      third_corner_projection = seg.Project(p0);
      test_point_projection = seg.Project(point);
    }

    if (!third_corner_projection.has_value() ||
        !test_point_projection.has_value()) {
      // The triangle is point-like, so the test point is only contained if it's
      // equal to the corners. There's no need to construct a `Segment` to do
      // the comparison.
      return point == p0;
    }

    float lower_bound = std::min(0.f, *third_corner_projection);
    float upper_bound = std::max(1.f, *third_corner_projection);
    return lower_bound <= *test_point_projection &&
           *test_point_projection <= upper_bound;
  }

  float d2 = Vec::Determinant(point - p2, p0 - p2);
  return (d1 * d2 >= 0) && (d0 * d2 >= 0);
}

Segment Triangle::GetEdge(int index) const {
  switch (index) {
    case 0:
      return Segment{p0, p1};
    case 1:
      return Segment{p1, p2};
    case 2:
      return Segment{p2, p0};
    default:
      ABSL_CHECK(false) << "Index " << index << " out of bounds";
  }
}

namespace triangle_internal {

std::string ToFormattedString(const Triangle& t) {
  return absl::StrFormat("{p0 = %v, p1 = %v, p2 = %v}", t.p0, t.p1, t.p2);
}

}  // namespace triangle_internal

}  // namespace ink
