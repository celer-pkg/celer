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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DIRECTED_PARTIAL_OUTLINE_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DIRECTED_PARTIAL_OUTLINE_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"

namespace ink {
namespace brush_tip_extruder_internal {

// Describes a "U" shaped partial outline created by connecting two ranges of
// vertices as shown below.
//
//   (*starting_indices)[start + n]    (*ending_indices)[start + n]
//          | xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx |
//          | xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx |
//   (*starting_indices)[start] ------ (*ending_indices)[start]
//
// Iteration will proceed backwards down the `starting_indices` and then
// forwards up the other side.
//
// For example, if:
//   starting_indices = {1, 2, 3}
//   ending_indices = {4, 5, 6}
//
// Then, looping over the outline gives:
//   {3, 2, 1, 4, 5, 6}
//
// Note: we do not use an `absl::Span` so that the vectors can be appended to
// without invalidating the outline.
class DirectedPartialOutline {
 public:
  DirectedPartialOutline() = default;
  DirectedPartialOutline(const std::vector<IndexType>* starting_indices,
                         uint32_t starting_indices_start,
                         uint32_t n_starting_indices,
                         const std::vector<IndexType>* ending_indices,
                         uint32_t ending_indices_start,
                         uint32_t n_ending_indices);

  uint32_t Size() const;
  uint32_t StartingSideSize() const;
  IndexType operator[](uint32_t i) const;

 private:
  const std::vector<IndexType>* starting_indices_ = nullptr;
  uint32_t starting_indices_start_ = 0;
  uint32_t n_starting_indices_ = 0;
  const std::vector<IndexType>* ending_indices_ = nullptr;
  uint32_t ending_indices_start_ = 0;
  uint32_t n_ending_indices_ = 0;
};

// Returns the offset into `Side::indices` of the last index that should be
// considered part of the stroke's outline. This is usually the last index on
// the side, but only if the side is not modifying triangulation.
//
// TODO: b/283981060 - This function got moved out of being source-only in the
// split, and can now be tested.
uint32_t LastOutlineIndexOffset(const Side& side);

// Creates the appropriate partial outline.
//
// Typically, the outline will start at the most recent partition given by
// `Side::indices_partition_offset` and extend forward to include either the
// rest of the indices or until the start of an ongoing intersection.
DirectedPartialOutline ConstructPartialOutline(const Side& starting_side,
                                               const Side& ending_side);

// Return type for `FindOutlineIntersection` below.
struct OutlineIntersectionResult {
  // Data representing successfully found intersection between a
  // `DirectedPartialOutline` and a `Segment`. The true intersection may
  // not be a single point due to the intersection between parallel line
  // segments, but we only care about one position of the intersection.
  struct SegmentIntersection {
    Point position;
    // Index into the `DirectedPartialOutline` for the start of the intersecting
    // segment.
    uint32_t starting_index;
    // Index into the `DirectedPartialOutline` for the end of the intersecting
    // segment. This will usually be `starting_index + 1`, but may be the same
    // as `starting_index` if the outline consisted of only a single index or
    // only degenerate positions.
    uint32_t ending_index;
    // Value in the range [0, 1] representing where the intersection occurred
    // along the outline segment given by `starting_index`.
    //
    // A value of 0 means the intersection took place exactly at the vertex with
    // index `outline[starting_index]`, and a value of 1 means the
    // intersection took place exactly at the vertex with index
    // `outline[ending_index]`.
    float outline_interpolation_value;
    // Value in the range [0, 1] representing where the intersection occurred
    // along `segment`.
    //
    // A value of 0 means the intersection took place exactly at `segment.from`,
    // and a value of 1 means the intersection took place exactly at
    // `segment.to`.
    float segment_interpolation_value;
  };
  std::optional<SegmentIntersection> segment_intersection;
  // Value representing how much search budget remains after the search.
  float remaining_search_budget;
};

// Searches for an intersection between a partial outline and a segment.
//
// The search tests for intersection using segments made from adjacent pairs of
// indices in `outline`. The search keeps track of the distance traveled along
// the outline and exits when the distance traveled by checked segments
// exceeds `search_budget`.
//
// The search tries to find the first nondegenerate intersecting segment
// created by the outline indices. If the outline consists of only one index or
// all of the vertices in the outline are at the same position, the result will
// be the last vertex to lie on `segment`.
//
// If `segment` is partially coincident with one of the outline segments, the
// returned value of `SegmentIntersection::interpolation_value` points to the
// earliest point of intersection along the outline.
//
// If `containing_triangle` is not nullopt, a valid intersection means all
// outline positions from the start to the intersection must be contained in the
// triangle.
OutlineIntersectionResult FindOutlineIntersection(
    const DirectedPartialOutline& outline, const Segment& segment,
    const MutableMeshView& mesh, float search_budget,
    std::optional<Triangle> containing_triangle = std::nullopt);

}  // namespace brush_tip_extruder_internal
}  // namespace ink

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DIRECTED_PARTIAL_OUTLINE_H_
