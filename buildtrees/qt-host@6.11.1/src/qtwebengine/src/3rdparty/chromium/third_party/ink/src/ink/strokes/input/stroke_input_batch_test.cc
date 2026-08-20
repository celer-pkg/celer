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

#include "ink/strokes/input/stroke_input_batch.h"

#include <cmath>
#include <limits>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/strokes/input/fuzz_domains.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/type_matchers.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {
namespace {

using ::testing::HasSubstr;

std::vector<StrokeInput> MakeValidTestInputSequence(
    StrokeInput::ToolType tool_type = StrokeInput::ToolType::kStylus) {
  return {{.tool_type = tool_type,
           .position = {10, 20},
           .elapsed_time = Duration32::Seconds(5),
           .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
           .pressure = 0.4,
           .tilt = Angle::Radians(1.f),
           .orientation = Angle::Radians(2.f)},
          {.tool_type = tool_type,
           .position = {10, 23},
           .elapsed_time = Duration32::Seconds(6),
           .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
           .pressure = 0.3,
           .tilt = Angle::Radians(0.9f),
           .orientation = Angle::Radians(0.9f)},
          {.tool_type = tool_type,
           .position = {10, 23},
           .elapsed_time = Duration32::Seconds(7),
           .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
           .pressure = 0.5,
           .tilt = Angle::Radians(0.8),
           .orientation = Angle::Radians(1.1)},
          {.tool_type = tool_type,
           .position = {5, 5},
           .elapsed_time = Duration32::Seconds(8),
           .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
           .pressure = 0.8,
           .tilt = Angle::Radians(1.5),
           .orientation = Angle::Radians(1.3)},
          {.tool_type = tool_type,
           .position = {4, 3},
           .elapsed_time = Duration32::Seconds(9),
           .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
           .pressure = 1.0,
           .tilt = Angle::Radians(1.3),
           .orientation = Angle::Radians(1.5)}};
}

StrokeInput MakeValidTestInput(
    StrokeInput::ToolType tool_type = StrokeInput::ToolType::kStylus) {
  return {.tool_type = tool_type,
          .position = {10, 20},
          .elapsed_time = Duration32::Seconds(5),
          .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
          .pressure = 0.4,
          .tilt = Angle::Radians(1),
          .orientation = Angle::Radians(2)};
}

TEST(StrokeInputBatchTest, Stringify) {
  StrokeInputBatch batch;
  EXPECT_EQ(absl::StrCat(batch), "StrokeInputBatch[]");
  ASSERT_EQ(absl::OkStatus(),
            batch.Append(
                {.position = {1, 2}, .elapsed_time = Duration32::Seconds(1)}));
  EXPECT_EQ(absl::StrCat(batch),
            "StrokeInputBatch[StrokeInput[Unknown, (1, 2), 1s]]");
  ASSERT_EQ(absl::OkStatus(),
            batch.Append(
                {.position = {3, 4}, .elapsed_time = Duration32::Seconds(2)}));
  EXPECT_EQ(absl::StrCat(batch),
            "StrokeInputBatch[StrokeInput[Unknown, (1, 2), 1s],"
            " StrokeInput[Unknown, (3, 4), 2s]]");
}

TEST(StrokeInputBatchTest, DefaultConstructed) {
  StrokeInputBatch batch;
  EXPECT_EQ(batch.Size(), 0);
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
  EXPECT_FALSE(batch.HasStrokeUnitLength());
  EXPECT_FALSE(batch.HasPressure());
  EXPECT_FALSE(batch.HasTilt());
  EXPECT_FALSE(batch.HasOrientation());
}

TEST(StrokeInputBatchTest, CreateFromEmptySpan) {
  auto empty_batch = StrokeInputBatch::Create({});
  ASSERT_EQ(absl::OkStatus(), empty_batch.status());
  EXPECT_EQ(empty_batch->Size(), 0);
  EXPECT_TRUE(empty_batch->IsEmpty());
  EXPECT_FALSE(empty_batch->HasStrokeUnitLength());
  EXPECT_FALSE(empty_batch->HasPressure());
  EXPECT_FALSE(empty_batch->HasTilt());
  EXPECT_FALSE(empty_batch->HasOrientation());
  EXPECT_EQ(empty_batch->GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, CreateFromNonEmptySpan) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  auto batch = StrokeInputBatch::Create(input_vector);
  ASSERT_EQ(absl::OkStatus(), batch.status());
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(input_vector));
  EXPECT_EQ(batch->Size(), input_vector.size());
  EXPECT_FALSE(batch->IsEmpty());
  EXPECT_TRUE(batch->HasStrokeUnitLength());
  EXPECT_TRUE(batch->HasPressure());
  EXPECT_TRUE(batch->HasTilt());
  EXPECT_TRUE(batch->HasOrientation());
  EXPECT_EQ(batch->GetToolType(), input_vector.front().tool_type);
}

TEST(StrokeInputBatchTest, AppendOneToEmpty) {
  StrokeInputBatch batch;

  StrokeInput input = MakeValidTestInput();
  EXPECT_EQ(absl::OkStatus(), batch.Append(input));

  EXPECT_EQ(batch.Size(), 1);
  EXPECT_FALSE(batch.IsEmpty());
  EXPECT_TRUE(batch.HasStrokeUnitLength());
  EXPECT_TRUE(batch.HasPressure());
  EXPECT_TRUE(batch.HasTilt());
  EXPECT_TRUE(batch.HasOrientation());
  EXPECT_THAT(batch, StrokeInputBatchIsArray({input}));
}

