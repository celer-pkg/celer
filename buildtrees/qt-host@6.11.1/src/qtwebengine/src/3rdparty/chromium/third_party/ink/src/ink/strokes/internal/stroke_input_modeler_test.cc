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

#include "ink/strokes/internal/stroke_input_modeler.h"

#include <cstddef>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/brush/brush_family.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"
#include "ink/types/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatNear;
using ::testing::Optional;

// Returns a vector of single-input `StrokeInputBatch` that can be used for a
// single synthetic stroke.
std::vector<StrokeInputBatch> MakeStylusInputBatchSequence() {
  StrokeInput::ToolType tool_type = StrokeInput::ToolType::kStylus;
  PhysicalDistance stroke_unit_length = PhysicalDistance::Centimeters(1);
  std::vector<StrokeInput> inputs = {{.tool_type = tool_type,
                                      .position = {10, 20},
                                      .elapsed_time = Duration32::Zero(),
                                      .stroke_unit_length = stroke_unit_length,
                                      .pressure = 0.4,
                                      .tilt = Angle::Radians(1),
                                      .orientation = Angle::Radians(2)},
                                     {.tool_type = tool_type,
                                      .position = {10, 23},
                                      .elapsed_time = Duration32::Seconds(1),
                                      .stroke_unit_length = stroke_unit_length,
                                      .pressure = 0.3,
                                      .tilt = Angle::Radians(0.9),
                                      .orientation = Angle::Radians(0.9)},
                                     {.tool_type = tool_type,
                                      .position = {10, 17},
                                      .elapsed_time = Duration32::Seconds(2),
                                      .stroke_unit_length = stroke_unit_length,
                                      .pressure = 0.5,
                                      .tilt = Angle::Radians(0.8),
                                      .orientation = Angle::Radians(1.1)},
                                     {.tool_type = tool_type,
                                      .position = {5, 5},
                                      .elapsed_time = Duration32::Seconds(3),
                                      .stroke_unit_length = stroke_unit_length,
                                      .pressure = 0.8,
                                      .tilt = Angle::Radians(1.5),
                                      .orientation = Angle::Radians(1.3)},
                                     {.tool_type = tool_type,
                                      .position = {4, 3},
                                      .elapsed_time = Duration32::Seconds(5),
                                      .stroke_unit_length = stroke_unit_length,
                                      .pressure = 1.0,
                                      .tilt = Angle::Radians(1.3),
                                      .orientation = Angle::Radians(1.5)}};

  std::vector<StrokeInputBatch> batches;
  for (const StrokeInput& input : inputs) {
    auto batch = StrokeInputBatch::Create({input});
    ABSL_CHECK_OK(batch);
    batches.push_back(*batch);
  }
  return batches;
}

MATCHER_P2(ModeledStrokeInputNearMatcher, expected, tolerance, "") {
  return ExplainMatchResult(
      AllOf(Field("position", &ModeledStrokeInput::position,
                  PointNear(expected.position, tolerance)),
            Field("velocity", &ModeledStrokeInput::velocity,
                  VecNear(expected.velocity, tolerance)),
            Field("acceleration", &ModeledStrokeInput::acceleration,
                  VecNear(expected.acceleration, tolerance)),
            Field("traveled_distance", &ModeledStrokeInput::traveled_distance,
                  FloatNear(expected.traveled_distance, tolerance)),
            Field("elapsed_time", &ModeledStrokeInput::elapsed_time,
                  Duration32Near(expected.elapsed_time, tolerance)),
            Field("pressure", &ModeledStrokeInput::pressure,
                  FloatNear(expected.pressure, tolerance)),
            Field("tilt", &ModeledStrokeInput::tilt,
                  AngleNear(expected.tilt, tolerance)),
            Field("orientation", &ModeledStrokeInput::orientation,
                  AngleNear(expected.orientation, tolerance))),
      arg, result_listener);
}
::testing::Matcher<ModeledStrokeInput> ModeledStrokeInputNear(
    const ModeledStrokeInput& expected, float tolerance) {
  return ModeledStrokeInputNearMatcher(expected, tolerance);
}

