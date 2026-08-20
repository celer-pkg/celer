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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_HELPERS_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_HELPERS_H_

#include <cstdint>
#include <optional>

#include "absl/types/span.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"

namespace ink::brush_tip_extruder_internal {

// Return type for `FindFirstExteriorVertices()` below.
struct OptionalSideIndexPair {
  std::optional<IndexType> left;
  std::optional<IndexType> right;
};

// Iterates over the `mesh` triangle indices beginning with `starting_triangle`
// to find the first encountered index for each side that has an exterior label.
//
// `vertex_side_ids` is expected to map each index to its `SideId`.
OptionalSideIndexPair FindFirstExteriorVertices(
    const MutableMeshView& mesh, absl::Span<const SideId> vertex_side_ids,
    uint32_t starting_triangle);

// Returns the offset into `side_indices` for the start of a "coincident" vertex
// range that includes the vertex with index `side_indices[included_offset]`.
//
// Any coincident vertices are at adjacent offsets in `side_indices` and have
// the same positions. However, the vertices must not be on the boundary of two
// disconnected partitions according to their `StrokeVertex::ForwardCategory`.
// The range may have size equal to one, in which case the function will return
// `included_offset`. In general, the returned value with be less than or equal
// to the passed-in offset.
//
// CHECK-fails if `included_offset` is not less than `side_indices.size()`.
uint32_t StartingOffsetForCoincidentConnectedVertices(
    const MutableMeshView& mesh, absl::Span<const IndexType> side_indices,
    uint32_t included_offset);

}  // namespace ink::brush_tip_extruder_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_HELPERS_H_
