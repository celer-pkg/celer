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

#include <cstddef>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"

namespace ink {
namespace {

Envelope GetEnvelope(const StrokeInputBatch& inputs) {
  Envelope envelope;
  for (const StrokeInput& input : inputs) {
    envelope.Add(input.position);
  }
  return envelope;
}

Envelope GetEnvelope(
    const std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& input) {
  Envelope envelope;
  for (const auto& [real, predicted] : input) {
    envelope.Add(GetEnvelope(real));
    envelope.Add(GetEnvelope(predicted));
  }
  return envelope;
}

TEST(RecordedTestInputsTest, MakeIncrementalStraightLineInputsHasPrediction) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {1, 1});
  auto straight_line_incremental = MakeIncrementalStraightLineInputs(bounds);

  // Make sure we have some predicted inputs
  bool has_predicted_inputs = false;
  for (const auto& [real, predicted] : straight_line_incremental) {
    if (predicted.Size() > 0) {
      has_predicted_inputs = true;
      break;
    }
  }

  EXPECT_TRUE(has_predicted_inputs);
}

TEST(RecordedTestInputsTest, MakeIncrementalStraightLineInputsRespectsBounds) {
  Rect bounds = Rect::FromTwoPoints({0, 1}, {2, 3});
  auto straight_line_incremental = MakeIncrementalStraightLineInputs(bounds);
  auto envelope = GetEnvelope(straight_line_incremental);

  EXPECT_THAT(envelope, EnvelopeNear(bounds, 0.001f));
}

TEST(RecordedTestInputsTest, MakeCompleteStraightLineInputs) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {1, 1});
  StrokeInputBatch straight_line_complete =
      MakeCompleteStraightLineInputs(bounds);

  auto straight_line_incremental = MakeIncrementalStraightLineInputs(bounds);
  size_t points = 0;
  for (const auto& [real, predicted] : straight_line_incremental) {
    points += real.Size();
  }

  EXPECT_EQ(straight_line_complete.Size(), points);
}

TEST(RecordedTestInputsTest, MakeCompleteStraightLineInputsRespectsBounds) {
  Rect bounds = Rect::FromTwoPoints({0, 1}, {2, 3});
  auto straight_line_complete = MakeCompleteStraightLineInputs(bounds);
  auto envelope = GetEnvelope(straight_line_complete);

  EXPECT_THAT(envelope, EnvelopeNear(bounds, 0.001f));
}

TEST(RecordedTestInputsTest, MakeIncrementalSpringShapeInputsHasPrediction) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {1, 1});
  auto spiral_incremental = MakeIncrementalSpringShapeInputs(bounds);

  // Make sure we have some predicted inputs
  bool has_predicted_inputs = false;
  for (const auto& [real, predicted] : spiral_incremental) {
    if (predicted.Size() > 0) {
      has_predicted_inputs = true;
      break;
    }
  }

  EXPECT_TRUE(has_predicted_inputs);
}

TEST(RecordedTestInputsTest, MakeIncrementalSpringShapeInputsRespectsBounds) {
  Rect bounds = Rect::FromTwoPoints({0, 1}, {2, 3});
  auto spiral_incremental = MakeIncrementalSpringShapeInputs(bounds);
  auto envelope = GetEnvelope(spiral_incremental);

  EXPECT_THAT(envelope, EnvelopeNear(bounds, 0.001f));
}

TEST(RecordedTestInputsTest, MakeCompleteSpringShapeInputs) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {1, 1});
  StrokeInputBatch spiral_complete = MakeCompleteSpringShapeInputs(bounds);

  auto spiral_incremental = MakeIncrementalSpringShapeInputs(bounds);
  size_t points = 0;
  for (const auto& [real, predicted] : spiral_incremental) {
    points += real.Size();
  }

  EXPECT_EQ(spiral_complete.Size(), points);
}

TEST(RecordedTestInputsTest, MakeCompleteSpringShapeInputsRespectsBounds) {
  Rect bounds = Rect::FromTwoPoints({0, 1}, {2, 3});
  StrokeInputBatch spiral_complete = MakeCompleteSpringShapeInputs(bounds);
  auto envelope = GetEnvelope(spiral_complete);

  EXPECT_THAT(envelope, EnvelopeNear(bounds, 0.001f));
}

}  // namespace
}  // namespace ink