TEST(StrokeInputBatchTest, AppendOneToNonEmpty) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
      absl::MakeSpan(&input_vector[0], input_vector.size() - 1));
  ASSERT_EQ(batch.status(), absl::OkStatus());

  EXPECT_EQ(absl::OkStatus(), batch->Append(input_vector.back()));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputBatchTest, AppendSpanToEmpty) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  StrokeInputBatch batch;
  EXPECT_EQ(absl::OkStatus(), batch.Append(input_vector));
  EXPECT_THAT(batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputBatchTest, AppendSpanToNonEmpty) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create({input_vector[0]});
  ASSERT_EQ(batch.status(), absl::OkStatus());

  EXPECT_EQ(absl::OkStatus(), batch->Append(absl::MakeSpan(
                                  &input_vector[1], input_vector.size() - 1)));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputBatchTest, AppendEmptyToEmpty) {
  StrokeInputBatch batch;

  // Create a vector of StrokeInput but don't add any values.
  std::vector<StrokeInput> input_vector;
  EXPECT_EQ(absl::OkStatus(), batch.Append(input_vector));
  EXPECT_EQ(batch.Size(), 0);
  EXPECT_TRUE(batch.IsEmpty());

  StrokeInputBatch input_batch;
  EXPECT_EQ(absl::OkStatus(), batch.Append(input_batch));
  EXPECT_EQ(batch.Size(), 0);
  EXPECT_TRUE(batch.IsEmpty());
}

TEST(StrokeInputBatchTest, AppendEmptyToNonEmpty) {
  StrokeInputBatch batch;

  StrokeInput input = MakeValidTestInput();
  ASSERT_EQ(absl::OkStatus(), batch.Append(input));

  EXPECT_EQ(absl::OkStatus(), batch.Append(absl::Span<const StrokeInput>()));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({input}));

  EXPECT_EQ(absl::OkStatus(), batch.Append(StrokeInputBatch()));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({input}));
}

TEST(StrokeInputBatchTest, SetReplacingOnlyExistingValue) {
  StrokeInputBatch batch;

  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(input_vector[0]));

  StrokeInput replacement = input_vector[1];
  EXPECT_EQ(absl::OkStatus(), batch.Set(0, replacement));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({replacement}));
}

TEST(StrokeInputBatchTest, Set) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
      {input_vector[0], input_vector[1], input_vector[3]});
  ASSERT_EQ(batch.status(), absl::OkStatus());

  EXPECT_EQ(absl::OkStatus(), batch->Set(1, input_vector[2]));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(
                          {input_vector[0], input_vector[2], input_vector[3]}));
}

TEST(StrokeInputBatchTest, SetReplacingOnlyValueWithDifferentFormat) {
  StrokeInput input = MakeValidTestInput(StrokeInput::ToolType::kStylus);
  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create({input});
  ASSERT_EQ(batch.status(), absl::OkStatus());

  StrokeInput replacement = input;
  replacement.tool_type = StrokeInput::ToolType::kMouse;
  replacement.tilt = StrokeInput::kNoTilt;

  EXPECT_EQ(absl::OkStatus(), batch->Set(0, replacement));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray({replacement}));
}

TEST(StrokeInputBatchTest, SetReplacingFirstValueOfMany) {
  std::vector<StrokeInput> input_vector =
      MakeValidTestInputSequence(StrokeInput::ToolType::kTouch);
  auto initial_inputs = absl::MakeSpan(&input_vector[1], 3);

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());

    StrokeInput replacement = input_vector[0];
    EXPECT_EQ(absl::OkStatus(), batch->Set(0, replacement));

    EXPECT_THAT(*batch, StrokeInputBatchIsArray({replacement, initial_inputs[1],
                                                 initial_inputs[2]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Attempt to replace first value with an incompatible input because it has
    // no orientation.
    StrokeInput replacement = input_vector[0];
    ASSERT_TRUE(replacement.HasOrientation());
    replacement.orientation = StrokeInput::kNoOrientation;
    absl::Status missing_orientation = batch->Set(0, replacement);
    EXPECT_EQ(missing_orientation.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(missing_orientation.message(), HasSubstr("orientation"));

    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
}

TEST(StrokeInputBatchTest, SetReplacingLastValueOfMany) {
  std::vector<StrokeInput> input_vector =
      MakeValidTestInputSequence(StrokeInput::ToolType::kStylus);
  auto initial_inputs = absl::MakeSpan(&input_vector[0], 3);

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());

    StrokeInput replacement = input_vector[initial_inputs.size()];
    EXPECT_EQ(absl::OkStatus(), batch->Set(batch->Size() - 1, replacement));

    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray(
                    {initial_inputs[0], initial_inputs[1], replacement}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Attempt to replace first value with an incompatible input because it has
    // a different tool type.
    StrokeInput replacement = input_vector[initial_inputs.size()];
    replacement.tool_type = StrokeInput::ToolType::kMouse;

    absl::Status wrong_tool_type = batch->Set(2, replacement);
    EXPECT_EQ(wrong_tool_type.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(wrong_tool_type.message(), HasSubstr("tool_type"));

    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
}

TEST(StrokeInputBatchTest, Clear) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(input_vector, /*noise_seed=*/12345);
  ASSERT_EQ(batch.status(), absl::OkStatus());

  ASSERT_FALSE(batch->IsEmpty());
  ASSERT_EQ(batch->Size(), input_vector.size());
  ASSERT_TRUE(batch->HasPressure());
  ASSERT_TRUE(batch->HasTilt());
  ASSERT_TRUE(batch->HasOrientation());
  ASSERT_EQ(batch->GetToolType(), StrokeInput::ToolType::kStylus);
  EXPECT_EQ(batch->GetNoiseSeed(), 12345u);

  batch->Clear();
  // Batch should now be empty and the tool type should be unknown.
  EXPECT_TRUE(batch->IsEmpty());
  EXPECT_EQ(batch->Size(), 0);
  ASSERT_FALSE(batch->HasPressure());
  ASSERT_FALSE(batch->HasTilt());
  ASSERT_FALSE(batch->HasOrientation());
  EXPECT_EQ(batch->GetToolType(), StrokeInput::ToolType::kUnknown);
  EXPECT_EQ(batch->GetNoiseSeed(), 0u);
}

TEST(StrokeInputBatchTest, AppendAfterClear) {
  std::vector<StrokeInput> input_vector =
      MakeValidTestInputSequence(StrokeInput::ToolType::kStylus);
  auto initial_inputs = absl::MakeSpan(&input_vector[1], 3);

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_FALSE(batch->IsEmpty());

    // Clear and then append with same format
    batch->Clear();
    StrokeInput input = input_vector[0];
    EXPECT_EQ(absl::OkStatus(), batch->Append(input));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input}));
    EXPECT_EQ(batch->GetToolType(), StrokeInput::ToolType::kStylus);
    EXPECT_TRUE(batch->HasPressure());
    EXPECT_TRUE(batch->HasTilt());
    EXPECT_TRUE(batch->HasOrientation());
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(input_vector);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_FALSE(batch->IsEmpty());

    // Clear and then append with a different format
    batch->Clear();

    StrokeInput input = input_vector[0];
    input.tool_type = StrokeInput::ToolType::kMouse;
    input.tilt = StrokeInput::kNoTilt;
    EXPECT_EQ(absl::OkStatus(), batch->Append(input));

    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input}));
    EXPECT_EQ(batch->GetToolType(), StrokeInput::ToolType::kMouse);
    EXPECT_TRUE(batch->HasPressure());
    EXPECT_FALSE(batch->HasTilt());
    EXPECT_TRUE(batch->HasOrientation());
  }
}

