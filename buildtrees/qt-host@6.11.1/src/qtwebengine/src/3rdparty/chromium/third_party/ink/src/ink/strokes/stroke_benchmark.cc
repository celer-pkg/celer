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
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/stroke.h"

namespace ink {
namespace {

constexpr float kInputBoundsWidth = 200;
constexpr float kInputBoundsHeight = 200;
constexpr float kBrushEpsilon = 0.01;

std::vector<StrokeInputBatch> MakeInputBatches() {
  Rect bounds =
      Rect::FromTwoPoints({0, 0}, {kInputBoundsWidth, kInputBoundsHeight});
  return {
      MakeCompleteStraightLineInputs(bounds),
      MakeCompleteSpringShapeInputs(bounds),
  };
}

Brush MakeBrush(BrushTip brush_tip, float brush_size) {
  absl::StatusOr<BrushFamily> family =
      BrushFamily::Create(brush_tip, BrushPaint{});
  ABSL_CHECK_OK(family);
  absl::StatusOr<Brush> brush = Brush::Create(
      *std::move(family), Color::Black(), brush_size, kBrushEpsilon);
  ABSL_CHECK_OK(brush);
  return *std::move(brush);
}

void BM_CircleBrushWithSize(benchmark::State& state) {
  std::vector<StrokeInputBatch> input_batches = MakeInputBatches();
  Brush brush = MakeBrush(BrushTip{.scale = {1, 1}, .corner_rounding = 1},
                          state.range(0));

  while (state.KeepRunningBatch(input_batches.size())) {
    for (const StrokeInputBatch& inputs : input_batches) {
      Stroke stroke(brush, inputs);
      benchmark::DoNotOptimize(stroke);
    }
  }
}
BENCHMARK(BM_CircleBrushWithSize)->RangeMultiplier(4)->Range(1, 32);

void BM_RoundedRectangleBrushWithSize(benchmark::State& state) {
  std::vector<StrokeInputBatch> input_batches = MakeInputBatches();
  Brush brush = MakeBrush(BrushTip{.scale = {0.1, 1.0}, .corner_rounding = 0.2},
                          state.range(0));

  while (state.KeepRunningBatch(input_batches.size())) {
    for (const StrokeInputBatch& inputs : input_batches) {
      Stroke stroke(brush, inputs);
      benchmark::DoNotOptimize(stroke);
    }
  }
}
BENCHMARK(BM_RoundedRectangleBrushWithSize)->RangeMultiplier(4)->Range(1, 32);

void BM_CircleBrushWithSizeSingleBehavior(benchmark::State& state) {
  std::vector<StrokeInputBatch> input_batches = MakeInputBatches();
  BrushTip brush_tip = {
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
  Brush brush = MakeBrush(brush_tip, state.range(0));

  while (state.KeepRunningBatch(input_batches.size())) {
    for (const StrokeInputBatch& inputs : input_batches) {
      Stroke stroke(brush, inputs);
      benchmark::DoNotOptimize(stroke);
    }
  }
}
BENCHMARK(BM_CircleBrushWithSizeSingleBehavior)
    ->RangeMultiplier(4)
    ->Range(1, 32);

void BM_RoundedRectangleBrushWithSizeMultipleBehavior(benchmark::State& state) {
  std::vector<StrokeInputBatch> input_batches = MakeInputBatches();
  BrushTip brush_tip = {
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
          }},
      }};
  Brush brush = MakeBrush(brush_tip, state.range(0));

  while (state.KeepRunningBatch(input_batches.size())) {
    for (const StrokeInputBatch& inputs : input_batches) {
      Stroke stroke(brush, inputs);
      benchmark::DoNotOptimize(stroke);
    }
  }
}
BENCHMARK(BM_RoundedRectangleBrushWithSizeMultipleBehavior)
    ->RangeMultiplier(4)
    ->Range(1, 32);

}  // namespace
}  // namespace ink
