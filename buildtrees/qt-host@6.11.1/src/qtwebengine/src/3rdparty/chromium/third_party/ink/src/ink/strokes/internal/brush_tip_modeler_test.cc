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

#include "ink/strokes/internal/brush_tip_modeler.h"

#include <limits>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/types/duration.h"

namespace ink::strokes_internal {
namespace {

using ::testing::AllOf;
using ::testing::Each;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::IsEmpty;
using ::testing::Ne;
using ::testing::Pointwise;

// Matcher that compares all fields of two tip states for equality except the
// positions.
MATCHER_P(NonPositionFieldsEq, expected, "") {
  return ExplainMatchResult(
      AllOf(Field("width", &BrushTipState::width, FloatEq(expected.width)),
            Field("height", &BrushTipState::height, FloatEq(expected.height)),
            Field("percent_radius", &BrushTipState::percent_radius,
                  FloatEq(expected.percent_radius)),
            Field("rotation", &BrushTipState::rotation,
                  AngleEq(expected.rotation))),
      arg, result_listener);
}

// Matcher on a tuple<BrushTipState, Point> to match just the positions in a
// container of tip states.
MATCHER(PositionsEq, "") {
  return ExplainMatchResult(Field("position", &BrushTipState::position,
                                  PointNear(std::get<1>(arg), 0.0001)),
                            std::get<0>(arg), result_listener);
}

TEST(BrushTipModelerTest, DefaultConstructed) {
  BrushTipModeler modeler;
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_TRUE(modeler.VolatileTipStates().empty());
  EXPECT_FALSE(modeler.HasUnfinishedTimeBehaviors(StrokeInputModeler::State{}));
}

TEST(BrushTipModelerTest, StartDefaultConstructed) {
  BrushTipModeler modeler;
  BrushTip brush_tip;
  modeler.StartStroke(&brush_tip, 1);
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_TRUE(modeler.VolatileTipStates().empty());
  EXPECT_FALSE(modeler.HasUnfinishedTimeBehaviors(StrokeInputModeler::State{}));
}

TEST(BrushTipModelerTest, StartWithTipWithTimeSinceBehavior) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kTimeSinceInputInSeconds,
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kSizeMultiplier,
              .target_modifier_range = {1, 2},
          },
      }}},
  };
  modeler.StartStroke(&brush_tip, 1);
  EXPECT_TRUE(modeler.HasUnfinishedTimeBehaviors(
      StrokeInputModeler::State{.complete_elapsed_time = Duration32::Zero()}));
  EXPECT_FALSE(modeler.HasUnfinishedTimeBehaviors(StrokeInputModeler::State{
      .complete_elapsed_time = Duration32::Seconds(1.1)}));
}

TEST(BrushTipModelerTest, UpdateWithEmptyState) {
  BrushTipModeler modeler;
  BrushTip brush_tip;
  modeler.StartStroke(&brush_tip, 1);
  modeler.UpdateStroke({}, {});
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_TRUE(modeler.VolatileTipStates().empty());
}

TEST(BrushTipModelerTest, UpdateWithAllStableInputs) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .scale = {0.5, 0.75},
      .corner_rounding = 0.5,
      .rotation = kQuarterTurn,
  };
  float brush_size = 1.5;
  BrushTipState expected_non_position_values = {
      .width = brush_tip.scale.x * brush_size,
      .height = brush_tip.scale.y * brush_size,
      .percent_radius = brush_tip.corner_rounding,
      .rotation = brush_tip.rotation,
  };

  modeler.StartStroke(&brush_tip, brush_size);
  std::vector<ModeledStrokeInput> inputs = {
      {.position = {1, 3}, .elapsed_time = Duration32::Zero()},
      {.position = {2, 3}, .elapsed_time = Duration32::Seconds(1 / 180.f)}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 2,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  EXPECT_THAT(modeler.NewFixedTipStates(),
              Pointwise(PositionsEq(), {Point{1, 3}, Point{2, 3}}));
  EXPECT_THAT(modeler.NewFixedTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));

  // The default brush tip has no behaviors, so there should be no volatile tip
  // states if all modeled inputs were stable:
  EXPECT_TRUE(modeler.VolatileTipStates().empty());
}

