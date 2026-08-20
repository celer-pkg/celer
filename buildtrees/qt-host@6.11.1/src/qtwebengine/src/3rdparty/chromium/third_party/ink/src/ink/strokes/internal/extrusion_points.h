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

#ifndef INK_STROKES_INTERNAL_EXTRUSION_POINTS_H_
#define INK_STROKES_INTERNAL_EXTRUSION_POINTS_H_

#include <vector>

#include "ink/geometry/point.h"

namespace ink::strokes_internal {

// Storage for positions at which the stroke geometry should be extruded.
//
// The labels "left" and "right" are applied to the positions by looking at the
// stroke from the positive z-axis with the "forward" direction oriented in the
// travel direction for each portion of the stroke.
struct ExtrusionPoints {
  std::vector<Point> left;
  std::vector<Point> right;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_EXTRUSION_POINTS_H_
