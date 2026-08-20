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

#ifndef INK_STROKES_INPUT_RECORDED_TEST_INPUTS_H_
#define INK_STROKES_INPUT_RECORDED_TEST_INPUTS_H_

#include <utility>
#include <vector>

#include "ink/geometry/rect.h"
#include "ink/strokes/input/stroke_input_batch.h"

namespace ink {

// Returns inputs for a short straight line based on collected input, scaled to
// fit within `bounds`. In the incremental case the scaling is based on both
// real and predicted inputs.
//
// Incremental returns a tuple of pairs of real and predicted input as collected
// at the time of drawing. Complete returns a single StrokeInputBatch that
// coalesces only the real inputs that make up the input of that stroke.
std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>
MakeIncrementalStraightLineInputs(const Rect& bounds);
StrokeInputBatch MakeCompleteStraightLineInputs(const Rect& bounds);

// Returns inputs for a spring shaped spiral with two loops based on collected
// input, scaled to fit within `bounds`. In the incremental case the scaling is
// based on both real and predicted inputs.
//
// Incremental returns a tuple of pairs of real and predicted input as collected
// at the time of drawing. Complete returns a single StrokeInputBatch that
// coalesces only the real inputs that make up the input of that stroke.
std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>
MakeIncrementalSpringShapeInputs(const Rect& bounds);
StrokeInputBatch MakeCompleteSpringShapeInputs(const Rect& bounds);

}  // namespace ink

#endif  // INK_STROKES_INPUT_RECORDED_TEST_INPUTS_H_
