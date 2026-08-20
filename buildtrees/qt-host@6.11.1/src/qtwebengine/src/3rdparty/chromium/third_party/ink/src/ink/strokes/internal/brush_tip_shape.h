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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_SHAPE_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_SHAPE_H_

#include <utility>

#include "absl/types/span.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/extrusion_points.h"
#include "ink/types/small_array.h"

namespace ink::strokes_internal {

// Helper type that stores the analytical representation of the brush tip's
// shape for a given `BrushTipState`. The shape is represented by the convex
// hull of 1 to 4 "perimeter" circles. This can be used to generate the
// positions that should be added to a stroke's geometry.
class BrushTipShape {
 public:
  // Constructs the shape from the given `tip_state` and a
  // `min_nonzero_radius_and_separation`.
  //
  // The shape will be centered at `tip_state.position`. The properties of the
  // tip state determine the number, position, and radii of the perimeter
  // circles created to represent the shape.
  //
  // The value of `min_nonzero_radius_and_separation` gives the smallest
  // non-zero radius or distance between centers for the control circles.
  //   * The radius will be set to zero if it would otherwise be smaller than
  //     this value.
  //   * Similarly, two or more of the circles will be combined if their
  //     separation would otherwise be smaller than this value.
  //
  // CHECK-fails if `tip_state` has `width` or `height` not greater than or
  // equal to 0, or `percent_radius` and `pinch` are outside the range [0, 1].
  BrushTipShape(const BrushTipState& tip_state,
                float min_nonzero_radius_and_separation);

  BrushTipShape(const BrushTipShape&) = default;
  BrushTipShape& operator=(const BrushTipShape&) = default;
  ~BrushTipShape() = default;

  // Appends incremental extrusion points given a sequence of three
  // `BrushTipShape`s. These points represent the portion of the stroke outline
  // contributed by `middle`.
  //
  // The value of `max_chord_height` determines the number of points used to
  // approximate curves; see comment on
  // `geometry_internal::Circle::AppendArcToPolyline()` for more details.
  static void AppendTurnExtrusionPoints(const BrushTipShape& start,
                                        const BrushTipShape& middle,
                                        const BrushTipShape& end,
                                        float max_chord_height,
                                        ExtrusionPoints& extrusion_points);

  // Appends extrusion points for the start of the stroke given the shapes
  // representing the first and second inputs. These points represent the
  // portion of the stroke outline contributed by `first`.
  //
  // The value of `max_chord_height` determines the number of points used to
  // approximate curves; see comment on
  // `geometry_internal::Circle::AppendArcToPolyline()` for more details.
  static void AppendStartcapExtrusionPoints(const BrushTipShape& first,
                                            const BrushTipShape& second,
                                            float max_chord_height,
                                            ExtrusionPoints& extrusion_points);

  // Appends the extrusion points for the end of the stroke given the shapes
  // representing the second to last and last inputs. These points represent the
  // portion of the stroke outline contributed by `last`.
  //
  // The value of `max_chord_height` determines the number of points used to
  // approximate curves; see comment on
  // `geometry_internal::Circle::AppendArcToPolyline()` for more details.
  static void AppendEndcapExtrusionPoints(const BrushTipShape& second_to_last,
                                          const BrushTipShape& last,
                                          float max_chord_height,
                                          ExtrusionPoints& extrusion_points);

  // Appends the extrusion points for the complete outline of the given `shape`.
  //
  // The points will be split into left and right points according to the
  // `forward_direction`. See comments on `ExtrusionPoints` for how "left" and
  // "right" are defined.
  //
  // The value of `max_chord_height` determines the number of points used to
  // approximate curves; see comment on
  // `geometry_internal::Circle::AppendArcToPolyline()` for more details.
  static void AppendWholeShapeExtrusionPoints(
      const BrushTipShape& shape, float max_chord_height, Vec forward_direction,
      ExtrusionPoints& extrusion_points);

  // Calculates the indices of perimeter circles in `first` and `second` that
  // should be connected by tangents when extruding.
  //
  // The function is similar to `geometry_internal::Circle::GetTangentAngles`,
  // but unlike for circles, the points on the two shapes that should be
  // connected by tangents don't necessarily lie at the same angles relative to
  // the shape centers. Hence two values returned per side.
  //
  // For example, `TangentCircleIndices::left.first` is the index into
  // `first.PerimeterCircles()`, and `TangentCircleIndices::left.second` is the
  // index into `second.PerimeterCircles()` that should connect the left side.
  struct TangentCircleIndices {
    std::pair<int, int> left;
    std::pair<int, int> right;
  };
  static TangentCircleIndices GetTangentCircleIndices(
      const BrushTipShape& first, const BrushTipShape& second);

  // Returns the center position, which is equivalent to the position of the
  // `BrushTipState` used to construct the shape.
  Point Center() const;

  // Returns the 1 to 4 circles that make up the tip shape's perimeter.
  absl::Span<const geometry_internal::Circle> PerimeterCircles() const;

  // Returns true if this shape completely contains the `other`.
  //
  // Containment can include touching boundaries, which means a shape will
  // always be considered to contain itself.
  bool Contains(const BrushTipShape& other) const;

  // Given an `index` into `PerimeterCircles()`, returns the index of the next
  // circle that is positioned counter-clockwise around the `Center()` when
  // viewed from the positive z-axis.
  int GetNextPerimeterIndexCcw(int index) const;

  // Given an `index` into `PerimeterCircles()`, returns the index of the next
  // circle that is positioned clockwise around the `Center()` when viewed from
  // the positive z-axis.
  int GetNextPerimeterIndexCw(int index) const;

  // Returns the minimum bounding rectangle of the `BrushTipShape`.
  Rect Bounds() const;

 private:
  Point center_;
  SmallArray<geometry_internal::Circle, 4> circles_;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_SHAPE_H_