TEST(StrokeInputBatchTest, ClearOnEmpty) {
  StrokeInputBatch batch;
  batch.Clear();
  EXPECT_TRUE(batch.IsEmpty());
}

TEST(StrokeInputBatchTest, ClearAfterCopy) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(input_vector);
  ASSERT_EQ(batch.status(), absl::OkStatus());

  StrokeInputBatch copied_batch = *batch;
  EXPECT_THAT(copied_batch, StrokeInputBatchEq(*batch));

  batch->Clear();
  EXPECT_TRUE(batch->IsEmpty());
  EXPECT_THAT(copied_batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputTest, HasOptionalFields) {
  {
    StrokeInputBatch batch;

    // Test HasPressure returns true when first value has pressure
    StrokeInput input = MakeValidTestInput();
    ASSERT_TRUE(input.HasPressure());

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_TRUE(batch.HasPressure());
  }
  {
    StrokeInputBatch batch;

    // Test HasPressure returns false when first value doesn't have pressure
    StrokeInput input = MakeValidTestInput();
    input.pressure = -1;

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_FALSE(batch.HasPressure());
  }
  {
    StrokeInputBatch batch;

    // Test HasTilt returns true when first value has pressure
    StrokeInput input = MakeValidTestInput();
    ASSERT_TRUE(input.HasTilt());

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_TRUE(batch.HasTilt());
  }
  {
    StrokeInputBatch batch;

    // Test HasTilt returns false when first value doesn't have pressure
    StrokeInput input = MakeValidTestInput();
    input.tilt = StrokeInput::kNoTilt;

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_FALSE(batch.HasTilt());
  }
  {
    StrokeInputBatch batch;

    // Test HasOrientation returns true when first value has pressure
    StrokeInput input = MakeValidTestInput();
    ASSERT_TRUE(input.HasOrientation());

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_TRUE(batch.HasOrientation());
  }
  {
    StrokeInputBatch batch;

    // Test HasOrientation returns false when first value doesn't have pressure
    StrokeInput input = MakeValidTestInput();
    input.orientation = StrokeInput::kNoOrientation;

    EXPECT_EQ(absl::OkStatus(), batch.Append(input));
    EXPECT_FALSE(batch.HasOrientation());
  }
}

