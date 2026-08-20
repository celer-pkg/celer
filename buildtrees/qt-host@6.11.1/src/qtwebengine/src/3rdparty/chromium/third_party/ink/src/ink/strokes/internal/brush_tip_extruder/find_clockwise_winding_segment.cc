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

#include "ink/strokes/internal/brush_tip_extruder/find_clockwise_winding_segment.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"

namespace ink {
namespace brush_tip_extruder_internal {

std::optional<Segment> FindLastClockwiseWindingTriangleFanSegment(
    const MutableMeshView& mesh, absl::Span<const IndexType> outer_indices,
    SideId outer_side_identifier, Point central_position) {
  if (outer_indices.size() < 2) return std::nullopt;

  // Construct each triangle that would make up the triangle fan and check that
  // none of them would have negative signed area, which indicates clockwise
  // winding. Degenerate triangles are ok. Left vs right `outline_side`
  // determines the order of positions in the proposed triangle.
  Point last_position = mesh.GetPosition(outer_indices.back());
  for (uint32_t i = outer_indices.size() - 1; i > 0; --i) {
    Point current_position = mesh.GetPosition(outer_indices[i - 1]);
    if (current_position == last_position) continue;
    Triangle triangle = {
        .p0 = central_position, .p1 = current_position, .p2 = last_position};
    if (outer_side_identifier == SideId::kLeft) {
      std::swap(triangle.p1, triangle.p2);
    }
    if (triangle.SignedArea() < 0) return triangle.GetEdge(1);
    last_position = current_position;
  }

  return std::nullopt;
}

std::optional<Segment> FindLastClockwiseWindingMultiTriangleFanSegment(
    const MutableMeshView& mesh, const Side& outer_side,
    Side::IndexOffsetRange outer_index_offset_range, Point central_position) {
  if (outer_side.indices.empty() ||
      outer_index_offset_range.last <= outer_index_offset_range.first) {
    return std::nullopt;
  }

  uint32_t upper_bound = outer_side.indices.size() - 1;
  for (uint32_t i = outer_side.intersection_discontinuities.size(); i > 0;
       --i) {
    if (upper_bound < outer_index_offset_range.first) return std::nullopt;

    const Side::IndexOffsetRange& discontinuity_range =
        outer_side.intersection_discontinuities[i - 1];
    uint32_t lower_bound = discontinuity_range.last;
    uint32_t first = std::max(lower_bound, outer_index_offset_range.first);
    uint32_t last = std::min(upper_bound, outer_index_offset_range.last);
    upper_bound = discontinuity_range.first;
    if (last < first) continue;

    auto indices =
        absl::MakeSpan(outer_side.indices.data() + first, last - first + 1);
    std::optional<Segment> segment = FindLastClockwiseWindingTriangleFanSegment(
        mesh, indices, outer_side.self_id, central_position);
    if (segment.has_value()) return segment;

    // Test the triangle connecting the first and last indices if necessary:
    if (outer_index_offset_range.first <= discontinuity_range.first) {
      std::optional<Segment> outer_segment =
          FindLastClockwiseWindingTriangleFanSegment(
              mesh,
              {outer_side.indices[discontinuity_range.first],
               outer_side.indices[discontinuity_range.last]},
              outer_side.self_id, central_position);
      if (outer_segment.has_value()) return outer_segment;
    }
  }

  uint32_t first = outer_index_offset_range.first;
  uint32_t last = std::min(upper_bound, outer_index_offset_range.last);
  if (last <= first) return std::nullopt;
  auto indices =
      absl::MakeConstSpan(outer_side.indices.data() + first, last - first + 1);
  return FindLastClockwiseWindingTriangleFanSegment(
      mesh, indices, outer_side.self_id, central_position);
}

}  // namespace brush_tip_extruder_internal
}  // namespace ink