MATCHER_P(PositionsAreSeparatedByAtLeast, min_distance, "") {
  for (size_t i = 1; i < arg.size(); ++i) {
    float distance = (arg[i].position - arg[i - 1].position).Magnitude();
    if (distance < min_distance) {
      *result_listener << absl::StrFormat(
          "Inputs at indices %d and %d, with positions %v and %v, are "
          "separated by %f",
          i - 1, i, arg[i - 1].position, arg[i].position, distance);
      return false;
    }
  }
  return true;
}

TEST(StrokeInputModelerTest, DefaultConstructed) {
  StrokeInputModeler modeler;

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kUnknown);
  EXPECT_EQ(modeler.GetState().stroke_unit_length, std::nullopt);
  EXPECT_EQ(modeler.GetState().complete_elapsed_time, Duration32::Zero());
  EXPECT_TRUE(modeler.GetModeledInputs().empty());
  EXPECT_EQ(modeler.GetState().stable_input_count, 0);
  EXPECT_EQ(modeler.GetState().real_input_count, 0);
}

TEST(StrokeInputModelerTest, StartOnDefaultConstructed) {
  StrokeInputModeler modeler;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), 0.01);

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kUnknown);
  EXPECT_EQ(modeler.GetState().stroke_unit_length, std::nullopt);
  EXPECT_EQ(modeler.GetState().complete_elapsed_time, Duration32::Zero());
  EXPECT_TRUE(modeler.GetModeledInputs().empty());

  EXPECT_EQ(modeler.GetState().stable_input_count, 0);
  EXPECT_EQ(modeler.GetState().real_input_count, 0);
}

TEST(StrokeInputModelerTest, FirstExtendWithEmptyInputs) {
  StrokeInputModeler modeler;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), 0.01);

  // This kind of function call is likely to never occur, but we check that the
  // `current_elapsed_time` parameter is not ignored in this case for
  // consistency of the API.
  modeler.ExtendStroke({}, {}, Duration32::Millis(10));

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kUnknown);
  EXPECT_EQ(modeler.GetState().stroke_unit_length, std::nullopt);
  EXPECT_EQ(modeler.GetState().complete_elapsed_time, Duration32::Millis(10));
  EXPECT_TRUE(modeler.GetModeledInputs().empty());
  EXPECT_EQ(modeler.GetState().stable_input_count, 0);
  EXPECT_EQ(modeler.GetState().real_input_count, 0);
}

TEST(StrokeInputModelerTest, ExtendWithEmptyPredictedInputs) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  float brush_epsilon = 0.001;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), brush_epsilon);

  StrokeInputBatch synthetic_real_inputs = input_batches[0];
  ASSERT_EQ(absl::OkStatus(), synthetic_real_inputs.Append(input_batches[1]));

  Duration32 current_elapsed_time = synthetic_real_inputs.Get(1).elapsed_time;
  modeler.ExtendStroke(synthetic_real_inputs, {}, current_elapsed_time);

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kStylus);
  EXPECT_THAT(modeler.GetState().stroke_unit_length,
              Optional(PhysicalDistanceEq(PhysicalDistance::Centimeters(1))));
  EXPECT_THAT(modeler.GetState().complete_elapsed_time.ToSeconds(),
              FloatNear(current_elapsed_time.ToSeconds(), 0.05));

  // Only the first of the two `StrokeInput` should be stable, which should
  // result in a single modeled result.
  EXPECT_EQ(modeler.GetState().stable_input_count, 1);
  EXPECT_GT(modeler.GetState().real_input_count,
            modeler.GetState().stable_input_count);
  EXPECT_EQ(modeler.GetModeledInputs().size(),
            modeler.GetState().real_input_count);

  EXPECT_GT(modeler.GetModeledInputs().back().traveled_distance, 0);
  EXPECT_GT(modeler.GetModeledInputs().back().elapsed_time, Duration32::Zero());

  EXPECT_THAT(modeler.GetModeledInputs(),
              PositionsAreSeparatedByAtLeast(brush_epsilon));
}