TEST(BrushTipModelerTest, UpdateWithNoStableInputs) {
  BrushTipModeler modeler;
  BrushTip brush_tip{
      .scale = {0, 1},
      .corner_rounding = 0,
      .rotation = kHalfTurn,
  };
  float brush_size = 1.7;
  BrushTipState expected_non_position_values = {
      .width = brush_tip.scale.x * brush_size,
      .height = brush_tip.scale.y * brush_size,
      .percent_radius = brush_tip.corner_rounding,
      .rotation = brush_tip.rotation,
  };

  modeler.StartStroke(&brush_tip, brush_size);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {1, 3}, .elapsed_time = Duration32::Zero()},
      {.position = {2, 3}, .elapsed_time = Duration32::Seconds(1 / 180.f)}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 0,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // All unstable modeled input should result in volatile tip states:
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_THAT(modeler.VolatileTipStates(),
              Pointwise(PositionsEq(), {Point{1, 3}, Point{2, 3}}));
  EXPECT_THAT(modeler.VolatileTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));
}

TEST(BrushTipModelerTest, UpdateClearsPreviousStableStates) {
  BrushTipModeler modeler;
  BrushTip brush_tip{
      .scale = {0.5, 0.5},
      .corner_rounding = 1,
      .rotation = Angle(),
  };
  float brush_size = 1.7;
  BrushTipState expected_non_position_values = {
      .width = brush_tip.scale.x * brush_size,
      .height = brush_tip.scale.y * brush_size,
      .percent_radius = brush_tip.corner_rounding,
      .rotation = brush_tip.rotation,
  };

  modeler.StartStroke(&brush_tip, brush_size);

  // Initially update with two stable inputs and one unstable input.
  std::vector<ModeledStrokeInput> inputs = {
      {.position = {1, 3}, .elapsed_time = Duration32::Zero()},
      {.position = {2, 3}, .elapsed_time = Duration32::Seconds(1 / 180.f)},
      {.position = {3, 3}, .elapsed_time = Duration32::Seconds(2 / 180.f)}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 2,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  EXPECT_THAT(modeler.NewFixedTipStates(),
              Pointwise(PositionsEq(), {Point{1, 3}, Point{2, 3}}));
  EXPECT_THAT(modeler.NewFixedTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));
  EXPECT_THAT(modeler.VolatileTipStates(),
              Pointwise(PositionsEq(), {Point{3, 3}}));
  EXPECT_THAT(modeler.VolatileTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));

  // Remove the unstable input, append a new stable and unstable input, and then
  // update the tip modeler:
  inputs.resize(input_modeler_state.stable_input_count);
  inputs.insert(
      inputs.end(),
      {{.position = {4, 3}, .elapsed_time = Duration32::Seconds(3 / 180.f)},
       {.position = {4, 5}, .elapsed_time = Duration32::Seconds(4 / 180.f)}});
  input_modeler_state.stable_input_count = inputs.size() - 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  // All of the new fixed tip states from the last extension should be gone:
  EXPECT_THAT(modeler.NewFixedTipStates(),
              Pointwise(PositionsEq(), {Point{4, 3}}));
  EXPECT_THAT(modeler.NewFixedTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));

  EXPECT_THAT(modeler.VolatileTipStates(),
              Pointwise(PositionsEq(), {Point{4, 5}}));
  EXPECT_THAT(modeler.VolatileTipStates(),
              Each(NonPositionFieldsEq(expected_non_position_values)));
}

TEST(BrushTipModelerTest, StartStrokeOver) {
  BrushTipModeler modeler;
  BrushTip brush_tip;
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {1, 3}, .elapsed_time = Duration32::Zero()},
      {.position = {2, 3}, .elapsed_time = Duration32::Seconds(1 / 180.f)},
      {.position = {2.5, 3.5}, .elapsed_time = Duration32::Seconds(2 / 180.f)}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 2,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);
  ASSERT_FALSE(modeler.NewFixedTipStates().empty());
  ASSERT_FALSE(modeler.VolatileTipStates().empty());

  modeler.StartStroke(&brush_tip, 2);
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_TRUE(modeler.VolatileTipStates().empty());

  modeler.UpdateStroke(input_modeler_state, inputs);
  ASSERT_FALSE(modeler.NewFixedTipStates().empty());
  ASSERT_FALSE(modeler.VolatileTipStates().empty());
}

