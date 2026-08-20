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

#ifndef INK_GEOMETRY_INTERNAL_POINT_TESSELLATION_HELPER_H_
#define INK_GEOMETRY_INTERNAL_POINT_TESSELLATION_HELPER_H_

#include <cstddef>
#include <cstdint>

#include "absl/types/span.h"
#include "ink/geometry/point.h"

namespace ink::geometry_internal {

struct PointTessellationHelper {
  using VertexType = Point;

  static float GetX(const Point& vertex) { return vertex.x; }

  static float GetY(const Point& vertex) { return vertex.y; }

  static constexpr size_t GetXOffSetBytes() { return offsetof(Point, x); }

  static constexpr size_t GetYOffSetBytes() { return offsetof(Point, y); }

  static Point CreateVertex(float x, float y) { return {.x = x, .y = y}; }

  static void PopulateNonPositionAttributes(
      absl::Span<const Point> vertices,
      absl::Span<const uint32_t> neighbor_vertex_input_indices, Point& dst) {}
};

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_POINT_TESSELLATION_HELPER_H_