TEST(StrokeInputModelerTest, ExtendWithEmptyRealInputs) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  float brush_epsilon = 0.01;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), brush_epsilon);

  StrokeInputBatch synthetic_predicted_inputs = input_batches[0];
  ASSERT_EQ(absl::OkStatus(),
            synthetic_predicted_inputs.Append(input_batches[1]));
  ASSERT_EQ(absl::OkStatus(),
            synthetic_predicted_inputs.Append(input_batches[2]));

  Duration32 current_elapsed_time = Duration32::Zero();
  modeler.ExtendStroke({}, synthetic_predicted_inputs, current_elapsed_time);

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kStylus);
  EXPECT_THAT(modeler.GetState().stroke_unit_length,
              Optional(PhysicalDistanceEq(PhysicalDistance::Centimeters(1))));

  Duration32 predicted_elapsed_time =
      synthetic_predicted_inputs.Get(synthetic_predicted_inputs.Size() - 1)
          .elapsed_time;
  EXPECT_THAT(modeler.GetState().complete_elapsed_time.ToSeconds(),
              FloatNear(predicted_elapsed_time.ToSeconds(), 0.05));

  EXPECT_FALSE(modeler.GetModeledInputs().empty());
  EXPECT_EQ(modeler.GetState().stable_input_count, 0);
  EXPECT_EQ(modeler.GetState().real_input_count, 0);

  EXPECT_GT(modeler.GetModeledInputs().back().traveled_distance, 0);
  EXPECT_GT(modeler.GetModeledInputs().back().elapsed_time, Duration32::Zero());

  EXPECT_THAT(modeler.GetModeledInputs(),
              PositionsAreSeparatedByAtLeast(brush_epsilon));
}

TEST(StrokeInputModelerTest, ExtendWithBothEmptyInputsClearsPrediction) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  float brush_epsilon = 0.08;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), brush_epsilon);

  Duration32 current_elapsed_time = input_batches[0].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[0], {}, current_elapsed_time);

  current_elapsed_time = input_batches[1].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[1], input_batches[4],
                       current_elapsed_time);

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kStylus);
  EXPECT_THAT(modeler.GetState().stroke_unit_length,
              Optional(PhysicalDistanceEq(PhysicalDistance::Centimeters(1))));
  Duration32 predicted_elapsed_time = input_batches[4].Get(0).elapsed_time;
  EXPECT_THAT(modeler.GetState().complete_elapsed_time.ToSeconds(),
              FloatNear(predicted_elapsed_time.ToSeconds(), 0.05));

  EXPECT_EQ(modeler.GetState().stable_input_count, 1);
  EXPECT_GT(modeler.GetState().real_input_count,
            modeler.GetState().stable_input_count);
  EXPECT_GT(modeler.GetModeledInputs().size(),
            modeler.GetState().real_input_count);

  EXPECT_GT(modeler.GetModeledInputs().back().traveled_distance, 0);
  EXPECT_GT(modeler.GetModeledInputs().back().elapsed_time, Duration32::Zero());

  size_t last_stable_modeled_count = modeler.GetState().stable_input_count;

  current_elapsed_time += Duration32::Seconds(0.2);
  modeler.ExtendStroke({}, {}, current_elapsed_time);
  EXPECT_EQ(modeler.GetState().complete_elapsed_time, current_elapsed_time);

  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kStylus);
  EXPECT_THAT(modeler.GetState().stroke_unit_length,
              Optional(PhysicalDistanceEq(PhysicalDistance::Centimeters(1))));

  EXPECT_EQ(modeler.GetState().stable_input_count, last_stable_modeled_count);
  EXPECT_GT(modeler.GetState().real_input_count,
            modeler.GetState().stable_input_count);
  EXPECT_EQ(modeler.GetState().real_input_count,
            modeler.GetModeledInputs().size());

  EXPECT_THAT(modeler.GetModeledInputs(),
              PositionsAreSeparatedByAtLeast(brush_epsilon));
}