TEST(BrushTipModelerTest, TipWithBehaviors) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .scale = {1, 0.5},
      .corner_rounding = 0,
      .behaviors =
          {BrushBehavior{{
               // Increasing pressure maps to decreasing width with a 1/10
               // second response time.
               BrushBehavior::SourceNode{
                   .source = BrushBehavior::Source::kNormalizedPressure,
                   .source_value_range = {0, 1},
               },
               BrushBehavior::ResponseNode{
                   .response_curve = {EasingFunction::Predefined::kEaseInOut},
               },
               BrushBehavior::DampingNode{
                   .damping_source =
                       BrushBehavior::DampingSource::kTimeInSeconds,
                   .damping_gap = 0.1,
               },
               BrushBehavior::TargetNode{
                   .target = BrushBehavior::Target::kWidthMultiplier,
                   .target_modifier_range = {1.08, 0.5},
               },
           }},
           BrushBehavior{{
               // Increasing tilt maps to increasing width
               BrushBehavior::SourceNode{
                   .source = BrushBehavior::Source::kTiltInRadians,
                   .source_value_range = {0, kQuarterTurn.ValueInRadians()},
               },
               BrushBehavior::TargetNode{
                   .target = BrushBehavior::Target::kWidthMultiplier,
                   .target_modifier_range = {0.3, 1.7},
               },
           }}},
  };
  modeler.StartStroke(&brush_tip, 1);

  // Extend the stroke with pressure and tilt that initially push both behaviors
  // toward negative offsets, and then quickly jump toward positive offsets and
  // stay there for the duration of the pressure behavior's response time.
  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 3},
       .elapsed_time = Duration32::Zero(),
       .pressure = 1,
       .tilt = Angle()},
      {.position = {1, 3},
       .elapsed_time = Duration32::Seconds(1.f / 180),
       .pressure = 0,
       .tilt = kQuarterTurn},
      {.position = {2, 3},
       .elapsed_time = Duration32::Seconds(0.1),
       .pressure = 0,
       .tilt = kQuarterTurn},
      {.position = {3, 3},
       .elapsed_time = Duration32::Seconds(0.125),
       .pressure = 0,
       .tilt = kQuarterTurn},
      {.position = {4, 3},
       .elapsed_time = Duration32::Seconds(0.5),
       .pressure = 0,
       .tilt = kQuarterTurn}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 4,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  EXPECT_FALSE(modeler.NewFixedTipStates().empty());
  EXPECT_FALSE(modeler.VolatileTipStates().empty());
  // The initial input has pressure = 1 and tilt = 0 degrees, for which the
  // behaviors map to multipliers of 0.5 and 0.3, respectively. The result
  // should combine to a multiplier of 0.15:
  EXPECT_FLOAT_EQ(modeler.NewFixedTipStates().front().width, 0.15);
  // After ~0.5 seconds, the offset for pressure will reflect the input value of
  // 0, which means the final width will be 1.08 * 1.7 = 1.836 times the default
  // value.
  EXPECT_THAT(modeler.VolatileTipStates().back().width, FloatNear(1.836, 0.01));

  // Extend with stable input that only increases the elapsed time by a small
  // amount to check that the state for behavior damping has been restored.
  inputs.resize(input_modeler_state.stable_input_count);
  inputs.push_back({.position = {2, 2},
                    .elapsed_time = Duration32::Seconds(0.15),
                    .pressure = 0,
                    .tilt = kQuarterTurn});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The effect of changing tilt should be an immediate 1.7 multiplier, but the
  // effect of changing pressure should still be delayed.
  EXPECT_FALSE(modeler.NewFixedTipStates().empty());
  EXPECT_THAT(modeler.NewFixedTipStates().back().width, FloatNear(1.62, 0.01));
  EXPECT_TRUE(modeler.VolatileTipStates().empty());

  // Clear and start a new stroke with just the most recent input. The pressure
  // and tilt values should now be immediately reflected in the tip state.
  modeler.StartStroke(&brush_tip, 1);
  inputs = {{.position = {2, 2},
             .elapsed_time = Duration32::Seconds(0.15),
             .pressure = 0,
             .tilt = kQuarterTurn}};
  input_modeler_state.stable_input_count = 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  EXPECT_EQ(modeler.NewFixedTipStates().size(), 1);
  EXPECT_FLOAT_EQ(modeler.NewFixedTipStates().front().width, 1.836);
  EXPECT_TRUE(modeler.VolatileTipStates().empty());
}

TEST(BrushTipModelerTest, TipWithFallbackFilter) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          // Map speed to width multiplier, but only if pressure is missing.
          BrushBehavior::SourceNode{
              .source =
                  BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond,
              .source_value_range = {0, 1},
          },
          BrushBehavior::FallbackFilterNode{
              .is_fallback_for =
                  BrushBehavior::OptionalInputProperty::kPressure,
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {1.5, 2},
          },
      }}}};
  modeler.StartStroke(&brush_tip, 1);

  // Extend the stroke, with pressure data present.
  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 3},
       .velocity = {10, 0},
       .elapsed_time = Duration32::Zero(),
       .pressure = 0.75},
      {.position = {1, 3},
       .velocity = {10, 0},
       .elapsed_time = Duration32::Millis(100),
       .pressure = 0.5},
  };
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 2,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Because the brush behavior disables the width multiplier when pressure data
  // is available, the brush width should be unchanged.
  ASSERT_FALSE(modeler.NewFixedTipStates().empty());
  EXPECT_FLOAT_EQ(modeler.NewFixedTipStates().front().width, 1);
}

