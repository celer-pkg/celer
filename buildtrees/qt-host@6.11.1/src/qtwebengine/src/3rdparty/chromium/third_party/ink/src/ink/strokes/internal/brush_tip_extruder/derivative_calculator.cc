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

#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {

using ::ink::geometry_internal::UniqueLineIntersectionRatio;
using ::ink::geometry_internal::VectorFromPointToSegmentProjection;
using ::ink::strokes_internal::StrokeVertex;

void DerivativeCalculator::AverageDerivative::Add(Vec v) {
  float m = v.Magnitude();
  if (m == 0) return;
  magnitude_sum_ += m;
  unit_vector_sum_ += v / m;
  ++count_;
}

void DerivativeCalculator::AverageDerivative::Add(
    const AverageDerivative& other) {
  magnitude_sum_ += other.magnitude_sum_;
  unit_vector_sum_ += other.unit_vector_sum_;
  count_ += other.count_;
}

Vec DerivativeCalculator::AverageDerivative::Value() const {
  if (count_ == 0) return {0, 0};
  return Vec::FromDirectionAndMagnitude(unit_vector_sum_.Direction(),
                                        magnitude_sum_ / count_);
}

void DerivativeCalculator::UpdateMesh(
    absl::Span<const uint32_t> left_indices_to_update,
    absl::Span<const uint32_t> right_indices_to_update, MutableMeshView& mesh) {
  if ((left_indices_to_update.empty() && right_indices_to_update.empty())) {
    return;
  }
  ABSL_CHECK(mesh.HasMeshData());

  ResetTrackedValues(left_indices_to_update, right_indices_to_update, mesh);

  AccumulateDerivatives(mesh);
  UpdateMeshDerivatives(left_indices_to_update, mesh);
  UpdateMeshDerivatives(right_indices_to_update, mesh);

  AccumulateMargins(mesh);
  UpdateMeshMargins(left_indices_to_update, mesh);
  UpdateMeshMargins(right_indices_to_update, mesh);
}

void DerivativeCalculator::ResetTrackedValues(
    absl::Span<const uint32_t> left_indices_to_update,
    absl::Span<const uint32_t> right_indices_to_update,
    const MutableMeshView& mesh) {
  if (!left_indices_to_update.empty() && !right_indices_to_update.empty()) {
    minimum_tracked_index_ = std::min(left_indices_to_update.front(),
                                      right_indices_to_update.front());
  } else if (!left_indices_to_update.empty()) {
    minimum_tracked_index_ = left_indices_to_update.front();
  } else {
    ABSL_DCHECK(!right_indices_to_update.empty());
    minimum_tracked_index_ = right_indices_to_update.front();
  }

  ABSL_CHECK_LT(minimum_tracked_index_, mesh.VertexCount());
  uint32_t tracked_vertex_count = mesh.VertexCount() - minimum_tracked_index_;

  tracked_average_derivatives_.clear();
  tracked_average_derivatives_.resize(tracked_vertex_count);

  tracked_side_margin_upper_bounds_.clear();
  tracked_side_margin_upper_bounds_.resize(tracked_vertex_count,
                                           StrokeVertex::kMaximumMargin);
}

void DerivativeCalculator::SaveSideDerivative(
    const std::array<uint32_t, 3>& indices, Vec derivative) {
  for (uint32_t index : indices) {
    if (index < minimum_tracked_index_) continue;
    tracked_average_derivatives_[index - minimum_tracked_index_].side.Add(
        derivative);
  }
}

void DerivativeCalculator::SaveForwardDerivative(
    const std::array<uint32_t, 3>& indices, Vec derivative) {
  for (uint32_t index : indices) {
    if (index < minimum_tracked_index_) continue;
    tracked_average_derivatives_[index - minimum_tracked_index_].forward.Add(
        derivative);
  }
}

namespace {

// Iterates over the triangles in the stroke `mesh` that have indices greater
// than or equal to `minimum_tracked_index` and calls `f` with `mesh` and a
// triplet of vertex indices. `Function` is expected to have a signature of
// `(const MutableMesh&, const std::array<uint32_t, 3>) -> void`. It accepts the
// vertex indices rather than the index of the triangle to avoid duplicate look
// up of the indices that make up the triangle.
template <typename Function>
void ForEachTriangleWithTrackedIndices(const MutableMeshView& mesh,
                                       uint32_t minimum_tracked_index,
                                       Function&& f) {
  for (uint32_t i = mesh.TriangleCount(); i > 0; --i) {
    std::array<uint32_t, 3> triangle_indices = mesh.GetTriangleIndices(i - 1);

    // The triangle indices produced by `brush_tip_extruder_internal::Geometry`
    // are expected to be in a sorted order such that once all indices of a
    // triangle are below the minimum tracked index, we can exit early.
    if (triangle_indices[0] < minimum_tracked_index &&
        triangle_indices[1] < minimum_tracked_index &&
        triangle_indices[2] < minimum_tracked_index) {
      break;
    }

    f(mesh, triangle_indices);
  }
}

}  // namespace