TEST(StrokeInputBatchTest, StrokeUnitLengthBelowValidRange) {
  StrokeInputBatch batch;
  StrokeInput input = MakeValidTestInput();
  input.stroke_unit_length = PhysicalDistance::Centimeters(-1);
  absl::Status stroke_unit_length_too_low = batch.Append(input);
  EXPECT_EQ(stroke_unit_length_too_low.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(stroke_unit_length_too_low.message(),
              HasSubstr("stroke_unit_length"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
  EXPECT_FALSE(batch.HasStrokeUnitLength());
}

TEST(StrokeInputBatchTest, PressureAboveValidRange) {
  StrokeInputBatch batch;
  // Valid pressure should be in the range [0, 1], setting to 2.
  StrokeInput input = MakeValidTestInput();
  input.pressure = 2;
  absl::Status pressure_too_high = batch.Append(input);
  EXPECT_EQ(pressure_too_high.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(pressure_too_high.message(), HasSubstr("pressure"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, PressureBelowValidRange) {
  StrokeInputBatch batch;
  // Valid pressure should be in the range [0, 1], setting to -0.5.
  StrokeInput input = MakeValidTestInput();
  input.pressure = -0.5;

  absl::Status pressure_too_low = batch.Append(input);
  EXPECT_EQ(pressure_too_low.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(pressure_too_low.message(), HasSubstr("pressure"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, TiltAboveValidRange) {
  StrokeInputBatch batch;
  // Valid tilt should be in the range [0, π/2], setting to 4.
  StrokeInput input = MakeValidTestInput();
  input.tilt = Angle::Radians(4);

  absl::Status tilt_too_high = batch.Append(input);
  EXPECT_EQ(tilt_too_high.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(tilt_too_high.message(), HasSubstr("tilt"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, TiltBelowValidRange) {
  StrokeInputBatch batch;
  // Valid tilt should be in the range [0, π/2], setting to -2.
  StrokeInput input = MakeValidTestInput();
  input.tilt = Angle::Radians(-2);

  absl::Status tilt_too_low = batch.Append(input);
  EXPECT_EQ(tilt_too_low.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(tilt_too_low.message(), HasSubstr("tilt"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, OrientationAboveValidRange) {
  StrokeInputBatch batch;
  // Valid orientation should be in the range [0, 2π), setting to 10.
  StrokeInput input = MakeValidTestInput();
  input.orientation = Angle::Radians(10.f);

  absl::Status orientation_too_high = batch.Append(input);
  EXPECT_EQ(orientation_too_high.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(orientation_too_high.message(), HasSubstr("orientation"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, OrientationBelowValidRange) {
  StrokeInputBatch batch;
  // Valid orientation should be in the range [0, 2π), setting to -3.
  StrokeInput input = MakeValidTestInput();
  input.orientation = Angle::Radians(-3.f);

  absl::Status orientation_too_low = batch.Append(input);
  EXPECT_EQ(orientation_too_low.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(orientation_too_low.message(), HasSubstr("orientation"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, ToolTypeNonEnumeratorValue) {
  StrokeInputBatch batch;
  // Valid tool type must be one of the enumerator values.
  StrokeInput input = MakeValidTestInput();
  input.tool_type = static_cast<StrokeInput::ToolType>(99);

  absl::Status invalid_tool_type = batch.Append(input);
  EXPECT_EQ(invalid_tool_type.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(invalid_tool_type.message(), HasSubstr("tool_type"));
  EXPECT_TRUE(batch.IsEmpty());
  EXPECT_EQ(batch.GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, AppendWithOnlyXChange) {
  StrokeInputBatch batch;
  StrokeInput first = MakeValidTestInput();

  ASSERT_EQ(absl::OkStatus(), batch.Append(first));
  // Make sure that we still pass if at least one of xyt changes.
  StrokeInput second = first;
  second.position.x += 1;
  EXPECT_EQ(absl::OkStatus(), batch.Append(second));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({first, second}));
}

TEST(StrokeInputBatchTest, AppendWithOnlyYChange) {
  StrokeInputBatch batch;
  StrokeInput first = MakeValidTestInput();

  ASSERT_EQ(absl::OkStatus(), batch.Append(first));
  // Make sure that we still pass if at least one of xyt changes.
  StrokeInput second = first;
  second.position.y += 1;
  EXPECT_EQ(absl::OkStatus(), batch.Append(second));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({first, second}));
}

TEST(StrokeInputBatchTest, AppendWithOnlyTChange) {
  StrokeInputBatch batch;
  StrokeInput first = MakeValidTestInput();

  ASSERT_EQ(absl::OkStatus(), batch.Append(first));
  // Make sure that we still pass if at least one of xyt changes.
  StrokeInput second = first;
  second.elapsed_time = first.elapsed_time + Duration32::Seconds(1);
  EXPECT_EQ(absl::OkStatus(), batch.Append(second));
  EXPECT_THAT(batch, StrokeInputBatchIsArray({first, second}));
}

void CanAppendAnyValidStrokeInputToAnEmptyBatch(const StrokeInput& input) {
  StrokeInputBatch batch;
  EXPECT_EQ(absl::OkStatus(), batch.Append(input));
  EXPECT_EQ(batch.Size(), 1);
  EXPECT_THAT(batch.Get(0), StrokeInputEq(input));
}
FUZZ_TEST(StrokeInputBatchTest, CanAppendAnyValidStrokeInputToAnEmptyBatch)
    .WithDomains(ValidStrokeInput());

TEST(StrokeInputBatchTest, SetNonFiniteValueOnSingleInputBatch) {
  StrokeInput input = MakeValidTestInput();

  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create({input});
  ASSERT_EQ(batch.status(), absl::OkStatus());
  ASSERT_THAT(*batch, StrokeInputBatchIsArray({input}));

  absl::Status non_finite =
      batch->Set(0, {.position = {std::numeric_limits<float>::infinity(), 0}});
  EXPECT_EQ(non_finite.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(non_finite.message(), HasSubstr("must be finite"));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray({input}));
}

TEST(StrokeInputBatchTest, SetCausingDecreasingTime) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  auto initial_inputs = absl::MakeSpan(&input_vector[1], 3);

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_GT(input_vector[4].elapsed_time,
              batch->Get(batch->Size() - 1).elapsed_time);

    absl::Status decreating_time = batch->Set(1, input_vector[4]);
    EXPECT_EQ(decreating_time.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(decreating_time.message(), HasSubstr("elapsed_time"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_LT(input_vector[0].elapsed_time, batch->Get(0).elapsed_time);

    absl::Status decreasing_time = batch->Set(1, input_vector[0]);
    EXPECT_EQ(decreasing_time.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(decreasing_time.message(), HasSubstr("elapsed_time"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
}

TEST(StrokeInputBatchTest, AppendDecreasingTime) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
      {input_vector[0], input_vector[1], input_vector[3]});
  ASSERT_EQ(batch.status(), absl::OkStatus());

  ASSERT_GT(batch->Get(batch->Size() - 1).elapsed_time,
            input_vector[2].elapsed_time);
  absl::Status decreasing_time = batch->Append(input_vector[2]);
  EXPECT_EQ(decreasing_time.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(decreasing_time.message(), HasSubstr("elapsed_time"));
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(
                          {input_vector[0], input_vector[1], input_vector[3]}));
}

TEST(StrokeInputBatchTest, NonFiniteValues) {
  {
    // Passing in an infinite value for x should fail.
    StrokeInput input = MakeValidTestInput();
    input.position.x = std::numeric_limits<float>::infinity();

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in a NaN value for x should fail.
    StrokeInput input = MakeValidTestInput();
    input.position.x = std::nanf("");

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
    }
    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in an infinite value for y should fail.
    StrokeInput input = MakeValidTestInput();
    input.position.y = std::numeric_limits<float>::infinity();

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in a NaN value for y should fail.
    StrokeInput input = MakeValidTestInput();
    input.position.y = std::nanf("");

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }
    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("StrokeInput::position"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in an infinite value for elapsed_time should fail.
    StrokeInput input = MakeValidTestInput();
    input.elapsed_time = Duration32::Infinite();

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("elapsed_time"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("elapsed_time"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("elapsed_time"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in an infinite value for stroke_unit_length should fail.
    StrokeInput input = MakeValidTestInput();
    input.stroke_unit_length =
        PhysicalDistance::Centimeters(std::numeric_limits<float>::infinity());

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("stroke_unit_length"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("stroke_unit_length"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("stroke_unit_length"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    StrokeInputBatch batch;
    // Passing in a NaN value for stroke_unit_length should fail.
    StrokeInput input = MakeValidTestInput();
    input.stroke_unit_length = PhysicalDistance::Centimeters(std::nanf(""));
    absl::Status status = batch.Append(input);
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("stroke_unit_length"));
    EXPECT_TRUE(batch.IsEmpty());
  }
  {
    // Passing in an infinite value for pressure should fail.
    StrokeInput input = MakeValidTestInput();
    input.pressure = std::numeric_limits<float>::infinity();

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("pressure"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("pressure"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("pressure"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    StrokeInputBatch batch;
    // Passing in a NaN value for pressure should fail.
    StrokeInput input = MakeValidTestInput();
    input.pressure = std::nanf("");
    absl::Status status = batch.Append(input);
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pressure"));
    EXPECT_TRUE(batch.IsEmpty());
  }
  {
    // Passing in an infinite value for tilt should fail.
    StrokeInput input = MakeValidTestInput();
    input.tilt = Angle::Radians(std::numeric_limits<float>::infinity());

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in a NaN value for tilt should fail.
    StrokeInput input = MakeValidTestInput();
    input.tilt = Angle::Radians(std::nanf(""));

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("tilt"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in an infinite value for tilt should fail.
    StrokeInput input = MakeValidTestInput();
    input.orientation = Angle::Radians(std::numeric_limits<float>::infinity());

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
  {
    // Passing in a NaN value for orientation should fail.
    StrokeInput input = MakeValidTestInput();
    input.orientation = Angle::Radians(std::nanf(""));

    {
      absl::Status status = StrokeInputBatch::Create({input}).status();
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
    }

    StrokeInputBatch batch;
    {
      absl::Status status = batch.Append(input);
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
      EXPECT_TRUE(batch.IsEmpty());
    }

    {
      absl::Status status = batch.Append({input});
      EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(status.message(), HasSubstr("orientation"));
      EXPECT_TRUE(batch.IsEmpty());
    }
  }
}

TEST(StrokeInputBatchTest, AppendSpanWithChangedFormatFails) {
  {
    std::vector<StrokeInput> input_vector =
        MakeValidTestInputSequence(StrokeInput::ToolType::kMouse);
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has a different tool type:
    StrokeInput changed_tool_type = input_vector[1];
    changed_tool_type.tool_type = StrokeInput::ToolType::kTouch;

    absl::Status mismatched_tool_type = batch->Append(changed_tool_type);
    EXPECT_EQ(mismatched_tool_type.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_tool_type.message(), HasSubstr("tool_type"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has a different stroke_unit_length:
    StrokeInput changed_stroke_unit_length = input_vector[1];
    changed_stroke_unit_length.stroke_unit_length +=
        PhysicalDistance::Centimeters(0.1);

    absl::Status mismatched_stroke_unit_length =
        batch->Append(changed_stroke_unit_length);
    EXPECT_EQ(mismatched_stroke_unit_length.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_stroke_unit_length.message(),
                HasSubstr("stroke_unit_length"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    // Original input has pressure set
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    ASSERT_TRUE(input_vector[0].HasPressure());
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has no value for pressure, append should fail.
    StrokeInput no_pressure = input_vector[1];
    no_pressure.pressure = -1;

    absl::Status mismatched_has_pressure = batch->Append(no_pressure);
    EXPECT_EQ(mismatched_has_pressure.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_pressure.message(), HasSubstr("pressure"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    input_vector[0].pressure = -1;
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has a value for pressure, append should fail.
    StrokeInput with_pressure = input_vector[1];
    ASSERT_TRUE(with_pressure.HasPressure());

    absl::Status mismatched_has_pressure = batch->Append(with_pressure);
    EXPECT_EQ(mismatched_has_pressure.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_pressure.message(), HasSubstr("pressure"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    // Original input has tilt set
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    ASSERT_TRUE(input_vector[0].HasTilt());
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has no value for tilt, append should fail.
    StrokeInput no_tilt = input_vector[1];
    no_tilt.tilt = StrokeInput::kNoTilt;

    absl::Status mismatched_has_tilt = batch->Append(no_tilt);
    EXPECT_EQ(mismatched_has_tilt.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_tilt.message(), HasSubstr("tilt"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    input_vector[0].tilt = StrokeInput::kNoTilt;
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has a value for tilt, append should fail.
    StrokeInput with_tilt = input_vector[1];
    ASSERT_TRUE(with_tilt.HasTilt());

    absl::Status mismatched_has_tilt = batch->Append(with_tilt);
    EXPECT_EQ(mismatched_has_tilt.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_tilt.message(), HasSubstr("tilt"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    // Original input has orientation set
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    ASSERT_TRUE(input_vector[0].HasOrientation());
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has no value for orientation, append should fail.
    StrokeInput no_orientation = input_vector[1];
    no_orientation.orientation = StrokeInput::kNoOrientation;

    absl::Status mismatched_has_orientation = batch->Append(no_orientation);
    EXPECT_EQ(mismatched_has_orientation.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_orientation.message(), HasSubstr("orientation"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
  {
    // Original input doesn't have orientation set
    std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
    input_vector[0].orientation = StrokeInput::kNoOrientation;
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Next input has a value for orientation, append should fail.
    StrokeInput with_orientation = input_vector[1];
    ASSERT_TRUE(with_orientation.HasOrientation());

    absl::Status mismatched_has_orientation = batch->Append(with_orientation);
    EXPECT_EQ(mismatched_has_orientation.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_orientation.message(), HasSubstr("orientation"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({input_vector[0]}));
  }
}

TEST(StrokeInputBatchTest, AppendRepeatedPositionAndTime) {
  StrokeInputBatch batch;

  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  ASSERT_EQ(absl::OkStatus(), batch.Append(input_vector));

  // Adding the last input again, thus repeating an xyt triplet should fail.
  absl::Status duplicate_input = batch.Append(input_vector.back());
  EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
  EXPECT_THAT(batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputBatchTest, SetCausingRepeatedPositionAndTime) {
  StrokeInputBatch batch;
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  ASSERT_EQ(absl::OkStatus(), batch.Append(input_vector));
  // Replacing the second value with the third or first would create a
  // duplicated triplet and thus should fail.
  {
    absl::Status duplicate_input = batch.Set(1, input_vector[2]);
    EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
    EXPECT_THAT(batch, StrokeInputBatchIsArray(input_vector));
  }
  {
    absl::Status duplicate_input = batch.Set(1, input_vector[0]);
    EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
    EXPECT_THAT(batch, StrokeInputBatchIsArray(input_vector));
  }
}

TEST(StrokeInputBatchTest, AppendInvalidSpan) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[1]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span where first StrokeInput repeats last existing x, y,
    // elapsed_time
    absl::Status duplicate_input =
        batch->Append({input_vector[1], input_vector[1], input_vector[2]});
    EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[1]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[1]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span where two StrokeInputBatch repeat x, y, elapsed_time
    absl::Status duplicate_input = batch->Append(
        {input_vector[2], input_vector[3], input_vector[3], input_vector[4]});
    EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[1]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[2]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span where first StrokeInput has decreasing elapsed_time to
    // existing last entry
    absl::Status decreasing_time =
        batch->Append({input_vector[1], input_vector[3], input_vector[4]});
    EXPECT_EQ(decreasing_time.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(decreasing_time.message(),
                HasSubstr("non-decreasing `elapsed_time`"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[2]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[1]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span containing decreasing `elapsed_time`.
    absl::Status decreasing_time =
        batch->Append({input_vector[3], input_vector[2], input_vector[4]});
    EXPECT_EQ(decreasing_time.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(decreasing_time.message(),
                HasSubstr("non-decreasing `elapsed_time`"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[1]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[1]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span with different optional fields from what is in the batch.
    StrokeInput no_pressure[2] = {input_vector[2], input_vector[3]};
    no_pressure[0].pressure = -1;
    no_pressure[1].pressure = -1;

    absl::Status status = batch->Append(no_pressure);
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("all or none"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[1]}));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create({input_vector[0], input_vector[1]});
    EXPECT_EQ(batch.status(), absl::OkStatus());
    // Append Span with inconsistent optional fields.
    StrokeInput no_pressure = input_vector[3];
    no_pressure.pressure = -1;

    absl::Status status =
        batch->Append({input_vector[2], no_pressure, input_vector[4]});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("all or none"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray({input_vector[0], input_vector[1]}));
  }
}

TEST(StrokeInputBatchTest, AppendBatch) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();

  absl::StatusOr<StrokeInputBatch> first_batch =
      StrokeInputBatch::Create(absl::MakeSpan(&input_vector[0], 3));
  ASSERT_EQ(first_batch.status(), absl::OkStatus());

  StrokeInputBatch batch;
  EXPECT_EQ(absl::OkStatus(), batch.Append(*first_batch));
  EXPECT_THAT(batch, StrokeInputBatchEq(*first_batch));
  EXPECT_THAT(batch,
              StrokeInputBatchIsArray(absl::MakeSpan(&input_vector[0], 3)));

  absl::StatusOr<StrokeInputBatch> second_batch = StrokeInputBatch::Create(
      absl::MakeSpan(&input_vector[3], input_vector.size() - 3));
  ASSERT_EQ(second_batch.status(), absl::OkStatus());

  EXPECT_EQ(absl::OkStatus(), batch.Append(*second_batch));
  EXPECT_THAT(batch, StrokeInputBatchIsArray(input_vector));
}

TEST(StrokeInputBatchTest, AppendIncompatibleBatch) {
  std::vector<StrokeInput> input_vector =
      MakeValidTestInputSequence(StrokeInput::ToolType::kTouch);
  auto initial_inputs = absl::MakeSpan(&input_vector[0], 3);

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Try to append a batch with a different tool type.
    StrokeInput changed_tool_type = input_vector[3];
    changed_tool_type.tool_type = StrokeInput::ToolType::kUnknown;
    absl::StatusOr<StrokeInputBatch> batch_to_append =
        StrokeInputBatch::Create({changed_tool_type});
    ASSERT_EQ(batch_to_append.status(), absl::OkStatus());

    absl::Status different_tool_type = batch->Append(*batch_to_append);
    EXPECT_EQ(different_tool_type.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(different_tool_type.message(), HasSubstr("tool_type"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_TRUE(batch->HasPressure());

    // Try to append a batch without pressure
    StrokeInput no_pressure = input_vector[3];
    no_pressure.pressure = -1;
    absl::StatusOr<StrokeInputBatch> batch_to_append =
        StrokeInputBatch::Create({no_pressure});
    ASSERT_EQ(batch_to_append.status(), absl::OkStatus());

    absl::Status mismatched_has_pressure = batch->Append(*batch_to_append);
    EXPECT_EQ(mismatched_has_pressure.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_pressure.message(), HasSubstr("all or none"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_TRUE(batch->HasTilt());

    // Try to append a batch without tilt
    StrokeInput no_tilt = input_vector[3];
    no_tilt.tilt = StrokeInput::kNoTilt;
    absl::StatusOr<StrokeInputBatch> batch_to_append =
        StrokeInputBatch::Create({no_tilt});
    ASSERT_EQ(batch_to_append.status(), absl::OkStatus());

    absl::Status mismatched_has_tilt = batch->Append(*batch_to_append);
    EXPECT_EQ(mismatched_has_tilt.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(mismatched_has_tilt.message(), HasSubstr("all or none"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(initial_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    ASSERT_TRUE(batch->HasOrientation());

    // Try to append a batch without orientation
    StrokeInput no_orientation = input_vector[3];
    no_orientation.orientation = StrokeInput::kNoOrientation;
    absl::StatusOr<StrokeInputBatch> batch_to_append =
        StrokeInputBatch::Create({no_orientation});
    ASSERT_EQ(batch_to_append.status(), absl::OkStatus());

    absl::Status status = batch->Append(*batch_to_append);
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("all or none"));
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(initial_inputs));
  }
  {
    absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
        {input_vector[0], input_vector[1], input_vector[3]});
    ASSERT_EQ(batch.status(), absl::OkStatus());

    // Try to append a batch with lower initial `elapsed_time`.
    absl::StatusOr<StrokeInputBatch> batch_to_append =
        StrokeInputBatch::Create({input_vector[2]});
    ASSERT_EQ(batch_to_append.status(), absl::OkStatus());

    absl::Status status = batch->Append(*batch_to_append);
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("non-decreasing `elapsed_time`"));
    EXPECT_THAT(*batch,
                StrokeInputBatchIsArray(
                    {input_vector[0], input_vector[1], input_vector[3]}));
  }
}

TEST(StrokeInputBatchTest, EraseWithZeroCount) {
  std::vector<StrokeInput> test_inputs = MakeValidTestInputSequence();
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(1, 0);
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(test_inputs));
}

TEST(StrokeInputBatchTest, EraseWithStartPlusCountLessThanSize) {
  std::vector<StrokeInput> test_inputs = MakeValidTestInputSequence();
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(1, 2);
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(
                          {test_inputs[0], test_inputs[3], test_inputs[4]}));
}

TEST(StrokeInputBatchTest, EraseWithStartPlusCountGreaterThanSize) {
  std::vector<StrokeInput> test_inputs = MakeValidTestInputSequence();
  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(test_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    batch->Erase(1, batch->Size());
    EXPECT_THAT(*batch, StrokeInputBatchIsArray({test_inputs[0]}));
  }

  {
    absl::StatusOr<StrokeInputBatch> batch =
        StrokeInputBatch::Create(test_inputs);
    ASSERT_EQ(batch.status(), absl::OkStatus());
    batch->Erase(3);
    EXPECT_THAT(*batch, StrokeInputBatchIsArray(
                            {test_inputs[0], test_inputs[1], test_inputs[2]}));
  }
}

TEST(StrokeInputBatchTest, EraseWithStartEqualToSize) {
  std::vector<StrokeInput> test_inputs = MakeValidTestInputSequence();
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());

  batch->Erase(test_inputs.size(), 0);
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(test_inputs));

  batch->Erase(test_inputs.size(), 4);
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(test_inputs));

  batch->Erase(test_inputs.size());
  EXPECT_THAT(*batch, StrokeInputBatchIsArray(test_inputs));
}

TEST(StrokeInputBatchTest, EraseAll) {
  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
      MakeValidTestInputSequence(StrokeInput::ToolType::kStylus));
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(0, batch->Size());
  EXPECT_TRUE(batch->IsEmpty());
  EXPECT_EQ(batch->GetToolType(), StrokeInput::ToolType::kUnknown);
}

TEST(StrokeInputBatchTest, EraseWithNoPressure) {
  std::vector<StrokeInput> test_inputs = {
      {.position = {10, 20},
       .elapsed_time = Duration32::Seconds(5),
       .pressure = StrokeInput::kNoPressure,
       .tilt = Angle::Radians(1),
       .orientation = Angle::Radians(2)},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(6),
       .pressure = StrokeInput::kNoPressure,
       .tilt = Angle::Radians(0.9),
       .orientation = Angle::Radians(0.9)},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(7),
       .pressure = StrokeInput::kNoPressure,
       .tilt = Angle::Radians(0.8),
       .orientation = Angle::Radians(1.1)}};
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(0, 1);
  EXPECT_THAT(*batch,
              StrokeInputBatchIsArray({test_inputs[1], test_inputs[2]}));
}

TEST(StrokeInputBatchTest, EraseWithNoTilt) {
  std::vector<StrokeInput> test_inputs = {
      {.position = {10, 20},
       .elapsed_time = Duration32::Seconds(5),
       .pressure = 0.4,
       .tilt = StrokeInput::kNoTilt,
       .orientation = Angle::Radians(2)},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(6),
       .pressure = 0.3,
       .tilt = StrokeInput::kNoTilt,
       .orientation = Angle::Radians(0.9)},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(7),
       .pressure = 0.5,
       .tilt = StrokeInput::kNoTilt,
       .orientation = Angle::Radians(1.1)},
  };
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(1, 1);
  EXPECT_THAT(*batch,
              StrokeInputBatchIsArray({test_inputs[0], test_inputs[2]}));
}

TEST(StrokeInputBatchTest, EraseWithNoOrientation) {
  std::vector<StrokeInput> test_inputs = {
      {.position = {10, 20},
       .elapsed_time = Duration32::Seconds(5),
       .pressure = 0.4,
       .tilt = Angle::Radians(1),
       .orientation = StrokeInput::kNoOrientation},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(6),
       .pressure = 0.3,
       .tilt = Angle::Radians(0.9),
       .orientation = StrokeInput::kNoOrientation},
      {.position = {10, 23},
       .elapsed_time = Duration32::Seconds(7),
       .pressure = 0.5,
       .tilt = Angle::Radians(0.8),
       .orientation = StrokeInput::kNoOrientation}};
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(test_inputs);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  batch->Erase(2, 1);
  EXPECT_THAT(*batch,
              StrokeInputBatchIsArray({test_inputs[0], test_inputs[1]}));
}

TEST(StrokeInputBatch, GetDurationOnEmptyInput) {
  StrokeInputBatch batch;
  EXPECT_EQ(batch.GetDuration(), Duration32::Zero());
}

TEST(StrokeInputBatch, GetDuration) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  auto batch = StrokeInputBatch::Create(input_vector);
  ASSERT_EQ(absl::OkStatus(), batch.status());

  EXPECT_EQ(batch->GetDuration(), input_vector.back().elapsed_time -
                                      input_vector.front().elapsed_time);
}

TEST(StrokeInputBatch, DeepCopy) {
  std::vector<StrokeInput> input_vector = MakeValidTestInputSequence();
  auto batch = StrokeInputBatch::Create(input_vector);
  ASSERT_EQ(absl::OkStatus(), batch.status());
  // Make the copy
  StrokeInputBatch copied_batch = batch->MakeDeepCopy();

  // Initially the two batches contain the same thing
  EXPECT_THAT(copied_batch, StrokeInputBatchEq(*batch));

  // Empty the original batch
  batch->Clear();

  // Clearing the original should not have changed the copied batch
  EXPECT_THAT(copied_batch, Not(StrokeInputBatchEq(*batch)));
  EXPECT_THAT(copied_batch, StrokeInputBatchIsArray(input_vector));

  // Adding another item to the copied batch
  ASSERT_EQ(absl::OkStatus(),
            copied_batch.Append(
                {.tool_type = StrokeInput::ToolType::kStylus,
                 .position = {4, 3.2},
                 .elapsed_time = Duration32::Seconds(10),
                 .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
                 .pressure = 1.0,
                 .tilt = Angle::Radians(1.3),
                 .orientation = Angle::Radians(1.5)}));

  // The original batch should still be empty
  EXPECT_TRUE(batch->IsEmpty());
}

TEST(StrokeInputBatchDeathTest, SetWithIndexOutOfBounds) {
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(MakeValidTestInputSequence());
  ASSERT_EQ(batch.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(auto status = batch->Set(batch->Size(), {}), "");
}

TEST(StrokeInputBatchDeathTest, GetWithIndexOutOfBounds) {
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(MakeValidTestInputSequence());
  ASSERT_EQ(batch.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(batch->Get(batch->Size()), "");
}

TEST(StrokeInputBatchDeathTest, EraseWithStartOutOfBounds) {
  absl::StatusOr<StrokeInputBatch> batch =
      StrokeInputBatch::Create(MakeValidTestInputSequence());
  ASSERT_EQ(batch.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(batch->Erase(batch->Size() + 1, 1), "");
}

}  // namespace
}  // namespace ink