TEST(BrushTipModelerTest, TipWithToolTypeFilter) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          // Map pressure to width multiplier, but only for touch input.
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_value_range = {0, 1},
          },
          BrushBehavior::ToolTypeFilterNode{
              .enabled_tool_types = {.touch = true},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {1.5, 2},
          },
      }}}};
  modeler.StartStroke(&brush_tip, 1);

  // Using stylus input, extend the stroke with pressure values that would
  // normally increase the size.
  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 3},
       .elapsed_time = Duration32::Zero(),
       .pressure = 0.75},
      {.position = {1, 3},
       .elapsed_time = Duration32::Millis(10),
       .pressure = 0.5},
  };
  StrokeInputModeler::State input_modeler_state = {
      .tool_type = StrokeInput::ToolType::kStylus,
      .stable_input_count = 2,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Because the brush behavior disables the width multiplier for non-touch
  // input, the brush width should be unchanged.
  ASSERT_FALSE(modeler.NewFixedTipStates().empty());
  EXPECT_FLOAT_EQ(modeler.NewFixedTipStates().front().width, 1);
}

TEST(BrushTipModelerTest, TipWithBinaryOpNode) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_value_range = {0, 1},
          },
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kTiltInRadians,
              .source_value_range = {0, 2},
          },
          BrushBehavior::BinaryOpNode{
              .operation = BrushBehavior::BinaryOp::kSum,
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {1, 2},
          },
      }}}};
  modeler.StartStroke(&brush_tip, 1);

  // Extend the stroke with pressure and tilt each 25% of the way through the
  // above source value ranges.
  std::vector<ModeledStrokeInput> inputs = {
      {.pressure = 0.25, .tilt = Angle::Radians(0.5)},
  };
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 1,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The two source nodes should each generate a value of 0.25, which the sum
  // node should add together to 0.5, resulting in a width multiplier of 1.5.
  ASSERT_FALSE(modeler.NewFixedTipStates().empty());
  EXPECT_FLOAT_EQ(modeler.NewFixedTipStates().front().width, 1.5);
}

TEST(BrushTipModelerTest, TipWithClampedDistanceRemainingBehavior) {
  float max_distance_remaining_multiple = 2;
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::
                  kDistanceRemainingInMultiplesOfBrushSize,
              .source_out_of_range_behavior = BrushBehavior::OutOfRange::kClamp,
              .source_value_range = {0, max_distance_remaining_multiple},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0.5, 1.5},
          },
      }}}};
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);
  float brush_size = 3;
  modeler.StartStroke(&brush_tip, brush_size);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .traveled_distance = 0},
      {.position = {0, 3.29}, .traveled_distance = 3.29},
      {.position = {0, 3.30}, .traveled_distance = 3.3},
      {.position = {0, 3.31}, .traveled_distance = 3.31},
      {.position = {0, 9.3}, .traveled_distance = 9.3},  // Last stable input
      {.position = {0, 9.5}, .traveled_distance = 9.5},
      {.position = {0, 9.7}, .traveled_distance = 9.7},
  };
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 5,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeler should create 3 fixed and 4 volatile tip states, because:
  //   * The stable but volatile length of the stroke is `brush_size *
  //     max_distance_remaining_multiple`, which equals 6 units.
  //   * Modeled inputs with traveled distance of 0, 3.29, and 3.3 should result
  //     in fixed tip states since they are >=6 units away from the last stable
  //     traveled distance of 9.3.
  //   * Modeled inputs with traveled distance of 3.31 and 9.3 should result in
  //     volatile tip states, because they are within 6 units of 9.3.
  //   * The modeled inputs with traveled distance of 9.5 and 9.7 should result
  //     in volatile tip states because they are unstable.
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 3);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 4);

  // Check again with the source value range reversed, as it should still cause
  // the same behavior upper bound.
  source_node->source_value_range = {2, 0};
  modeler.StartStroke(&brush_tip, brush_size);
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 3);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 4);
}

TEST(BrushTipModelerTest, TipWithNonClampedDistanceRemainingBehavior) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::
                  kDistanceRemainingInMultiplesOfBrushSize,
              .source_out_of_range_behavior =
                  BrushBehavior::OutOfRange::kRepeat,
              .source_value_range = {0, 2},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0.5, 1.5},
          },
      }}}};
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .traveled_distance = 0},
      {.position = {0, 3}, .traveled_distance = 3},
      {.position = {0, 5}, .traveled_distance = 5},
      {.position = {0, 9}, .traveled_distance = 9},
      {.position = {0, 100000}, .traveled_distance = 100000},
  };
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 5,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // All of the tip states should be volatile even though all inputs were
  // stable, because there is no upper bound to how far back in the stroke the
  // behavior affects when the `source_out_of_range_behavior` is not `kClamp`.
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_EQ(modeler.VolatileTipStates().size(), 5);

  // Should have the same result with `kMirror` instead of `kRepeat`.
  source_node->source_out_of_range_behavior =
      BrushBehavior::OutOfRange::kMirror;
  modeler.StartStroke(&brush_tip, 1);
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_EQ(modeler.VolatileTipStates().size(), 5);
}