void DerivativeCalculator::AccumulateDerivatives(const MutableMeshView& mesh) {
  ForEachTriangleWithTrackedIndices(
      mesh, minimum_tracked_index_,
      [&](const MutableMeshView& mesh,
          const std::array<uint32_t, 3>& triangle_indices) {
        AddDerivativesForTriangle(mesh, triangle_indices);
      });
}

namespace {

// Returns a `Triangle` from `mesh` using its three vertex `indices`. When the
// indices have already been retrieved, this avoids repeating lookups that
// happen when calling `mesh.GetTriangle()`.
Triangle GetTriangleFromIndices(const MutableMeshView& mesh,
                                const std::array<uint32_t, 3>& indices) {
  return {mesh.GetPosition(indices[0]), mesh.GetPosition(indices[1]),
          mesh.GetPosition(indices[2])};
}

// Returns the projections from triangle points toward their opposite edges. The
// `Vec` at index 0 corresponds to the projection toward `triangle.GetEdge(0)`.
std::optional<std::array<Vec, 3>> PointToEdgeProjectionsForTriangle(
    const Triangle& triangle) {
  std::array<Vec, 3> result;
  Point transposed_points[3] = {triangle.p2, triangle.p0, triangle.p1};
  for (int i = 0; i < 3; ++i) {
    std::optional<Vec> v = VectorFromPointToSegmentProjection(
        transposed_points[i], triangle.GetEdge(i));
    if (!v.has_value()) return std::nullopt;
    result[i] = *v;
  }
  return result;
}

}  // namespace

void DerivativeCalculator::AddDerivativesForTriangle(
    const MutableMeshView& mesh,
    const std::array<uint32_t, 3>& triangle_indices) {
  Triangle triangle = GetTriangleFromIndices(mesh, triangle_indices);

  // For each triangle, we are trying to calculate plus or minus the derivative
  // of position with respect to one or more chosen barycentric coordinate.
  //
  // This operation is relatively simple, because each barycentric coordinate
  // ranges from 1 at a particular vertex to 0 at the edge opposite that vertex.
  // This means the derivative is the vector delta between that vertex position
  // and its projection on the line coinciding with the opposite edge. See also
  // the definition of "triangle altitude"
  // (https://en.wikipedia.org/wiki/Altitude_(triangle)).
  //
  // For the "side" derivative, we will use the barycentric coordinate that
  // increases either left-to-right or right-to-left relative to the direction
  // of travel. This depends on whether we find two of the triangle's vertices
  // along the left-exterior, or if two are along the right-exterior. We will
  // negate the case of right-to-left, so that the result always points
  // left-to-right.
  //
  // For the "forward" derivative we will average any coordinate for which the
  // two opposite vertices have the same forward category.
  //
  // The triangle in the stroke mesh will something like one of the following
  // diagrams:
  //
  // A)
  //   2         2--1     travel direction; approximately λ_2 / -λ_0
  //   |\        | /            ^
  //   | \   or  |/             |
  //   0--1      0               ---> λ_1
  //
  // OR
  // B)
  //      2      0--2     travel direction; approximately λ_2 / -λ_1
  //     /|       \ |           ^
  //    / |  or    \|           |
  //   0--1         1   λ_0 <---

  auto projections_to_edge = PointToEdgeProjectionsForTriangle(triangle);
  if (!projections_to_edge.has_value()) {
    // If the values were not found, it is because the triangle is degenerate.
    // This is expected to be because two of the points are coincident, and the
    // case is handled later in `UpdateMeshDerivativeValues()`.
    return;
  }

  StrokeVertex::SideCategory side_categories[3] = {
      mesh.GetSideLabel(triangle_indices[0]).DecodeSideCategory(),
      mesh.GetSideLabel(triangle_indices[1]).DecodeSideCategory(),
      mesh.GetSideLabel(triangle_indices[2]).DecodeSideCategory()};
  if (side_categories[2] == StrokeVertex::SideCategory::kExteriorLeft &&
      side_categories[0] == side_categories[2]) {
    SaveSideDerivative(triangle_indices, -(*projections_to_edge)[2]);
  } else if (side_categories[2] == StrokeVertex::SideCategory::kExteriorRight &&
             side_categories[1] == side_categories[2]) {
    SaveSideDerivative(triangle_indices, (*projections_to_edge)[1]);
  }

  StrokeVertex::ForwardCategory forward_categories[3] = {
      mesh.GetForwardLabel(triangle_indices[0]).DecodeForwardCategory(),
      mesh.GetForwardLabel(triangle_indices[1]).DecodeForwardCategory(),
      mesh.GetForwardLabel(triangle_indices[2]).DecodeForwardCategory()};

  // For the forward derivative, we will average in every value for which the
  // corresponding edge has vertices with the same cateogory.
  if (forward_categories[0] == forward_categories[1]) {
    SaveForwardDerivative(triangle_indices, -(*projections_to_edge)[0]);
  }
  if (forward_categories[1] == forward_categories[2]) {
    SaveForwardDerivative(triangle_indices, (*projections_to_edge)[1]);
  }
  if (forward_categories[0] == forward_categories[2]) {
    SaveForwardDerivative(triangle_indices, (*projections_to_edge)[2]);
  }
}

