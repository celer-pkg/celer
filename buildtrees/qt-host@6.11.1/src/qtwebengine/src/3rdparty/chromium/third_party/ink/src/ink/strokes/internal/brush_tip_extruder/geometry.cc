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

#include "ink/strokes/internal/brush_tip_extruder/geometry.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/base/nullability.h"
#include "absl/cleanup/cleanup.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/legacy_segment_intersection.h"
#include "ink/geometry/internal/legacy_triangle_contains.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator_helpers.h"
#include "ink/strokes/internal/brush_tip_extruder/directed_partial_outline.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/brush_tip_extruder/find_clockwise_winding_segment.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_extruder/simplify.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::ink::geometry_internal::LegacyIntersects;
using ::ink::geometry_internal::LegacyTriangleContains;
using ::ink::strokes_internal::BrushTipState;
using ::ink::strokes_internal::LegacyVertex;
using ::ink::strokes_internal::StrokeVertex;

// This is the special value assigned to `Vertex::texture_coords` for vertices
// that represent the pivot of a triangle-fan using winding textures.
const Point kWindingTextureCoordinateSentinelValue{0, -1};

float DistanceBetween(Point a, Point b) { return (a - b).Magnitude(); }

}  // namespace

Geometry::Geometry() {
  left_side_.self_id = SideId::kLeft;
  left_side_.first_triangle_vertex = 0;
  right_side_.self_id = SideId::kRight;
  right_side_.first_triangle_vertex = 1;
}

Geometry::Geometry(const MutableMeshView& mesh) : Geometry() {
  mesh_ = mesh;
  if (mesh_.HasMeshData()) mesh_.Resize(0, 0);
}

void Geometry::SetSavePoint() {
  if (!mesh_.HasMeshData()) return;

  auto set_side_state = [](const Side& side,
                           GeometrySavePointState::SideState& side_state) {
    side_state.n_indices = static_cast<uint32_t>(side.indices.size());
    side_state.n_intersection_discontinuities =
        static_cast<uint32_t>(side.intersection_discontinuities.size());
    side_state.saved_indices.clear();
    side_state.saved_intersection_discontinuities.clear();
    side_state.partition_start = side.partition_start;
    side_state.first_simplifiable_index_offset =
        side.first_simplifiable_index_offset;
    side_state.vertex_buffer = side.vertex_buffer;
    side_state.next_buffered_vertex_offset = side.next_buffered_vertex_offset;
    side_state.intersection = side.intersection;
    side_state.last_simplified_vertex_positions =
        side.last_simplified_vertex_positions;
  };

  save_point_state_.is_active = true;
  save_point_state_.contains_all_geometry_since_last_extrusion_break = false;
  save_point_state_.n_mesh_vertices = mesh_.VertexCount();
  save_point_state_.n_mesh_triangles = mesh_.TriangleCount();
  save_point_state_.saved_vertex_side_ids.clear();
  save_point_state_.saved_side_offsets.clear();
  save_point_state_.saved_vertices.clear();
  save_point_state_.saved_triangle_indices.clear();
  save_point_state_.saved_opposite_side_offsets.clear();
  set_side_state(left_side_, save_point_state_.left_side_state);
  set_side_state(right_side_, save_point_state_.right_side_state);
  save_point_state_.saved_last_extrusion_break = last_extrusion_break_;

  // TODO(b/201002500): `SimplifyBufferedVertices()` cannot currently take color
  // or texture coordinates into account when removing vertices. This can be a
  // problem when a solid colored input stroke has a partially transparent
  // prediction after the save point. So for now, we must make sure to keep the
  // last vertex prior to the save point:
  if (left_side_.vertex_buffer.size() > 1) {
    left_side_.vertex_buffer[0] = left_side_.vertex_buffer.back();
    left_side_.vertex_buffer.resize(1);
    left_side_.next_buffered_vertex_offset = 1;
  }
  if (right_side_.vertex_buffer.size() > 1) {
    right_side_.vertex_buffer[0] = right_side_.vertex_buffer.back();
    right_side_.vertex_buffer.resize(1);
    right_side_.next_buffered_vertex_offset = 1;
  }
}

namespace {

// Returns the envelope of positions in all `mesh` triangles from
// `first_triangle` until `mesh.TriangleCount()`.
//
// This is less efficient than walking vertices, because each vertex will tend
// to be a part of multiple triangles. However, walking triangles is a simpler
// way to ensure accurately picking up all desired positions when calculating
// the envelope of a strict subset of the mesh.
Envelope EnvelopeOfTriangles(const MutableMeshView& mesh,
                             uint32_t first_triangle) {
  Envelope envelope;
  for (uint32_t i = first_triangle; i < mesh.TriangleCount(); ++i) {
    for (int j = 0; j < 3; ++j) {
      envelope.Add(mesh.GetPosition(mesh.GetVertexIndex(i, j)));
    }
  }
  return envelope;
}

// Returns the envelope of all `mesh` positions.
//
// This can be used instead of walking triangle indices when all mesh triangles
// are being considered for envelope calculation, because every vertex added to
// the mesh will become part of a triangle.
Envelope EnvelopeOfAllPositions(const MutableMeshView& mesh) {
  Envelope envelope;
  for (uint32_t i = 0; i < mesh.VertexCount(); ++i) {
    envelope.Add(mesh.GetPosition(i));
  }
  return envelope;
}

}  // namespace

Envelope Geometry::CalculateVisuallyUpdatedRegion() const {
  if (!mesh_.HasMeshData()) return Envelope();

  Envelope visually_mutated_region = envelope_of_removed_geometry_;
  if (mesh_.TriangleCount() == 0) return visually_mutated_region;
  uint32_t first_visually_mutated_triangle = FirstVisuallyMutatedTriangle();
  if (first_visually_mutated_triangle == 0) {
    // This is the simpler case where every triangle is either new or has been
    // modified, then we can walk over all of the vertices instead of the
    // triangle indices.
    visually_mutated_region.Add(EnvelopeOfAllPositions(mesh_));
  } else {
    // Otherwise, we calculate the bounds by walking the visually mutated
    // triangle indices. This is expected to walk over some vertices multiple
    // times, but makes sure we do not miss or pick up extra vertices that may
    // noticeably change the envelope for incremental updates. The algorithm
    // stays a little simpler, and the amount of duplicated work is expected to
    // be small relative to the overall cost of extrusion.
    visually_mutated_region.Add(
        EnvelopeOfTriangles(mesh_, first_visually_mutated_triangle));
  }
  return visually_mutated_region;
}

void Geometry::RevertToSavePoint() {
  if (!save_point_state_.is_active || !mesh_.HasMeshData()) return;

  // Before we mutate the mesh, Record the envelope of triangles past the start
  // of the save point, all of which are about to be erased or changed.
  envelope_of_removed_geometry_.Add(
      EnvelopeOfTriangles(mesh_, save_point_state_.n_mesh_triangles));

  uint32_t old_vertex_count = mesh_.VertexCount();
  mesh_.Resize(save_point_state_.n_mesh_vertices,
               save_point_state_.n_mesh_triangles);
  vertex_side_ids_.resize(save_point_state_.n_mesh_vertices);
  side_offsets_.resize(save_point_state_.n_mesh_vertices);
  opposite_side_offsets_.resize(save_point_state_.n_mesh_vertices);

  for (const auto& index_vert_pair : save_point_state_.saved_vertices) {
    // We don't update `envelope_of_removed_geometry_` for vertices that were
    // just added if the `Resize` call above grew the mesh; their positions
    // aren't meaningful, they just default to (0, 0).
    SetVertex(index_vert_pair.first, index_vert_pair.second,
              /* update_save_state = */ false,
              /* update_envelope_of_removed_geometry = */
              index_vert_pair.first < old_vertex_count);
  }
  // Revert mutated triangulation:
  for (const auto& indices_pair : save_point_state_.saved_triangle_indices) {
    SetTriangleIndices(indices_pair.first, indices_pair.second,
                       /* update_save_state = */ false);
  }

  for (const auto& index_offset_pair :
       save_point_state_.saved_opposite_side_offsets) {
    UpdateOppositeSideOffset(index_offset_pair.first, index_offset_pair.second,
                             /* update_save_state = */ false);
  }

  absl::c_copy(
      save_point_state_.saved_vertex_side_ids,
      vertex_side_ids_.end() - save_point_state_.saved_vertex_side_ids.size());
  absl::c_copy(
      save_point_state_.saved_side_offsets,
      side_offsets_.end() - save_point_state_.saved_side_offsets.size());

  auto revert_side = [](Side& side, uint32_t& first_mutated_index_offset,
                        GeometrySavePointState::SideState& side_state) {
    side.indices.resize(side_state.n_indices);
    absl::c_copy(side_state.saved_indices,
                 side.indices.end() - side_state.saved_indices.size());
    first_mutated_index_offset = std::min<uint32_t>(
        first_mutated_index_offset,
        side_state.n_indices - side_state.saved_indices.size());

    side.intersection_discontinuities.resize(
        side_state.n_intersection_discontinuities);
    absl::c_copy(side_state.saved_intersection_discontinuities,
                 side.intersection_discontinuities.end() -
                     side_state.saved_intersection_discontinuities.size());
    side.partition_start = side_state.partition_start;
    side.first_simplifiable_index_offset =
        side_state.first_simplifiable_index_offset;
    std::swap(side.vertex_buffer, side_state.vertex_buffer);
    side.next_buffered_vertex_offset = side_state.next_buffered_vertex_offset;
    std::swap(side.intersection, side_state.intersection);
    std::swap(side.last_simplified_vertex_positions,
              side_state.last_simplified_vertex_positions);
  };
  revert_side(left_side_, first_mutated_left_index_offset_in_current_partition_,
              save_point_state_.left_side_state);
  revert_side(right_side_,
              first_mutated_right_index_offset_in_current_partition_,
              save_point_state_.right_side_state);
  last_extrusion_break_ = save_point_state_.saved_last_extrusion_break;

  save_point_state_.is_active = false;
}

void Geometry::SetIntersectionHandling(
    IntersectionHandling intersection_handling) {
  handle_self_intersections_ =
      intersection_handling == IntersectionHandling::kEnabled;
  if (!handle_self_intersections_) {
    left_side_.intersection.reset();
    right_side_.intersection.reset();
  }
}

void Geometry::ResetMutationTracking() {
  mesh_.ResetMutationTracking();
  envelope_of_removed_geometry_.Reset();
  first_mutated_left_index_.reset();
  first_mutated_right_index_.reset();
  first_mutated_left_index_offset_in_current_partition_ =
      left_side_.indices.size();
  first_mutated_right_index_offset_in_current_partition_ =
      right_side_.indices.size();
}

namespace {

float InitialOutlineRepositionBudget(float average_tip_dimension) {
  // The factor of 1 gives a good balance of handling a good deal of
  // self-intersection while keeping the intersecting submesh convex, but it
  // may stretch 2D winding textures too much.
  return 1 * average_tip_dimension;
}

float IntersectionTravelLimit(float average_tip_dimension) {
  // The factor of 1.25 was chosen based on manual testing. It should generally
  // be equal to or greater than the factor for the outline reposition budget.
  return 1.25 * average_tip_dimension;
}

float RetriangulationTravelThreshold(float average_tip_dimension) {
  // The factor of 0.125 was chosen based on manual testing. If the value is too
  // close to 0, there is unnecessary retriangulation and a risk of introducing
  // artifacts in slow moving lines. If the value is too high, then there will
  // be a more noticeable jump in geometry when retriangulation starts.
  return 0.125 * average_tip_dimension;
}

float SimplificationTravelLimit(float average_tip_dimension) {
  // This ensures that geometry simplifications does not create triangles that
  // stretch longer than ~8x the stroke width because the stroke geometry is
  // relatively straight.
  return 8 * average_tip_dimension;
}

}  // namespace

void Geometry::ProcessNewVertices(float simplification_threshold,
                                  const BrushTipState& last_tip_state) {
  if (left_side_.vertex_buffer.empty() || right_side_.vertex_buffer.empty() ||
      !mesh_.HasMeshData()) {
    // Need vertices on both sides to process. Note that the vertex buffers are
    // only empty immediately after the start of the stroke or the start of a
    // new disconnected partition, as one or two vertices are always left in the
    // buffer after processing to allow for simplification.
    return;
  }

  float average_tip_dimension =
      0.5 * (last_tip_state.width + last_tip_state.height);
  float outline_reposition_budget =
      InitialOutlineRepositionBudget(average_tip_dimension);
  SimplifyBufferedVertices(outline_reposition_budget, simplification_threshold,
                           SimplificationTravelLimit(average_tip_dimension));

  uint32_t left_index_count_before = left_side_.indices.size();
  uint32_t right_index_count_before = right_side_.indices.size();

  TriangulateBufferedVertices(
      outline_reposition_budget, IntersectionTravelLimit(average_tip_dimension),
      RetriangulationTravelThreshold(average_tip_dimension));

  auto conditionally_clear_simplified_positions =
      [&](uint32_t side_index_count_before_triangulation, Side& side) {
        // `TriangulateBufferedVertices` will only append new indices, so we can
        // check if any new vertices were appended by comparing the size of
        // `side.indices` to the value before the call. If any new vertices were
        // appended during triangulation, we should clear any positions saved in
        // `SimplifyBufferedVertices()`. If it is non-empty, these would be in
        // addition to the replacement vertex value set by simplification, and
        // the saved positions must immediately precede `side`'s last vertex.
        if (side_index_count_before_triangulation != side.indices.size()) {
          side.last_simplified_vertex_positions.clear();
        }
      };
  conditionally_clear_simplified_positions(left_index_count_before, left_side_);
  conditionally_clear_simplified_positions(right_index_count_before,
                                           right_side_);
}

namespace {

void SetExtrusionBreakPartitionOnSide(Side& side, uint32_t first_triangle_index,
                                      uint32_t opposite_side_index_count) {
  side.partition_start = {
      .adjacent_first_index_offset = static_cast<uint32_t>(side.indices.size()),
      .opposite_first_index_offset = opposite_side_index_count,
      .first_triangle = first_triangle_index,
      .outline_connects_sides = true,
      .is_forward_exterior = true};
  side.first_simplifiable_index_offset =
      side.partition_start.adjacent_first_index_offset;
  side.vertex_buffer.clear();
  side.next_buffered_vertex_offset = 0;
  side.intersection.reset();
  side.last_simplified_vertex_positions.clear();
}

bool IsSidePerformingRetriangulation(const Side& side) {
  return side.intersection.has_value() &&
         side.intersection->retriangulation_started;
}

}  // namespace

void Geometry::AddExtrusionBreak() {
  auto label_last_vertex_as_exterior = [this](const Side& side) {
    if (side.partition_start.adjacent_first_index_offset >=
        side.indices.size()) {
      // No new indices have been added since the start of the current
      // partition.
      return;
    }

    ExtrudedVertex vertex = LastVertex(side);
    vertex.new_non_position_attributes.forward_label =
        StrokeVertex::kExteriorBackLabel;
    SetVertex(side.indices.back(), vertex);
  };

  if (!IsSidePerformingRetriangulation(left_side_) &&
      !IsSidePerformingRetriangulation(right_side_)) {
    label_last_vertex_as_exterior(left_side_);
    label_last_vertex_as_exterior(right_side_);
  }

  SetExtrusionBreakPartitionOnSide(left_side_, mesh_.TriangleCount(),
                                   right_side_.indices.size());
  SetExtrusionBreakPartitionOnSide(right_side_, mesh_.TriangleCount(),
                                   left_side_.indices.size());

  auto side_extrusion_break_info =
      [](const Side& side) -> GeometryLastExtrusionBreakMetadata::SideInfo {
    return {
        .index_count = static_cast<uint32_t>(side.indices.size()),
        .intersection_discontinuity_count =
            static_cast<uint32_t>(side.intersection_discontinuities.size()),
    };
  };
  last_extrusion_break_ = {
      .break_count = last_extrusion_break_.break_count + 1,
      .vertex_count = mesh_.VertexCount(),
      .triangle_count = mesh_.TriangleCount(),
      .left_side_info = side_extrusion_break_info(left_side_),
      .right_side_info = side_extrusion_break_info(right_side_),
  };
  first_mutated_left_index_offset_in_current_partition_ =
      left_side_.indices.size();
  first_mutated_right_index_offset_in_current_partition_ =
      right_side_.indices.size();
}

uint32_t Geometry::ExtrusionBreakCount() const {
  return last_extrusion_break_.break_count;
}

Geometry::IndexCounts Geometry::IndexCountsAtLastExtrusionBreak() const {
  return {
      .left = last_extrusion_break_.left_side_info.index_count,
      .right = last_extrusion_break_.right_side_info.index_count,
  };
}