TEST(BrushTipModelerTest, TipWithMultipleDistanceRemainingBehaviors) {
  float max_distance_remaining_multiple = 3;
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceRemainingInMultiplesOfBrushSize,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kClamp,
                  .source_value_range = {0, 2},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kWidthMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }},
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceRemainingInMultiplesOfBrushSize,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kClamp,
                  .source_value_range = {max_distance_remaining_multiple, 1},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kHeightMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }}}};
  float brush_size = 2;
  modeler.StartStroke(&brush_tip, brush_size);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .traveled_distance = 0},
      {.position = {0, 2}, .traveled_distance = 2},
      {.position = {0, 4}, .traveled_distance = 4},
      {.position = {0, 6}, .traveled_distance = 6},
      {.position = {0, 8}, .traveled_distance = 8},  // Last stable input
      {.position = {0, 10}, .traveled_distance = 10},
      {.position = {0, 12}, .traveled_distance = 12},
      {.position = {0, 14}, .traveled_distance = 14},
  };
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 5,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeler should create 2 fixed and 6 volatile tip states, because:
  //   * The stable but volatile length of the stroke is `brush_size *
  //     max_distance_remaining_multiple`, which equals 6 units.
  //   * Modeled inputs with traveled distance of 0 and 2 should result in fixed
  //     tip states since they are >=6 units away from the last stable traveled
  //     distance of 8.
  //   * Modeled inputs with traveled distance of 4, 6, and 8 should result in
  //     volatile tip states, because they are within 6 units of 8.
  //   * The modeled inputs with traveled distance of 10, 12, and 14 should
  //     result in volatile tip states because they are unstable.
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 2);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 6);
}

TEST(BrushTipModelerTest, StartStrokeWithDistanceRemainingBehaviorsOver) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceRemainingInMultiplesOfBrushSize,
                  .source_value_range = {0, 1},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kWidthMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }},
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceRemainingAsFractionOfStrokeLength,
                  .source_value_range = {0, 0.5},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kHeightMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }},
      }};
  float brush_size = 3;
  modeler.StartStroke(&brush_tip, brush_size);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .traveled_distance = 0},
      {.position = {0, 2}, .traveled_distance = 2},
      {.position = {0, 4}, .traveled_distance = 4},
      {.position = {0, 6}, .traveled_distance = 6},
  };
  StrokeInputModeler::State state = {.stable_input_count = inputs.size()};
  modeler.UpdateStroke(state, inputs);

  // The modeler should create 2 fixed and 2 volatile tip states, because:
  //   * The `kDistanceRemainingInMultiplesOfBrushSize` behavior above keeps the
  //     last `brush_size` = 3 units of the stroke length volatile.
  //   * The `kDistanceRemainingAsFractionOfStrokeLength` behavior above keeps
  //     the latter half of the stroke length volatile.
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 2);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 2);

  // Start the stroke over, using the same inputs with a `BrushTip` with no
  // behaviors.  This time, there should be no volatile tip states, because the
  // distance-remaining behaviors are gone, and `StartStroke` should have reset
  // the private `BrushTipModeler::*_upper_bound_` fields.
  BrushTip no_behaviors;
  modeler.StartStroke(&no_behaviors, brush_size);
  modeler.UpdateStroke(state, inputs);
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 4);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 0);
}

TEST(BrushTipModelerTest, TipWithSecondsRemainingBehavior) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kTimeSinceInputInSeconds,
              .source_out_of_range_behavior = BrushBehavior::OutOfRange::kClamp,
              .source_value_range = {0, 5},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0.5, 1.5},
          },
      }}}};
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .elapsed_time = Duration32::Zero()},
      {.position = {0, 1}, .elapsed_time = Duration32::Seconds(3)},
      {.position = {0, 2}, .elapsed_time = Duration32::Seconds(5.1)},
      {.position = {0, 3}, .elapsed_time = Duration32::Seconds(6)},
      {.position = {0, 4},
       .elapsed_time = Duration32::Seconds(10)},  // Last stable input
      {.position = {0, 5}, .elapsed_time = Duration32::Seconds(13)},
  };
  StrokeInputModeler::State input_modeler_state = {
      .complete_elapsed_time = Duration32::Seconds(13),
      .stable_input_count = 5,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The tip states created from the last three stable inputs should be volatile
  // because they are less than 5 seconds from the stable elapsed time:
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 2);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 4);

  // Check again with the source value range reversed, as it should still cause
  // the same behavior upper bound.
  source_node->source_value_range = {5, 0};
  modeler.StartStroke(&brush_tip, 1);
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 2);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 4);
}

