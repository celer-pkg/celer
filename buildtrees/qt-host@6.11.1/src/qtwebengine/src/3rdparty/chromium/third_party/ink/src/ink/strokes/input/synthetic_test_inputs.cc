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

#include "ink/strokes/input/synthetic_test_inputs.h"

#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {

StrokeInputBatch MakeCompleteLissajousCurveInputs(
    Duration32 full_stroke_duration, const Rect& bounds, int input_count,
    PhysicalDistance stroke_unit_length) {
  auto wave_function = [](float min, float max, float progress,
                          float frequency) {
    return 0.5f * (min + max) +
           0.5f * (max - min) * Cos(frequency * kHalfTurn * progress);
  };

  constexpr float kXFrequency = 7;
  constexpr float kYFrequency = 9;

  std::vector<StrokeInput> inputs;
  for (int i = 0; i < input_count; ++i) {
    float progress = i / (input_count - 1.f);
    float x =
        wave_function(bounds.XMin(), bounds.XMax(), progress, kXFrequency);
    float y =
        wave_function(bounds.YMin(), bounds.YMax(), progress, kYFrequency);
    inputs.push_back({
        .position = {x, y},
        .elapsed_time = progress * full_stroke_duration,
        .stroke_unit_length = stroke_unit_length,
    });
  }

  auto input_batch = StrokeInputBatch::Create(inputs);
  ABSL_CHECK_OK(input_batch);
  return *std::move(input_batch);
}

}  // namespace ink
