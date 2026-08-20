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

#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "benchmark/benchmark.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/color/color.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/input/recorded_test_inputs.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/stroke_shape_builder.h"
#include "ink/strokes/internal/stroke_shape_update.h"
#include "ink/types/duration.h"

namespace ink::strokes_internal {
namespace {

// Starts a new stroke on the passed in builder and adds StrokeInputBatch.
//
// inputs holds pairs of real and predicted inputs that should be processed
// together.
void BuildStrokeShapeIncrementally(
    const Brush& brush,
    const std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& inputs,
    StrokeShapeBuilder& builder) {
  ABSL_CHECK_EQ(brush.CoatCount(), 1u);
  builder.StartStroke(BrushFamily::DefaultInputModel(), brush.GetCoats()[0],
                      brush.GetSize(), brush.GetEpsilon());
  benchmark::DoNotOptimize(builder);
  for (const auto& [real_inputs, predicted_inputs] : inputs) {
    StrokeShapeUpdate update = builder.ExtendStroke(
        real_inputs, predicted_inputs,
        real_inputs.Get(real_inputs.Size() - 1).elapsed_time);
    benchmark::DoNotOptimize(builder);
    benchmark::DoNotOptimize(update);
  }
}

// Creates a new `StrokeShapeBuilder` and calls `BuildStrokeShapeIncrementally`
// with that.
//
// Requires a valid `Brush` and `inputs` in the form of a vector of
// `StrokeInputBatch` pairs, where the first in the pair is the real input, the
// second the predicted input for each call to `ExtendStroke()`.
void BuildStrokeShapeIncrementally(
    const Brush& brush,
    const std::vector<std::pair<StrokeInputBatch, StrokeInputBatch>>& inputs) {
  StrokeShapeBuilder builder;
  benchmark::DoNotOptimize(builder);
  BuildStrokeShapeIncrementally(brush, inputs, builder);
}

// Starts a new stroke on the passed in builder and adds StrokeInputBatch.
void BuildStrokeShapeAllAtOnce(const Brush& brush,
                               const StrokeInputBatch& inputs,
                               StrokeShapeBuilder& builder) {
  benchmark::DoNotOptimize(builder);
  ABSL_CHECK_EQ(brush.CoatCount(), 1u);
  builder.StartStroke(BrushFamily::DefaultInputModel(), brush.GetCoats()[0],
                      brush.GetSize(), brush.GetEpsilon());
  StrokeShapeUpdate update =
      builder.ExtendStroke(inputs, {}, Duration32::Infinite());
  benchmark::DoNotOptimize(update);
}

// Creates a new `StrokeShapebuilder` and calls `BuildStrokeShapeAllAtOnce` with
// that.
//
// Requires a valid `Brush` and `inputs` in the form of a `StrokeInputBatch`
// that combines all Input count for this stroke.
void BuildStrokeShapeAllAtOnce(const Brush& brush,
                               const StrokeInputBatch& inputs) {
  StrokeShapeBuilder builder;
  benchmark::DoNotOptimize(builder);
  BuildStrokeShapeAllAtOnce(brush, inputs, builder);
}

Brush MakeDefaultBrush(float size, float epsilon) {
  BrushFamily family;
  Color color;
  absl::StatusOr<Brush> brush = Brush::Create(family, color, size, epsilon);
  ABSL_CHECK_OK(brush);
  return *std::move(brush);
}

Brush MakeSingleBehaviorBrush(float size, float epsilon) {
  BrushTip tip = {
      .scale = {1, 1},
      .corner_rounding = 1,
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::
                  kDistanceTraveledInMultiplesOfBrushSize,
              .source_out_of_range_behavior =
                  BrushBehavior::OutOfRange::kRepeat,
              .source_value_range = {1, 10},
          },
          BrushBehavior::ResponseNode{
              .response_curve = {EasingFunction::Steps{
                  .step_count = 2,
                  .step_position = EasingFunction::StepPosition::kJumpNone,
              }},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kSizeMultiplier,
              .target_modifier_range = {0, 100},
          },
      }}}};
  absl::StatusOr<BrushFamily> family =
      BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(family);
  Color color;
  absl::StatusOr<Brush> brush = Brush::Create(*family, color, size, epsilon);
  ABSL_CHECK_OK(brush);
  return *std::move(brush);
}

