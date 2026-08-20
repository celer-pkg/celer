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

#include "ink/strokes/internal/brush_tip_extruder/simplify.h"

#include <iterator>
#include <vector>

#include "ink/geometry/distance.h"
#include "ink/geometry/segment.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"

namespace ink::brush_tip_extruder_internal {

namespace {

void SimplifyPolylineInterior(
    Segment seg, std::vector<ExtrudedVertex>::const_iterator interior_begin,
    std::vector<ExtrudedVertex>::const_iterator interior_end, float epsilon,
    std::vector<ExtrudedVertex>& output) {
  if (std::distance(interior_begin, interior_end) < 1) {
    // If there are no points in the interior, we're done simplifying.
    return;
  }

  // Find the point that is farthest from the segment.
  std::vector<ExtrudedVertex>::const_iterator farthest_point = interior_begin;
  float max_distance = 0;
  for (auto it = interior_begin; it != interior_end; ++it) {
    float distance = Distance(seg, it->position);
    if (distance > max_distance) {
      farthest_point = it;
      max_distance = distance;
    }
  }

  if (max_distance > epsilon) {
    // Recursively simplify the points before the farthest point.
    SimplifyPolylineInterior(Segment{seg.start, farthest_point->position},
                             interior_begin, farthest_point, epsilon, output);

    // Add the farthest point.
    output.push_back(*farthest_point);

    // Recursively simplify the points after the farthest point.
    SimplifyPolylineInterior(Segment{farthest_point->position, seg.end},
                             farthest_point + 1, interior_end, epsilon, output);
  }
}

}  // namespace

void SimplifyPolyline(std::vector<ExtrudedVertex>::const_iterator begin,
                      std::vector<ExtrudedVertex>::const_iterator end,
                      float epsilon, std::vector<ExtrudedVertex>& output) {
  if (std::distance(begin, end) < 3) {
    // There are less than three points, so there's nothing to do -- just copy
    // the input points (if any).
    output.insert(output.end(), begin, end);
    return;
  }

  // Add the first point.
  output.push_back(*begin);

  // Recursively simplify the interior points.
  SimplifyPolylineInterior(Segment{begin->position, (end - 1)->position},
                           begin + 1, end - 1, epsilon, output);

  // Add the last point.
  output.push_back(*(end - 1));
}

}  // namespace ink::brush_tip_extruder_internal