TEST(BrushTipModelerTest, TipWithMillisecondsRemainingBehavior) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kTimeSinceInputInMillis,
              .source_out_of_range_behavior = BrushBehavior::OutOfRange::kClamp,
              .source_value_range = {0, 1500},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0.5, 1.5},
          },
      }}}};
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .elapsed_time = Duration32::Zero()},
      {.position = {0, 1}, .elapsed_time = Duration32::Seconds(3)},
      {.position = {0, 2}, .elapsed_time = Duration32::Seconds(5)},
      {.position = {0, 3}, .elapsed_time = Duration32::Seconds(8.4)},
      {.position = {0, 4}, .elapsed_time = Duration32::Seconds(10)},
  };
  StrokeInputModeler::State input_modeler_state = {
      .complete_elapsed_time = Duration32::Seconds(14),

      .stable_input_count = 5,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Even though all of the inputs in the state were stable, the tip state
  // created from the last input state should be volatile because it is less
  // than 1500 milliseconds from the current stable end of the stroke:
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 4);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 1);

  // Check again with the source value range reversed, as it should still cause
  // the same behavior upper bound.
  source_node->source_value_range = {1500, 0};
  modeler.StartStroke(&brush_tip, 1);
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 4);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 1);
}

TEST(BrushTipModelerTest, TipWithMultipleTimeSinceInputBehaviors) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::kTimeSinceInputInSeconds,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kClamp,
                  .source_value_range = {0, 5},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kWidthMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }},
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::kTimeSinceInputInMillis,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kClamp,
                  .source_value_range = {4000, 0},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kHeightMultiplier,
                  .target_modifier_range = {0.5, 1.5},
              },
          }}}};
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs = {
      {.position = {0, 0}, .elapsed_time = Duration32::Zero()},
      {.position = {0, 1}, .elapsed_time = Duration32::Seconds(1)},
      {.position = {0, 2}, .elapsed_time = Duration32::Seconds(2)},
      {.position = {0, 3}, .elapsed_time = Duration32::Seconds(3)},
      {.position = {0, 4}, .elapsed_time = Duration32::Seconds(4)},
      {.position = {0, 5},
       .elapsed_time = Duration32::Seconds(5)},  // Last stable input
      {.position = {0, 6}, .elapsed_time = Duration32::Seconds(6)},
  };
  StrokeInputModeler::State input_modeler_state = {
      .complete_elapsed_time = Duration32::Seconds(7.5),

      .stable_input_count = 6,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  // With multiple behaviors targeting time remaining, the volatile stroke
  // duration upper bound should be the maximum of the values calculated for
  // each behavior, which in this case is 5 seconds from the stable end of the
  // stroke.
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 1);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 6);
}

TEST(BrushTipModelerTest, TipWithNonZeroParticleGapDistance) {
  BrushTipModeler modeler;

  // Particles should be emitted once every 3 stroke units of distance traveled.
  BrushTip brush_tip = {.particle_gap_distance_scale = 1.f};
  modeler.StartStroke(&brush_tip, /* brush_size = */ 3);

  StrokeInputModeler::State input_modeler_state;
  std::vector<ModeledStrokeInput> inputs = {
      {.position = Point{0, 0}, .traveled_distance = 0},
  };
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Two fixed states should be created for the first input: one for the
  // particle, and a second state with zero size that will be used to create an
  // extrusion-break.
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{1, 0}, .traveled_distance = 1});
  inputs.push_back({.position = Point{2, 0}, .traveled_distance = 2});
  inputs.push_back({.position = Point{2.9, 0}, .traveled_distance = 2.9});
  input_modeler_state.stable_input_count = inputs.size() - 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeled inputs have not traveled an additional three units, so no new
  // tip states should be have been created:
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{3, 0}, .traveled_distance = 3});
  input_modeler_state.stable_input_count = inputs.size() - 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeled inputs have crossed the threshold of traveling three units of
  // distance since the last particle, so another two new states are expected:
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(
      modeler.VolatileTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));

  inputs.push_back({.position = Point{10, 0}, .traveled_distance = 10});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // New modeled inputs have crossed the distance threshold three times, because
  // the input with `traveled_distance = 3` was unstable. So we expect 6 new tip
  // states:
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0))),
                  AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0))),
                  AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());
}

