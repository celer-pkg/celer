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

#include "ink/strokes/input/internal/stroke_input_validation_helpers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "ink/geometry/angle.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink::stroke_input_internal {
namespace {

using ::testing::HasSubstr;

TEST(ValidateConsecutiveInputsTest, ValidInputsWithNoOptionalProperties) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs(
                {.position = {1, 2}, .elapsed_time = Duration32::Millis(5)},
                {.position = {2, 3}, .elapsed_time = Duration32::Millis(10)}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsWithAllOptionalProperties) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs({.position = {1, 2},
                                       .elapsed_time = Duration32::Millis(5),
                                       .pressure = 0.5,
                                       .tilt = kFullTurn / 8,
                                       .orientation = kHalfTurn},
                                      {.position = {2, 3},
                                       .elapsed_time = Duration32::Millis(10),
                                       .pressure = 0.6,
                                       .tilt = kQuarterTurn,
                                       .orientation = kFullTurn}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsWithOnlyPressure) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs({.position = {1, 2},
                                       .elapsed_time = Duration32::Millis(5),
                                       .pressure = 0.5},
                                      {.position = {2, 3},
                                       .elapsed_time = Duration32::Millis(10),
                                       .pressure = 0.6}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsWithOnlyTilt) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs({.position = {1, 2},
                                       .elapsed_time = Duration32::Millis(5),
                                       .tilt = kFullTurn / 8},
                                      {.position = {2, 3},
                                       .elapsed_time = Duration32::Millis(10),
                                       .tilt = kQuarterTurn}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsWithOnlyOrientation) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs({.position = {1, 2},
                                       .elapsed_time = Duration32::Millis(5),
                                       .orientation = kHalfTurn},
                                      {.position = {2, 3},
                                       .elapsed_time = Duration32::Millis(10),
                                       .orientation = kFullTurn}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsDuplicatePosition) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs(
                {.position = {1, 2}, .elapsed_time = Duration32::Millis(5)},
                {.position = {1, 2}, .elapsed_time = Duration32::Millis(10)}));
}

TEST(ValidateConsecutiveInputsTest, ValidInputsDuplicateElapsedTime) {
  EXPECT_EQ(absl::OkStatus(),
            ValidateConsecutiveInputs(
                {.position = {1, 2}, .elapsed_time = Duration32::Millis(5)},
                {.position = {2, 3}, .elapsed_time = Duration32::Millis(5)}));
}

TEST(ValidateConsecutiveInputsTest, MismatchedToolTypes) {
  absl::Status mismatched_tool_type =
      ValidateConsecutiveInputs({.tool_type = StrokeInput::ToolType::kMouse,
                                 .position = {1, 2},
                                 .elapsed_time = Duration32::Millis(5)},
                                {.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {2, 3},
                                 .elapsed_time = Duration32::Millis(10)});
  EXPECT_EQ(mismatched_tool_type.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_tool_type.message(), HasSubstr("tool_type"));
}

TEST(ValidateConsecutiveInputsTest, DuplicatePositionAndElapsedTime) {
  absl::Status duplicate_input = ValidateConsecutiveInputs(
      {.position = {1, 2}, .elapsed_time = Duration32::Millis(5)},
      {.position = {1, 2}, .elapsed_time = Duration32::Millis(5)});
  EXPECT_EQ(duplicate_input.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(duplicate_input.message(), HasSubstr("duplicate"));
}

TEST(ValidateConsecutiveInputsTest, DecreasingElapsedTime) {
  absl::Status decreasing_time = ValidateConsecutiveInputs(
      {.position = {1, 2}, .elapsed_time = Duration32::Seconds(1)},
      {.position = {2, 3}, .elapsed_time = Duration32::Seconds(0.99)});
  EXPECT_EQ(decreasing_time.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(decreasing_time.message(), HasSubstr("non-decreasing"));
}

TEST(ValidateConsecutiveInputsTest, MismatchedStrokeUnitLength) {
  absl::Status mismatched_tool_type = ValidateConsecutiveInputs(
      {.position = {1, 2},
       .elapsed_time = Duration32::Millis(5),
       .stroke_unit_length = PhysicalDistance::Centimeters(1)},
      {.position = {2, 3},
       .elapsed_time = Duration32::Millis(10),
       .stroke_unit_length = PhysicalDistance::Centimeters(2)});
  EXPECT_EQ(mismatched_tool_type.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_tool_type.message(), HasSubstr("stroke_unit_length"));
}

TEST(ValidateConsecutiveInputsTest, MismatchedOptionalPressure) {
  absl::Status mismatched_has_pressure =
      ValidateConsecutiveInputs({.position = {1, 2},
                                 .elapsed_time = Duration32::Millis(5),
                                 .pressure = StrokeInput::kNoPressure},
                                {.position = {2, 3},
                                 .elapsed_time = Duration32::Millis(10),
                                 .pressure = 0.5});
  EXPECT_EQ(mismatched_has_pressure.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_has_pressure.message(), HasSubstr("pressure"));
}

TEST(ValidateConsecutiveInputsTest, MismatchedOptionalTilt) {
  absl::Status mismatched_has_tilt =
      ValidateConsecutiveInputs({.position = {1, 2},
                                 .elapsed_time = Duration32::Millis(5),
                                 .tilt = StrokeInput::kNoTilt},
                                {.position = {2, 3},
                                 .elapsed_time = Duration32::Millis(10),
                                 .tilt = kQuarterTurn});
  EXPECT_EQ(mismatched_has_tilt.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_has_tilt.message(), HasSubstr("tilt"));
}

TEST(ValidateConsecutiveInputsTest, MismatchedOptionalOrientation) {
  absl::Status mismatched_has_orientation =
      ValidateConsecutiveInputs({.position = {1, 2},
                                 .elapsed_time = Duration32::Millis(5),
                                 .orientation = StrokeInput::kNoOrientation},
                                {.position = {2, 3},
                                 .elapsed_time = Duration32::Millis(10),
                                 .orientation = kHalfTurn});
  EXPECT_EQ(mismatched_has_orientation.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_has_orientation.message(), HasSubstr("orientation"));
}

}  // namespace
}  // namespace ink::stroke_input_internal
