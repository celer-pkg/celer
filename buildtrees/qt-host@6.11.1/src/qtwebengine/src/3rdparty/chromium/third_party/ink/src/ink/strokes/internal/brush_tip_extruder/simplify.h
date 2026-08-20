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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIMPLIFY_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIMPLIFY_H_

#include <vector>

#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"

namespace ink::brush_tip_extruder_internal {

// Simplifies the portion of a polyline (represented by const iterators
// indicating the beginning and end of a portion of a vector of ExtrudedVertex)
// using the Ramer-Douglas-Peucker algorithm (https://w.wiki/8Dvo), appending
// the result to the output vector. This copies the polyline, omitting vertices
// which don't change the position of any point in the polyline by more than
// epsilon.
void SimplifyPolyline(std::vector<ExtrudedVertex>::const_iterator begin,
                      std::vector<ExtrudedVertex>::const_iterator end,
                      float epsilon, std::vector<ExtrudedVertex>& output);

}  // namespace ink::brush_tip_extruder_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_SIMPLIFY_H_
