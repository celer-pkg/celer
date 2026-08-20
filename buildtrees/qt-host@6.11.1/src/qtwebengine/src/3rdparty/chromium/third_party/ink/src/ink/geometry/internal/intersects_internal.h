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

#ifndef INK_GEOMETRY_INTERNAL_INTERSECTS_INTERNAL_H_
#define INK_GEOMETRY_INTERNAL_INTERSECTS_INTERNAL_H_

#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace geometry_internal {

// These functions contain the logic for the public `Intersects` functions. The
// logic has been extracted here to avoid a circular dependency between the
// public `Intersects` functions and `PartitionedMesh`, as
// `PartitionedMesh::VisitIntersectedTriangles` requires intersection, but we
// also have `Intersects` functions that operate on `PartitionedMesh`.

bool IntersectsInternal(Point a, Point b);
bool IntersectsInternal(Point a, const Segment& b);
bool IntersectsInternal(Point a, const Triangle& b);
bool IntersectsInternal(Point a, const Rect& b);
bool IntersectsInternal(Point a, const Quad& b);

bool IntersectsInternal(const Segment& a, Point b);
bool IntersectsInternal(const Segment& a, const Segment& b);
bool IntersectsInternal(const Segment& a, const Triangle& b);
bool IntersectsInternal(const Segment& a, const Rect& b);
bool IntersectsInternal(const Segment& a, const Quad& b);

bool IntersectsInternal(const Triangle& a, Point b);
bool IntersectsInternal(const Triangle& a, const Segment& b);
bool IntersectsInternal(const Triangle& a, const Triangle& b);
bool IntersectsInternal(const Triangle& a, const Rect& b);
bool IntersectsInternal(const Triangle& a, const Quad& b);

bool IntersectsInternal(const Rect& a, Point b);
bool IntersectsInternal(const Rect& a, const Segment& b);
bool IntersectsInternal(const Rect& a, const Triangle& b);
bool IntersectsInternal(const Rect& a, const Rect& b);
bool IntersectsInternal(const Rect& a, const Quad& b);

bool IntersectsInternal(const Quad& a, Point b);
bool IntersectsInternal(const Quad& a, const Segment& b);
bool IntersectsInternal(const Quad& a, const Triangle& b);
bool IntersectsInternal(const Quad& a, const Rect& b);
bool IntersectsInternal(const Quad& a, const Quad& b);

}  // namespace geometry_internal
}  // namespace ink

#endif  // INK_GEOMETRY_INTERNAL_INTERSECTS_INTERNAL_H_