namespace {

void CaptureGeometrySinceLastExtrusionBreak(
    const MutableMeshView& mesh, absl::Span<const SideId> vertex_side_ids,
    absl::Span<const uint32_t> side_offsets,
    absl::Span<const uint32_t> opposite_side_offsets, const Side& left_side,
    const Side& right_side,
    const GeometryLastExtrusionBreakMetadata& last_extrusion_break,
    GeometrySavePointState& save_point_state) {
  ABSL_CHECK_LE(last_extrusion_break.vertex_count,
                save_point_state.n_mesh_vertices);
  ABSL_CHECK_LE(last_extrusion_break.triangle_count,
                save_point_state.n_mesh_triangles);

  std::copy(vertex_side_ids.begin() + last_extrusion_break.vertex_count,
            vertex_side_ids.begin() + save_point_state.n_mesh_vertices,
            std::back_inserter(save_point_state.saved_vertex_side_ids));
  std::copy(side_offsets.begin() + last_extrusion_break.vertex_count,
            side_offsets.begin() + save_point_state.n_mesh_vertices,
            std::back_inserter(save_point_state.saved_side_offsets));

  for (uint32_t t_idx = last_extrusion_break.triangle_count;
       t_idx < save_point_state.n_mesh_triangles; ++t_idx) {
    save_point_state.saved_triangle_indices.emplace(
        t_idx, mesh.GetTriangleIndices(t_idx));
  }
  for (uint32_t v_idx = last_extrusion_break.vertex_count;
       v_idx < save_point_state.n_mesh_vertices; ++v_idx) {
    save_point_state.saved_vertices.emplace(v_idx, mesh.GetVertex(v_idx));
    save_point_state.saved_opposite_side_offsets.emplace(
        v_idx, opposite_side_offsets[v_idx]);
  }

  using SideInfo = GeometryLastExtrusionBreakMetadata::SideInfo;
  auto capture_side = [](const Side& side, const SideInfo& side_extrusion_break,
                         GeometrySavePointState::SideState& side_save_state) {
    ABSL_CHECK_LE(side_extrusion_break.index_count, side_save_state.n_indices);

    std::copy(side.indices.begin() + side_extrusion_break.index_count,
              side.indices.begin() + side_save_state.n_indices,
              std::back_inserter(side_save_state.saved_indices));

    std::copy(
        side.intersection_discontinuities.begin() +
            side_extrusion_break.intersection_discontinuity_count,
        side.intersection_discontinuities.begin() +
            side_save_state.n_intersection_discontinuities,
        std::back_inserter(side_save_state.saved_intersection_discontinuities));
  };
  capture_side(left_side, last_extrusion_break.left_side_info,
               save_point_state.left_side_state);
  capture_side(right_side, last_extrusion_break.right_side_info,
               save_point_state.right_side_state);

  save_point_state.contains_all_geometry_since_last_extrusion_break = true;
}

}  // namespace

void Geometry::ClearSinceLastExtrusionBreak() {
  if (mesh_.VertexCount() == last_extrusion_break_.vertex_count) {
    // We have not added any vertices since the start of the current connected
    // extrusion; there is nothing to clear.
    return;
  }

  // If we have a save point that was set after the last extrusion break, we
  // need to capture that geometry before we clear it.
  //
  // However, we don't want to do this if we've already captured geometry since
  // the last extrusion break (i.e. if `ClearSinceLastExtrusionBreak` is called
  // multiple times after the save point was set). Doing so would overwrite the
  // state of the geometry when the save point was set with geometry that was
  // created after the save point.
  if (save_point_state_.is_active &&
      !save_point_state_.contains_all_geometry_since_last_extrusion_break &&
      save_point_state_.n_mesh_triangles >=
          last_extrusion_break_.triangle_count) {
    CaptureGeometrySinceLastExtrusionBreak(
        mesh_, vertex_side_ids_, side_offsets_, opposite_side_offsets_,
        left_side_, right_side_, last_extrusion_break_, save_point_state_);
  }

  // Record the envelope of the geometry we are about to delete.
  envelope_of_removed_geometry_.Add(
      EnvelopeOfTriangles(mesh_, last_extrusion_break_.triangle_count));

  using SideInfo = GeometryLastExtrusionBreakMetadata::SideInfo;
  auto delete_side_geometry = [](const SideInfo& side_info, Side& side) {
    side.indices.resize(side_info.index_count);
    side.intersection_discontinuities.resize(
        side_info.intersection_discontinuity_count);
  };

  delete_side_geometry(last_extrusion_break_.left_side_info, left_side_);
  delete_side_geometry(last_extrusion_break_.right_side_info, right_side_);

  mesh_.Resize(last_extrusion_break_.vertex_count,
               last_extrusion_break_.triangle_count);

  vertex_side_ids_.resize(last_extrusion_break_.vertex_count);
  side_offsets_.resize(last_extrusion_break_.vertex_count);
  opposite_side_offsets_.resize(last_extrusion_break_.vertex_count);

  first_mutated_left_index_offset_in_current_partition_ =
      std::min<uint32_t>(first_mutated_left_index_offset_in_current_partition_,
                         left_side_.indices.size());
  first_mutated_right_index_offset_in_current_partition_ =
      std::min<uint32_t>(first_mutated_right_index_offset_in_current_partition_,
                         right_side_.indices.size());

  // Reset the partition states.
  SetExtrusionBreakPartitionOnSide(left_side_, mesh_.TriangleCount(),
                                   right_side_.indices.size());
  SetExtrusionBreakPartitionOnSide(right_side_, mesh_.TriangleCount(),
                                   left_side_.indices.size());
}

namespace {

void UpdateFirstMutatedSideIndexValue(
    IndexType index, std::optional<IndexType>& first_mutated_index) {
  if (first_mutated_index.has_value()) {
    *first_mutated_index = std::min(*first_mutated_index, index);
  } else {
    first_mutated_index = index;
  }
}

}  // namespace

void Geometry::UpdateMeshDerivatives() {
  absl::Span<const IndexType> left_indices_to_update;
  absl::Span<const IndexType> right_indices_to_update;

  uint32_t first_visually_mutated_triangle = FirstVisuallyMutatedTriangle();
  if (first_visually_mutated_triangle == 0) {
    left_indices_to_update = left_side_.indices;
    right_indices_to_update = right_side_.indices;
  } else {
    OptionalSideIndexPair index_pair = FindFirstExteriorVertices(
        mesh_, vertex_side_ids_, first_visually_mutated_triangle);

    // Returns the subspan of `all_side_indices` that need to be updated.
    auto make_subspan = [this](absl::Span<const IndexType> all_side_indices,
                               IndexType first_exterior_side_index) {
      // Backtrack to the start of a coincident vertex range, if one is present,
      // because derivatives must get averaged across coincident vertices.
      return all_side_indices.subspan(
          StartingOffsetForCoincidentConnectedVertices(
              mesh_, all_side_indices,
              side_offsets_[first_exterior_side_index]));
    };
    if (index_pair.left.has_value()) {
      left_indices_to_update =
          make_subspan(left_side_.indices, *index_pair.left);
    }
    if (index_pair.right.has_value()) {
      right_indices_to_update =
          make_subspan(right_side_.indices, *index_pair.right);
    }
  }

  if (!left_indices_to_update.empty()) {
    UpdateFirstMutatedSideIndexValue(left_indices_to_update.front(),
                                     first_mutated_left_index_);
  }
  if (!right_indices_to_update.empty()) {
    UpdateFirstMutatedSideIndexValue(right_indices_to_update.front(),
                                     first_mutated_right_index_);
  }

  derivative_calculator_.UpdateMesh(left_indices_to_update,
                                    right_indices_to_update, mesh_);
}

void Geometry::DebugMakeMeshAfterSavePoint(MutableMeshView mesh) const {
  if (!mesh.HasMeshData()) return;
  mesh.Resize(0, 0);

  if (!mesh_.HasMeshData() || !save_point_state_.is_active ||
      save_point_state_.n_mesh_triangles == mesh_.TriangleCount() ||
      mesh_.TriangleCount() == 0) {
    return;
  }

  // Since this is for testing visualization only, we do not try to take
  // intersection handling into account and just grab a copy of all triangles
  // after `save_point_state_.n_mesh_triangles`.

  auto min_triangle_index = [this](uint32_t triangle) {
    return std::min({mesh_.GetVertexIndex(triangle, 0),
                     mesh_.GetVertexIndex(triangle, 1),
                     mesh_.GetVertexIndex(triangle, 2)});
  };

  IndexType min_index_after_save =
      min_triangle_index(mesh_.TriangleCount() - 1);
  for (uint32_t i = save_point_state_.n_mesh_triangles;
       i + 1 < mesh_.TriangleCount(); ++i) {
    min_index_after_save =
        std::min(min_index_after_save, min_triangle_index(i));
  }
  for (IndexType i = min_index_after_save; i < mesh_.VertexCount(); ++i) {
    mesh.AppendVertex(mesh_.GetVertex(i));
  }
  for (uint32_t i = save_point_state_.n_mesh_triangles;
       i < mesh_.TriangleCount(); ++i) {
    std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i);
    mesh.AppendTriangleIndices({indices[0] - min_index_after_save,
                                indices[1] - min_index_after_save,
                                indices[2] - min_index_after_save});
  }
}

uint32_t Geometry::NStableTriangles() const {
  if (!mesh_.HasMeshData()) return 0;

  if (handle_self_intersections_) {
    // In practice there will be some triangles that are too far away for
    // intersection handling to modify, but we don't bother doing the complex
    // calculation to figure that out.
    return 0;
  }

  // Up to the last two triangles of the mesh beyond the last save point can
  // have their vertices changed because of line simplification.
  uint32_t n_triangles = save_point_state_.is_active
                             ? save_point_state_.n_mesh_triangles
                             : mesh_.TriangleCount();
  return n_triangles - std::min<uint32_t>(n_triangles, 2);
}

namespace {

// Resets the `Side` member variables for starting a new stroke so that we can
// reuse the allocations in e.g. `side.indices`.
void ClearSide(Side& side) {
  side.indices.clear();
  side.intersection_discontinuities.clear();
  side.partition_start = {};
  side.first_simplifiable_index_offset = 0;
  side.vertex_buffer.clear();
  side.next_buffered_vertex_offset = 0;
  side.intersection.reset();
  side.last_simplified_vertex_positions.clear();
}

}  // namespace

void Geometry::Reset(const MutableMeshView& mesh) {
  mesh_ = mesh;
  if (mesh_.HasMeshData()) mesh_.Resize(0, 0);
  vertex_side_ids_.clear();
  side_offsets_.clear();
  opposite_side_offsets_.clear();
  // We do this instead of just typing e.g. `left_side_ = {};` to re-use the
  // capacity allocated in `Side::indices`.
  ClearSide(left_side_);
  ClearSide(right_side_);
  last_extrusion_break_ = {};
  save_point_state_.is_active = false;
  ResetMutationTracking();
}

namespace {

const ExtrudedVertex& NextBufferedVertex(const Side& side) {
  return side.vertex_buffer[side.next_buffered_vertex_offset];
}

}  // namespace

void Geometry::AppendVertexToSide(Side& side, const ExtrudedVertex& vertex) {
  if (!mesh_.HasMeshData()) return;

  side.vertex_buffer.push_back(vertex);
  if (side.indices.size() == side.partition_start.adjacent_first_index_offset) {
    ExtrudedVertex next_vertex = NextBufferedVertex(side);
    if (side.partition_start.is_forward_exterior) {
      next_vertex.new_non_position_attributes.forward_label =
          StrokeVertex::kExteriorFrontLabel;
    }

    AppendVertexToMesh(side, next_vertex);
    ++side.next_buffered_vertex_offset;
  }
}

void Geometry::AppendLeftVertex(Point position, float opacity_shift,
                                const std::array<float, 3>& hsl_shift,
                                Point surface_uv, float animation_offset) {
  AppendVertexToSide(left_side_,
                     {.position = position,
                      .new_non_position_attributes = {
                          .opacity_shift = opacity_shift,
                          .hsl_shift = hsl_shift,
                          .side_label = StrokeVertex::kExteriorLeftLabel,
                          .surface_uv = surface_uv,
                          .animation_offset = animation_offset,
                      }});
}

void Geometry::AppendRightVertex(Point position, float opacity_shift,
                                 const std::array<float, 3>& hsl_shift,
                                 Point surface_uv, float animation_offset) {
  AppendVertexToSide(right_side_,
                     {.position = position,
                      .new_non_position_attributes = {
                          .opacity_shift = opacity_shift,
                          .hsl_shift = hsl_shift,
                          .side_label = StrokeVertex::kExteriorRightLabel,
                          .surface_uv = surface_uv,
                          .animation_offset = animation_offset,
                      }});
}

void Geometry::AppendLeftVertex(const LegacyVertex& vertex) {
  AppendVertexToSide(left_side_, ExtrudedVertex::FromLegacy(vertex));
}

void Geometry::AppendRightVertex(const LegacyVertex& vertex) {
  AppendVertexToSide(right_side_, ExtrudedVertex::FromLegacy(vertex));
}

ExtrudedVertex Geometry::LastVertex(const Side& side) const {
  return mesh_.GetVertex(side.indices.back());
}

Point Geometry::LastPosition(const Side& side) const {
  return mesh_.GetPosition(side.indices.back());
}

Geometry::TriangleWinding Geometry::ProposedTriangleWinding(
    Point proposed_position) const {
  float signed_area = Triangle{.p0 = LastVertex(left_side_).position,
                               .p1 = LastVertex(right_side_).position,
                               .p2 = proposed_position}
                          .SignedArea();
  if (signed_area > 0) return TriangleWinding::kCounterClockwise;
  if (signed_area < 0) return TriangleWinding::kClockwise;
  return TriangleWinding::kDegenerate;
}

Geometry::TriangleWinding Geometry::ProposedIntersectionTriangleWinding(
    const Side& intersecting_side, Point proposed_position) const {
  ABSL_DCHECK(intersecting_side.intersection.has_value());
  if (!intersecting_side.intersection.has_value()) {
    return TriangleWinding::kDegenerate;
  }

  Triangle triangle = {
      .p0 = LastPosition(OpposingSide(intersecting_side)),
      .p1 = intersecting_side.intersection->last_proposed_vertex.position,
      .p2 = proposed_position};
  if (intersecting_side.self_id == SideId::kLeft) {
    std::swap(triangle.p0, triangle.p1);
  }
  float signed_area = triangle.SignedArea();
  if (signed_area > 0) return TriangleWinding::kCounterClockwise;
  if (signed_area < 0) return TriangleWinding::kClockwise;
  return TriangleWinding::kDegenerate;
}

Side& Geometry::OpposingSide(Side& side) {
  if (side.self_id == SideId::kLeft) return right_side_;
  ABSL_DCHECK(side.self_id == SideId::kRight);
  return left_side_;
}

const Side& Geometry::OpposingSide(const Side& side) const {
  if (side.self_id == SideId::kLeft) return right_side_;
  ABSL_DCHECK(side.self_id == SideId::kRight);
  return left_side_;
}

void Geometry::AppendVertexToMesh(Side& side, const ExtrudedVertex& vertex) {
  IndexType new_index = mesh_.VertexCount();
  mesh_.AppendVertex(vertex);
  vertex_side_ids_.push_back(side.self_id);
  side_offsets_.push_back(side.indices.size());
  side.indices.push_back(new_index);

  Side& opposite_side = OpposingSide(side);
  uint32_t n = opposite_side.indices.size();
  uint32_t partition_offset =
      std::max(side.partition_start.opposite_first_index_offset,
               opposite_side.partition_start.adjacent_first_index_offset);
  opposite_side_offsets_.push_back(n > partition_offset ? n - 1
                                                        : partition_offset);
}

void Geometry::TryAppendVertexAndTriangleToMesh(Side& side,
                                                const ExtrudedVertex& vertex) {
  if (ProposedTriangleWinding(vertex.position) !=
      TriangleWinding::kCounterClockwise) {
    return;
  }
  IndexType last_left = left_side_.indices.back();
  IndexType last_right = right_side_.indices.back();
  AppendVertexToMesh(side, vertex);
  mesh_.AppendTriangleIndices({last_left, last_right, side.indices.back()});
}

