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

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/input/type_matchers.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

constexpr auto kPreserveDuration =
    StrokeInputBatch::TransformInvariant::kPreserveDuration;

std::vector<StrokeInput> MakeValidTestInputSequence(
    StrokeInput::ToolType tool_type = StrokeInput::ToolType::kStylus) {
  return {{.tool_type = tool_type,
           .position = {10, 20},
           .elapsed_time = Duration32::Seconds(5),
           .pressure = 0.4,
           .tilt = Angle::Radians(1.f),
           .orientation = Angle::Radians(2.f)},
          {.tool_type = tool_type,
           .position = {10, 23},
           .elapsed_time = Duration32::Seconds(6),
           .pressure = 0.3,
           .tilt = Angle::Radians(0.9f),
           .orientation = Angle::Radians(0.9f)},
          {.tool_type = tool_type,
           .position = {10, 23},
           .elapsed_time = Duration32::Seconds(7),
           .pressure = 0.5,
           .tilt = Angle::Radians(0.8),
           .orientation = Angle::Radians(1.1)},
          {.tool_type = tool_type,
           .position = {5, 5},
           .elapsed_time = Duration32::Seconds(8),
           .pressure = 0.8,
           .tilt = Angle::Radians(1.5),
           .orientation = Angle::Radians(1.3)},
          {.tool_type = tool_type,
           .position = {4, 3},
           .elapsed_time = Duration32::Seconds(9),
           .pressure = 1.0,
           .tilt = Angle::Radians(1.3),
           .orientation = Angle::Radians(1.5)}};
}

StrokeInput MakeValidTestInput(
    StrokeInput::ToolType tool_type = StrokeInput::ToolType::kStylus) {
  return {.tool_type = tool_type,
          .position = {10, 20},
          .elapsed_time = Duration32::Seconds(5),
          .pressure = 0.4,
          .tilt = Angle::Radians(1),
          .orientation = Angle::Radians(2)};
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationEmptyInputIdentityTransform) {
  AffineTransform transform;
  StrokeInputBatch batch;

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(StrokeInputBatch()));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationOneInputPointIdentityTransform) {
  AffineTransform transform;
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  StrokeInput input = MakeValidTestInput();
  ASSERT_EQ(absl::OkStatus(), batch.Append(input));
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(input));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest, PreserveDurationOneInputPointScaleX) {
  AffineTransform transform(10, 0, 0, 0, 1, 0);
  StrokeInputBatch batch;
  StrokeInput input = MakeValidTestInput();
  StrokeInputBatch expected_batch;
  ASSERT_EQ(absl::OkStatus(), batch.Append(input));
  input.position.x *= 10;
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(input));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationMultipleInputPointsIdentityTransform) {
  AffineTransform transform;
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest, PreserveDurationMultipleInputPointsScaleX) {
  AffineTransform transform(10, 0, 0, 0, 1, 0);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.x *= 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest, PreserveDurationMultipleInputPointsScaleY) {
  AffineTransform transform(1, 0, 0, 0, 10, 0);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.y *= 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationMultipleInputPointsScaleXY) {
  AffineTransform transform(5, 0, 0, 0, 10, 0);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.x *= 5;
    inputs[i].position.y *= 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationMultipleInputPointsMovePositiveX) {
  AffineTransform transform(1, 0, 10, 0, 1, 0);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.x += 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationMultipleInputPointsMoveNegativeX) {
  AffineTransform transform(1, 0, -10, 0, 1, 0);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.x -= 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

TEST(StrokeInputBatchTransformTest,
     PreserveDurationMultipleInputPointsMovePositiveY) {
  AffineTransform transform(1, 0, 0, 0, 1, 10);
  StrokeInputBatch batch;
  StrokeInputBatch expected_batch;
  std::vector<StrokeInput> inputs = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(inputs));
  for (int i = 0; i < inputs.size(); ++i) {
    inputs[i].position.y += 10;
  }
  ASSERT_EQ(absl::OkStatus(), expected_batch.Append(inputs));

  batch.Transform(transform, kPreserveDuration);

  EXPECT_THAT(batch, StrokeInputBatchEq(expected_batch));
}

}  // namespace
}  // namespace ink
