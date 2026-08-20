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

#include "ink/strokes/input/stroke_input.h"

#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {
namespace {

TEST(StrokeInputTest, Stringify) {
  EXPECT_EQ(absl::StrCat(StrokeInput{
                .position = {3, 7},
                .elapsed_time = Duration32::Seconds(1.5f),
            }),
            "StrokeInput[Unknown, (3, 7), 1.5s]");
  EXPECT_EQ(absl::StrCat(StrokeInput{
                .tool_type = StrokeInput::ToolType::kMouse,
                .position = {2, 11},
                .elapsed_time = Duration32::Seconds(2.5f),
            }),
            "StrokeInput[Mouse, (2, 11), 2.5s]");
  EXPECT_EQ(absl::StrCat(StrokeInput{
                .tool_type = StrokeInput::ToolType::kTouch,
                .position = {-3, 0},
                .elapsed_time = Duration32::Seconds(3.5f),
                .pressure = 0.5f,
            }),
            "StrokeInput[Touch, (-3, 0), 3.5s, pressure=0.5]");
  EXPECT_EQ(absl::StrCat(StrokeInput{
                .tool_type = StrokeInput::ToolType::kStylus,
                .position = {0, -4},
                .elapsed_time = Duration32::Seconds(4.5f),
                .stroke_unit_length = PhysicalDistance::Centimeters(6.5f),
                .pressure = 1.0f,
                .tilt = kFullTurn / 8,
                .orientation = kFullTurn * 0.75f,
            }),
            "StrokeInput[Stylus, (0, -4), 4.5s, stroke_unit_length=6.5cm, "
            "pressure=1, tilt=0.25π, orientation=1.5π]");
}

TEST(StrokeInputTest, DefaultInitializedInput) {
  StrokeInput input;
  EXPECT_FALSE(input.HasStrokeUnitLength());
  EXPECT_FALSE(input.HasPressure());
  EXPECT_FALSE(input.HasTilt());
  EXPECT_FALSE(input.HasOrientation());
}

TEST(StrokeInputTest, NoStrokeUnitLength) {
  StrokeInput input = {.stroke_unit_length = StrokeInput::kNoStrokeUnitLength,
                       .pressure = 0,
                       .tilt = Angle::Radians(0),
                       .orientation = Angle::Radians(0)};
  EXPECT_FALSE(input.HasStrokeUnitLength());
  EXPECT_TRUE(input.HasPressure());
  EXPECT_TRUE(input.HasTilt());
  EXPECT_TRUE(input.HasOrientation());
}

TEST(StrokeInputTest, NoPressure) {
  StrokeInput input = {.stroke_unit_length = PhysicalDistance::Inches(1),
                       .pressure = StrokeInput::kNoPressure,
                       .tilt = Angle::Radians(0),
                       .orientation = Angle::Radians(0)};
  EXPECT_TRUE(input.HasStrokeUnitLength());
  EXPECT_FALSE(input.HasPressure());
  EXPECT_TRUE(input.HasTilt());
  EXPECT_TRUE(input.HasOrientation());
}

TEST(StrokeInputTest, NoTilt) {
  StrokeInput input = {.stroke_unit_length = PhysicalDistance::Inches(1),
                       .pressure = 0,
                       .tilt = StrokeInput::kNoTilt,
                       .orientation = Angle::Radians(0)};
  EXPECT_TRUE(input.HasStrokeUnitLength());
  EXPECT_TRUE(input.HasPressure());
  EXPECT_FALSE(input.HasTilt());
  EXPECT_TRUE(input.HasOrientation());
}

TEST(StrokeInputTest, NoOrientation) {
  StrokeInput input = {.stroke_unit_length = PhysicalDistance::Inches(1),
                       .pressure = 0,
                       .tilt = Angle::Radians(0),
                       .orientation = StrokeInput::kNoOrientation};
  EXPECT_TRUE(input.HasStrokeUnitLength());
  EXPECT_TRUE(input.HasPressure());
  EXPECT_TRUE(input.HasTilt());
  EXPECT_FALSE(input.HasOrientation());
}

}  // namespace
}  // namespace ink
