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

#ifndef INK_GEOMETRY_CONVEX_HULL_H_
#define INK_GEOMETRY_CONVEX_HULL_H_

#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/internal/convex_hull_helper.h"
#include "ink/geometry/internal/point_tessellation_helper.h"
#include "ink/geometry/point.h"

namespace ink {

inline std::vector<Point> ConvexHull(absl::Span<const Point> points) {
  return ink::geometry_internal::ConvexHull<
      geometry_internal::PointTessellationHelper>(points);
}

}  // namespace ink

#endif  // INK_GEOMETRY_CONVEX_HULL_H_