TEST(BrushTipModelerTest, TipWithNonZeroParticleGapDuration) {
  BrushTipModeler modeler;

  // Particles should be emitted once every 100 ms.
  BrushTip brush_tip = {.particle_gap_duration = Duration32::Millis(100)};
  modeler.StartStroke(&brush_tip, /* brush_size = */ 1);

  StrokeInputModeler::State input_modeler_state;
  std::vector<ModeledStrokeInput> inputs = {
      {.position = Point{0, 0}, .elapsed_time = Duration32::Zero()},
  };
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Two fixed states should be created for the first input: one for the
  // particle, and a second state with zero size that will be used to create an
  // extrusion-break.
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back(
      {.position = Point{1, 0}, .elapsed_time = Duration32::Millis(50)});
  inputs.push_back(
      {.position = Point{2, 0}, .elapsed_time = Duration32::Millis(75)});
  input_modeler_state.stable_input_count = inputs.size() - 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeled inputs have not traveled for 100 ms, so no new tip states
  // should be have been created:
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back(
      {.position = Point{3, 0}, .elapsed_time = Duration32::Millis(100)});
  input_modeler_state.stable_input_count = inputs.size() - 1;
  modeler.UpdateStroke(input_modeler_state, inputs);

  // The modeled inputs have crossed the threshold of traveling for 100 ms since
  // the last particle, so another two new tip states are expected:
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(
      modeler.VolatileTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));

  inputs.push_back(
      {.position = Point{4, 0}, .elapsed_time = Duration32::Millis(399)});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // New modeled inputs have traveled just shy of four multiples of the
  // threshold duration, because the `elapsed_time = 100ms` inputs was unstable.
  // So six new tip states (for three particles) are expected:
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0))),
                  AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0))),
                  AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());
}

TEST(BrushTipModelerTest, TipWithNonZeroParticleGapDistanceAndDuration) {
  BrushTipModeler modeler;

  // Particles should be emitted whenever inputs have traveled both a distance
  // of 1 unit and a duration of 50 ms since the last emitted particle.
  BrushTip brush_tip = {.particle_gap_distance_scale = 0.5,
                        .particle_gap_duration = Duration32::Millis(50)};
  modeler.StartStroke(&brush_tip, /* brush_size = */ 2);

  StrokeInputModeler::State input_modeler_state;
  std::vector<ModeledStrokeInput> inputs = {
      {.position = Point{0, 0},
       .traveled_distance = 0,
       .elapsed_time = Duration32::Zero()},
  };
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Two fixed states should be created for the first input: one for the
  // particle, and a second state with zero size that will be used to create an
  // extrusion-break.
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{1.5, 0},
                    .traveled_distance = 1.5,
                    .elapsed_time = Duration32::Zero()});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // No new tip states should have been created, because the inputs have
  // traveled far enough in distance, but not in time.
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{1.6, 0},
                    .traveled_distance = 1.6,
                    .elapsed_time = Duration32::Millis(125)});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Now, the inputs have crossed the time threshold in addition to distance:
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{1.9, 0},
                    .traveled_distance = 1.9,
                    .elapsed_time = Duration32::Millis(200)});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // This time, there should be no new tip states because even though the time
  // has advanced past the theshold since the last emitted particle, the
  // distance has not.
  EXPECT_THAT(modeler.NewFixedTipStates(), IsEmpty());
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());

  inputs.push_back({.position = Point{2.6, 0},
                    .traveled_distance = 2.6,
                    .elapsed_time = Duration32::Millis(200)});
  input_modeler_state.stable_input_count = inputs.size();
  modeler.UpdateStroke(input_modeler_state, inputs);

  // Now, the inputs have crossed the distance threshold in addition to time:
  EXPECT_THAT(
      modeler.NewFixedTipStates(),
      ElementsAre(AllOf(Field("width", &BrushTipState::width, Ne(0)),
                        Field("height", &BrushTipState::height, Ne(0))),
                  AllOf(Field("width", &BrushTipState::width, Eq(0)),
                        Field("height", &BrushTipState::height, Eq(0)))));
  EXPECT_THAT(modeler.VolatileTipStates(), IsEmpty());
}

