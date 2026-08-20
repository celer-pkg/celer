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

#include "ink/strokes/input/recorded_test_inputs.h"

#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/input/recorded_test_inputs_data.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"

namespace ink {

namespace {

// Gets the bounding box for both real and predicted input.
Rect GetBoundingBox(
    const std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& batches) {
  Envelope envelope;
  for (const auto& [real, predicted] : batches) {
    for (StrokeInput real_input : real) {
      envelope.Add(real_input.position);
    }
    for (StrokeInput predicted_input : predicted) {
      envelope.Add(predicted_input.position);
    }
  }
  ABSL_CHECK(!envelope.IsEmpty());

  return *envelope.AsRect();
}

// Gets the bounding box for a single StrokeInputBatch.
Rect GetBoundingBox(const StrokeInputBatch& batch) {
  Envelope envelope;
  for (StrokeInput input : batch) {
    envelope.Add(input.position);
  }
  ABSL_CHECK(!envelope.IsEmpty());

  return *envelope.AsRect();
}

void ApplyAffineTransform(
    const AffineTransform& transform,
    std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& batches) {
  for (auto& [real, predicted] : batches) {
    real.Transform(transform);
    predicted.Transform(transform);
  }
}

void BoundTestInputs(
    const Rect& bounds,
    std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& batches) {
  Rect raw_bounds = GetBoundingBox(batches);
  auto transform = AffineTransform::Find(raw_bounds, bounds);
  ABSL_CHECK(transform.has_value());

  ApplyAffineTransform(*transform, batches);
}

void BoundTestInputs(const Rect& bounds, StrokeInputBatch& batch) {
  Rect raw_bounds = GetBoundingBox(batch);
  auto transform = AffineTransform::Find(raw_bounds, bounds);
  ABSL_CHECK(transform.has_value());

  batch.Transform(*transform);
}

StrokeInputBatch GetRealCombinedInputs(
    const std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& batches) {
  StrokeInputBatch real_inputs;
  for (const auto& inputs : batches) {
    ABSL_CHECK_OK(real_inputs.Append(inputs.first));
  }
  return real_inputs;
}

}  // namespace

std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>
MakeIncrementalStraightLineInputs(const Rect& bounds) {
  auto batches = MakeStraightLineRaw();
  BoundTestInputs(bounds, batches);
  return batches;
}

StrokeInputBatch MakeCompleteStraightLineInputs(const Rect& bounds) {
  auto batches = MakeStraightLineRaw();
  StrokeInputBatch combined_batch = GetRealCombinedInputs(batches);
  BoundTestInputs(bounds, combined_batch);
  return combined_batch;
}

std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>
MakeIncrementalSpringShapeInputs(const Rect& bounds) {
  auto batches = MakeSpringShapeRaw();
  BoundTestInputs(bounds, batches);
  return batches;
}

StrokeInputBatch MakeCompleteSpringShapeInputs(const Rect& bounds) {
  auto batches = MakeSpringShapeRaw();
  StrokeInputBatch combined_batch = GetRealCombinedInputs(batches);
  BoundTestInputs(bounds, combined_batch);
  return combined_batch;
}

}  // namespace ink