namespace {

struct VertexDerivatives {
  Vec side;
  Vec forward;
};

// Returns the average derivative across the vertices at `indices` using the
// values in `tracked_average_derivatives`.
VertexDerivatives CalculateAverages(
    absl::Span<const uint32_t> indices,
    absl::Span<const DerivativeCalculator::AverageVertexDerivatives>
        tracked_average_derivatives,
    uint32_t minimum_tracked_index) {
  DerivativeCalculator::AverageVertexDerivatives averages;

  for (uint32_t i : indices) {
    const auto& values = tracked_average_derivatives[i - minimum_tracked_index];
    averages.side.Add(values.side);
    averages.forward.Add(values.forward);
  }

  return {.side = averages.side.Value(), .forward = averages.forward.Value()};
}

// Returns the number of elements at the start of `indices` that refer to
// vertices in `mesh` located at the same position and having the same side and
// forward categories. Expects to be called with non-empty `indices`.
uint32_t StartingSameCategoryCoincidentCount(absl::Span<const uint32_t> indices,
                                             const MutableMeshView& mesh) {
  ABSL_DCHECK(!indices.empty());

  Point first_point = mesh.GetPosition(indices.front());
  StrokeVertex::SideCategory first_side_category =
      mesh.GetSideLabel(indices.front()).DecodeSideCategory();
  StrokeVertex::ForwardCategory first_forward_category =
      mesh.GetForwardLabel(indices.front()).DecodeForwardCategory();
  uint32_t count = 1;
  for (uint32_t index : indices.subspan(1)) {
    if (mesh.GetPosition(index) != first_point ||
        mesh.GetSideLabel(index).DecodeSideCategory() != first_side_category ||
        mesh.GetForwardLabel(index).DecodeForwardCategory() !=
            first_forward_category) {
      break;
    }
    ++count;
  }
  return count;
}

}  // namespace

void DerivativeCalculator::UpdateMeshDerivatives(
    absl::Span<const uint32_t> indices_to_update, MutableMeshView& mesh) {
  while (!indices_to_update.empty()) {
    uint32_t count =
        StartingSameCategoryCoincidentCount(indices_to_update, mesh);
    absl::Span<const uint32_t> coincident_indices =
        indices_to_update.first(count);

    VertexDerivatives averages =
        CalculateAverages(coincident_indices, tracked_average_derivatives_,
                          minimum_tracked_index_);

    for (uint32_t index : coincident_indices) {
      mesh.SetSideDerivative(index, averages.side);
      mesh.SetForwardDerivative(index, averages.forward);
    }
    indices_to_update.remove_prefix(count);
  }
}

void DerivativeCalculator::AccumulateMargins(const MutableMeshView& mesh) {
  ForEachTriangleWithTrackedIndices(
      mesh, minimum_tracked_index_,
      [&](const MutableMeshView& mesh,
          const std::array<uint32_t, 3>& triangle_indices) {
        AddMarginUpperBoundsForTriangle(mesh, triangle_indices);
      });
}

void DerivativeCalculator::SaveSideMarginUpperBound(uint32_t index,
                                                    float margin) {
  if (index < minimum_tracked_index_) return;
  float& current =
      tracked_side_margin_upper_bounds_[index - minimum_tracked_index_];
  current = std::min(current, margin);
}