Brush MakeMultiBehaviorBrush(float size, float epsilon) {
  BrushTip tip = {
      .scale = {0.1, 1.0},
      .corner_rounding = 0.2,
      .behaviors = {
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceTraveledInMultiplesOfBrushSize,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kMirror,
                  .source_value_range = {0, 3},
              },
              BrushBehavior::ResponseNode{
                  .response_curve = {EasingFunction::Linear{
                      {{0.2, 0.5}, {0.5, 0.5}}}},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kSizeMultiplier,
                  .target_modifier_range = {0, 1},
              },
          }},
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceTraveledInMultiplesOfBrushSize,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kRepeat,
                  .source_value_range = {1, 10},
              },
              BrushBehavior::ResponseNode{
                  .response_curve = {EasingFunction::Steps{
                      .step_count = 2,
                      .step_position = EasingFunction::StepPosition::kJumpNone,
                  }},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kSizeMultiplier,
                  .target_modifier_range = {0, 100},
              },
          }},
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::
                      kDistanceTraveledInMultiplesOfBrushSize,
                  .source_out_of_range_behavior =
                      BrushBehavior::OutOfRange::kRepeat,
                  .source_value_range = {0, 10},
              },
              BrushBehavior::ResponseNode{
                  .response_curve = {EasingFunction::CubicBezier{
                      .x1 = 0.3, .y1 = 0.2, .x2 = 0.2, .y2 = 1.4}},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kHueOffsetInRadians,
                  .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
              },
          }}}};
  absl::StatusOr<BrushFamily> family =
      BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(family);
  Color color;
  absl::StatusOr<Brush> brush = Brush::Create(*family, color, size, epsilon);
  ABSL_CHECK_OK(brush);
  return *std::move(brush);
}

StrokeInputBatch MakeSyntheticStraightLineInputs(
    const Rect& bounds, int input_count, Duration32 full_stroke_duration) {
  Duration32 time_per_input = full_stroke_duration / input_count;
  float x_per_input = bounds.Width() / input_count;
  float y_center = bounds.Center().y;
  std::vector<StrokeInput> inputs;
  inputs.reserve(input_count);
  for (int i = 0; i < input_count; ++i) {
    inputs.push_back({
        .position = {bounds.XMin() + x_per_input * i, y_center},
        .elapsed_time = time_per_input * i,
    });
  }
  absl::StatusOr<StrokeInputBatch> result = StrokeInputBatch::Create(inputs);
  ABSL_CHECK_OK(result);
  return *std::move(result);
}

// ********************** Benchmark Tests **********************************
//
// For the following tests that have meaningful input (not empty or dot) the
// tests for any given shape will be
//
// - Incremental input (including real and predicted) on cold StrokeShapeBuilder
// - Incremental input on pre-warmed StrokeShapeBuilder
// - All Input count in one StrokeInputBatch on cold StrokeShapeBuilder
// - All Input count in one StrokeInputBatch on pre-warmed StrokeShapeBuilder

// Empty input.
void BM_Empty(benchmark::State& state) {
  Brush brush = MakeDefaultBrush(20, 0.05);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, StrokeInputBatch());
  }
}
BENCHMARK(BM_Empty);

// Runs on a single input point, no predicted input.
void BM_Dot(benchmark::State& state) {
  auto dot_input =
      StrokeInputBatch::Create({{.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {.x = 1888.5, .y = 592.5},
                                 .elapsed_time = Duration32::Millis(17.000001),
                                 .pressure = 0.657875,
                                 .tilt = 0.0833333 * kHalfTurn,
                                 .orientation = 1.5 * kHalfTurn}});
  ABSL_CHECK_OK(dot_input);
  Brush brush = MakeDefaultBrush(20, 0.05);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, *dot_input);
  }
  state.SetLabel("Input count: 1");
}
BENCHMARK(BM_Dot);

