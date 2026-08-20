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

#ifndef INK_STROKES_INTERNAL_ROUNDED_POLYGON_H_
#define INK_STROKES_INTERNAL_ROUNDED_POLYGON_H_

#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink::strokes_internal {

// A polygon-like shape with rounded corners, each of which may have a different
// rounding radius.
//
// This shape is defined by taking two or more circles, then connecting each of
// them, in order, by their right-side exterior tangents (see
// `geometry_internal::Circle::GuaranteedRightTangentAngle`). The result is a
// shape whose boundary is a composite curve made up of alternating circular
// arcs (which may be degenerate) and line segments (which may not be
// degenerate).
//
// Note that it is possible to construct a `RoundedPolygon` with a
// self-intersecting boundary. However, this class is intended to fulfill a
// narrow use case in constraining `BrushTipShapes`, which is guaranteed to not
// produce a self-intersecting `RoundedPolygon`, so we don't detect or handle
// self-intersections in this class.
class RoundedPolygon {
 public:
  // An arc component of the boundary. It covers the portion of `circle` between
  // `start_unit_vector` to `end_unit_vector`, travelling in the direction of
  // increasing angle (counter-clockwise when viewed from the positive z-axis).
  //
  // The endpoints are stored as `Vec`s rather than `Angle`s to avoid repeated
  // calls to trig functions.
  struct Arc {
    geometry_internal::Circle circle;
    Vec start_unit_vector;
    Vec end_unit_vector;
  };

  // Constructs a `RoundedPolygon` from `circles`. This `CHECK`-fails if
  // `circles.size()` < 2, or if any circle contains either the next or previous
  // one (including the last and first).
  explicit RoundedPolygon(absl::Span<const geometry_internal::Circle> circles);

  // Returns true if `circle` is contained within the `RoundedPolygon`. Note
  // that `circle` is considered to be contained even if it touches the
  // boundary.
  bool ContainsCircle(const geometry_internal::Circle& circle) const;

  // Returns the `Arc` components of the boundary. The index of the arc
  // corresponds to the index of the `Circle` passed in to the constructor.
  absl::Span<const Arc> Arcs() const { return arcs_; }

  // Returns the `Segment` component of the boundary that connects the `Arc`s at
  // `index` and (`index` + 1) % `Arcs().size`. This `DCHECK`-fails if
  // `index` < 0 or `index` >= `Arcs().size()`.
  Segment GetSegment(int index) const;

 private:
  // The `Arc` components of the boundary. The `Segment` components are stored
  // implicitly; each spans the `end` of one `Arc` to the `start` of the next.
  //
  // We use a capacity of 8 because we're expecting the `RoundedPolyshape` to be
  // the shape you get by connecting two `BrushTipShapes` by their tangents, and
  // each of them is made up of at most 4 circles.
  absl::InlinedVector<Arc, 8> arcs_;
};

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline Segment RoundedPolygon::GetSegment(int index) const {
  ABSL_DCHECK_GE(index, 0);
  ABSL_DCHECK_LT(index, arcs_.size());
  const Arc& first_arc = arcs_[index];
  const Arc& second_arc = arcs_[(index + 1) % arcs_.size()];
  return {.start = first_arc.circle.Center() +
                   first_arc.circle.Radius() * first_arc.end_unit_vector,
          .end = second_arc.circle.Center() +
                 second_arc.circle.Radius() * second_arc.start_unit_vector};
}

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_ROUNDED_POLYGON_H_
