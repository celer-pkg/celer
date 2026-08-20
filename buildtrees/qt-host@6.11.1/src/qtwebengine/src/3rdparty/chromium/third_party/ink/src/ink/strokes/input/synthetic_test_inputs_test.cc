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

#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

using ::testing::Optional;

Envelope CalculateEnvelope(const StrokeInputBatch& batch) {
  Envelope envelope;
  for (const StrokeInput& input : batch) {
    envelope.Add(input.position);
  }
  return envelope;
}

TEST(InputGeneratorsTest, CompleteLissajousCurveRespectsDuration) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {1, 1});

  EXPECT_FLOAT_EQ(MakeCompleteLissajousCurveInputs(Duration32::Zero(), bounds)
                      .GetDuration()
                      .ToSeconds(),
                  0);

  EXPECT_FLOAT_EQ(
      MakeCompleteLissajousCurveInputs(Duration32::Seconds(5), bounds)
          .GetDuration()
          .ToSeconds(),
      5);
}

TEST(InputGeneratorsTest, CompleteLissajousCurveRespectsBounds) {
  Rect bounds = Rect::FromTwoPoints({0, 1}, {2, 3});
  EXPECT_THAT(CalculateEnvelope(MakeCompleteLissajousCurveInputs(
                                    Duration32::Seconds(1), bounds))
                  .AsRect(),
              Optional(RectNear(bounds, 0.001f)));

  bounds = Rect::FromTwoPoints({2, 1}, {2, 3});
  EXPECT_THAT(CalculateEnvelope(MakeCompleteLissajousCurveInputs(
                                    Duration32::Seconds(1), bounds))
                  .AsRect(),
              Optional(RectNear(bounds, 0.001f)));

  bounds = Rect::FromTwoPoints({0, 3}, {2, 3});
  EXPECT_THAT(CalculateEnvelope(MakeCompleteLissajousCurveInputs(
                                    Duration32::Seconds(1), bounds))
                  .AsRect(),
              Optional(RectNear(bounds, 0.001f)));

  bounds = Rect::FromTwoPoints({3, 3}, {3, 3});
  EXPECT_THAT(CalculateEnvelope(MakeCompleteLissajousCurveInputs(
                                    Duration32::Seconds(1), bounds))
                  .AsRect(),
              Optional(RectNear(bounds, 0.001f)));
}

TEST(InputGeneratorsDeathTest, CompleteLissajousCurveInfiniteDuration) {
  EXPECT_DEATH_IF_SUPPORTED(
      MakeCompleteLissajousCurveInputs(Duration32::Infinite(),
                                       Rect::FromTwoPoints({0, 0}, {1, 1})),
      "");
}

TEST(InputGeneratorsDeathTest, CompleteLissajousCurveInfiniteBounds) {
  constexpr float kInf = std::numeric_limits<float>::infinity();
  EXPECT_DEATH_IF_SUPPORTED(
      MakeCompleteLissajousCurveInputs(
          Duration32::Seconds(1),
          Rect::FromTwoPoints({-kInf, -kInf}, {kInf, kInf})),
      "");
}

}  // namespace
}  // namespace ink