void Geometry::SimplifyBufferedVertices(Side& side,
                                        float initial_outline_reposition_budget,
                                        float simplification_threshold,
                                        float simplification_travel_limit) {
  if (simplification_threshold <= 0 || side.vertex_buffer.size() < 3) return;

  simplification_vertex_buffer_.clear();
  auto starting_vertex_iter = side.vertex_buffer.cbegin();

  // Skip simplification for the next vertex in some cases where that would be
  // too aggressive.
  if (side.next_buffered_vertex_offset == 2) {
    // Don't consider the i = 1 vertex (the last from the previous extrusion)
    // for simplification, if removing it would create too large of a gap.
    bool skip_vertex = DistanceBetween(side.vertex_buffer[0].position,
                                       side.vertex_buffer[2].position) >
                       simplification_travel_limit;
    // Also skip that vertex if it would make a previous simplification
    // invalid.
    if (!skip_vertex && !side.last_simplified_vertex_positions.empty()) {
      Segment segment = {side.vertex_buffer[0].position,
                         side.vertex_buffer[2].position};
      for (const Point& position : side.last_simplified_vertex_positions) {
        if (Distance(segment, position) > simplification_threshold) {
          skip_vertex = true;
          break;
        }
      }
    }
    if (skip_vertex) {
      simplification_vertex_buffer_.push_back(side.vertex_buffer[0]);
      ++starting_vertex_iter;
    }
  }

  SimplifyPolyline(starting_vertex_iter, side.vertex_buffer.cend(),
                   simplification_threshold, simplification_vertex_buffer_);

  if (simplification_vertex_buffer_.size() == side.vertex_buffer.size()) {
    // No vertices were removed.
    return;
  }

  // If `next_buffered_vertex_offset` is 2, we are considering removing the last
  // vertex of the previous extrusion, which was placed in `vertex_buffer[1]` by
  // `PrepBufferedVerticesForNextExtrusion`. If it is not in `kept`, it was
  // removed by `SimplifyPolyline` above, and we will try to replace it.
  bool last_vertex_simplified =
      side.next_buffered_vertex_offset == 2 &&
      simplification_vertex_buffer_[1].position != LastVertex(side).position;
  bool should_replace_last_vertex = last_vertex_simplified;

  if (should_replace_last_vertex &&
      ProposedTriangleWinding(simplification_vertex_buffer_[1].position) !=
          TriangleWinding::kCounterClockwise) {
    // This is an edge case where simplification would have made us miss a
    // non-CCW triangle, so we don't replace the last vertex.
    should_replace_last_vertex = false;
  }

  // If the opposite side is undergoing an intersection, then replacing a vertex
  // on the opposite side is conceptually similar to appending a CCW opposite
  // triangle. We must perform a similar action to line_mesh_generation.md#5 or
  // `TriangleBuilder::HandleCcwOppositeIntersectingTriangle` below.
  Side& opposite_side = OpposingSide(side);
  if (should_replace_last_vertex && opposite_side.intersection.has_value() &&
      opposite_side.intersection->retriangulation_started) {
    Point current_last_position = LastVertex(side).position;
    Point replacement_last_position = simplification_vertex_buffer_[1].position;
    Point intersection_position = LastVertex(opposite_side).position;
    Segment left_right_edge = {.start = replacement_last_position,
                               .end = intersection_position};
    Triangle containing_triangle{.p0 = intersection_position,
                                 .p1 = current_last_position,
                                 .p2 = replacement_last_position};
    bool intersection_found = MoveStartingVerticesToIntersection(
        opposite_side, ConstructPartialOutline(opposite_side, side),
        left_right_edge, initial_outline_reposition_budget,
        containing_triangle);
    if (!intersection_found) {
      GiveUpIntersectionHandling(opposite_side);
      should_replace_last_vertex = false;
    }
  }

  if (should_replace_last_vertex) {
    // Save the position of the vertex that is about to be replaced to
    // potentially be used in subsequent calls to this function.
    // `ProcessNewVertices()` will keep track of when any additional vertices
    // are appended to `side` after the replacement, at which point this buffer
    // will be cleared.
    side.last_simplified_vertex_positions.push_back(LastPosition(side));
    SetVertex(side.indices.back(), simplification_vertex_buffer_[1]);
  }

  if (last_vertex_simplified && !should_replace_last_vertex) {
    side.vertex_buffer.resize(2);
    side.vertex_buffer.insert(side.vertex_buffer.end(),
                              simplification_vertex_buffer_.begin() + 1,
                              simplification_vertex_buffer_.end());
  } else {
    std::swap(side.vertex_buffer, simplification_vertex_buffer_);
  }
}

void Geometry::SimplifyBufferedVertices(float initial_outline_reposition_budget,
                                        float simplification_threshold,
                                        float simplification_travel_limit) {
  SimplifyBufferedVertices(left_side_, initial_outline_reposition_budget,
                           simplification_threshold,
                           simplification_travel_limit);
  SimplifyBufferedVertices(right_side_, initial_outline_reposition_budget,
                           simplification_threshold,
                           simplification_travel_limit);
}

bool Geometry::TriangleIndicesAreLeftRightConforming(
    absl::Span<const IndexType> indices) const {
  ABSL_DCHECK_EQ(left_side_.first_triangle_vertex, 0);
  ABSL_DCHECK_EQ(right_side_.first_triangle_vertex, 1);
  return vertex_side_ids_[indices[0]] == SideId::kLeft &&
         vertex_side_ids_[indices[1]] == SideId::kRight;
}

bool Geometry::TriangleIndicesAllBelongTo(absl::Span<const IndexType> indices,
                                          SideId side) const {
  return vertex_side_ids_[indices[0]] == side &&
         vertex_side_ids_[indices[1]] == side &&
         vertex_side_ids_[indices[2]] == side;
}

std::optional<uint32_t> Geometry::FindLastTriangleContainingSegmentEnd(
    const Side& search_along_side, const Segment& segment,
    uint32_t max_early_exit_triangle) const {
  // The threshold for an index on the adjacent side that could be a pivot of
  // the current intersection, if one exists.
  IndexType current_pivot_index_threshold =
      std::numeric_limits<IndexType>::max();
  if (search_along_side.intersection.has_value() &&
      search_along_side.intersection->retriangulation_started) {
    current_pivot_index_threshold =
        search_along_side
            .indices[search_along_side.intersection->starting_offset];
  }
  const Side& opposite_side = OpposingSide(search_along_side);

  for (uint32_t i = mesh_.TriangleCount();
       i > search_along_side.partition_start.first_triangle; --i) {
    std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i - 1);

    // The triangle is a candidate if it is left-right conforming or if it is
    // one of the triangles split in the current intersection.
    if (!TriangleIndicesAreLeftRightConforming(indices) &&
        !(TriangleIndicesAllBelongTo(indices, search_along_side.self_id) &&
          indices[opposite_side.first_triangle_vertex] >=
              current_pivot_index_threshold)) {
      continue;
    }

    Triangle triangle = mesh_.GetTriangle(i - 1);
    if (LegacyTriangleContains(triangle, segment.end)) return i - 1;

    // See if we can end the search already:
    if (i - 1 <= max_early_exit_triangle &&
        TriangleIndicesAreLeftRightConforming(indices)) {
      // If `segment` does not intersect the left-to-right edge of the triangle,
      // that means a portion of the segment lies outside of the partition and
      // we should stop.
      Segment triangle_edge = triangle.GetEdge(0);
      if (!LegacyIntersects(segment, triangle_edge)) break;
    }
  }
  return std::nullopt;
}

namespace {

StrokeVertex::Label DefaultExteriorSideLabel(const Side& side) {
  return side.self_id == SideId::kLeft ? StrokeVertex::kExteriorLeftLabel
                                       : StrokeVertex::kExteriorRightLabel;
}

ExtrudedVertex LerpAlongExterior(
    const Side& exterior_side, const ExtrudedVertex& from,
    const ExtrudedVertex& to, float t,
    float margin = std::numeric_limits<float>::infinity()) {
  ExtrudedVertex result = Lerp(from, to, t);
  result.new_non_position_attributes.side_label =
      DefaultExteriorSideLabel(exterior_side).WithMargin(margin);
  return result;
}

}  // namespace

bool Geometry::MoveStartingVerticesToIntersection(
    Side& outline_starting_side, const DirectedPartialOutline& outline,
    const Segment& segment, float default_outline_reposition_budget,
    std::optional<Triangle> containing_triangle) {
  if (outline.Size() == 0 ||
      (outline_starting_side.intersection.has_value() &&
       outline_starting_side.intersection->outline_reposition_budget == 0)) {
    return false;
  }

  float search_budget =
      outline_starting_side.intersection.has_value()
          ? outline_starting_side.intersection->outline_reposition_budget
          : default_outline_reposition_budget;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, segment, mesh_, search_budget, std::move(containing_triangle));
  if (!result.segment_intersection.has_value() ||
      MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
          outline_starting_side, outline, *result.segment_intersection,
          result.segment_intersection->position)) {
    return false;
  }

  ExtrudedVertex from_vert =
      mesh_.GetVertex(outline[result.segment_intersection->starting_index]);
  ExtrudedVertex to_vert =
      mesh_.GetVertex(outline[result.segment_intersection->ending_index]);

  // Interpolate with zero margin since this function is called to shift outline
  // vertices during ongoing intersection and this helps not introduce small
  // gaps in the geometry.
  ExtrudedVertex result_vertex = LerpAlongExterior(
      outline_starting_side, from_vert, to_vert,
      result.segment_intersection->outline_interpolation_value,
      /* margin = */ 0);

  // TODO(b/148543402): Add logic to interpolate winding texture coordinates
  MoveOutlineVerticesToTarget(
      outline, 0, result.segment_intersection->ending_index, result_vertex);

  if (outline_starting_side.intersection.has_value()) {
    outline_starting_side.intersection->outline_reposition_budget =
        result.remaining_search_budget;
  }

  return true;
}

void Geometry::MoveOutlineVerticesToTarget(
    const DirectedPartialOutline& outline, uint32_t start, uint32_t end,
    const ExtrudedVertex& target) {
  for (uint32_t i = start; i < end; ++i) {
    SetVertex(outline[i], target);
  }
}

std::optional<uint32_t> Geometry::FirstVertexNotAtOutlineStart(
    const DirectedPartialOutline& outline) const {
  if (outline.Size() == 0) return std::nullopt;

  Point first = mesh_.GetPosition(outline[0]);
  for (uint32_t i = 1; i < outline.Size(); ++i) {
    if (mesh_.GetPosition(outline[i]) != first) return i;
  }

  return std::nullopt;
}

bool Geometry::ExtendOutlineToSegment(Side& outline_starting_side,
                                      const DirectedPartialOutline& outline,
                                      const Segment& segment,
                                      float max_extension_distance) {
  // Under normal conditions, max_extension_distance should be positive and not
  // NaN, but it can sometimes end up being NaN for adversarial (valid) stroke
  // inputs that make intermediate calculations start blowing up.
  if (!outline_starting_side.intersection.has_value() ||
      max_extension_distance <= 0 || std::isnan(max_extension_distance)) {
    return false;
  }

  std::optional<uint32_t> non_start_vertex =
      FirstVertexNotAtOutlineStart(outline);

  if (non_start_vertex.has_value()) {
    // Extend the first non-degenerate segment of `outline` by
    // `max_extension_distance` and search for an intersection between it and
    // `segment`.
    ExtrudedVertex from = mesh_.GetVertex(outline[*non_start_vertex]);
    ExtrudedVertex to = mesh_.GetVertex(outline[0]);
    Vec delta_vector = to.position - from.position;
    float t = 1 + max_extension_distance / delta_vector.Magnitude();
    ExtrudedVertex extended_to = Lerp(from, to, t);
    Segment extended_outline_segment = {.start = from.position,
                                        .end = extended_to.position};

    std::optional<std::pair<float, float>> result =
        geometry_internal::SegmentIntersectionRatio(extended_outline_segment,
                                                    segment);
    if (result.has_value()) {
      ExtrudedVertex result_vertex = Lerp(from, extended_to, result->first);

      // Make sure to give back to `outline_reposition_budget` since we are
      // undoing a little bit of the work of
      // `MoveStartingVerticesToIntersection`:
      outline_starting_side.intersection->outline_reposition_budget +=
          DistanceBetween(to.position, result_vertex.position);
      MoveOutlineVerticesToTarget(outline, 0, *non_start_vertex, result_vertex);
      return true;
    }
  }

  return false;
}

void Geometry::AssignVerticesInRange(std::vector<IndexType>::iterator begin,
                                     std::vector<IndexType>::iterator end,
                                     const ExtrudedVertex& target) {
  for (auto it = begin; it < end; ++it) {
    SetVertex(*it, target);
  }
}

namespace {

void SetSideLabelToInterior(ExtrudedVertex& vertex) {
  vertex.new_non_position_attributes.side_label = StrokeVertex::kInteriorLabel;
}

}  // namespace

bool Geometry::TryBeginIntersectionRetriangulation(
    Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
    uint32_t intersection_vertex_triangle) {
  if (!intersecting_side.intersection.has_value() ||
      intersecting_side.intersection->retriangulation_started) {
    return false;
  }

  bool can_begin = true;
  std::optional<ExtrudedVertex> corrected_vertex =
      MakeWindingCorrectedIntersectionVertex(
          intersecting_side, intersection_vertex, intersection_vertex_triangle);
  if (!corrected_vertex.has_value()) {
    // Breaking up triangles would cause bad winding.
    can_begin = false;
  }

  std::array<IndexType, 3> indices =
      mesh_.GetTriangleIndices(intersection_vertex_triangle);
  if (!TriangleIndicesAreLeftRightConforming(indices)) {
    // This *should* be impossible, but protect against it anyway. Any old
    // triangles an intersection is allowed to start on should be L-R-(L|R).
    can_begin = false;
  }
  IndexType saved_left = indices[left_side_.first_triangle_vertex];
  IndexType saved_right = indices[right_side_.first_triangle_vertex];

  if (!can_begin) {
    // If the propsed winding is counter-clockwise, we give up the intersection.
    // Otherwise, we just reject the vertex.
    if (ProposedTriangleWinding(intersection_vertex.position) ==
        TriangleWinding::kCounterClockwise) {
      GiveUpIntersectionHandling(intersecting_side);
      TryAppendVertexAndTriangleToMesh(intersecting_side, intersection_vertex);
    }
    return false;
  }

  TriangleWinding proposed_winding =
      ProposedTriangleWinding(corrected_vertex->position);

  // Append two new vertices to the intersecting side. The first will
  // potentially be repositioned to the opposite side below, and the second will
  // follow the intersection position.
  AppendVertexToMesh(intersecting_side, LastVertex(intersecting_side));
  SetSideLabelToInterior(*corrected_vertex);
  AppendVertexToMesh(intersecting_side, *corrected_vertex);

  auto intersection_start_index =
      intersecting_side.indices.begin() +
      intersecting_side.intersection->starting_offset;

  // We must also make sure that the saved index on the intersecting side is not
  // one of the vertices we will reposition from its original location. This can
  // happen in the case that `intersection_vertex_triangle` is greater than or
  // equal to `undo_stack_starting_triangle`.
  if (intersecting_side.self_id == SideId::kLeft) {
    saved_left = std::min(saved_left, *(intersection_start_index - 1));
  } else {
    saved_right = std::min(saved_right, *(intersection_start_index - 1));
  }

  // "Unzip" the triangles around the second point of intersection by swapping
  // one index of each triangle, creating an incomplete fan around the second
  // point of intersection:
  //
  //    X----X      X    X      X    X
  //    | \  |      | \ /|      |\  /|
  //    | o\ |  =>  | o\ |  =>  | o  |
  //    |   \|      |   \|      |/  \|
  //    X----X      X----X      X    X
  //
  for (uint32_t i = mesh_.TriangleCount(); i > intersection_vertex_triangle;
       --i) {
    std::array<IndexType, 3> mesh_indices = mesh_.GetTriangleIndices(i - 1);

    // Try to save every triangle, because they will all be shifted when we
    // insert a new triangle after this loop.
    if (save_point_state_.is_active &&
        i - 1 < save_point_state_.n_mesh_triangles) {
      save_point_state_.saved_triangle_indices.emplace(i - 1, mesh_indices);
    }

    if (i <= intersecting_side.intersection->undo_stack_starting_triangle) {
      // Push the triangle onto the stack so it can be restored later if needed.
      intersecting_side.intersection->undo_triangulation_stack.push_back(
          mesh_indices);
    }

    if (!TriangleIndicesAreLeftRightConforming(mesh_indices)) {
      // If the triangle does not conform to {left side vertex, right side
      // vertex, left or right}, then we skip it. This kind of triangle comes
      // from a previous call to this function during a past intersection
      // handling.
      continue;
    }

    if (vertex_side_ids_[mesh_indices[2]] == SideId::kLeft) {
      mesh_indices[right_side_.first_triangle_vertex] =
          *(intersection_start_index + 1);
    } else {
      mesh_indices[left_side_.first_triangle_vertex] =
          *(intersection_start_index + 1);
    }

    SetTriangleIndices(i - 1, mesh_indices, /* update_save_state = */ false);
  }

  // The previous step leaves a gap before the "unzipped" triangles.
  // We fill it by inserting a new triangle before the zip. Depending on the
  // winding of the proposed new triangle, there may also be a gap after, for
  // which we append a new triangle.

  // Before:
  mesh_.InsertTriangleIndices(
      intersection_vertex_triangle,
      {saved_left, saved_right, *(intersection_start_index + 1)});

  if (proposed_winding != TriangleWinding::kCounterClockwise) {
    // After:
    std::array<IndexType, 3> intersection_indices;
    if (intersecting_side.self_id == SideId::kLeft) {
      intersection_indices[0] = *(intersection_start_index - 1);
      intersection_indices[1] = *(intersection_start_index + 1);
    } else {
      intersection_indices[0] = *(intersection_start_index + 1);
      intersection_indices[1] = *(intersection_start_index - 1);
    }
    // For the last index of the appended triangle, we reuse the first
    // intersecting vertex while moving it to the opposite side. It will get
    // pushed back to the adjacent side with subsequent extrusions as this part
    // of the line is overwritten by the outgoing triangles.
    intersection_indices[2] = *intersection_start_index;
    mesh_.AppendTriangleIndices(intersection_indices);

    // We add to the `outline_reposition_budget` since we are about to
    // reposition the first intersection vertex across to the other side of the
    // line. Make sure it retains the "exterior" side label of the intersecting
    // side. As the intersection progresses, this vertex will get shifted back
    // to the appropriate side, and we want it to be able to make it all the way
    // back.
    ExtrudedVertex newest_opposite_vertex_copy =
        LastVertex(OpposingSide(intersecting_side));
    intersecting_side.intersection->outline_reposition_budget +=
        DistanceBetween(mesh_.GetPosition(*intersection_start_index),
                        newest_opposite_vertex_copy.position);
    newest_opposite_vertex_copy.new_non_position_attributes.side_label =
        DefaultExteriorSideLabel(intersecting_side);
    SetVertex(*intersection_start_index, newest_opposite_vertex_copy);
  }

  intersecting_side.intersection->retriangulation_started = true;
  intersecting_side.intersection->oldest_retriangulation_triangle =
      intersection_vertex_triangle;
  return true;
}

