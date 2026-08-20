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

#include "ink/geometry/intersects.h"

#include <optional>

#include "ink/geometry/affine_transform.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace {

// This is a helper function for the `Intersects` overloads that take a
// `PartitionedMesh` and a different primitive, and handles the logic that does
// not depend on the primitive type itself.
template <typename ObjectType>
bool IntersectsPartitionedMesh(const PartitionedMesh& a,
                               const AffineTransform& a_to_b_transform,
                               const ObjectType& b) {
  // An empty shape does not intersect anything.
  if (a.Meshes().empty()) return false;

  std::optional<AffineTransform> inverse_a_transform =
      a_to_b_transform.Inverse();
  if (inverse_a_transform.has_value()) {
    // The transform is invertible, so we can simply transform `b` to `a`'s
    // coordinate space.

    // This is an `auto` instead of `ObjectType` because the `Rect` overload of
    // `AffineTransform::Apply` returns a `Quad`, not a `Rect`.
    auto transformed_b = inverse_a_transform->Apply(b);
    bool found_intersection = false;
    a.VisitIntersectedTriangles(
        transformed_b,
        [&found_intersection](const PartitionedMesh::TriangleIndexPair) {
          found_intersection = true;
          return PartitionedMesh::FlowControl::kBreak;
        });
    return found_intersection;
  } else {
    // A non-invertible transform collapses the `PartitionedMesh` to a
    // `Segment`, so we can defer to the `Segment` overload of `Intersects`.
    Segment collapsed_shape = geometry_internal::CalculateCollapsedSegment(
        a.Meshes(), *a.Bounds().AsRect(), a_to_b_transform);
    return Intersects(collapsed_shape, b);
  }
}

}  // namespace

bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, Point b) {
  return IntersectsPartitionedMesh(a, a_to_b_transform, b);
}
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Segment& b) {
  return IntersectsPartitionedMesh(a, a_to_b_transform, b);
}
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Triangle& b) {
  return IntersectsPartitionedMesh(a, a_to_b_transform, b);
}
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Rect& b) {
  return IntersectsPartitionedMesh(a, a_to_b_transform, b);
}
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Quad& b) {
  return IntersectsPartitionedMesh(a, a_to_b_transform, b);
}

namespace {

// Attempts to check whether `lhs` intersects `rhs` in `rhs`'s coordinate space.
// Checking in `rhs`'s coordinate space requires that `rhs_transform` is
// invertible. If `rhs_transform` is non-invertible, this will return
// `std::nullopt`; otherwise, it will return whether they intersect.
std::optional<bool> TryOneWayPartitionedMeshToPartitionedMeshIntersects(
    const PartitionedMesh& lhs, const AffineTransform& lhs_transform,
    const PartitionedMesh& rhs, const AffineTransform& rhs_transform) {
  std::optional<AffineTransform> inverse_rhs_transform =
      rhs_transform.Inverse();
  if (!inverse_rhs_transform.has_value()) return std::nullopt;

  bool found_intersection = false;
  rhs.VisitIntersectedTriangles(
      lhs,
      [&found_intersection](const PartitionedMesh::TriangleIndexPair) {
        found_intersection = true;
        return PartitionedMesh::FlowControl::kBreak;
      },
      *inverse_rhs_transform * lhs_transform);
  return found_intersection;
}

}  // namespace

bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_common_transform,
                const PartitionedMesh& b,
                const AffineTransform& b_to_common_transform) {
  // An empty shape does not intersect anything.
  if (a.Meshes().empty() || b.Meshes().empty()) return false;

  // Try checking for intersection in `b`'s coordinate space, and if that fails,
  // try in `a`'s coordinate space.
  if (auto result = TryOneWayPartitionedMeshToPartitionedMeshIntersects(
          a, a_to_common_transform, b, b_to_common_transform);
      result.has_value()) {
    return *result;
  }
  if (auto result = TryOneWayPartitionedMeshToPartitionedMeshIntersects(
          b, b_to_common_transform, a, a_to_common_transform);
      result.has_value()) {
    return *result;
  }

  // Neither transform is invertible, so both shapes must collapse to `Segment`s
  // in the space we're testing in; collapse the shapes and defer to the
  // `Segment` against `Segment` overload.
  Segment collapsed_a = geometry_internal::CalculateCollapsedSegment(
      a.Meshes(), *a.Bounds().AsRect(), a_to_common_transform);
  Segment collapsed_b = geometry_internal::CalculateCollapsedSegment(
      b.Meshes(), *b.Bounds().AsRect(), b_to_common_transform);
  return Intersects(collapsed_a, collapsed_b);
}

}  // namespace ink
