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

#include "ink/geometry/internal/legacy_triangle_contains.h"

#include "ink/geometry/internal/legacy_vector_utils.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace geometry_internal {

bool LegacyTriangleContains(const Triangle &triangle, Point p) {
  if (p == triangle.p0 || p == triangle.p1 || p == triangle.p2) return true;

  auto relative_pos1 = PositionRelativeToLine(triangle.p0, triangle.p1, p);
  auto relative_pos2 = PositionRelativeToLine(triangle.p1, triangle.p2, p);
  if (relative_pos1 == ReverseRelativePosition(relative_pos2)) return false;

  auto relative_pos3 = PositionRelativeToLine(triangle.p2, triangle.p0, p);
  return relative_pos1 != ReverseRelativePosition(relative_pos3) &&
         relative_pos2 != ReverseRelativePosition(relative_pos3);
}

}  // namespace geometry_internal
}  // namespace ink