void Geometry::ContinueIntersectionRetriangulation(
    Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
    uint32_t intersection_vertex_triangle) {
  if (!intersecting_side.intersection.has_value() ||
      !intersecting_side.intersection->retriangulation_started ||
      intersection_vertex_triangle ==
          intersecting_side.intersection->oldest_retriangulation_triangle) {
    return;
  }

  if (intersection_vertex_triangle >
      intersecting_side.intersection->oldest_retriangulation_triangle) {
    // If the intersection triangle is increasing, the intersection point
    // may have started to travel forward within the line. We try to undo
    // retriangulation to keep interior triangles from expanding too much.
    UndoIntersectionRetriangulation(intersecting_side,
                                    intersection_vertex.position);
    return;
  }

  IndexType intersection_pivot_index =
      intersecting_side
          .indices[intersecting_side.intersection->starting_offset + 1];

  // We perform a similar "unzipping" action of the triangles as in
  // `TryBeginIntersectionRetriangulation`. The difference here is that we also
  // shift by one triangle toward the end of the line as we go. This moves the
  // inserted gap-covering triangle to its new needed location at
  // `intersection_vertex_triangle`.
  for (uint32_t i =
           intersecting_side.intersection->oldest_retriangulation_triangle;
       i > intersection_vertex_triangle; --i) {
    std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i - 1);

    // Push the triangle onto the stack so it can be restored later if needed.
    intersecting_side.intersection->undo_triangulation_stack.push_back(indices);

    if (TriangleIndicesAreLeftRightConforming(indices)) {
      // Only modify triangles if they conform to L-R-(L|R). Other triangles are
      // part of a previous intersection and will only be shifted.
      if (vertex_side_ids_[indices[2]] == SideId::kLeft) {
        indices[right_side_.first_triangle_vertex] = intersection_pivot_index;
      } else {
        indices[left_side_.first_triangle_vertex] = intersection_pivot_index;
      }
    }
    SetTriangleIndices(i, indices);
  }

  // The first two indices of the new gap-filling triangle are already correct,
  // so we only have to reset the third.
  std::array<IndexType, 3> indices =
      mesh_.GetTriangleIndices(intersection_vertex_triangle);
  indices[2] = intersection_pivot_index;
  SetTriangleIndices(intersection_vertex_triangle, indices);

  intersecting_side.intersection->oldest_retriangulation_triangle =
      intersection_vertex_triangle;
}

void Geometry::UndoIntersectionRetriangulation(
    Side& intersecting_side, std::optional<Point> stop_at_position) {
  if (!intersecting_side.intersection.has_value() ||
      !intersecting_side.intersection->retriangulation_started ||
      mesh_.TriangleCount() == 0) {
    // Nothing to undo.
    return;
  }

  std::vector<std::array<IndexType, 3>>& triangle_stack =
      intersecting_side.intersection->undo_triangulation_stack;
  uint32_t triangle_index =
      intersecting_side.intersection->oldest_retriangulation_triangle;

  IndexType last_left = mesh_.GetVertexIndex(triangle_index, 0);
  IndexType last_right = mesh_.GetVertexIndex(triangle_index, 1);

  while (!triangle_stack.empty() && triangle_index < mesh_.TriangleCount()) {
    const std::array<IndexType, 3>& indices = triangle_stack.back();

    if (stop_at_position.has_value() &&
        LegacyTriangleContains(Triangle{.p0 = mesh_.GetPosition(indices[0]),
                                        .p1 = mesh_.GetPosition(indices[1]),
                                        .p2 = mesh_.GetPosition(indices[2])},
                               *stop_at_position)) {
      // Once we reach an old triangle that contains the position at which to
      // stop, we know this triangle should remain broken up.
      break;
    }

    // Put the original triangle back into the mesh triangulation:
    SetTriangleIndices(triangle_index, indices);

    if (TriangleIndicesAreLeftRightConforming(indices)) {
      // Only keep track of the last left and right vertices seen in triangles
      // that conform to {left side vertex, right side vertex, left or right}.
      if (vertex_side_ids_[indices[2]] == SideId::kLeft) {
        last_left = indices[2];
        last_right = indices[right_side_.first_triangle_vertex];
      } else {
        last_left = indices[left_side_.first_triangle_vertex];
        last_right = indices[2];
      }
    }

    ++triangle_index;
    triangle_stack.pop_back();
  }

  if (triangle_index ==
      intersecting_side.intersection->oldest_retriangulation_triangle) {
    // No triangles were restored.
    return;
  }

  if (triangle_index >= mesh_.TriangleCount()) {
    intersecting_side.intersection->oldest_retriangulation_triangle =
        mesh_.TriangleCount() - 1;
    return;
  }

  intersecting_side.intersection->oldest_retriangulation_triangle =
      triangle_index;

  // The following is the extra gap-filling triangle inserted by
  // `TryBeginIntersectionRetriangulation`. Similar to how we move it backwards
  // in the line when we continue triangulation, we move it forward in the line
  // as we undo triangulation.
  SetTriangleIndices(
      triangle_index,
      {last_left, last_right,
       intersecting_side
           .indices[intersecting_side.intersection->starting_offset + 1]});
}

Side::IndexOffsetRanges Geometry::GetIntersectionTriangleFanOffsetRanges(
    const Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
    uint32_t intersection_vertex_triangle) const {
  if (!intersecting_side.intersection.has_value()) return {};

  // 1. Find the oldest triangle with indices conforming to L-R-(L|R) that would
  //    contain `intersection_vertex`.
  // 2. From this triangle, get the first left and right offsets of the vertices
  //    that will be triangulated together with the intersection vertex.
  // 3. For each of the left and right sides, check that the triangle fan made
  //    from the `intersection_vertex` position and the outline following the
  //    found first vertex on that side would not have clockwise triangles.

  bool undo_stack_triangle_found = false;
  std::array<IndexType, 3> triangle_indices;
  if (intersecting_side.intersection->retriangulation_started &&
      intersection_vertex_triangle >
          intersecting_side.intersection->oldest_retriangulation_triangle) {
    // Since the `intersection_vertex_triangle` is increasing, we first search
    // through the undo stack as does `UndoIntersectionRetriangulation`. This
    // way, we will try to find the indices of what will be
    // `oldest_retriangulation_triangle` at the end of processing this vertex.
    const std::vector<std::array<IndexType, 3>>& undo_stack =
        intersecting_side.intersection->undo_triangulation_stack;
    for (auto it = undo_stack.rbegin(); it != undo_stack.rend(); ++it) {
      if (TriangleIndicesAreLeftRightConforming(*it) &&
          LegacyTriangleContains({.p0 = mesh_.GetPosition((*it)[0]),
                                  .p1 = mesh_.GetPosition((*it)[1]),
                                  .p2 = mesh_.GetPosition((*it)[2])},
                                 intersection_vertex.position)) {
        undo_stack_triangle_found = true;
        triangle_indices = *it;
        break;
      }
    }
  }
  if (!undo_stack_triangle_found) {
    triangle_indices = mesh_.GetTriangleIndices(intersection_vertex_triangle);

    // It should be impossible for the triangle at
    // `intersection_vertex_triangle` to not be "left-right conforming", but we
    // check to be extra careful.
    if (!TriangleIndicesAreLeftRightConforming(triangle_indices) &&
        intersection_vertex_triangle >
            intersecting_side.partition_start.first_triangle) {
      for (uint32_t i = intersection_vertex_triangle;
           i > intersecting_side.partition_start.first_triangle; --i) {
        triangle_indices = mesh_.GetTriangleIndices(i - 1);
        if (TriangleIndicesAreLeftRightConforming(triangle_indices)) break;
      }
    }
  }

  return {
      .left =
          {
              .first =
                  vertex_side_ids_[triangle_indices[0]] == SideId::kLeft
                      ? side_offsets_[triangle_indices[0]]
                      : static_cast<uint32_t>(left_side_.indices.size()) - 1,
              .last = LastOutlineIndexOffset(left_side_),
          },
      .right = {
          .first = vertex_side_ids_[triangle_indices[1]] == SideId::kRight
                       ? side_offsets_[triangle_indices[1]]
                       : static_cast<uint32_t>(right_side_.indices.size()) - 1,
          .last = LastOutlineIndexOffset(right_side_),
      }};
}

std::optional<ExtrudedVertex> Geometry::MakeWindingCorrectedIntersectionVertex(
    const Side& intersecting_side, const ExtrudedVertex& intersection_vertex,
    uint32_t intersection_vertex_triangle) const {
  if (!intersecting_side.intersection.has_value()) return std::nullopt;

  // 1. Get the indices on the left and right sides corresponding to the
  //    triangle fans that would be made around the intersection position.
  // 2. Try to find the last (i.e. most forward along the line) segments on the
  //    left and right sides that would create a clockwise winding triangle with
  //    the intersection position.
  // 3. If these do not exist, the intersection vertex works as is.
  // 4. Otherwise, try to find a correction along the segment between the
  //    proposed intersection vertex toward the opposite side that would no
  //    longer cause clockwise triangles.

  Side::IndexOffsetRanges affected_offset_ranges =
      GetIntersectionTriangleFanOffsetRanges(
          intersecting_side, intersection_vertex, intersection_vertex_triangle);

  std::optional<Segment> last_cw_left_segment =
      FindLastClockwiseWindingMultiTriangleFanSegment(
          mesh_, left_side_, affected_offset_ranges.left,
          intersection_vertex.position);
  std::optional<Segment> last_cw_right_segment =
      FindLastClockwiseWindingMultiTriangleFanSegment(
          mesh_, right_side_, affected_offset_ranges.right,
          intersection_vertex.position);
  if (!last_cw_left_segment.has_value() && !last_cw_right_segment.has_value()) {
    // No correction needed.
    return intersection_vertex;
  }

  if (intersecting_side.intersection->retriangulation_started &&
      intersecting_side.intersection->outline_reposition_budget <
          intersecting_side.intersection->initial_outline_reposition_budget) {
    // If the reposition budget is already below the initial value, we will not
    // try to correct the vertex.
    return std::nullopt;
  }

  ExtrudedVertex opposite_vertex = LastVertex(OpposingSide(intersecting_side));
  float interpolation = 0;

  // E.g. we are hunting for "X" below:
  //
  //          `opposite_vertex`          `interpolation = 1`
  //                  |
  //                  |
  //   L----L         X
  //                  |       R-----R
  //                  |
  //        `intersection_vertex`        `interpolation = 0`

  // Returns the interpolation amount along the segment from
  // `intersection_vertex` to `opposite_vertex` that lies along the line defined
  // by `outline_segment`.
  auto non_cw_interpolation_amount = [&intersection_vertex, &opposite_vertex](
                                         Segment outline_segment) -> float {
    Segment adjacent_opposite_segment = {.start = intersection_vertex.position,
                                         .end = opposite_vertex.position};
    float extension_distance = std::max(
        {DistanceBetween(adjacent_opposite_segment.start,
                         outline_segment.start),
         DistanceBetween(adjacent_opposite_segment.start, outline_segment.end),
         DistanceBetween(adjacent_opposite_segment.end, outline_segment.start),
         DistanceBetween(adjacent_opposite_segment.end, outline_segment.end)});
    float outline_segment_length = outline_segment.Length();
    if (outline_segment_length == 0) return 0;

    float ratio = extension_distance / outline_segment_length;
    outline_segment = {.start = outline_segment.Lerp(-ratio),
                       .end = outline_segment.Lerp(1 + ratio)};
    std::optional<std::pair<float, float>> result =
        geometry_internal::SegmentIntersectionRatio(adjacent_opposite_segment,
                                                    outline_segment);
    return result.has_value() ? result->first : 0;
  };

  if (last_cw_left_segment.has_value()) {
    interpolation = std::max(
        interpolation, non_cw_interpolation_amount(*last_cw_left_segment));
  }
  if (last_cw_right_segment.has_value()) {
    interpolation = std::max(
        interpolation, non_cw_interpolation_amount(*last_cw_right_segment));
  }

  // Reject if the correction would not be strictly between
  // `intersection_vertex` and `opposite_vertex`.
  if (interpolation <= 0 || interpolation >= 1) {
    return std::nullopt;
  }

  // Bump the interpolation a little toward the opposite vertex to try and avoid
  // a triangle with very slightly negative signed area.
  constexpr float kInterpolationBump = 0.01;
  if (interpolation < 1 - kInterpolationBump) {
    interpolation += kInterpolationBump;
  }

  Point corrected_position = geometry_internal::Lerp(
      intersection_vertex.position, opposite_vertex.position, interpolation);

  // The correction attempt must
  //   1. Still be contained in a mesh triangle.
  //   2. Form a non-CW triangle with the starting positions of the left and
  //      right range. This must be checked because we will continue to use the
  //      same triangle that contained the uncorrected vertex.
  //   3. Not form CW winding triangles with the same indices that would have
  //      been affected by the uncorrected vertex.
  if (!FindLastTriangleContainingSegmentEnd(
           intersecting_side,
           Segment{.start = intersection_vertex.position,
                   .end = corrected_position},
           intersection_vertex_triangle)
           .has_value() ||
      Triangle{.p0 = mesh_.GetPosition(
                   left_side_.indices[affected_offset_ranges.left.first]),
               .p1 = mesh_.GetPosition(
                   right_side_.indices[affected_offset_ranges.right.first]),
               .p2 = corrected_position}
              .SignedArea() < 0 ||
      FindLastClockwiseWindingMultiTriangleFanSegment(
          mesh_, left_side_, affected_offset_ranges.left, corrected_position)
          .has_value() ||
      FindLastClockwiseWindingMultiTriangleFanSegment(
          mesh_, right_side_, affected_offset_ranges.right, corrected_position)
          .has_value()) {
    return std::nullopt;
  }

  return Lerp(intersection_vertex, opposite_vertex, interpolation);
}