namespace {

// Returns the segment starting at a triangle's `vertex_position`, and ending
// at `vertex_position +/- derivative` such that the segment is oriented away
// from the triangle's interior.
//
// `outset_sign` is expected to be the return value of
// `StrokeVertex::Label::DerivativeOutsetSign()`.
Segment MakeOutsetSegment(Point vertex_position, float outset_sign,
                          Vec derivative) {
  return {.start = vertex_position,
          .end = vertex_position + outset_sign * derivative};
}

// Returns one of the segments that will be used to constrain vertex outsets.
//
// For a triangle, the returned segment will start at the given
// `vertex_position` and end at a point on the `opposite_edge`.
Segment MakeBoundingSegment(Point vertex_position,
                            const Segment& opposite_edge) {
  // The chosen position on `opposite_edge` is weighted so that the segment is
  // as close to perpendicular to the edge without getting too close to one of
  // its endpoints. This gives more room for both vertices of `opposite_edge` to
  // be repositioned without crossing the segment.
  float ratio = std::clamp(
      opposite_edge.Project(vertex_position).value_or(0.5f), 0.1f, 0.9f);
  return {.start = vertex_position, .end = opposite_edge.Lerp(ratio)};
}

// Returns the upper bound on the margin based on a single
// `bounding_line_segment`.
//
// The `derivative_outset_segment` should start at the vertex position and gives
// the path along which the vertex will be moved.
float MarginUpperBound(const Segment& derivative_outset_segment,
                       const Segment& bounding_line_segment) {
  // The margin upper bound is determined by the point of intersection between
  // the `derivative_outset_segment` and the `bounding_line_segment`. We are
  // only interested in the lerp ratio of the intersection, because the margin
  // is defined in multiples of `derivative_outset_segment.Magnitude()`. We are
  // also only interested in lerp ratios greater than or equal to zero along
  // both segments because the vertex will only be outset, not inset.
  std::optional<std::pair<float, float>> intersection =
      UniqueLineIntersectionRatio(derivative_outset_segment,
                                  bounding_line_segment);
  if (intersection.has_value() && intersection->first >= 0) {
    return intersection->first;
  }

  // The lines either do not intersect or they intersect such that
  // `intersection->first` is less than zero, meaning there is no upper bound
  // caused by this segment.
  return StrokeVertex::kMaximumMargin;
}

Point TrianglePosition(const Triangle& triangle, int vertex_index) {
  if (vertex_index == 0) return triangle.p0;
  if (vertex_index == 1) return triangle.p1;
  return triangle.p2;
}

}  // namespace

void DerivativeCalculator::AddMarginUpperBoundsForTriangle(
    const MutableMeshView& mesh,
    const std::array<uint32_t, 3>& triangle_indices) {
  Triangle triangle = GetTriangleFromIndices(mesh, triangle_indices);

  // Degenerate triangles must be handled separately:
  if (triangle.SignedArea() == 0) {
    // Check if the triangle is degenerate, but no two vertices of the triangle
    // are coincident. In that case, the three vertices must always remain
    // collinear, so we need to set all of the side margins to 0.
    if (triangle.p0 != triangle.p1 && triangle.p0 != triangle.p2 &&
        triangle.p1 != triangle.p2) {
      for (int i = 0; i < 3; ++i) {
        SaveSideMarginUpperBound(triangle_indices[i], 0);
      }
    }
    // Otherwise, since two of the vertices share the same position, we can skip
    // the entire triangle. Coincident vertices will be given the same
    // derivative values and be repositioned the same way in the shader, so the
    // degenerate triangle does not impact the margins of any of its vertices.
    return;
  }

  // Each triangle splits its exterior into three regions according to the
  // values of `bounding_segments` below. Each region is bounded by two rays,
  // each of which start at a triangle vertex and cross through that vertex's
  // opposing edge. The margins are calculated such that each vertex is
  // restricted to stay within its starting region.
  //
  // (In the diagram below, the rays pointing out of the triangle are meant to
  // radiate out from each opposite vertex, but ASCII art has its limitations).
  //
  //             \    region for   /
  //              \    vertex 0   /
  //               \             /
  //                \     0     /
  //                 \   / \   /
  //     region       \ /   \ /      region
  //     for           /     \       for
  //     vertex 1     /       \      vertex 2
  //                 1---------2
  //                      |
  //                      |
  //                      |
  //
  // It is possible to find a set of margins that are both more robust and
  // less restrictive by including the three outset segments in the calculation.
  // However, doing so for the general case requires solving a system of
  // quadratic equations, so we opt for this simpler approach until a more
  // sophisticated one proves necessary. The method used here keeps changes in
  // derivative and margin values from having any backward-propagating effect on
  // incremental stroke extrusion.
  std::array<Segment, 3> bounding_segments{
      MakeBoundingSegment(triangle.p0, triangle.GetEdge(1)),
      MakeBoundingSegment(triangle.p1, triangle.GetEdge(2)),
      MakeBoundingSegment(triangle.p2, triangle.GetEdge(0))};

  for (int i = 0; i < 3; ++i) {
    Point vertex_position = TrianglePosition(triangle, i);
    float side_outset_sign =
        mesh.GetSideLabel(triangle_indices[i]).DerivativeOutsetSign();
    if (side_outset_sign == 0) {
      // If the outset sign is 0, this vertex should not be repositioned at
      // all, and the upper bound is 0. This happens for an interior vertex
      // label.
      SaveSideMarginUpperBound(triangle_indices[i], 0);
      continue;
    }
    Segment side_outset_segment =
        MakeOutsetSegment(vertex_position, side_outset_sign,
                          mesh.GetSideDerivative(triangle_indices[i]));
    float margin_upper_bound = std::min(
        MarginUpperBound(side_outset_segment, bounding_segments[(i + 1) % 3]),
        MarginUpperBound(side_outset_segment, bounding_segments[(i + 2) % 3]));
    SaveSideMarginUpperBound(triangle_indices[i], margin_upper_bound);
  }
}

