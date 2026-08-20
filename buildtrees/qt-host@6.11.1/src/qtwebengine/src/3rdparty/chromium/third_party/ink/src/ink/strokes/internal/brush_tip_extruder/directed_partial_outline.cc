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

#include "ink/strokes/internal/brush_tip_extruder/directed_partial_outline.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/internal/legacy_segment_intersection.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::ink::geometry_internal::LegacyIntersection;
using ::ink::geometry_internal::LegacySegmentIntersection;

float DistanceBetween(Point a, Point b) { return (a - b).Magnitude(); }

}  // namespace

DirectedPartialOutline::DirectedPartialOutline(
    const std::vector<IndexType>* starting_indices,
    uint32_t starting_indices_start, uint32_t n_starting_indices,
    const std::vector<IndexType>* ending_indices, uint32_t ending_indices_start,
    uint32_t n_ending_indices)
    : starting_indices_(starting_indices),
      starting_indices_start_(starting_indices_start),
      n_starting_indices_(n_starting_indices),
      ending_indices_(ending_indices),
      ending_indices_start_(ending_indices_start),
      n_ending_indices_(n_ending_indices) {}

uint32_t DirectedPartialOutline::Size() const {
  return n_starting_indices_ + n_ending_indices_;
}

uint32_t DirectedPartialOutline::StartingSideSize() const {
  return n_starting_indices_;
}

IndexType DirectedPartialOutline::operator[](uint32_t i) const {
  ABSL_DCHECK_LT(i, Size());
  if (i < n_starting_indices_) {
    return (*starting_indices_)[starting_indices_start_ + n_starting_indices_ -
                                i - 1];
  }
  return (*ending_indices_)[ending_indices_start_ + i - n_starting_indices_];
}

uint32_t LastOutlineIndexOffset(const Side& side) {
  return side.intersection.has_value() &&
                 side.intersection->retriangulation_started
             ? side.intersection->starting_offset
             : side.indices.size() - 1;
}

DirectedPartialOutline ConstructPartialOutline(const Side& starting_side,
                                               const Side& ending_side) {
  const Side::MeshPartitionStart& partition_start =
      starting_side.partition_start;
  uint32_t starting_side_first = partition_start.adjacent_first_index_offset;
  uint32_t starting_side_last = LastOutlineIndexOffset(starting_side);
  uint32_t n_starting_indices = starting_side_last - starting_side_first + 1;

  uint32_t ending_side_first = partition_start.opposite_first_index_offset;
  uint32_t ending_side_last = LastOutlineIndexOffset(ending_side);
  uint32_t n_ending_indices =
      starting_side.partition_start.outline_connects_sides
          ? ending_side_last - ending_side_first + 1
          : 0;
  return DirectedPartialOutline(&starting_side.indices, starting_side_first,
                                n_starting_indices, &ending_side.indices,
                                ending_side_first, n_ending_indices);
}

OutlineIntersectionResult FindOutlineIntersection(
    const DirectedPartialOutline& outline, const Segment& segment,
    const MutableMeshView& mesh, float search_budget,
    std::optional<Triangle> containing_triangle) {
  for (uint32_t i = 1; i < outline.Size() && search_budget > 0; ++i) {
    Segment outline_segment = {.start = mesh.GetPosition(outline[i - 1]),
                               .end = mesh.GetPosition(outline[i])};
    if (outline_segment.start == outline_segment.end) continue;

    std::optional<LegacySegmentIntersection> result =
        LegacyIntersection(outline_segment, segment);
    if (result.has_value()) {
      OutlineIntersectionResult::SegmentIntersection intersection = {
          .position = outline_segment.Lerp(result->segment1_interval[0]),
          .starting_index = i - 1,
          .ending_index = i,
          .outline_interpolation_value = result->segment1_interval[0],
          .segment_interpolation_value = result->segment2_interval[0]};
      search_budget -=
          DistanceBetween(outline_segment.start,
                          outline_segment.Lerp(result->segment1_interval[0]));
      return {.segment_intersection = intersection,
              .remaining_search_budget = std::max(0.f, search_budget)};
    }
    if (containing_triangle.has_value() &&
        !containing_triangle->Contains(outline_segment.end)) {
      search_budget = 0;
      break;
    }
    search_budget -= outline_segment.Length();
  }

  // If we made it this far, we check the degenerate segment made up of just the
  // last index's position.
  if (outline.Size() > 0 && search_budget > 0) {
    uint32_t last = outline[outline.Size() - 1];
    Point last_position = mesh.GetPosition(last);
    Segment outline_segment = {.start = last_position, .end = last_position};
    std::optional<LegacySegmentIntersection> result =
        LegacyIntersection(outline_segment, segment);
    if (result.has_value()) {
      OutlineIntersectionResult::SegmentIntersection intersection = {
          .starting_index = outline.Size() - 1,
          .ending_index = outline.Size() - 1,
          .outline_interpolation_value = result->segment1_interval[0],
          .segment_interpolation_value = result->segment2_interval[0]};
      return {.segment_intersection = intersection,
              .remaining_search_budget = std::max(0.f, search_budget)};
    }
  }

  return {.segment_intersection = std::nullopt,
          .remaining_search_budget = std::max(0.f, search_budget)};
}

}  // namespace brush_tip_extruder_internal
}  // namespace ink