bool Geometry::MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
    const Side& outline_starting_side, const DirectedPartialOutline& outline,
    const OutlineIntersectionResult::SegmentIntersection& intersection,
    Point target_position, bool stop_at_oldest_retriangulation_triangle) const {
  // 1. Get the oldest index on `outline_starting_side` that may be
  //    repositioned as a result of this outline intersection.
  // 2. Find the offset into the opposite side's `indices` for the first vertex
  //    that could be triangulated together with the one found in step 1.
  // 3. Find the last offset on the opposite side that should be included in the
  //    tested triangle fan, depending on the intersection status of
  //    `outline_starting_side` and the value of
  //    `stop_at_oldest_retriangulation_triangle`.
  // 4. Test the triangle fan made from the `target_position` and the found
  //    indices on the opposite side.

  IndexType oldest_to_be_moved = outline[intersection.starting_index];

  const Side& opposite_side = OpposingSide(outline_starting_side);
  Side::IndexOffsetRange opposite_offset_range;
  if (vertex_side_ids_[oldest_to_be_moved] == outline_starting_side.self_id) {
    opposite_offset_range.first = opposite_side_offsets_[oldest_to_be_moved];
  } else {
    // The intersection indices are already on the opposite side, and any
    // triangle of the fan before `outline[intersection.ending_index]`
    // would be made degenerate by the repositioning.
    opposite_offset_range.first =
        side_offsets_[outline[intersection.ending_index]];
  }

  if (outline_starting_side.intersection.has_value() &&
      outline_starting_side.intersection->retriangulation_started &&
      stop_at_oldest_retriangulation_triangle) {
    IndexType opposite_last_index = mesh_.GetVertexIndex(
        outline_starting_side.intersection->oldest_retriangulation_triangle,
        opposite_side.first_triangle_vertex);
    if (vertex_side_ids_[opposite_last_index] != opposite_side.self_id) {
      // This *should* be impossible - `oldest_retriangulation_triangle` indices
      // should always conform to L-R-(L|R). Protect against undefined behavior
      // by returning.
      return false;
    }
    opposite_offset_range.last = side_offsets_[opposite_last_index];
  } else {
    opposite_offset_range.last = LastOutlineIndexOffset(opposite_side);
  }
  return FindLastClockwiseWindingMultiTriangleFanSegment(
             mesh_, opposite_side, opposite_offset_range, target_position)
      .has_value();
}

void Geometry::UpdateIntersectionPivotVertices(
    Side& intersecting_side, const ExtrudedVertex& new_pivot_vertex) {
  if (!intersecting_side.intersection.has_value() ||
      !intersecting_side.intersection->retriangulation_started) {
    return;
  }

  if (texture_coord_type_ == TextureCoordType::kTiling) {
    // Without the special case of winding texture coordinates, the vertex at
    // the pivot of the intersection follows the most recent proposed vertex.
    AssignVerticesInRange(intersecting_side.indices.end() - 1,
                          intersecting_side.indices.end(), new_pivot_vertex);
    return;
  }

  // In the case of winding texture coordinates, we continue appending
  // superimposed vertices at the intersection.
  // TODO(b/148543402): try to do without appending extra vertices to see if the
  // complexity elsewhere is not greatly affected.
  AppendVertexToMesh(intersecting_side, new_pivot_vertex);

  auto intersection_start_index =
      intersecting_side.indices.begin() +
      intersecting_side.intersection->starting_offset;

  // For winding texture coordinates, only the positions for the pivot will
  // follow the most recent vertex.

  // First, we calculate the Vertex that will start the pivot. It must connect
  // to vertices preceding the intersection, so it is interpolated based on the
  // triangle we are currently intersecting.
  std::array<IndexType, 3> triangle_indices;
  if (!intersecting_side.intersection->undo_triangulation_stack.empty()) {
    triangle_indices =
        intersecting_side.intersection->undo_triangulation_stack.back();
  } else {
    triangle_indices = mesh_.GetTriangleIndices(
        intersecting_side.intersection->oldest_retriangulation_triangle);
  }
  ExtrudedVertex a = mesh_.GetVertex(triangle_indices[0]);
  ExtrudedVertex b = mesh_.GetVertex(triangle_indices[1]);
  ExtrudedVertex c = mesh_.GetVertex(triangle_indices[2]);
  ExtrudedVertex replacement =
      BarycentricLerp(a, b, c, new_pivot_vertex.position);

  AssignVerticesInRange(intersection_start_index + 1,
                        intersection_start_index + 2, replacement);

  // The rest of the vertices up to the new vertex get the special texture
  // coordinate value.
  replacement.texture_coords = kWindingTextureCoordinateSentinelValue;
  AssignVerticesInRange(intersection_start_index + 2,
                        intersecting_side.indices.end() - 1, replacement);

  // For any triangles newly broken up by `TryBeginIntersectionRetriangulation`
  // or `ContinueIntersectionRetriangulation` we need to swap one vertex. The
  // inside of the turn must continue to use the interpolated vertex located at
  // `*(intersection_start_index + 1)`, but the outside of the turn must now be
  // part of the triangle fan and use the special coordinates found in
  // `*(intersection_start_index + 2)`.
  if (intersecting_side.indices.end() - intersection_start_index > 2) {
    uint32_t replacement_triangle_vertex =
        intersecting_side.first_triangle_vertex;
    IndexType replacement_index = *(intersection_start_index + 2);
    uint32_t starting_triangle =
        intersecting_side.intersection->oldest_retriangulation_triangle + 1;
    for (uint32_t i = starting_triangle; i < mesh_.TriangleCount(); ++i) {
      // The triangles of interest have two vertices opposite to
      // `intersecting_side`, which requires the last index to not be on
      // `intersecting_side`.
      std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i);
      if (!TriangleIndicesAreLeftRightConforming(indices) ||
          vertex_side_ids_[indices[2]] == intersecting_side.self_id) {
        continue;
      }
      if (indices[replacement_triangle_vertex] >= replacement_index) break;

      indices[replacement_triangle_vertex] = replacement_index;
      SetTriangleIndices(i, indices);
    }
  }
}

void Geometry::UpdateIntersectionOuterVertices(Side& intersecting_side,
                                               IndexType pivot_start,
                                               IndexType pivot_end) {
  if (texture_coord_type_ == TextureCoordType::kTiling) return;

  // Update the secondary_texture_coords for the outside of the intersection:
  //   * Find the start and end of the outside of the intersection.
  //   * Calculate the total distance traveled by the outside of the
  //     intersection.
  //   * Calculate the starting and ending texture coordinates that the pivot of
  //     the intersection must represent.
  //   * Iterate over the outside of the intersection and interpolate the
  //     secondary_texture_coords from the start to the end of the pivot using
  //     the fraction of the total distance traveled.

  // Iterate backwards through triangles to find the latest one that includes a
  // pivot vertex with the sentinel texture coordinate value.
  uint32_t pivot_last_triangle = std::numeric_limits<uint32_t>::max();
  uint32_t i = mesh_.TriangleCount();
  while (i > 0) {
    std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i - 1);
    if (!TriangleIndicesAreLeftRightConforming(indices) ||
        vertex_side_ids_[indices[2]] == intersecting_side.self_id) {
      --i;
      continue;
    }
    IndexType pivoting_index = indices[intersecting_side.first_triangle_vertex];
    if (pivoting_index <= pivot_start) break;
    if (mesh_.GetVertex(pivoting_index).texture_coords ==
        kWindingTextureCoordinateSentinelValue) {
      pivot_last_triangle = i - 1;
      break;
    }
    --i;
  }
  if (pivot_last_triangle == std::numeric_limits<uint32_t>::max()) {
    // Nothing to update because we failed to find any triangles with the
    // special texture coordinates.
    return;
  }

  // Continue iterating to find the oldest pivoting triangle.
  uint32_t pivot_first_triangle = pivot_last_triangle;
  while (i > 0) {
    std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(i - 1);
    if (!TriangleIndicesAreLeftRightConforming(indices) ||
        vertex_side_ids_[indices[2]] == intersecting_side.self_id) {
      --i;
      continue;
    }
    if (mesh_.GetVertex(indices[intersecting_side.first_triangle_vertex])
            .texture_coords != kWindingTextureCoordinateSentinelValue) {
      break;
    }
    pivot_first_triangle = i - 1;
    --i;
  }

  // Use the first and last triangles in the pivot to get the first and last
  // indices along the outside so that we can iterate over
  // `opposite_side.indices`.
  IndexType last_outside_index = mesh_.GetVertexIndex(pivot_last_triangle, 2);
  Side& opposite_side = OpposingSide(intersecting_side);
  std::vector<IndexType>::iterator it = opposite_side.indices.end() - 1;
  while (it != opposite_side.indices.begin() && *it > last_outside_index) {
    --it;
  }

  IndexType first_outside_index = mesh_.GetVertexIndex(
      pivot_first_triangle, opposite_side.first_triangle_vertex);
  ABSL_DCHECK(vertex_side_ids_[first_outside_index] == opposite_side.self_id);
  if (vertex_side_ids_[first_outside_index] != opposite_side.self_id) {
    // This *should* be impossible, but protect against it anyway.
    return;
  }

  // Get the total distance traveled from `last_outside_index` to
  // `first_outside_index`.
  float total_distance_covered = 0;
  Point current_position = mesh_.GetPosition(*it);
  while (it != opposite_side.indices.begin() && *it > first_outside_index) {
    --it;
    Point previous_position = mesh_.GetPosition(*it);
    total_distance_covered +=
        DistanceBetween(current_position, previous_position);
    current_position = previous_position;
  }

  Point secondary_coords_start = mesh_.GetVertex(pivot_start).texture_coords;
  Point secondary_coords_end = mesh_.GetVertex(pivot_end).texture_coords;

  // Iterate from `first_outside_index` to `last_outside_index` again to
  // interpolate the secondary texture coordinates.
  // TODO(b/148543402): Should also modify primary texture coordinates to
  // potentially decrease the overall distance traveled by the texture
  // coordinates.
  float current_distance_covered = 0;
  Envelope modified_region(Point({current_position.x, current_position.y}));
  while (*it < last_outside_index) {
    // TODO(b/148543402): Try to interpolate with a smoothstep or similar to
    // ease the transition around the pivot.
    float t = current_distance_covered / total_distance_covered;
    Point interpolated_secondary_coords = geometry_internal::Lerp(
        secondary_coords_start, secondary_coords_end, t);
    ExtrudedVertex vertex = mesh_.GetVertex(*it);
    vertex.secondary_texture_coords = interpolated_secondary_coords;
    SetVertex(*it, vertex);
    ++it;
    Point next_position = mesh_.GetPosition(*it);
    current_distance_covered +=
        DistanceBetween(current_position, next_position);
    current_position = next_position;
  }
  ExtrudedVertex vertex = mesh_.GetVertex(*it);
  vertex.secondary_texture_coords = secondary_coords_end;
  SetVertex(*it, vertex);
}

void Geometry::TryFinishIntersectionHandling(
    Side& intersecting_side, const ExtrudedVertex& new_vertex,
    const DirectedPartialOutline& outline) {
  absl::Cleanup append_new_vertex = [&] {
    TryAppendVertexAndTriangleToMesh(intersecting_side, new_vertex);
  };

  Segment segment = {
      .start = intersecting_side.intersection->last_proposed_vertex.position,
      .end = new_vertex.position};
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, segment, mesh_,
      intersecting_side.intersection->outline_reposition_budget);
  if (!result.segment_intersection.has_value()) {
    GiveUpIntersectionHandling(intersecting_side);
    return;
  }

  ExtrudedVertex pivot_start_vertex;
  ExtrudedVertex pivot_end_vertex;
  if (result.remaining_search_budget <
      intersecting_side.intersection->initial_outline_reposition_budget) {
    ExtrudedVertex outline_from_vert =
        mesh_.GetVertex(outline[result.segment_intersection->starting_index]);
    ExtrudedVertex outline_to_vert =
        mesh_.GetVertex(outline[result.segment_intersection->ending_index]);
    pivot_start_vertex = LerpAlongExterior(
        intersecting_side, outline_from_vert, outline_to_vert,
        result.segment_intersection->outline_interpolation_value);
    pivot_end_vertex = LerpAlongExterior(
        intersecting_side, intersecting_side.intersection->last_proposed_vertex,
        new_vertex, result.segment_intersection->segment_interpolation_value);
    // Make the positions exactly equal:
    pivot_end_vertex.position = pivot_start_vertex.position;
  } else {
    // This is an edge case where the reposition budget is greater than its
    // initial value. Use the ending index of the outline intersection instead
    // of the actual intersection location to prevent a sharp concavity in the
    // outline.
    pivot_start_vertex =
        mesh_.GetVertex(outline[result.segment_intersection->ending_index]);
    pivot_end_vertex = pivot_start_vertex;
    result.segment_intersection->outline_interpolation_value = 1;
    result.segment_intersection->position = pivot_start_vertex.position;
  }

  if (MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
          intersecting_side, outline, *result.segment_intersection,
          result.segment_intersection->position,
          /* stop_at_oldest_retriangulation_triangle = */ false)) {
    // We give up, because cannot exit cleanly without causing CW triangles.

    if (intersecting_side.intersection->retriangulation_started) {
      // If retriangulation has started, it is possible to reduce overlap even
      // though we are giving up.

      // Since giving up means we are not undoing any of the retriangulation, we
      // check again if the outline intersection does not cause CW triangles,
      // but this time when stopping at the oldest retriangulation triangle.
      if (!MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
              intersecting_side, outline, *result.segment_intersection,
              result.segment_intersection->position)) {
        MoveOutlineVerticesToTarget(outline, 0,
                                    result.segment_intersection->ending_index,
                                    pivot_start_vertex);
      } else {
        // Otherwise, we can try to at least collapse all of the triangles on
        // `intersecting_side` that make a triangle fan with the intersection
        // vertex.
        IndexType result_start =
            outline[result.segment_intersection->starting_index];
        std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(
            intersecting_side.intersection->oldest_retriangulation_triangle);
        if (vertex_side_ids_[result_start] == intersecting_side.self_id &&
            TriangleIndicesAreLeftRightConforming(indices)) {
          auto target =
              intersecting_side.indices.begin() +
              std::max(side_offsets_
                           [indices[intersecting_side.first_triangle_vertex]],
                       side_offsets_[result_start]);
          auto intersection_pivot =
              intersecting_side.indices.begin() +
              intersecting_side.intersection->starting_offset + 1;
          AssignVerticesInRange(target + 1, intersection_pivot,
                                mesh_.GetVertex(*target));
        }
      }
    }

    GiveUpIntersectionHandling(intersecting_side);
    TryAppendVertexAndTriangleToMesh(intersecting_side, pivot_end_vertex);
    return;
  }

  if (texture_coord_type_ == TextureCoordType::kTiling) {
    MoveOutlineVerticesToTarget(outline, 0,
                                result.segment_intersection->ending_index,
                                pivot_start_vertex);

    if (intersecting_side.intersection->retriangulation_started) {
      auto intersection_start_index =
          intersecting_side.indices.begin() +
          intersecting_side.intersection->starting_offset;
      AssignVerticesInRange(intersection_start_index + 1,
                            intersecting_side.indices.end(),
                            mesh_.GetVertex(*intersection_start_index));
      UndoIntersectionRetriangulation(intersecting_side);
    }
    intersecting_side.intersection.reset();
    return;
  }

  uint32_t i = result.segment_intersection->starting_index;
  SetVertex(outline[i], pivot_start_vertex);
  pivot_start_vertex.texture_coords = kWindingTextureCoordinateSentinelValue;
  MoveOutlineVerticesToTarget(outline, 0, i, pivot_start_vertex);

  if (intersecting_side.intersection->starting_offset <
      intersecting_side.indices.size()) {
    auto intersection_start_index =
        intersecting_side.indices.begin() +
        intersecting_side.intersection->starting_offset;
    AssignVerticesInRange(intersection_start_index + 1,
                          intersecting_side.indices.end() - 1,
                          mesh_.GetVertex(*intersection_start_index));
  }
  SetVertex(intersecting_side.indices.back(), pivot_end_vertex);
  UndoIntersectionRetriangulation(intersecting_side);
  UpdateIntersectionOuterVertices(
      intersecting_side, outline[result.segment_intersection->starting_index],
      intersecting_side.indices.back());
  intersecting_side.intersection.reset();
}

