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

#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator_helpers.h"

#include <cstdint>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::StrokeVertex;

bool VertexIsExterior(const MutableMeshView& mesh, IndexType index) {
  return mesh.GetSideLabel(index) != StrokeVertex::kInteriorLabel ||
         mesh.GetForwardLabel(index) != StrokeVertex::kInteriorLabel;
}

}  // namespace

OptionalSideIndexPair FindFirstExteriorVertices(
    const MutableMeshView& mesh, absl::Span<const SideId> vertex_side_ids,
    uint32_t starting_triangle) {
  OptionalSideIndexPair index_pair;

  for (uint32_t i = starting_triangle; i < mesh.TriangleCount(); ++i) {
    for (IndexType index : mesh.GetTriangleIndices(i)) {
      if (vertex_side_ids[index] == SideId::kLeft) {
        if (!index_pair.left.has_value() && VertexIsExterior(mesh, index)) {
          index_pair.left = index;
        }
      } else if (!index_pair.right.has_value() &&
                 VertexIsExterior(mesh, index)) {
        index_pair.right = index;
      }
    }
    if (index_pair.left.has_value() && index_pair.right.has_value()) break;
  }

  return index_pair;
}

uint32_t StartingOffsetForCoincidentConnectedVertices(
    const MutableMeshView& mesh, absl::Span<const IndexType> side_indices,
    uint32_t included_offset) {
  ABSL_CHECK_LT(included_offset, side_indices.size());

  IndexType index = side_indices[included_offset];
  Point position = mesh.GetPosition(index);
  StrokeVertex::ForwardCategory forward_category =
      mesh.GetForwardLabel(index).DecodeForwardCategory();
  for (uint32_t i = included_offset; i > 0; --i) {
    index = side_indices[i - 1];
    StrokeVertex::ForwardCategory current_forward_category =
        mesh.GetForwardLabel(index).DecodeForwardCategory();
    if (mesh.GetPosition(index) != position ||
        (current_forward_category != forward_category &&
         forward_category == StrokeVertex::ForwardCategory::kExteriorFront)) {
      // This is a boundary between disconnected partitions that happen to have
      // coincident vertices.
      return i;
    }
    forward_category = current_forward_category;
  }
  return 0;
}

}  // namespace ink::brush_tip_extruder_internal
