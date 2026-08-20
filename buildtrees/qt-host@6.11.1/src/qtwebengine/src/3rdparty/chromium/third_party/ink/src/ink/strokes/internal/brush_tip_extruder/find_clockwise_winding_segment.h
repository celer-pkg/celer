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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_FIND_CLOCKWISE_WINDING_SEGMENT_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_FIND_CLOCKWISE_WINDING_SEGMENT_H_

#include <optional>

#include "absl/types/span.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"

namespace ink {
namespace brush_tip_extruder_internal {

// Finds the last segment along the outside of a triangle fan that is part of a
// clockwise-winding triangle, if any exist.
//
// The triangle fan constructed from the positions of `outer_indices` and a
// `central_position` assumed to be in the interior of the stroke.
// `outer_indices` is expected to consist of indices into `vertices` and
// represents a portion of either the "left" or "right" outline of the stroke,
// as given by `outer_side_identifier`. The indices in `outer_indices` are
// assumed to be ordered from the back of the stroke to the front.
std::optional<Segment> FindLastClockwiseWindingTriangleFanSegment(
    const MutableMeshView& mesh, absl::Span<const IndexType> outer_indices,
    SideId outer_side_identifier, Point central_position);

// Finds the last segment along the outside of a triangle fan that is part of a
// clockwise-winding triangle, if any exist.
//
// The triangle fan is constructed of positions along `outer_side` from
// `outer_index_offset_range.first` to `outer_index_offset_range.last` and
// `central_position`.

// This is similar to the function above, but takes into account that the outer
// vertices of the triangle fan can come in multiple contiguous sections of
// `outer_side.indices` separated by the offset ranges in
// `outer_side.intersection_discontinuities`.
std::optional<Segment> FindLastClockwiseWindingMultiTriangleFanSegment(
    const MutableMeshView& mesh, const Side& outer_side,
    Side::IndexOffsetRange outer_index_offset_range, Point central_position);

}  // namespace brush_tip_extruder_internal
}  // namespace ink

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_FIND_CLOCKWISE_WINDING_SEGMENT_H_
