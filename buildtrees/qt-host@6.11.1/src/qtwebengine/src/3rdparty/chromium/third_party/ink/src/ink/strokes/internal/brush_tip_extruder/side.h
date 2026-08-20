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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIDE_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIDE_H_

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"

namespace ink {
namespace brush_tip_extruder_internal {

using IndexType = uint32_t;
static_assert(sizeof(IndexType) ==
              sizeof(decltype(std::declval<MutableMeshView>()
                                  .GetTriangleIndices(0))::value_type));

// Identifies to which side of the geometry something belongs.
//
// "Left" and "right" are defined at each section of the stroke when viewed from
// the positive z-axis in the direction of travel.
enum class SideId { kLeft, kRight };

// Stores the current state of the left or right side of the stroke as stored in
// the extruder internal `Geometry` type.
struct Side {
  struct SelfIntersection {
    // The position after which the intersection began.
    Point starting_position;
    // The last vertex we tried to append in this self-intersection handling.
    ExtrudedVertex last_proposed_vertex;
    // Index of the mesh triangle that contained `last_proposed_vertex`.
    uint32_t last_proposed_vertex_triangle;
    // Offset into `Side::indices` at which to find the index corresponding to
    // the start of this intersection.
    uint32_t starting_offset;
    // True if the intersection has begun modifying previously appended mesh
    // triangles.
    bool retriangulation_started;
    // The newest triangle that should be put into the undo stack. We do not
    // want to put every triangle into the stack, since retriangulation can be
    // delayed.
    uint32_t undo_stack_starting_triangle;
    // Once retriangulation has started, this is the index of the oldest mesh
    // triangle that has been modified.
    uint32_t oldest_retriangulation_triangle;
    // Triangle indices that were written over by retriangulation. Depending on
    // how the intersecting position travels inside the line, some or all of the
    // triangles will be restored. Since retriangulation travels backwards,
    // newest triangles are at the bottom of the stack and oldest are at the
    // top.
    std::vector<std::array<IndexType, 3>> undo_triangulation_stack;
    // The maximum remaining distance that vertices in the outline may be moved
    // while handling this intersection.
    float outline_reposition_budget;
    // The reposition budget when intersection begins. Note that
    // `outline_reposition_budget` will initially become larger than this value
    // when retriangulation starts.
    float initial_outline_reposition_budget;
    // The maximum distance that proposed intersection vertices are allowed to
    // travel from `starting_position`.
    float travel_limit_from_starting_position;
  };

  struct MeshPartitionStart {
    // Offset into this side's `indices` for the first index that should be part
    // of the `DirectedPartialOutline`.
    uint32_t adjacent_first_index_offset = 0;
    // Offset into the opposite side's `indices` for the first index that should
    // be part of the `DirectedPartialOutline`.
    uint32_t opposite_first_index_offset = 0;
    // The first triangle in the mesh that is considered part of this partition.
    uint32_t first_triangle = 0;
    // If set, this is the position of the vertex at
    // `opposite_first_index_offset` at the time when this partition is created.
    std::optional<Point> opposite_side_initial_position;
    // If set, this is an index for a helper vertex that may be used when
    // handling non-ccw proposed triangles that extend to the beginning of this
    // partition.
    std::optional<IndexType> non_ccw_connection_index;
    // Determines if the `DirectedPartialOutline` connects the first adjacent
    // and opposite vertices.
    bool outline_connects_sides = true;
    // Determines if this partition's starting position lies on the exterior of
    // stroke geometry with respect to the "forward" direction.
    bool is_forward_exterior = true;
  };

  // A range of this side's indices given by offsets into `Side::indices` below.
  struct IndexOffsetRange {
    uint32_t first;
    uint32_t last;
  };

  struct IndexOffsetRanges {
    Side::IndexOffsetRange left;
    Side::IndexOffsetRange right;
  };

  SideId self_id;
  // Given three indices of a triangle that return `true` when passed to
  // `Geometry::TriangleIndicesAreLeftRightConforming`, this is the first vertex
  // of the triangle that belongs to this side.
  //
  // Equal to `0` for the left side and `1` for the right side.
  int first_triangle_vertex;
  // Indices into e.g. `MutableMeshView::GetVertex()` for getting the vertices
  // that make up a side of the line. These are ordered from the start of the
  // line to the end.
  std::vector<IndexType> indices;
  // Ranges of offsets into `indices` that represent discontinuities from giving
  // up intersection handling. Indices within each range will permanently be
  // part of triangles whose vertices all belong to this side. The first and
  // last index in each range are the only ones that will be part of triangles
  // connecting to the opposite side of the geometry.
  std::vector<IndexOffsetRange> intersection_discontinuities;
  // The start of the current partition used for searching mesh triangles and
  // creating `DirectedPartialOutline`s for intersection handling.
  MeshPartitionStart partition_start;
  // Offset into this side's `indices` for the first index whose vertex may be
  // removed by the simplification algorithm.
  uint32_t first_simplifiable_index_offset = 0;
  // Vertices that need to be processed. Will contain one or two vertices that
  // are already appended to the mesh, because they are needed here for the
  // simplification algorithm.
  std::vector<ExtrudedVertex> vertex_buffer;
  // Index into `vertex_buffer` for the next buffered vertex that should be
  // appended.
  uint32_t next_buffered_vertex_offset = 0;
  std::optional<SelfIntersection> intersection;
  // Sequence of consecutive vertices simplified away that immediately precede
  // the most recent vertex in the mesh from this side.
  //
  // This is used to double-check that previously dropped vertices won't become
  // relevant again if the next vertex is also dropped. (For example, you can
  // have a scenario where vertex 1 is close to segment 0-2, 2 is close to 0-3,
  // etc. but i in [1,n-1] is far enough from 0-n.)
  std::vector<Point> last_simplified_vertex_positions;
};

}  // namespace brush_tip_extruder_internal
}  // namespace ink

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIDE_H_