// Straight line tests.
void BM_StraightLineIncremental(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalStraightLineInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);

  while (state.KeepRunningBatch(inputs.size()))
    BuildStrokeShapeIncrementally(brush, inputs);
}
BENCHMARK(BM_StraightLineIncremental);

void BM_StraightLineIncrementalPrewarmed(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalStraightLineInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeIncrementally(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);

  while (state.KeepRunningBatch(inputs.size())) {
    BuildStrokeShapeIncrementally(brush, inputs, builder);
  }
}
BENCHMARK(BM_StraightLineIncrementalPrewarmed);

void BM_StraightLineComplete(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteStraightLineInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_StraightLineComplete);

void BM_StraightLineCompletePrewarmed(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteStraightLineInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_StraightLineCompletePrewarmed);

// Spring shape tests.
void BM_SpringShapeIncremental(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);

  while (state.KeepRunningBatch(inputs.size()))
    BuildStrokeShapeIncrementally(brush, inputs);
}
BENCHMARK(BM_SpringShapeIncremental);

void BM_SpringShapeIncrementalPrewarmed(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeIncrementally(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);

  while (state.KeepRunningBatch(inputs.size())) {
    BuildStrokeShapeIncrementally(brush, inputs, builder);
  }
}
BENCHMARK(BM_SpringShapeIncrementalPrewarmed);

void BM_SpringShapeComplete(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeComplete);

void BM_SpringShapeCompletePreWarmed(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeDefaultBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeCompletePreWarmed);

// Spring shape tests with single behavior.
void BM_SpringShapeIncrementalSingleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeSingleBehaviorBrush(20, 0.05);

  while (state.KeepRunningBatch(inputs.size()))
    BuildStrokeShapeIncrementally(brush, inputs);
}
BENCHMARK(BM_SpringShapeIncrementalSingleBehavior);

void BM_SpringShapeIncrementalPrewarmedSingleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeSingleBehaviorBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeIncrementally(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);

  while (state.KeepRunningBatch(inputs.size())) {
    BuildStrokeShapeIncrementally(brush, inputs, builder);
  }
}
BENCHMARK(BM_SpringShapeIncrementalPrewarmedSingleBehavior);

void BM_SpringShapeCompleteSingleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeSingleBehaviorBrush(20, 0.05);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeCompleteSingleBehavior);

void BM_SpringShapeCompletePreWarmedSingleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeSingleBehaviorBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeCompletePreWarmedSingleBehavior);

// Spring shape tests with multiple behaviors.
void BM_SpringShapeIncrementalMultipleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeMultiBehaviorBrush(20, 0.05);

  while (state.KeepRunningBatch(inputs.size()))
    BuildStrokeShapeIncrementally(brush, inputs);
}
BENCHMARK(BM_SpringShapeIncrementalMultipleBehavior);

void BM_SpringShapeIncrementalPrewarmedMultipleBehavior(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  auto inputs = MakeIncrementalSpringShapeInputs(bounds);
  Brush brush = MakeMultiBehaviorBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeIncrementally(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);

  while (state.KeepRunningBatch(inputs.size())) {
    BuildStrokeShapeIncrementally(brush, inputs, builder);
  }
}
BENCHMARK(BM_SpringShapeIncrementalPrewarmedMultipleBehavior);

void BM_SpringShapeCompleteMultipleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeMultiBehaviorBrush(20, 0.05);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeCompleteMultipleBehavior);

void BM_SpringShapeCompletePreWarmedMultipleBehavior(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs = MakeCompleteSpringShapeInputs(bounds);
  Brush brush = MakeMultiBehaviorBrush(20, 0.05);
  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SpringShapeCompletePreWarmedMultipleBehavior);