TEST(StrokeInputModelerTest, ExtendKeepsRealInputAndReplacesPrediction) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  float brush_epsilon = 0.004;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), brush_epsilon);

  Duration32 current_elapsed_time = input_batches[0].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[0], {}, current_elapsed_time);

  current_elapsed_time = input_batches[1].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[1], input_batches[4],
                       current_elapsed_time);

  EXPECT_EQ(modeler.GetState().stable_input_count, 1);
  EXPECT_GT(modeler.GetState().real_input_count,
            modeler.GetState().stable_input_count);
  EXPECT_GT(modeler.GetModeledInputs().size(),
            modeler.GetState().real_input_count);

  EXPECT_GT(modeler.GetModeledInputs().back().traveled_distance, 0);
  EXPECT_GT(modeler.GetModeledInputs().back().elapsed_time, Duration32::Zero());

  size_t last_real_modeled_count = modeler.GetState().real_input_count;
  float last_real_distance =
      modeler.GetModeledInputs()[last_real_modeled_count - 1].traveled_distance;
  Duration32 last_real_elapsed_time =
      modeler.GetModeledInputs()[last_real_modeled_count - 1].elapsed_time;
  float last_total_distance =
      modeler.GetModeledInputs().back().traveled_distance;
  Duration32 last_total_elapsed_time =
      modeler.GetModeledInputs().back().elapsed_time;

  current_elapsed_time = input_batches[2].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[2], input_batches[3],
                       current_elapsed_time);

  EXPECT_GT(modeler.GetState().real_input_count, last_real_modeled_count);
  EXPECT_GT(modeler.GetModeledInputs().size(),
            modeler.GetState().real_input_count);

  // The real traveled_distance and elapsed time of the stroke should increase,
  // but the totals should decrease as the new prediction is prior to the one
  // used for the previous extension:

  size_t real_count = modeler.GetState().real_input_count;
  EXPECT_GT(modeler.GetModeledInputs()[real_count - 1].traveled_distance,
            last_real_distance);
  EXPECT_GT(modeler.GetModeledInputs()[real_count - 1].elapsed_time,
            last_real_elapsed_time);

  EXPECT_LT(modeler.GetModeledInputs().back().traveled_distance,
            last_total_distance);
  EXPECT_LT(modeler.GetModeledInputs().back().elapsed_time,
            last_total_elapsed_time);

  EXPECT_THAT(modeler.GetModeledInputs(),
              PositionsAreSeparatedByAtLeast(brush_epsilon));
}

TEST(StrokeInputModelerTest, StartClearsAfterExtending) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), 0.01);

  Duration32 current_elapsed_time = input_batches[0].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[0], {}, current_elapsed_time);

  current_elapsed_time = input_batches[1].Get(0).elapsed_time;
  modeler.ExtendStroke(input_batches[1], input_batches[2],
                       current_elapsed_time);

  ASSERT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kStylus);
  EXPECT_THAT(modeler.GetState().stroke_unit_length,
              Optional(PhysicalDistanceEq(PhysicalDistance::Centimeters(1))));
  ASSERT_NE(modeler.GetState().complete_elapsed_time, Duration32::Zero());
  ASSERT_FALSE(modeler.GetModeledInputs().empty());
  ASSERT_NE(modeler.GetState().stable_input_count, 0);
  ASSERT_NE(modeler.GetState().real_input_count, 0);

  modeler.StartStroke(BrushFamily::DefaultInputModel(), 0.01);
  EXPECT_EQ(modeler.GetState().tool_type, StrokeInput::ToolType::kUnknown);
  EXPECT_EQ(modeler.GetState().stroke_unit_length, std::nullopt);
  EXPECT_EQ(modeler.GetState().complete_elapsed_time, Duration32::Zero());
  EXPECT_TRUE(modeler.GetModeledInputs().empty());

  EXPECT_EQ(modeler.GetState().stable_input_count, 0);
  EXPECT_EQ(modeler.GetState().real_input_count, 0);
}

