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

#ifndef INK_GEOMETRY_INTERNAL_LEGACY_VECTOR_UTILS_H_
#define INK_GEOMETRY_INTERNAL_LEGACY_VECTOR_UTILS_H_

#include "ink/geometry/point.h"

namespace ink::geometry_internal {

enum class RelativePos { kIndeterminate, kLeft, kCollinear, kRight };

inline RelativePos ReverseRelativePosition(RelativePos pos) {
  if (pos == RelativePos::kLeft) return RelativePos::kRight;
  if (pos == RelativePos::kRight) return RelativePos::kLeft;
  return pos;
}

// Returns the relative position of the test point w.r.t. the line through
// points p1 and p2. To account for floating-point error, the test point is
// considered collinear with the line if taking the next representable values of
// either the test point or the line results in a change in sign of the
// determinant.
RelativePos PositionRelativeToLine(Point p1, Point p2, Point test_point);

}  // namespace ink::geometry_internal
#endif  // INK_GEOMETRY_INTERNAL_LEGACY_VECTOR_UTILS_H_