// ********************** Benchmark Tests **********************************
//
// The following tests we will use synthetically created inputs to test the
// impact of checking for partially overlapping `BrushTip`s. We will use the
// following `BrushTip`s:
// - Rectangle with overlapping corners
// - Pill with overlapping corners
// - Rectangle with no overlapping corners
// - Pill with no overlapping corners
//
// We will also contrast `rotation` and `hue offset` behaviors to rule out the
// impact of just adding a `BrushBehavior`.
//
// We will benchmark the following cases:
// - All Input count in one StrokeInputBatch on cold StrokeShapeBuilder
// - All Input count in one StrokeInputBatch on pre-warmed StrokeShapeBuilder

void BM_SyntheticStraightLineOverlappingRectangleBrushtips(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::ResponseNode{
                      .response_curve = {EasingFunction::Predefined::kLinear},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kRotationOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineOverlappingRectangleBrushtips);

void BM_SyntheticStraightLineOverlappingRectangleBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kRotationOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineOverlappingRectangleBrushtipsPrewarmed);

void BM_SyntheticStraightLineHueChangeRectangleBrushtips(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::ResponseNode{
                      .response_curve = {EasingFunction::Predefined::kLinear},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kHueOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineHueChangeRectangleBrushtips);

void BM_SyntheticStraightLineHueChangeRectangleBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kHueOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineHueChangeRectangleBrushtipsPrewarmed);

void BM_SyntheticStraightLineNoRotationRectangleBrushtips(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineNoRotationRectangleBrushtips);

void BM_SyntheticStraightLineNoRotationRectangleBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 0,
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineNoRotationRectangleBrushtipsPrewarmed);

void BM_SyntheticStraightLineOverlappingPillBrushtips(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::ResponseNode{
                      .response_curve = {EasingFunction::Predefined::kLinear},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kRotationOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineOverlappingPillBrushtips);

void BM_SyntheticStraightLineOverlappingPillBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kRotationOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineOverlappingPillBrushtipsPrewarmed);

void BM_SyntheticStraightLineHueChangePillBrushtips(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::ResponseNode{
                      .response_curve = {EasingFunction::Predefined::kLinear},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kHueOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineHueChangePillBrushtips);

void BM_SyntheticStraightLineHueChangePillBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
      .behaviors =
          {
              BrushBehavior{{
                  BrushBehavior::SourceNode{
                      .source = BrushBehavior::Source::
                          kDistanceTraveledInMultiplesOfBrushSize,
                      .source_out_of_range_behavior =
                          BrushBehavior::OutOfRange::kRepeat,
                      .source_value_range = {0, 4},
                  },
                  BrushBehavior::TargetNode{
                      .target = BrushBehavior::Target::kHueOffsetInRadians,
                      .target_modifier_range = {0, kHalfTurn.ValueInRadians()},
                  },
              }},
          },
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineHueChangePillBrushtipsPrewarmed);

void BM_SyntheticStraightLineNoRotationPillBrushtips(benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineNoRotationPillBrushtips);

void BM_SyntheticStraightLineNoRotationPillBrushtipsPrewarmed(
    benchmark::State& state) {
  Rect bounds = Rect::FromTwoPoints({0, 0}, {100, 100});
  StrokeInputBatch inputs =
      MakeSyntheticStraightLineInputs(bounds, 200, Duration32::Seconds(5));
  BrushTip tip = {
      .scale = {2, 8},
      .corner_rounding = 1,
  };
  auto brush_family = BrushFamily::Create(tip, BrushPaint{}, "");
  ABSL_CHECK_OK(brush_family);
  auto brush = Brush::Create(*brush_family, Color::GoogleBlue(), 1, 0.25);
  ABSL_CHECK_OK(brush);

  StrokeShapeBuilder builder;
  BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
  benchmark::DoNotOptimize(builder);
  for (auto s : state) {
    BuildStrokeShapeAllAtOnce(*brush, inputs, builder);
    benchmark::DoNotOptimize(builder);
  }
  state.SetLabel(absl::StrCat("Input count: ", inputs.Size()));
}
BENCHMARK(BM_SyntheticStraightLineNoRotationPillBrushtipsPrewarmed);

}  // namespace
}  // namespace ink::strokes_internal