TEST(StrokeInputModelerTest, LargeBrushEpsilonIsRespected) {
  std::vector<StrokeInputBatch> input_batches = MakeStylusInputBatchSequence();

  StrokeInputModeler modeler;
  float brush_epsilon = 3;
  modeler.StartStroke(BrushFamily::DefaultInputModel(), brush_epsilon);

  modeler.ExtendStroke(input_batches[0], input_batches[1], Duration32::Zero());
  modeler.ExtendStroke(input_batches[1], input_batches[2], Duration32::Zero());
  modeler.ExtendStroke(input_batches[2], input_batches[3], Duration32::Zero());
  modeler.ExtendStroke(input_batches[3], input_batches[4], Duration32::Zero());

  EXPECT_THAT(
      modeler.GetModeledInputs(),
      ElementsAre(
          ModeledStrokeInputNear({.position = {10, 20},
                                  .velocity = {0, 0},
                                  .acceleration = {0, 0},
                                  .traveled_distance = 0,
                                  .elapsed_time = Duration32::Seconds(0),
                                  .pressure = 0.4,
                                  .tilt = Angle::Radians(1),
                                  .orientation = Angle::Radians(2)},
                                 0.01),
          ModeledStrokeInputNear({.position = {9.98, 16.99},
                                  .velocity = {-1.67, -8.00},
                                  .acceleration = {-120.38, -145.56},
                                  .traveled_distance = 3.01,
                                  .elapsed_time = Duration32::Seconds(2.02),
                                  .pressure = 0.5,
                                  .tilt = Angle::Radians(0.80),
                                  .orientation = Angle::Radians(1.1)},
                                 0.01),
          ModeledStrokeInputNear({.position = {8.82, 14.16},
                                  .velocity = {-4.11, -10.93},
                                  .acceleration = {-0.27, -1.92},
                                  .traveled_distance = 6.07,
                                  .elapsed_time = Duration32::Seconds(2.26),
                                  .pressure = 0.57,
                                  .tilt = Angle::Radians(0.97),
                                  .orientation = Angle::Radians(1.15)},
                                 0.01),
          ModeledStrokeInputNear({.position = {7.65, 11.36},
                                  .velocity = {-4.36, -11.24},
                                  .acceleration = {-0.57, -1.82},
                                  .traveled_distance = 9.1,
                                  .elapsed_time = Duration32::Seconds(2.49),
                                  .pressure = 0.64,
                                  .tilt = Angle::Radians(1.13),
                                  .orientation = Angle::Radians(1.19)},
                                 0.01),
          ModeledStrokeInputNear({.position = {6.48, 8.56},
                                  .velocity = {-4.63, -11.56},
                                  .acceleration = {-0.87, -1.71},
                                  .traveled_distance = 12.14,
                                  .elapsed_time = Duration32::Seconds(2.72),
                                  .pressure = 0.71,
                                  .tilt = Angle::Radians(1.30),
                                  .orientation = Angle::Radians(1.24)},
                                 0.01),
          ModeledStrokeInputNear({.position = {5.32, 5.76},
                                  .velocity = {-4.92, -11.90},
                                  .acceleration = {-1.20, -1.59},
                                  .traveled_distance = 15.17,
                                  .elapsed_time = Duration32::Seconds(2.96),
                                  .pressure = 0.78,
                                  .tilt = Angle::Radians(1.46),
                                  .orientation = Angle::Radians(1.29)},
                                 0.01),
          ModeledStrokeInputNear({.position = {4.02, 3.05},
                                  .velocity = {-0.50, -1.01},
                                  .acceleration = {0.05, 0.12},
                                  .traveled_distance = 18.18,
                                  .elapsed_time = Duration32::Seconds(4.98),
                                  .pressure = 1,
                                  .tilt = Angle::Radians(1.3),
                                  .orientation = Angle::Radians(1.5)},
                                 0.01)));
  EXPECT_THAT(modeler.GetModeledInputs(),
              PositionsAreSeparatedByAtLeast(brush_epsilon));
}

TEST(StrokeInputModelerDeathTest, ExtendWithoutStart) {
  EXPECT_DEATH_IF_SUPPORTED(
      StrokeInputModeler().ExtendStroke({}, {}, Duration32::Zero()),
      "`StartStroke\\(\\)` has not been called\\.");
}

TEST(StrokeInputModelerDeathTest, StartWithZeroEpsilon) {
  EXPECT_DEATH_IF_SUPPORTED(
      StrokeInputModeler().StartStroke(BrushFamily::DefaultInputModel(), 0),
      "brush_epsilon");
}

}  // namespace
}  // namespace ink::strokes_internal