void Geometry::GiveUpIntersectionHandling(Side& intersecting_side) {
  if (!intersecting_side.intersection.has_value()) return;

  if (!intersecting_side.intersection->retriangulation_started) {
    // We do not start a new partition if retriangulation hasn't started.

    // Try to append the last proposed vertex in case it got rejected.
    TryAppendVertexAndTriangleToMesh(
        intersecting_side,
        intersecting_side.intersection->last_proposed_vertex);
    intersecting_side.intersection.reset();
    return;
  }

  // We append one extra vertex per side to start the next partition on
  // `intersecting_side`.
  //
  // Be sure to set the partition first before appending new vertices to get
  // correct values of `opposite_side_offsets_`.
  Side& opposite_side = OpposingSide(intersecting_side);

  // Start a new partition on `intersecting_side`. This will be used for better
  // subjective continuity of self-overlap if the side continues to turn in the
  // same direction.
  intersecting_side.partition_start = {
      .adjacent_first_index_offset =
          static_cast<uint32_t>(intersecting_side.indices.size()),
      .opposite_first_index_offset =
          static_cast<uint32_t>(opposite_side.indices.size()),
      .first_triangle = mesh_.TriangleCount(),
      .opposite_side_initial_position = LastVertex(opposite_side).position,
      .non_ccw_connection_index =
          intersecting_side
              .indices[intersecting_side.intersection->starting_offset],
      .outline_connects_sides = true,
      .is_forward_exterior = false};

  // The split triangles will not be undone, so we try to update values in
  // `opposite_side_offsets_` for the intersection pivot and for the outer
  // triangle fan vertices that now only connect to the pivot.
  uint32_t intersection_pivot_offset =
      intersecting_side.intersection->starting_offset + 1;
  std::optional<uint32_t> first_outer_triangle;
  for (uint32_t i =
           intersecting_side.intersection->oldest_retriangulation_triangle + 1;
       i < mesh_.TriangleCount(); ++i) {
    // Outer fan triangles will be left-right conforming while inner fan
    // triangle indices will all belong to the intersecing side:
    if (TriangleIndicesAreLeftRightConforming(mesh_.GetTriangleIndices(i))) {
      first_outer_triangle = i;
      break;
    }
  }
  if (first_outer_triangle.has_value() &&
      vertex_side_ids_[mesh_.GetTriangleIndices(*first_outer_triangle)[2]] ==
          opposite_side.self_id) {
    // Update the pivot as connected to the first opposing side vertex of
    // `first_outer_triangle`:
    UpdateOppositeSideOffset(
        intersecting_side.indices[intersection_pivot_offset],
        side_offsets_[mesh_.GetTriangleIndices(
            *first_outer_triangle)[opposite_side.first_triangle_vertex]]);

    // Update the subsequent opposite side vertices as connected to the pivot.
    for (uint32_t i =
             side_offsets_[mesh_.GetTriangleIndices(*first_outer_triangle)[2]];
         i < opposite_side.indices.size(); ++i) {
      UpdateOppositeSideOffset(opposite_side.indices[i],
                               intersection_pivot_offset);
    }
  }

  // We disconnect the partition sides on the `opposite_side` if this
  // intersection traveled backward enough to break up the first triangle, as
  // this breaks the seamless connection of self-overlap anyway.
  if (intersecting_side.intersection->oldest_retriangulation_triangle <
      opposite_side.partition_start.first_triangle) {
    DisconnectPartitionSides(opposite_side);
  }

  // Add the "discontinuity" caused by the leftover non-left-right-conforming
  // triangles, which span the indices belonging to the intersecting side in
  // `oldest_retriangulation_triangle`.
  std::array<IndexType, 3> indices = mesh_.GetTriangleIndices(
      intersecting_side.intersection->oldest_retriangulation_triangle);
  if (TriangleIndicesAreLeftRightConforming(indices) &&
      vertex_side_ids_[indices[2]] == intersecting_side.self_id &&
      side_offsets_[indices[intersecting_side.first_triangle_vertex]] <
          side_offsets_[indices[2]]) {
    intersecting_side.intersection_discontinuities.push_back({
        .first =
            side_offsets_[indices[intersecting_side.first_triangle_vertex]],
        .last = side_offsets_[indices[2]],
    });
  }

  // Start the new "connected" partition with a copy of the last vertex from
  // each side. Make sure to set the "margin" on the intersecting side to zero.
  // This way the vertex cannot be repositioned at all in the shader, because
  // that would cause a small gap in geometry.
  ExtrudedVertex intersection_pivot_copy = LastVertex(intersecting_side);
  intersection_pivot_copy.new_non_position_attributes.side_label =
      DefaultExteriorSideLabel(intersecting_side).WithMargin(0);
  AppendVertexToMesh(intersecting_side, intersection_pivot_copy);
  AppendVertexToMesh(opposite_side, LastVertex(opposite_side));
  left_side_.first_simplifiable_index_offset = left_side_.indices.size();
  right_side_.first_simplifiable_index_offset = right_side_.indices.size();

  intersecting_side.intersection.reset();
  if (opposite_side.intersection.has_value()) {
    // If the opposite side is already intersecting, it will not have started
    // retriangulation yet, but we need to update its `starting_offset` because
    // we just appended a duplicate vertex.
    opposite_side.intersection->starting_offset = opposite_side.indices.size();
  }
}

void Geometry::UndoNonCcwPartitionSeparationIfNeeded(
    TriangleWinding proposed_winding, Side& new_vertex_side,
    const ExtrudedVertex& proposed_vertex) {
  // Returns true if all of the vertices since the start of the partition are in
  // the same position and that position is not the same as the last position in
  // the previous partition.
  auto partition_was_collapsed_and_moved = [this](const Side& side) -> bool {
    ABSL_DCHECK_GT(side.partition_start.adjacent_first_index_offset, 0);

    uint32_t first_offset = side.partition_start.adjacent_first_index_offset;
    Point last_position = LastVertex(side).position;
    for (uint32_t i = side.indices.size() - 1; i > first_offset; --i) {
      Point position = mesh_.GetPosition(side.indices[i - 1]);
      if (position != last_position) return false;
    }

    return last_position != mesh_.GetPosition(side.indices[first_offset - 1]);
  };

  // Returns the proposed triangle winding when using the last position of the
  // previous partition instead of the last current position on this side.
  auto proposed_winding_from_last_partition =
      [this](const Side& side,
             const ExtrudedVertex& proposed_vertex) -> TriangleWinding {
    ABSL_DCHECK_GT(side.partition_start.adjacent_first_index_offset, 0);

    uint32_t first_offset = side.partition_start.adjacent_first_index_offset;
    Triangle triangle = {
        .p0 = LastPosition(OpposingSide(side)),
        .p1 = mesh_.GetPosition(side.indices[first_offset - 1]),
        .p2 = proposed_vertex.position};
    if (side.self_id == SideId::kLeft) std::swap(triangle.p0, triangle.p1);

    float signed_area = triangle.SignedArea();
    if (signed_area > 0) return TriangleWinding::kCounterClockwise;
    if (signed_area < 0) return TriangleWinding::kClockwise;
    return TriangleWinding::kDegenerate;
  };

  auto move_vertices_to_end_of_last_partition = [this](Side& side) {
    ABSL_DCHECK_GT(side.partition_start.adjacent_first_index_offset, 0);

    auto target_index = side.indices.begin() +
                        side.partition_start.adjacent_first_index_offset - 1;
    AssignVerticesInRange(target_index + 1, side.indices.end(),
                          mesh_.GetVertex(*target_index));
  };

  if (proposed_winding == TriangleWinding::kCounterClockwise &&
      !new_vertex_side.intersection.has_value() &&
      new_vertex_side.partition_start.adjacent_first_index_offset > 0 &&
      !new_vertex_side.partition_start.is_forward_exterior &&
      partition_was_collapsed_and_moved(new_vertex_side) &&
      proposed_winding_from_last_partition(new_vertex_side, proposed_vertex) ==
          TriangleWinding::kCounterClockwise) {
    move_vertices_to_end_of_last_partition(new_vertex_side);
    return;
  }

  Side& opposite_side = OpposingSide(new_vertex_side);
  if (proposed_winding == TriangleWinding::kClockwise &&
      !opposite_side.intersection.has_value() &&
      opposite_side.partition_start.adjacent_first_index_offset > 0 &&
      !opposite_side.partition_start.is_forward_exterior &&
      partition_was_collapsed_and_moved(opposite_side) &&
      proposed_winding_from_last_partition(opposite_side, proposed_vertex) ==
          TriangleWinding::kClockwise) {
    move_vertices_to_end_of_last_partition(opposite_side);
    return;
  }
}

bool Geometry::OppositeSideMovedPartitionInitialPosition(
    const Side& side) const {
  if (!side.partition_start.opposite_side_initial_position.has_value()) {
    return false;
  }

  Point opposite_first_position = mesh_.GetPosition(
      OpposingSide(side)
          .indices[side.partition_start.opposite_first_index_offset]);
  if (opposite_first_position ==
      side.partition_start.opposite_side_initial_position) {
    return false;
  }

  Point adjacent_first_position = mesh_.GetPosition(
      side.indices[side.partition_start.adjacent_first_index_offset]);
  if (opposite_first_position == adjacent_first_position) {
    // The first opposite vertex has moved, but not by the opposite side because
    // the first adjacent and opposite vertices are on top of each other.
    return false;
  }

  return true;
}

void Geometry::DisconnectPartitionSides(Side& side) {
  side.partition_start.first_triangle = 0;
  side.partition_start.opposite_side_initial_position.reset();
  side.partition_start.non_ccw_connection_index.reset();
  side.partition_start.outline_connects_sides = false;

  // TODO: b/290231022 - Figure out if this always requires setting
  // `is_forward_exterior` to true and updating the labels on the relevant
  // vertices.
}

Geometry::TriangleBuilder::TriangleBuilder(
    Geometry* absl_nonnull geometry, float initial_outline_reposition_budget,
    float intersection_travel_limit, float retriangulation_travel_threshold)
    : geometry_(geometry),
      initial_outline_reposition_budget_(initial_outline_reposition_budget),
      intersection_travel_limit_(intersection_travel_limit),
      retriangulation_travel_threshold_(retriangulation_travel_threshold) {}

// LINT.IfChange
bool Geometry::TriangleBuilder::SidesTouch(const SlowPathTriangleInfo& info) {
  return DistanceBetween(info.proposed_vertex.position,
                         info.opposite_position) == 0 ||
         DistanceBetween(info.adjacent_position, info.opposite_position) == 0;
}

Geometry::TriangleBuilder::SlowPathTriangleInfo
Geometry::TriangleBuilder::MakeSlowPathInfo(
    TriangleWinding proposed_winding, Side& new_vertex_side,
    const ExtrudedVertex& proposed_vertex) {
  SlowPathTriangleInfo info = {
      .adjacent_side = &new_vertex_side,
      .opposite_side = &geometry_->OpposingSide(new_vertex_side),
      .adjacent_position = geometry_->LastPosition(*info.adjacent_side),
      .opposite_position = geometry_->LastPosition(*info.opposite_side),
      .proposed_vertex = proposed_vertex,
  };

  if (info.adjacent_side->intersection.has_value() ||
      proposed_winding != TriangleWinding::kCounterClockwise) {
    // Search through the end of the mesh to find if the new point is in the
    // interior. If we are currently handling self-intersection, we want to at
    // least look through all of the modified triangulation.
    uint32_t max_early_exit_triangle = std::numeric_limits<uint32_t>::max();
    Point segment_start = info.adjacent_position;
    if (info.adjacent_side->intersection.has_value()) {
      max_early_exit_triangle = std::min(
          {max_early_exit_triangle,
           info.adjacent_side->intersection->last_proposed_vertex_triangle,
           info.adjacent_side->intersection->oldest_retriangulation_triangle});
      segment_start =
          info.adjacent_side->intersection->last_proposed_vertex.position;
    }
    if (info.opposite_side->intersection.has_value()) {
      max_early_exit_triangle = std::min(
          {max_early_exit_triangle,
           info.opposite_side->intersection->oldest_retriangulation_triangle});
    }
    info.proposed_vertex_triangle =
        geometry_->FindLastTriangleContainingSegmentEnd(
            *info.adjacent_side,
            Segment{.start = segment_start,
                    .end = info.proposed_vertex.position},
            max_early_exit_triangle);
  }

  return info;
}

void Geometry::TriangleBuilder::TryAppend(Side& new_vertex_side) {
  const ExtrudedVertex& next_vertex = NextBufferedVertex(new_vertex_side);
  TriangleWinding proposed_winding =
      geometry_->ProposedTriangleWinding(next_vertex.position);

  // First we must check if the position and winding of the new vertex means we
  // should undo the handling of a previous non-ccw vertex.
  geometry_->UndoNonCcwPartitionSeparationIfNeeded(
      proposed_winding, new_vertex_side, next_vertex);

  // First, treat the case where we can append the new triangle and be done:
  //   * We are not already handling self-intersection.
  //   * The new triangle has the desired counter-clockwise winding order.
  if (!geometry_->left_side_.intersection.has_value() &&
      !geometry_->right_side_.intersection.has_value() &&
      proposed_winding == TriangleWinding::kCounterClockwise) {
    IndexType last_left = geometry_->left_side_.indices.back();
    IndexType last_right = geometry_->right_side_.indices.back();
    geometry_->AppendVertexToMesh(new_vertex_side, next_vertex);
    geometry_->mesh_.AppendTriangleIndices(
        {last_left, last_right, new_vertex_side.indices.back()});
    return;
  }

  if (!geometry_->handle_self_intersections_) {
    geometry_->AppendVertexToMesh(new_vertex_side, next_vertex);
    return;
  }

  if (geometry_->OppositeSideMovedPartitionInitialPosition(new_vertex_side)) {
    geometry_->DisconnectPartitionSides(new_vertex_side);
  }

  SlowPathTriangleInfo info =
      MakeSlowPathInfo(proposed_winding, new_vertex_side, next_vertex);
  TryAppendSlowPath(proposed_winding, info);

  if (new_vertex_side.intersection.has_value() &&
      info.proposed_vertex_triangle.has_value()) {
    new_vertex_side.intersection->last_proposed_vertex = info.proposed_vertex;
    new_vertex_side.intersection->last_proposed_vertex_triangle =
        *info.proposed_vertex_triangle;
  }
}

void Geometry::TriangleBuilder::TryAppendSlowPath(
    TriangleWinding proposed_winding, const SlowPathTriangleInfo& info) {
  if (proposed_winding == TriangleWinding::kCounterClockwise) {
    ABSL_DCHECK(info.adjacent_side->intersection.has_value() ||
                info.opposite_side->intersection.has_value());

    if (!info.opposite_side->intersection.has_value()) {
      HandleCcwAdjacentIntersectingTriangle(info);
      return;
    }

    if (!info.adjacent_side->intersection.has_value()) {
      HandleCcwOppositeIntersectingTriangle(info);
      return;
    }

    HandleCcwBothSidesIntersectingTriangle(info);
    return;
  }

  if (SidesTouch(info)) {
    // In the edge case of the two sides touching we accept the new vertex,
    // but skip this triangle and stop any ongoing intersections. Otherwise,
    // we may continue to reject all future vertices because all future
    // triangles will be degenerate.
    if (info.adjacent_side->intersection.has_value()) {
      geometry_->GiveUpIntersectionHandling(*info.adjacent_side);
    }
    if (info.opposite_side->intersection.has_value()) {
      geometry_->GiveUpIntersectionHandling(*info.opposite_side);
    }
    geometry_->AppendVertexToMesh(*info.adjacent_side, info.proposed_vertex);
    return;
  }

  if (!info.adjacent_side->intersection.has_value() &&
      !info.opposite_side->intersection.has_value()) {
    HandleNonCcwNonIntersectingTriangle(info);
    return;
  }

  if (!info.opposite_side->intersection.has_value()) {
    HandleNonCcwAdjacentIntersectingTriangle(info);
    return;
  }

  if (!info.adjacent_side->intersection.has_value()) {
    HandleNonCcwOppositeIntersectingTriangle(info);
    return;
  }

  HandleNonCcwBothSidesIntersectingTriangle(info);
}

