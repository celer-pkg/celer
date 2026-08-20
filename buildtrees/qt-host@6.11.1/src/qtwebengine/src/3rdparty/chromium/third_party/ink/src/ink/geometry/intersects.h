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

#ifndef INK_GEOMETRY_INTERSECTS_H_
#define INK_GEOMETRY_INTERSECTS_H_

#include "ink/geometry/affine_transform.h"
#include "ink/geometry/internal/intersects_internal.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {

// These functions return true if the given pair of objects intersect or
// overlap, i.e. if there is any point that is contained in both objects. Note
// that, because it is expensive to apply a transform to a mesh, overloads that
// operate on a PartitionedMesh also take a transform; this transform maps from
// the PartitionedMesh's coordinate space to the coordinate space that the
// intersection should be checked in.
inline bool Intersects(Point a, Point b);
inline bool Intersects(Point a, const Segment& b);
inline bool Intersects(Point a, const Triangle& b);
inline bool Intersects(Point a, const Rect& b);
inline bool Intersects(Point a, const Quad& b);
inline bool Intersects(Point a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform);

inline bool Intersects(const Segment& a, Point b);
inline bool Intersects(const Segment& a, const Segment& b);
inline bool Intersects(const Segment& a, const Triangle& b);
inline bool Intersects(const Segment& a, const Rect& b);
inline bool Intersects(const Segment& a, const Quad& b);
inline bool Intersects(const Segment& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform);

inline bool Intersects(const Triangle& a, Point b);
inline bool Intersects(const Triangle& a, const Segment& b);
inline bool Intersects(const Triangle& a, const Triangle& b);
inline bool Intersects(const Triangle& a, const Rect& b);
inline bool Intersects(const Triangle& a, const Quad& b);
inline bool Intersects(const Triangle& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform);

inline bool Intersects(const Rect& a, Point b);
inline bool Intersects(const Rect& a, const Segment& b);
inline bool Intersects(const Rect& a, const Triangle& b);
inline bool Intersects(const Rect& a, const Rect& b);
inline bool Intersects(const Rect& a, const Quad& b);
inline bool Intersects(const Rect& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform);

inline bool Intersects(const Quad& a, Point b);
inline bool Intersects(const Quad& a, const Segment& b);
inline bool Intersects(const Quad& a, const Triangle& b);
inline bool Intersects(const Quad& a, const Rect& b);
inline bool Intersects(const Quad& a, const Quad& b);
inline bool Intersects(const Quad& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform);

bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, Point b);
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Segment& b);
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Triangle& b);
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Rect& b);
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, const Quad& b);
bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_common_transform,
                const PartitionedMesh& b,
                const AffineTransform& b_to_common_transform);

bool Intersects(const PartitionedMesh& a,
                const AffineTransform& a_to_b_transform, Point b);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool Intersects(Point a, Point b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(Point a, const Segment& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(Point a, const Triangle& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(Point a, const Rect& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(Point a, const Quad& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(Point a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform) {
  return Intersects(b, b_to_a_transform, a);
}
inline bool Intersects(const Segment& a, Point b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Segment& a, const Segment& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Segment& a, const Triangle& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Segment& a, const Rect& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Segment& a, const Quad& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Segment& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform) {
  return Intersects(b, b_to_a_transform, a);
}
inline bool Intersects(const Triangle& a, Point b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Triangle& a, const Segment& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Triangle& a, const Triangle& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Triangle& a, const Rect& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Triangle& a, const Quad& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Triangle& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform) {
  return Intersects(b, b_to_a_transform, a);
}
inline bool Intersects(const Rect& a, Point b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Rect& a, const Segment& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Rect& a, const Triangle& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Rect& a, const Rect& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Rect& a, const Quad& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Rect& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform) {
  return Intersects(b, b_to_a_transform, a);
}
inline bool Intersects(const Quad& a, Point b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Quad& a, const Segment& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Quad& a, const Triangle& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Quad& a, const Rect& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Quad& a, const Quad& b) {
  return geometry_internal::IntersectsInternal(a, b);
}
inline bool Intersects(const Quad& a, const PartitionedMesh& b,
                       const AffineTransform& b_to_a_transform) {
  return Intersects(b, b_to_a_transform, a);
}

}  // namespace ink

#endif  // INK_GEOMETRY_INTERSECTS_H_
