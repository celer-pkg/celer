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

#ifndef INK_STROKES_INTERNAL_CIRCULAR_EXTRUSION_HELPERS_H_
#define INK_STROKES_INTERNAL_CIRCULAR_EXTRUSION_HELPERS_H_

#include <vector>

#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"

namespace ink::strokes_internal {

// Option for `AppendCircularTurnExtrusionPoints()` to adjust how the cases in
// circular_turn_extrusion_points.svg are handled.
enum class AddCircularTangentIntersections {
  // No intersections should be added, only circular arcs. This effectively
  // makes all configurations handled as though they were case 4.
  kNo,
  // Points of intersection for the circular tangents should be added where
  // appropriate as depicted in cases 1 - 6.
  kYes,
};

// Given a trio of circles, this constructs the portion of the left and right
// outline of the turn that corresponds to the middle circle.
//
// "Left" and "right" are lebels determined by viewing the xy plane from the
// positive z-axis in the direction of travel. The value of `max_chord_height`
// determines the accuracy of approximated cicular arcs. See the declaration of
// `geometry_internal::Circle::AppendArcToPolyline()`.
//
// In the first version below, constructed points are appended, in order, to
// `left_points` and `right_points`, which may be `nullptr` to generate points
// only on one side. The second version always returns the points from both
// sides in a new `ExtrusionPoints` object.
void AppendCircularTurnExtrusionPoints(
    const geometry_internal::Circle& start,
    const geometry_internal::Circle& middle,
    const geometry_internal::Circle& end, float max_chord_height,
    AddCircularTangentIntersections add_intersections,
    std::vector<Point>* left_points, std::vector<Point>* right_points);

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_CIRCULAR_EXTRUSION_HELPERS_H_