void Geometry::TriangleBuilder::HandleCcwAdjacentIntersectingTriangleHelper(
    const SlowPathTriangleInfo& info, bool allowed_to_begin_retriangulation) {
  DirectedPartialOutline start_adjacent_outline =
      ConstructPartialOutline(*info.adjacent_side, *info.opposite_side);
  if (!info.proposed_vertex_triangle.has_value()) {
    // The new point is considered to be outside the line, so finish up
    // intersection handling.
    geometry_->TryFinishIntersectionHandling(
        *info.adjacent_side, info.proposed_vertex, start_adjacent_outline);
    return;
  }

  // Give up intersection if the proposed vertex has traveled too far.
  if (DistanceBetween(info.proposed_vertex.position,
                      info.adjacent_side->intersection->starting_position) >
          info.adjacent_side->intersection
              ->travel_limit_from_starting_position &&
      info.adjacent_side->intersection->outline_reposition_budget <=
          info.adjacent_side->intersection->initial_outline_reposition_budget) {
    geometry_->GiveUpIntersectionHandling(*info.adjacent_side);
    geometry_->TryAppendVertexAndTriangleToMesh(*info.adjacent_side,
                                                info.proposed_vertex);
    return;
  }

  // Continue processing the self-intersection:
  bool should_continue_retriangulation =
      info.adjacent_side->intersection->retriangulation_started;
  if (!info.adjacent_side->intersection->retriangulation_started) {
    bool exceeds_travel_threshold =
        DistanceBetween(info.proposed_vertex.position,
                        info.adjacent_side->intersection->starting_position) >=
        retriangulation_travel_threshold_;
    if (allowed_to_begin_retriangulation && exceeds_travel_threshold) {
      if (!geometry_->TryBeginIntersectionRetriangulation(
              *info.adjacent_side, info.proposed_vertex,
              *info.proposed_vertex_triangle)) {
        return;
      }
      Segment proposed_left_right_edge = {.start = info.opposite_position,
                                          .end = info.proposed_vertex.position};
      // TODO: b/301288962 - Investigate if we don't give up self-intersection
      // on a false return below on purpose or if this is a latent bug.
      (void)geometry_->MoveStartingVerticesToIntersection(
          *info.adjacent_side, start_adjacent_outline, proposed_left_right_edge,
          initial_outline_reposition_budget_);
      return;
    }
  }

  // At this point, the new triangle has correct winding order and the
  // opposite part of the triangle is outside the existing geometry.
  Segment proposed_left_right_edge = {.start = info.opposite_position,
                                      .end = info.proposed_vertex.position};
  float saved_budget =
      info.adjacent_side->intersection->outline_reposition_budget;
  ExtrudedVertex saved_adjacent =
      geometry_->mesh_.GetVertex(start_adjacent_outline[0]);
  bool intersection_found = geometry_->MoveStartingVerticesToIntersection(
      *info.adjacent_side, start_adjacent_outline, proposed_left_right_edge,
      initial_outline_reposition_budget_);
  if (!intersection_found) {
    // Either `proposed_left_right_edge` does not intersect with the outline at
    // all or we have run out of `outline_reposition_budget`. We give up on
    // handling the self-intersection and accept this amount of self-overlap.
    geometry_->GiveUpIntersectionHandling(*info.adjacent_side);
    geometry_->TryAppendVertexAndTriangleToMesh(*info.adjacent_side,
                                                info.proposed_vertex);
    return;
  }

  std::optional<ExtrudedVertex> corrected_vertex;
  if (should_continue_retriangulation) {
    corrected_vertex = geometry_->MakeWindingCorrectedIntersectionVertex(
        *info.adjacent_side, info.proposed_vertex,
        *info.proposed_vertex_triangle);
    if (!corrected_vertex.has_value()) {
      // We cannot accept the new position and continue intersection handling,
      // so we follow one of two options:
      //    * If the outline reposition budget is less than its initial value we
      //      give up intersection handling.
      //    * Otherwise, we skip this vertex. We are in the early stage of
      //      intersection handling, where BeginIntersectionRetriangluation has
      //      temporarily moved an adjacent vertex to opposite side and it has
      //      not been moved back (see diagrams in line_mesh_generation.md#5).
      //      The CW triangle that would have been created by the new position
      //      is caused by a small amount concavity on the adjacent side that
      //      will be drawn over by the outgoing stroke.
      if (info.adjacent_side->intersection->outline_reposition_budget <
          info.adjacent_side->intersection->initial_outline_reposition_budget) {
        geometry_->GiveUpIntersectionHandling(*info.adjacent_side);
        geometry_->TryAppendVertexAndTriangleToMesh(*info.adjacent_side,
                                                    info.proposed_vertex);
      } else {
        info.adjacent_side->intersection->outline_reposition_budget =
            saved_budget;
      }
      geometry_->SetVertex(start_adjacent_outline[0], saved_adjacent);
      return;
    }

    geometry_->ContinueIntersectionRetriangulation(
        *info.adjacent_side,
        /* uncorrected_intersection_vertex = */ info.proposed_vertex,
        *info.proposed_vertex_triangle);

    SetSideLabelToInterior(*corrected_vertex);
    geometry_->UpdateIntersectionPivotVertices(*info.adjacent_side,
                                               *corrected_vertex);
  }

  if (info.adjacent_side->intersection->retriangulation_started) {
    auto intersection_start_index =
        info.adjacent_side->indices.begin() +
        info.adjacent_side->intersection->starting_offset;

    geometry_->UpdateIntersectionOuterVertices(
        *info.adjacent_side, *(intersection_start_index + 1),
        info.adjacent_side->indices.back());
  }
}

// See line_mesh_generation.md#4
void Geometry::TriangleBuilder::HandleCcwAdjacentIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  // We only allow one side to modify the triangulation at a time. Since only
  // the adjacent side is undergoing intersection, it is also allowed to start
  // modifying the triangulation.
  HandleCcwAdjacentIntersectingTriangleHelper(
      info, /* allowed_to_begin_retriangulation = */ true);
}

void Geometry::TriangleBuilder::HandleCcwOppositeIntersectingTriangleHelper(
    const SlowPathTriangleInfo& info) {
  // Since the opposite side is undergoing intersection, the opposite side will
  // intersect the new triangle and needs to be moved out of the way.
  Segment left_right_edge = {.start = info.opposite_position,
                             .end = info.proposed_vertex.position};
  bool intersection_found = geometry_->MoveStartingVerticesToIntersection(
      *info.opposite_side,
      ConstructPartialOutline(*info.opposite_side, *info.adjacent_side),
      left_right_edge, initial_outline_reposition_budget_,
      Triangle{.p0 = info.adjacent_position,
               .p1 = info.opposite_position,
               .p2 = info.proposed_vertex.position});
  if (!intersection_found) {
    // Either `left_right_edge` does not intersect with the outline at all, we
    // have run out of `outline_reposition_budget`, or the move would have
    // caused a CW triangle. We give up on handling the self-intersection and
    // accept this amount of self-overlap.
    geometry_->GiveUpIntersectionHandling(*info.opposite_side);
  }
}

// See line_mesh_generation.md#5
void Geometry::TriangleBuilder::HandleCcwOppositeIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  // The helper may call `GiveUpIntersectionHandling` so it must be called
  // before appending the next buffered vertex.
  HandleCcwOppositeIntersectingTriangleHelper(info);
  geometry_->TryAppendVertexAndTriangleToMesh(*info.adjacent_side,
                                              info.proposed_vertex);
}

// See line_mesh_generation.md#6
void Geometry::TriangleBuilder::HandleCcwBothSidesIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  HandleCcwOppositeIntersectingTriangleHelper(info);

  // We only allow one side to modify the triangulation at a time. The side that
  // meets the criteria for modifying triangles gets to take ownership. The
  // adjacent side may begin retriangulation only if the opposite side has not
  // yet started to do so.
  bool adjacent_side_allowed_to_retriangulate =
      !(info.opposite_side->intersection.has_value() &&
        info.opposite_side->intersection->retriangulation_started);
  HandleCcwAdjacentIntersectingTriangleHelper(
      info, adjacent_side_allowed_to_retriangulate);

  // Since the adjacent side may have modified the triangulation, we must check
  // if the `last_proposed_vertex_triangle` of the opposite side's intersection
  // needs to be updated.
  if (adjacent_side_allowed_to_retriangulate &&
      info.opposite_side->intersection.has_value()) {
    Point position =
        info.opposite_side->intersection->last_proposed_vertex.position;
    std::optional<uint32_t> triangle =
        geometry_->FindLastTriangleContainingSegmentEnd(
            *info.opposite_side, Segment{.start = position, .end = position},
            info.opposite_side->partition_start.first_triangle);
    if (triangle.has_value()) {
      info.opposite_side->intersection->last_proposed_vertex_triangle =
          *triangle;
    }
  }
}

Side::SelfIntersection Geometry::TriangleBuilder::MakeAdjacentSelfIntersection(
    const SlowPathTriangleInfo& info) const {
  return {
      .starting_position = info.adjacent_position,
      .last_proposed_vertex = info.proposed_vertex,
      .last_proposed_vertex_triangle = *info.proposed_vertex_triangle,
      .starting_offset =
          static_cast<uint32_t>(info.adjacent_side->indices.size()),
      .retriangulation_started = false,
      .undo_stack_starting_triangle = geometry_->mesh_.TriangleCount(),
      .oldest_retriangulation_triangle = std::numeric_limits<uint32_t>::max(),
      .outline_reposition_budget = initial_outline_reposition_budget_,
      .initial_outline_reposition_budget = initial_outline_reposition_budget_,
      .travel_limit_from_starting_position = intersection_travel_limit_};
}

// See line_mesh_generation.md#2
void Geometry::TriangleBuilder::HandleNonCcwNonIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  if (info.proposed_vertex_triangle.has_value()) {
    info.adjacent_side->intersection = MakeAdjacentSelfIntersection(info);

    if (DistanceBetween(info.proposed_vertex.position,
                        info.adjacent_position) >=
        retriangulation_travel_threshold_) {
      geometry_->TryBeginIntersectionRetriangulation(
          *info.adjacent_side, info.proposed_vertex,
          *info.proposed_vertex_triangle);
    }
    return;
  }

  // The adjacent side is travelling backwards and the new position is on the
  // outside of the line. We try to move vertices in the adjacent side backwards
  // so that the new position would create a CCW triangle.

  DirectedPartialOutline start_adjacent_outline =
      ConstructPartialOutline(*info.adjacent_side, *info.opposite_side);
  Segment segment = {.start = info.opposite_position,
                     .end = info.proposed_vertex.position};
  OutlineIntersectionResult intersection_result =
      FindOutlineIntersection(start_adjacent_outline, segment, geometry_->mesh_,
                              initial_outline_reposition_budget_);

  // For a clockwise triangle, we perform a slightly different action from
  // `MoveStartingVerticesToIntersection`. The target where starting
  // vertices of the outline are moved will not be the outline intersection
  // result position.
  ExtrudedVertex target_vertex;
  if (intersection_result.segment_intersection.has_value()) {
    uint32_t ending_index =
        intersection_result.segment_intersection->ending_index;
    if (ending_index == start_adjacent_outline.StartingSideSize()) {
      // The outline intersection is crossing the segment that connects the two
      // sides of the stroke at the start of the partition. For better
      // subjective results in the case of partial transparency, we try to
      // connect overlapping regions using the partition's
      // `non_ccw_connection_index`.

      Point opposite_side_current_first_position =
          geometry_->mesh_.GetPosition(start_adjacent_outline[ending_index]);
      if (info.adjacent_side->partition_start.opposite_side_initial_position ==
              opposite_side_current_first_position &&
          info.adjacent_side->partition_start.non_ccw_connection_index
              .has_value()) {
        // Use the connection index if it exists and if the opposite side's
        // first vertex has not been repositioned since the partition was
        // created.
        target_vertex = geometry_->mesh_.GetVertex(
            *info.adjacent_side->partition_start.non_ccw_connection_index);
      } else {
        // Otherwise, we will try to move the starting vertices of the outline
        // to the proposed new vertex.
        target_vertex = info.proposed_vertex;
      }
    } else {
      // If we are not intersecting the segment that connects the two sides of
      // the stroke, the target will be the next vertex in the outline.
      target_vertex =
          geometry_->mesh_.GetVertex(start_adjacent_outline[ending_index]);
    }
  }

  if (intersection_result.segment_intersection.has_value() &&
      !geometry_->MovingStartingOutlineVerticesWouldCauseClockwiseTriangle(
          *info.adjacent_side, start_adjacent_outline,
          *intersection_result.segment_intersection, target_vertex.position)) {
    geometry_->MoveOutlineVerticesToTarget(
        start_adjacent_outline, 0,
        intersection_result.segment_intersection->ending_index, target_vertex);
    geometry_->TryAppendVertexAndTriangleToMesh(*info.adjacent_side,
                                                info.proposed_vertex);
    return;
  }

  // If we didn't find a workable intersection between the adjacent side and the
  // left-right edge, we check if the sides have crossed or are about to cross.
  // This would mean the line is starting to travel in the opposite direction
  // over itself. We reject the adjacent position in favor of the opposite
  // position to force the sides to touch.
  bool sides_cross_over = false;
  Segment adjacent_segment = {.start = info.adjacent_position,
                              .end = info.proposed_vertex.position};
  DirectedPartialOutline start_opposite_outline =
      ConstructPartialOutline(*info.opposite_side, *info.adjacent_side);
  auto result = FindOutlineIntersection(start_opposite_outline,
                                        adjacent_segment, geometry_->mesh_,
                                        initial_outline_reposition_budget_)
                    .segment_intersection;
  if (result.has_value() &&
      result->ending_index < start_opposite_outline.StartingSideSize()) {
    sides_cross_over = true;
  } else if (info.opposite_side->next_buffered_vertex_offset <
             info.opposite_side->vertex_buffer.size()) {
    Segment opposite_segment = {
        .start = info.opposite_position,
        .end = NextBufferedVertex(*info.opposite_side).position};
    sides_cross_over = geometry_internal::SegmentIntersectionRatio(
                           adjacent_segment, opposite_segment)
                           .has_value();
  }
  if (sides_cross_over) {
    ExtrudedVertex opposite_vertex = geometry_->LastVertex(*info.opposite_side);
    geometry_->AppendVertexToMesh(*info.adjacent_side, opposite_vertex);
  }
}

// See line_mesh_generation.md#3
void Geometry::TriangleBuilder::HandleNonCcwAdjacentIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  if (!info.proposed_vertex_triangle.has_value()) {
    if (geometry_->ProposedIntersectionTriangleWinding(
            *info.adjacent_side, info.proposed_vertex.position) !=
        TriangleWinding::kCounterClockwise) {
      // Reject the vertex and wait to decide the correct action.
      return;
    }

    geometry_->TryFinishIntersectionHandling(
        *info.adjacent_side, info.proposed_vertex,
        ConstructPartialOutline(*info.adjacent_side, *info.opposite_side));
    return;
  }

  if (!info.adjacent_side->intersection->retriangulation_started) {
    if (DistanceBetween(info.proposed_vertex.position,
                        info.adjacent_side->intersection->starting_position) >=
        retriangulation_travel_threshold_) {
      geometry_->TryBeginIntersectionRetriangulation(
          *info.adjacent_side, info.proposed_vertex,
          *info.proposed_vertex_triangle);
    }
    return;
  }

  std::optional<ExtrudedVertex> corrected_vertex =
      geometry_->MakeWindingCorrectedIntersectionVertex(
          *info.adjacent_side, info.proposed_vertex,
          *info.proposed_vertex_triangle);
  if (!corrected_vertex.has_value()) {
    // Reject the proposed vertex, and wait for a proposed CCW triangle or an
    // exterior position before giving up.
    return;
  }

  // As the intersection progresses, we would like to follow the new vertices
  // even though the triangle has CCW winding. But doing this naively would
  // create a gap in the line's geometry. We try to undo a little of what
  // `MoveStartingVerticesToIntersection` did and fill the gap.
  float max_extension_distance = std::max(
      DistanceBetween(info.adjacent_position, info.opposite_position),
      DistanceBetween(info.adjacent_position, corrected_vertex->position));
  bool extension_succeeded = geometry_->ExtendOutlineToSegment(
      *info.adjacent_side,
      ConstructPartialOutline(*info.adjacent_side, *info.opposite_side),
      Segment{.start = info.opposite_position,
              .end = corrected_vertex->position},
      max_extension_distance);
  if (!extension_succeeded) {
    // In case we were not able to successfully backtrack shifting geometry, we
    // reject the new vertex.
    // TODO(b/128436730): This path shouldn't happen but can be triggered in the
    // demo. Find a test case to repro it.
    return;
  }

  geometry_->ContinueIntersectionRetriangulation(
      *info.adjacent_side,
      /* uncorrected_intersection_vertex = */ info.proposed_vertex,
      *info.proposed_vertex_triangle);

  SetSideLabelToInterior(*corrected_vertex);
  geometry_->UpdateIntersectionPivotVertices(*info.adjacent_side,
                                             *corrected_vertex);
  IndexType pivot_start =
      info.adjacent_side
          ->indices[info.adjacent_side->intersection->starting_offset + 1];
  geometry_->UpdateIntersectionOuterVertices(
      *info.adjacent_side, pivot_start, info.adjacent_side->indices.back());
}

// See line_mesh_generation.md#7
void Geometry::TriangleBuilder::HandleNonCcwOppositeIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  if (info.proposed_vertex_triangle.has_value()) {
    info.adjacent_side->intersection = MakeAdjacentSelfIntersection(info);

    // If the opposite side is not yet breaking up triangles, and the first
    // intersecting point has traveled far enough, we try to begin
    // retriangulation on the adjacent side.
    if (!info.opposite_side->intersection->retriangulation_started &&
        DistanceBetween(info.proposed_vertex.position,
                        info.adjacent_position) >=
            retriangulation_travel_threshold_) {
      geometry_->TryBeginIntersectionRetriangulation(
          *info.adjacent_side, info.proposed_vertex,
          *info.proposed_vertex_triangle);
    }
  }
}