TEST(BrushTipModelerTest, UnstableTargetModifierReplacedWithNull) {
  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationAboutZeroInRadians,
              .source_value_range = {0, kHalfTurn.ValueInRadians()},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0.5, 1.5},
          },
      }}},
  };
  modeler.StartStroke(&brush_tip, 1);

  // The two inputs represent a stylus close to perpendicular to the screen,
  // oriented in two opposite directions.
  std::vector<ModeledStrokeInput> inputs = {
      {.tilt = Angle::Degrees(1), .orientation = Angle::Degrees(0)},
      {.tilt = Angle::Degrees(1), .orientation = Angle::Degrees(180)}};
  StrokeInputModeler::State input_modeler_state = {
      .stable_input_count = 1,
  };
  modeler.UpdateStroke(input_modeler_state, inputs);

  ASSERT_EQ(modeler.NewFixedTipStates().size(), 1);
  EXPECT_THAT(modeler.NewFixedTipStates().front().width, FloatNear(0.5, 0.01));
  ASSERT_EQ(modeler.VolatileTipStates().size(), 1);
  EXPECT_THAT(modeler.VolatileTipStates().front().width, FloatNear(1.5, 0.01));

  // Replace the unstable input so that the tilt is zero, which should
  // disable the orientation behavior.
  inputs.back() = {.tilt = Angle(), .orientation = Angle::Degrees(60)};

  modeler.UpdateStroke(input_modeler_state, inputs);

  EXPECT_TRUE(modeler.NewFixedTipStates().empty());

  // This incorrectly reports a value of 1.5 without `fixed_behavior_modifiers_`
  ASSERT_EQ(modeler.VolatileTipStates().size(), 1);
  EXPECT_THAT(modeler.VolatileTipStates().front().width, FloatNear(0.5, 0.01));
}

class BrushTipModelerSourceParamTest
    : public testing::TestWithParam<BrushBehavior::Source> {};

TEST_P(BrushTipModelerSourceParamTest, LastStableInputCreatesVolatileTipState) {
  BrushBehavior::Source behavior_source = GetParam();

  BrushTipModeler modeler;
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = behavior_source,
              .source_value_range = {0, kFullTurn.ValueInRadians()},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kRotationOffsetInRadians,
              .target_modifier_range = {0, kFullTurn.ValueInRadians()},
          },
      }}},
  };
  modeler.StartStroke(&brush_tip, 1);

  std::vector<ModeledStrokeInput> inputs;
  StrokeInputModeler::State input_modeler_state = {};

  // A single stable input should be used to create a single volatile tip state:
  inputs.resize(1);
  input_modeler_state.stable_input_count = 1;
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_EQ(modeler.VolatileTipStates().size(), 1);

  // Adding an unstable input should result in it and the first stable input
  // being used to create volatile tip states:
  inputs.resize(2);
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_TRUE(modeler.NewFixedTipStates().empty());
  EXPECT_EQ(modeler.VolatileTipStates().size(), 2);

  // If the second input is switched to stable, it should still create a
  // volatile tip state along with the new unstable input. The first input
  // should now be able to create a fixed tip state:
  inputs.resize(3);
  input_modeler_state.stable_input_count = 2;
  modeler.UpdateStroke(input_modeler_state, inputs);
  EXPECT_EQ(modeler.NewFixedTipStates().size(), 1);
  EXPECT_EQ(modeler.VolatileTipStates().size(), 2);
}

INSTANTIATE_TEST_SUITE_P(
    SourceUsesNextInput, BrushTipModelerSourceParamTest,
    ::testing::Values(BrushBehavior::Source::kDirectionInRadians,
                      BrushBehavior::Source::kDirectionAboutZeroInRadians,
                      BrushBehavior::Source::kNormalizedDirectionX,
                      BrushBehavior::Source::kNormalizedDirectionY));

TEST(BrushTipModelerDeathTest, NullptrBrushTip) {
  BrushTipModeler modeler;
  // The `brush_tip` parameter is annotated nonnull, but we want to test the
  // defensive null check. Use a variable instead of passing nullptr directly
  // to avoid a `-Wnonnull` warning.
  BrushTip* null_brush_tip = nullptr;
  EXPECT_DEATH_IF_SUPPORTED(modeler.StartStroke(null_brush_tip, 1), "");
}

TEST(BrushTipModelerDeathTest, ZeroBrushSize) {
  BrushTipModeler modeler;
  BrushTip tip;
  EXPECT_DEATH_IF_SUPPORTED(modeler.StartStroke(&tip, 0), "");
}

TEST(BrushTipModelerDeathTest, InfiniteBrushSize) {
  BrushTipModeler modeler;
  BrushTip tip;
  EXPECT_DEATH_IF_SUPPORTED(
      modeler.StartStroke(&tip, std::numeric_limits<float>::infinity()), "");
}

TEST(BrushTipModelerDeathTest, NanBrushSize) {
  BrushTipModeler modeler;
  BrushTip tip;
  static_assert(std::numeric_limits<float>::has_quiet_NaN);
  EXPECT_DEATH_IF_SUPPORTED(
      modeler.StartStroke(&tip, std::numeric_limits<float>::quiet_NaN()), "");
}

}  // namespace
}  // namespace ink::strokes_internal