namespace {

// Returns the number of elements at the start of `indices` that refer to
// vertices in `mesh` located at the same position. Expects to be called with
// non-empty `indices`.
//
// Coincident vertices included in the return value are allowed to have
// different label categories as long as they would still be in the same
// geometrically continuous section of the mesh.
uint32_t StartingCoincidentConnectedCount(absl::Span<const uint32_t> indices,
                                          const MutableMeshView& mesh) {
  ABSL_DCHECK(!indices.empty());

  Point first_point = mesh.GetPosition(indices.front());
  StrokeVertex::ForwardCategory previous_category =
      mesh.GetForwardLabel(indices.front()).DecodeForwardCategory();
  uint32_t count = 1;
  for (uint32_t index : indices.subspan(1)) {
    if (mesh.GetPosition(index) != first_point) break;

    StrokeVertex::ForwardCategory current_category =
        mesh.GetForwardLabel(index).DecodeForwardCategory();
    if (previous_category == StrokeVertex::ForwardCategory::kExteriorBack &&
        current_category != previous_category) {
      // This is a boundary between disconnected partitions that happen to have
      // coincident vertices.
      break;
    }
    previous_category = current_category;
    ++count;
  }
  return count;
}

// Aggregates and returns the minimum margin values for every index from
// `coincident_connected_indices`.
float CalculateSideMargin(
    absl::Span<const uint32_t> coincident_connected_indices,
    absl::Span<const float> tracked_side_margin_upper_bounds,
    uint32_t minimum_tracked_index) {
  // Note that it generally does not make sense to compare the margins of
  // two vertices with different label categories. This is because margins are
  // in multiples of derivative magnitude, so in general, comparing margins
  // requires that the two vertices share the same derivative. This is not the
  // case for coincident vertices that have different labels, because we do
  // not average their derivatives together in the code above. However, for
  // any adjacent labels that have different categories, one of those
  // categories will always be interior. This means that the minimum will end
  // up being set to zero, which has the same meaning for every vertex
  // regardless of derivative magnitude.
  return absl::c_accumulate(
      coincident_connected_indices, StrokeVertex::kMaximumMargin,
      [&](float current_min, uint32_t index) {
        return std::min(
            current_min,
            tracked_side_margin_upper_bounds[index - minimum_tracked_index]);
      });
}

}  // namespace

void DerivativeCalculator::UpdateMeshMargins(
    absl::Span<const uint32_t> indices_to_update, MutableMeshView& mesh) {
  while (!indices_to_update.empty()) {
    uint32_t count = StartingCoincidentConnectedCount(indices_to_update, mesh);
    absl::Span<const uint32_t> coincident_indices =
        indices_to_update.first(count);

    float side_margin = CalculateSideMargin(coincident_indices,
                                            tracked_side_margin_upper_bounds_,
                                            minimum_tracked_index_);
    for (uint32_t index : coincident_indices) {
      mesh.SetSideLabel(index,
                        mesh.GetSideLabel(index).WithMargin(side_margin));
    }
    indices_to_update.remove_prefix(count);
  }
}

}  // namespace ink::brush_tip_extruder_internal