// See line_mesh_generation.md#8
void Geometry::TriangleBuilder::HandleNonCcwBothSidesIntersectingTriangle(
    const SlowPathTriangleInfo& info) {
  // When both sides are handling self-intersection, we don't allow both to
  // retriangulate the mesh at the same time. The first side that attempts
  // retriangulation takes ownership until that self-intersection is complete.

  if (!info.opposite_side->intersection->retriangulation_started) {
    // The opposite side does not currently own mesh retriangulation, so we can
    // handle this case as though that side is not undergoing intersection.
    HandleNonCcwAdjacentIntersectingTriangle(info);

    // Since the adjacent side may have modified the triangulation, we must
    // check if the `last_proposed_vertex_triangle` of the opposite side's
    // intersection needs to be updated.
    Point position =
        info.opposite_side->intersection->last_proposed_vertex.position;
    std::optional<uint32_t> triangle =
        geometry_->FindLastTriangleContainingSegmentEnd(
            *info.opposite_side, Segment{.start = position, .end = position},
            info.opposite_side->partition_start.first_triangle);
    if (triangle.has_value()) {
      info.opposite_side->intersection->last_proposed_vertex_triangle =
          *triangle;
    }

    return;
  }

  if (info.proposed_vertex_triangle.has_value()) {
    // Nothing to do, since the adjacent side isn't allowed to modify the mesh.
    return;
  }

  if (geometry_->ProposedIntersectionTriangleWinding(
          *info.adjacent_side, info.proposed_vertex.position) !=
      TriangleWinding::kCounterClockwise) {
    // Reject the vertex and wait to decide the correct action.
    return;
  }

  geometry_->TryFinishIntersectionHandling(
      *info.adjacent_side, info.proposed_vertex,
      ConstructPartialOutline(*info.adjacent_side, *info.opposite_side));

  // Similarly to `HandleNonCcwAdjacentIntersectingTriangle`, the leading
  // left-right edge of the stroke has moved backwards. Here, this is because of
  // finishing the adjacent side intersection on a CW proposed vertex. We must
  // extend the opposite side to fill in the resulting gap.
  DirectedPartialOutline start_opposite_outline =
      ConstructPartialOutline(*info.opposite_side, *info.adjacent_side);
  Point opposite_outline_first_position =
      geometry_->mesh_.GetPosition(start_opposite_outline[0]);
  Segment leading_left_right_edge = {
      .start = geometry_->LastPosition(*info.opposite_side),
      .end = geometry_->LastPosition(*info.adjacent_side)};
  float max_extension_distance =
      std::max(DistanceBetween(opposite_outline_first_position,
                               leading_left_right_edge.start),
               DistanceBetween(opposite_outline_first_position,
                               leading_left_right_edge.end));
  bool extension_succeeded = geometry_->ExtendOutlineToSegment(
      *info.opposite_side, start_opposite_outline, leading_left_right_edge,
      max_extension_distance);
  if (extension_succeeded) return;

  // The outline couldn't be extended along its first segment because of how
  // far backward the adjacent side has moved. We try to move the first vertex
  // of the opposite outline to the new latest adjacent side vertex. This is the
  // same action that happens to the first outline vertex in
  // `TryBeginIntersectionRetriangulation`

  // When trying to move the first opposite outline vertex to the adjacent
  // side, we must check that the triangle made from the leading left-right
  // edge and the second outline position would have correct winding order.
  ExtrudedVertex opposite_outline_second_vertex =
      geometry_->mesh_.GetVertex(start_opposite_outline[1]);
  if (geometry_->ProposedTriangleWinding(
          opposite_outline_second_vertex.position) ==
      TriangleWinding::kCounterClockwise) {
    IndexType opposite_outline_first_index = start_opposite_outline[0];
    ExtrudedVertex adjacent_last_vertex =
        geometry_->LastVertex(*info.adjacent_side);

    info.opposite_side->intersection->outline_reposition_budget -=
        DistanceBetween(
            geometry_->mesh_.GetPosition(opposite_outline_first_index),
            opposite_outline_second_vertex.position);
    info.opposite_side->intersection->outline_reposition_budget +=
        DistanceBetween(opposite_outline_second_vertex.position,
                        adjacent_last_vertex.position);

    geometry_->SetVertex(opposite_outline_first_index, adjacent_last_vertex);
  } else {
    // The adjacent side has moved backwards to the point that the
    // intersection position on the opposite side is no longer contained in
    // the stroke. We try to finish intersection handling by using the second
    // outline vertex.
    ExtrudedVertex end_intersection_vertex = opposite_outline_second_vertex;
    geometry_->TryFinishIntersectionHandling(
        *info.opposite_side, end_intersection_vertex, start_opposite_outline);
  }
}
// LINT.ThenChange(./line_mesh_generation.md)

void Geometry::SetVertex(IndexType index, const ExtrudedVertex& new_vertex,
                         bool update_save_state,
                         bool update_envelope_of_removed_geometry) {
  if (update_save_state && save_point_state_.is_active &&
      index < save_point_state_.n_mesh_vertices) {
    save_point_state_.saved_vertices.emplace(index, mesh_.GetVertex(index));
  }

  if (update_envelope_of_removed_geometry) {
    envelope_of_removed_geometry_.Add(mesh_.GetPosition(index));
  }

  if (vertex_side_ids_[index] == SideId::kLeft) {
    UpdateFirstMutatedSideIndexValue(index, first_mutated_left_index_);
  } else {
    UpdateFirstMutatedSideIndexValue(index, first_mutated_right_index_);
  }

  mesh_.SetVertex(index, new_vertex);
}

void Geometry::SetTriangleIndices(uint32_t triangle_index,
                                  const std::array<IndexType, 3>& new_indices,
                                  bool update_save_state) {
  if (update_save_state && save_point_state_.is_active &&
      triangle_index < save_point_state_.n_mesh_triangles) {
    save_point_state_.saved_triangle_indices.emplace(
        triangle_index, mesh_.GetTriangleIndices(triangle_index));
  }

  mesh_.SetTriangleIndices(triangle_index, new_indices);
}

void Geometry::UpdateOppositeSideOffset(IndexType index, uint32_t new_offset,
                                        bool update_save_state) {
  uint32_t& current_offset = opposite_side_offsets_[index];
  if (current_offset == new_offset) return;
  if (update_save_state && save_point_state_.is_active &&
      index < save_point_state_.n_mesh_vertices) {
    save_point_state_.saved_opposite_side_offsets.emplace(index,
                                                          current_offset);
  }
  current_offset = new_offset;
}

void Geometry::BeginSuperImposedPivotFan(Side& fan_pivot_side,
                                         Side& fan_outer_side) {
  // Update the texture coordinates of the pivot start to sync with the outside
  // of the turn and append a new vertex to becoming the central vertex that
  // will be part of the triangle fan.
  ExtrudedVertex pivot = mesh_.GetVertex(fan_pivot_side.indices.back());
  pivot.texture_coords.x = LastVertex(fan_outer_side).texture_coords.x;
  SetVertex(fan_pivot_side.indices.back(), pivot);
  pivot.texture_coords = kWindingTextureCoordinateSentinelValue;
  AppendVertexToMesh(fan_pivot_side, pivot);
}

void Geometry::EndSuperImposedPivotFan(Side& fan_pivot_side,
                                       Side& fan_outer_side) {
  // Append a new vertex to become the end of the pivot, and ensure that its
  // texture coordinates sync with the outside of the turn.
  ExtrudedVertex pivot_end =
      mesh_.GetVertex(*(fan_pivot_side.indices.rbegin() + 1));
  pivot_end.texture_coords.x = LastVertex(fan_outer_side).texture_coords.x;
  AppendVertexToMesh(fan_pivot_side, pivot_end);
}

bool Geometry::MeshEndsInSuperImposedPivot() const {
  if (texture_coord_type_ == TextureCoordType::kTiling) {
    return false;
  }
  return (left_side_.indices.size() > 1 &&
          mesh_.GetVertex(*(left_side_.indices.rbegin() + 1)).texture_coords ==
              kWindingTextureCoordinateSentinelValue) ||
         (right_side_.indices.size() > 1 &&
          mesh_.GetVertex(*(right_side_.indices.rbegin() + 1)).texture_coords ==
              kWindingTextureCoordinateSentinelValue);
}

void Geometry::PrepBufferedVerticesForNextExtrusion(
    Side& side, bool must_keep_last_vertex) {
  // The simplification algorithm will act on the buffered vertices and will
  // consider removing any vertices between the first and last. Therefore we
  // want to pass in all of the newly extruded vertices plus up to two vertices
  // per side from the previous extrusion. This way, we can try to remove the
  // last vertex per side from the previous extrusion. All older vertices have
  // already passed through the algorithm before.
  //
  // Extra caveats:
  //   * We do not want to consider removing vertices that are part of a
  //     self-intersection as those will be moved around by the intersection
  //     handling logic.
  //   * We do not want to consider removing a vertex that is the pivot of a
  //     triangle fan as this can lead to poor interpolation of winding texture
  //     coordinates as the pivot can be moved repeatedly.

  side.vertex_buffer.clear();
  side.next_buffered_vertex_offset = 0;

  if (side.indices.empty()) return;

  if (!must_keep_last_vertex &&
      side.indices.size() > side.first_simplifiable_index_offset &&
      !side.intersection.has_value()) {
    uint32_t n_triangles = mesh_.TriangleCount();
    if ((n_triangles > 0 &&
         side.indices.back() == mesh_.GetVertexIndex(n_triangles - 1, 2)) ||
        (n_triangles > 1 &&
         side.indices.back() == mesh_.GetVertexIndex(n_triangles - 2, 2))) {
      side.vertex_buffer.push_back(
          mesh_.GetVertex(*(side.indices.rbegin() + 1)));
      ++side.next_buffered_vertex_offset;
    }
  }

  side.vertex_buffer.push_back(LastVertex(side));
  ++side.next_buffered_vertex_offset;
}

uint32_t Geometry::FirstVisuallyMutatedTriangle() const {
  if (mesh_.FirstMutatedTriangle() == 0 ||
      (!first_mutated_left_index_.has_value() &&
       !first_mutated_right_index_.has_value())) {
    // There is no point trying to find a lower value if the first mutated
    // triangle is already 0 or if there haven't been any mutations to old
    // vertices.
    return mesh_.FirstMutatedTriangle();
  }

  IndexType left_index_lower_bound =
      first_mutated_left_index_.value_or(mesh_.VertexCount());
  IndexType right_index_lower_bound =
      first_mutated_right_index_.value_or(mesh_.VertexCount());

  uint32_t i =
      std::min(mesh_.FirstMutatedTriangle() + 1, mesh_.TriangleCount());
  for (; i > 0; --i) {
    std::array<uint32_t, 3> triangle_indices = mesh_.GetTriangleIndices(i - 1);
    if (!TriangleIndicesAreLeftRightConforming(triangle_indices)) continue;

    uint32_t max_left_index_in_triangle;
    uint32_t max_right_index_in_triangle;
    if (vertex_side_ids_[triangle_indices[2]] == SideId::kLeft) {
      max_left_index_in_triangle = triangle_indices[2];
      max_right_index_in_triangle = triangle_indices[1];
    } else {
      max_left_index_in_triangle = triangle_indices[0];
      max_right_index_in_triangle = triangle_indices[2];
    }

    if (max_left_index_in_triangle < left_index_lower_bound &&
        max_right_index_in_triangle < right_index_lower_bound) {
      break;
    }
  }

  return std::min(mesh_.FirstMutatedTriangle(), i);
}

namespace {

// Returns the number of quads that be created as a first step in triangulating
// a new batch of vertices.
uint32_t NumberOfStartingQuads(uint32_t n_left, uint32_t n_right) {
  return std::min(n_left, n_right) / 2;
}

// Returns the size of the triangle fan that should be created after the first
// batch of quads.
uint32_t NumberOfFanTriangles(uint32_t n_left, uint32_t n_right) {
  return std::max(n_left, n_right) - std::min(n_left, n_right);
}

// Returns the number of quads that should be created as the last step in
// triangulating a batch of vertices.
uint32_t NumberOfEndingQuads(uint32_t n_left, uint32_t n_right) {
  return (std::min(n_left, n_right) - 1) / 2;
}

// Returns true if `side` is undergoing intersection and appending the
// `Side::SelfIntersection::last_proposed_vertex` was rejected.
bool LastProposedVertexWasRejected(const Side& side,
                                   const MutableMeshView& mesh,
                                   float retriangulation_travel_threshold) {
  if (!side.intersection.has_value()) return false;
  if (side.intersection->retriangulation_started) {
    return side.intersection->last_proposed_vertex.position !=
           mesh.GetPosition(side.indices.back());
  }
  return DistanceBetween(side.intersection->last_proposed_vertex.position,
                         side.intersection->starting_position) >=
         retriangulation_travel_threshold;
}

}  // namespace

void Geometry::TriangulateBufferedVertices(
    float initial_outline_reposition_budget, float intersection_travel_limit,
    float retriangulation_travel_threshold) {
  uint32_t n_left_positions = left_side_.vertex_buffer.size() -
                              left_side_.next_buffered_vertex_offset + 1;
  uint32_t n_right_positions = right_side_.vertex_buffer.size() -
                               right_side_.next_buffered_vertex_offset + 1;

  if (n_left_positions < 2 && n_right_positions < 2) {
    // No new triangles to add.
    return;
  }

  // We split the triangulation into quads and a triangle fan. Each quad is made
  // from two vertices coming from each of the left and right sides. If the
  // number of left and right vertices is unequal, we use the leftover vertices
  // on the side with more plus one vertex on the side with fewer to create a
  // triangle fan.
  //
  // We make half of the quads first (rounded up), then the triangle fan
  // followed by the rest of the quads. This makes a forward-backward symmetric
  // triangulation and helps `SimplifyBufferedVertices()` be able to remove more
  // vertices.

  TriangleBuilder builder(this, initial_outline_reposition_budget,
                          intersection_travel_limit,
                          retriangulation_travel_threshold);
  // First half of the quads:
  for (uint32_t i = 0;
       i < NumberOfStartingQuads(n_left_positions, n_right_positions); ++i) {
    builder.TryAppend(right_side_);
    builder.TryAppend(left_side_);
    ++right_side_.next_buffered_vertex_offset;
    ++left_side_.next_buffered_vertex_offset;
  }

  // Then, make a triangle fan with extra vertices on the left or right.
  uint32_t n_fan_triangles =
      NumberOfFanTriangles(n_left_positions, n_right_positions);

  if (n_fan_triangles != 0) {
    bool needs_extra_pivot_vertices =
        n_fan_triangles >= 2 &&
        texture_coord_type_ == TextureCoordType::kWinding;
    Side* fan_pivot_side =
        n_left_positions > n_right_positions ? &right_side_ : &left_side_;
    Side* fan_outer_side =
        n_left_positions > n_right_positions ? &left_side_ : &right_side_;

    if (needs_extra_pivot_vertices) {
      BeginSuperImposedPivotFan(*fan_pivot_side, *fan_outer_side);
    }
    for (uint32_t i = 0; i < n_fan_triangles; ++i) {
      builder.TryAppend(*fan_outer_side);
      ++fan_outer_side->next_buffered_vertex_offset;
    }
    if (needs_extra_pivot_vertices) {
      EndSuperImposedPivotFan(*fan_pivot_side, *fan_outer_side);
    }
  }

  // Finish with the second half of the quads:
  for (uint32_t i = 0;
       i < NumberOfEndingQuads(n_left_positions, n_right_positions); ++i) {
    builder.TryAppend(right_side_);
    builder.TryAppend(left_side_);
    ++right_side_.next_buffered_vertex_offset;
    ++left_side_.next_buffered_vertex_offset;
  }

  // If the last vertex of an intersection handling side was rejected, try to
  // append it one more time. Whether a vertex is rejected can depend on the
  // order in which left and right vertices are interleaved during extrusion.
  if (n_right_positions > 1 &&
      LastProposedVertexWasRejected(left_side_, mesh_,
                                    retriangulation_travel_threshold)) {
    left_side_.vertex_buffer.push_back(
        left_side_.intersection->last_proposed_vertex);
    builder.TryAppend(left_side_);
  }
  if (n_left_positions > 1 &&
      LastProposedVertexWasRejected(right_side_, mesh_,
                                    retriangulation_travel_threshold)) {
    right_side_.vertex_buffer.push_back(
        right_side_.intersection->last_proposed_vertex);
    builder.TryAppend(right_side_);
  }

  bool must_keep_last_vertices = MeshEndsInSuperImposedPivot();
  PrepBufferedVerticesForNextExtrusion(left_side_, must_keep_last_vertices);
  PrepBufferedVerticesForNextExtrusion(right_side_, must_keep_last_vertices);
}

}  // namespace brush_tip_extruder_internal
}  // namespace ink
