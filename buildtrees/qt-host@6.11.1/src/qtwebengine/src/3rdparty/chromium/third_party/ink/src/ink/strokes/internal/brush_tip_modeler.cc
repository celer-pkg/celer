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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <variant>

#include "absl/algorithm/container.h"
#include "absl/base/nullability.h"
#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_modeler_helpers.h"
#include "ink/strokes/internal/easing_implementation.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/types/duration.h"

namespace ink::strokes_internal {
namespace {

std::pair<BrushBehavior::Target, BrushBehavior::Target> PolarTargetXyPair(
    BrushBehavior::PolarTarget polar_target) {
  switch (polar_target) {
    case BrushBehavior::PolarTarget::
        kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize:
      return {BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize,
              BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize};
    case BrushBehavior::PolarTarget::
        kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize:
      return {
          BrushBehavior::Target::kPositionOffsetForwardInMultiplesOfBrushSize,
          BrushBehavior::Target::kPositionOffsetLateralInMultiplesOfBrushSize};
  }
  ABSL_LOG(FATAL)
      << "`polar_target` should not be able to have non-enumerator value: "
      << static_cast<int>(polar_target);
}

float InitialTargetModifierValue(BrushBehavior::Target target) {
  switch (target) {
    case BrushBehavior::Target::kWidthMultiplier:
    case BrushBehavior::Target::kHeightMultiplier:
    case BrushBehavior::Target::kSizeMultiplier:
    case BrushBehavior::Target::kSaturationMultiplier:
    case BrushBehavior::Target::kOpacityMultiplier:
      return 1.f;
    case BrushBehavior::Target::kSlantOffsetInRadians:
    case BrushBehavior::Target::kPinchOffset:
    case BrushBehavior::Target::kRotationOffsetInRadians:
    case BrushBehavior::Target::kCornerRoundingOffset:
    case BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize:
    case BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize:
    case BrushBehavior::Target::kPositionOffsetForwardInMultiplesOfBrushSize:
    case BrushBehavior::Target::kPositionOffsetLateralInMultiplesOfBrushSize:
    case BrushBehavior::Target::kTextureAnimationProgressOffset:
    case BrushBehavior::Target::kHueOffsetInRadians:
    case BrushBehavior::Target::kLuminosity:
      return 0.f;
  }
  ABSL_LOG(FATAL)
      << "`target` should not be able to have non-enumerator value: "
      << static_cast<int>(target);
}

bool SourceOutOfRangeBehaviorHasUpperBound(
    BrushBehavior::OutOfRange source_out_of_range_behavior) {
  switch (source_out_of_range_behavior) {
    case BrushBehavior::OutOfRange::kClamp:
      return true;
    case BrushBehavior::OutOfRange::kRepeat:
    case BrushBehavior::OutOfRange::kMirror:
      return false;
  }
  ABSL_LOG(FATAL)
      << "`source_out_of_range_behavior` should not be able to have: "
         "non-enumerator value: "
      << static_cast<int>(source_out_of_range_behavior);
}

// Returns the upper bound for values of input source that are affected by this
// `behavior`. This means that values greater than or equal to the returned
// value will all result in the same calculated target modification.
float SourceValueUpperBound(const BrushBehavior::SourceNode& node) {
  if (!SourceOutOfRangeBehaviorHasUpperBound(
          node.source_out_of_range_behavior)) {
    return std::numeric_limits<float>::infinity();
  }
  return std::max(node.source_value_range[0], node.source_value_range[1]);
}

float DistanceRemainingUpperBound(const BrushBehavior::SourceNode& node,
                                  float brush_size) {
  switch (node.source) {
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
      return brush_size * SourceValueUpperBound(node);
    case BrushBehavior::Source::kNormalizedPressure:
    case BrushBehavior::Source::kTiltInRadians:
    case BrushBehavior::Source::kTiltXInRadians:
    case BrushBehavior::Source::kTiltYInRadians:
    case BrushBehavior::Source::kOrientationInRadians:
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kDirectionInRadians:
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
    case BrushBehavior::Source::kNormalizedDirectionX:
    case BrushBehavior::Source::kNormalizedDirectionY:
    case BrushBehavior::Source::kTimeOfInputInSeconds:
    case BrushBehavior::Source::kTimeOfInputInMillis:
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
    case BrushBehavior::Source::kTimeSinceInputInMillis:
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      return 0;
  }
  ABSL_LOG(FATAL)
      << "`node.source` should not be able to have non-enumerator value: "
      << static_cast<int>(node.source);
}

Duration32 TimeRemainingUpperBound(const BrushBehavior::SourceNode& node) {
  switch (node.source) {
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
      return Duration32::Seconds(SourceValueUpperBound(node));
    case BrushBehavior::Source::kTimeSinceInputInMillis:
      return Duration32::Millis(SourceValueUpperBound(node));
    case BrushBehavior::Source::kNormalizedPressure:
    case BrushBehavior::Source::kTiltInRadians:
    case BrushBehavior::Source::kTiltXInRadians:
    case BrushBehavior::Source::kTiltYInRadians:
    case BrushBehavior::Source::kOrientationInRadians:
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kDirectionInRadians:
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
    case BrushBehavior::Source::kNormalizedDirectionX:
    case BrushBehavior::Source::kNormalizedDirectionY:
    case BrushBehavior::Source::kTimeOfInputInSeconds:
    case BrushBehavior::Source::kTimeOfInputInMillis:
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      return Duration32::Zero();
  }
  ABSL_LOG(FATAL)
      << "`node.source` should not be able to have non-enumerator value: "
      << static_cast<int>(node.source);
}

bool SourceDependsOnNextModeledInput(BrushBehavior::Source source) {
  switch (source) {
    case BrushBehavior::Source::kDirectionInRadians:
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
    case BrushBehavior::Source::kNormalizedDirectionX:
    case BrushBehavior::Source::kNormalizedDirectionY:
      return true;
    case BrushBehavior::Source::kNormalizedPressure:
    case BrushBehavior::Source::kTiltInRadians:
    case BrushBehavior::Source::kTiltXInRadians:
    case BrushBehavior::Source::kTiltYInRadians:
    case BrushBehavior::Source::kOrientationInRadians:
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kTimeOfInputInSeconds:
    case BrushBehavior::Source::kTimeOfInputInMillis:
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
    case BrushBehavior::Source::kTimeSinceInputInMillis:
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      break;
  }
  return false;
}

Duration32 TimeSinceLastInput(
    const StrokeInputModeler::State& input_modeler_state) {
  // TODO: b/287041801 - Do we need to consider predicted inputs here too?
  return input_modeler_state.complete_elapsed_time -
         input_modeler_state.total_real_elapsed_time;
}

}  // namespace

void BrushTipModeler::StartStroke(const BrushTip* absl_nonnull brush_tip,
                                  float brush_size, uint32_t noise_seed) {
  ABSL_CHECK_NE(brush_tip, nullptr);
  ABSL_CHECK(std::isfinite(brush_size));
  ABSL_CHECK_GT(brush_size, 0);

  brush_tip_ = brush_tip;
  brush_size_ = brush_size;
  noise_seed_ = noise_seed;

  // These fields will be updated as the stroke progresses.
  input_index_for_next_fixed_state_ = 0;
  particle_gap_metrics_ = {
      .traveled_distance = brush_tip->particle_gap_distance_scale * brush_size,
      .elapsed_time = brush_tip->particle_gap_duration,
  };
  last_fixed_modeled_tip_state_metrics_.reset();
  saved_tip_states_.clear();
  new_fixed_tip_state_count_ = 0;

  // These fields will be updated by the `AppendBehaviorNode()` loop below.
  distance_remaining_behavior_upper_bound_ = 0;
  distance_fraction_behavior_upper_bound_ = 0;
  time_remaining_behavior_upper_bound_ = Duration32::Zero();
  behaviors_depend_on_next_input_ = false;

  behavior_nodes_.clear();
  current_noise_generators_.clear();
  fixed_noise_generators_.clear();
  current_damped_values_.clear();
  fixed_damped_values_.clear();
  behavior_targets_.clear();
  current_target_modifiers_.clear();
  fixed_target_modifiers_.clear();

  for (const BrushBehavior& behavior : brush_tip_->behaviors) {
    for (const BrushBehavior::Node& node : behavior.nodes) {
      std::visit([this](const auto& node) { this->AppendBehaviorNode(node); },
                 node);
    }
  }
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::SourceNode& node) {
  distance_remaining_behavior_upper_bound_ =
      std::max(distance_remaining_behavior_upper_bound_,
               DistanceRemainingUpperBound(node, brush_size_));
  time_remaining_behavior_upper_bound_ = std::max(
      time_remaining_behavior_upper_bound_, TimeRemainingUpperBound(node));
  if (node.source ==
      BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength) {
    distance_fraction_behavior_upper_bound_ = std::max(
        distance_fraction_behavior_upper_bound_, SourceValueUpperBound(node));
  }
  behavior_nodes_.push_back(node);
  if (SourceDependsOnNextModeledInput(node.source)) {
    behaviors_depend_on_next_input_ = true;
  }
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::ConstantNode& node) {
  behavior_nodes_.push_back(node);
}

void BrushTipModeler::AppendBehaviorNode(const BrushBehavior::NoiseNode& node) {
  behavior_nodes_.push_back(NoiseNodeImplementation{
      .generator_index = current_noise_generators_.size(),
      .vary_over = node.vary_over,
      .base_period = node.base_period,
  });
  uint64_t combined_seed = (static_cast<uint64_t>(noise_seed_) << 32) |
                           static_cast<uint64_t>(node.seed);
  current_noise_generators_.emplace_back(combined_seed);
  fixed_noise_generators_.push_back(current_noise_generators_.back());
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::FallbackFilterNode& node) {
  behavior_nodes_.push_back(node);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::ToolTypeFilterNode& node) {
  behavior_nodes_.push_back(node);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::DampingNode& node) {
  behavior_nodes_.push_back(DampingNodeImplementation{
      .damping_index = current_damped_values_.size(),
      .damping_source = node.damping_source,
      .damping_gap = node.damping_gap,
  });
  current_damped_values_.push_back(kNullBehaviorNodeValue);
  fixed_damped_values_.push_back(kNullBehaviorNodeValue);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::ResponseNode& node) {
  behavior_nodes_.push_back(EasingImplementation(node.response_curve));
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::BinaryOpNode& node) {
  behavior_nodes_.push_back(node);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::InterpolationNode& node) {
  behavior_nodes_.push_back(node);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::TargetNode& node) {
  behavior_nodes_.push_back(TargetNodeImplementation{
      .target_index = behavior_targets_.size(),
      .target_modifier_range = node.target_modifier_range,
  });
  behavior_targets_.push_back(node.target);
  float initial_modifier = InitialTargetModifierValue(node.target);
  current_target_modifiers_.push_back(initial_modifier);
  fixed_target_modifiers_.push_back(initial_modifier);
}

void BrushTipModeler::AppendBehaviorNode(
    const BrushBehavior::PolarTargetNode& node) {
  auto [target_x, target_y] = PolarTargetXyPair(node.target);
  behavior_nodes_.push_back(PolarTargetNodeImplementation{
      .target_x_index = behavior_targets_.size(),
      .target_y_index = behavior_targets_.size() + 1,
      .angle_range = node.angle_range,
      .magnitude_range = node.magnitude_range,
  });
  behavior_targets_.push_back(target_x);
  behavior_targets_.push_back(target_y);
  float initial_modifier_x = InitialTargetModifierValue(target_x);
  float initial_modifier_y = InitialTargetModifierValue(target_y);
  current_target_modifiers_.push_back(initial_modifier_x);
  current_target_modifiers_.push_back(initial_modifier_y);
  fixed_target_modifiers_.push_back(initial_modifier_x);
  fixed_target_modifiers_.push_back(initial_modifier_y);
}

namespace {

std::optional<Angle> GetTravelDirection(
    absl::Span<const ModeledStrokeInput> inputs, size_t i) {
  const Point& start = i > 0 ? inputs[i - 1].position : inputs[i].position;
  const Point& end =
      i + 1 < inputs.size() ? inputs[i + 1].position : inputs[i].position;
  if (start == end) return std::nullopt;
  return (end - start).Direction();
}

}  // namespace

void BrushTipModeler::UpdateStroke(
    const StrokeInputModeler::State& input_modeler_state,
    absl::Span<const ModeledStrokeInput> inputs) {
  ABSL_CHECK_NE(brush_tip_, nullptr);

  saved_tip_states_.clear();
  if (inputs.empty()) return;

  InputMetrics max_fixed_metrics =
      CalculateMaxFixedInputMetrics(input_modeler_state, inputs);
  ABSL_DCHECK_EQ(fixed_noise_generators_.size(),
                 current_noise_generators_.size());
  absl::c_copy(fixed_noise_generators_, current_noise_generators_.begin());
  ABSL_DCHECK_EQ(fixed_damped_values_.size(), current_damped_values_.size());
  absl::c_copy(fixed_damped_values_, current_damped_values_.begin());
  ABSL_DCHECK_EQ(fixed_target_modifiers_.size(),
                 current_target_modifiers_.size());
  absl::c_copy(fixed_target_modifiers_, current_target_modifiers_.begin());

  const ModeledStrokeInput* absl_nullable previous_input =
      input_index_for_next_fixed_state_ > 0
          ? &inputs[input_index_for_next_fixed_state_ - 1]
          : nullptr;
  std::optional<InputMetrics> last_modeled_tip_state_metrics =
      last_fixed_modeled_tip_state_metrics_;

  // Generate new fixed tip states, making sure to only use stable input and
  // reserving the last stable input if any behaviors would actually depend on
  // the first unstable input.
  int reserved_stable_input = behaviors_depend_on_next_input_ ? 1 : 0;
  while (input_index_for_next_fixed_state_ + reserved_stable_input <
         input_modeler_state.stable_input_count) {
    const ModeledStrokeInput& current_input =
        inputs[input_index_for_next_fixed_state_];

    // If the current `brush_tip_` has behaviors targeting distance or time
    // remaining, not all "stable" `ModeledStrokeInput` can be used to make
    // "fixed" `BrushTipState`. We stop considering the tip states "fixed"
    // once we've reached either of the maxima:
    if (current_input.traveled_distance > max_fixed_metrics.traveled_distance ||
        current_input.elapsed_time > max_fixed_metrics.elapsed_time) {
      break;
    }

    ProcessSingleInput(
        input_modeler_state, current_input,
        GetTravelDirection(inputs, input_index_for_next_fixed_state_),
        previous_input, last_modeled_tip_state_metrics);
    previous_input = &current_input;
    ++input_index_for_next_fixed_state_;
  }

  // Save the necessary fixed properties:
  last_fixed_modeled_tip_state_metrics_ = last_modeled_tip_state_metrics;
  new_fixed_tip_state_count_ = saved_tip_states_.size();
  ABSL_DCHECK_EQ(current_noise_generators_.size(),
                 fixed_noise_generators_.size());
  absl::c_copy(current_noise_generators_, fixed_noise_generators_.begin());
  ABSL_DCHECK_EQ(current_damped_values_.size(), fixed_damped_values_.size());
  absl::c_copy(current_damped_values_, fixed_damped_values_.begin());
  ABSL_DCHECK_EQ(current_target_modifiers_.size(),
                 fixed_target_modifiers_.size());
  absl::c_copy(current_target_modifiers_, fixed_target_modifiers_.begin());

  // Generate the remaining tip states, which are volatile:
  for (size_t i = input_index_for_next_fixed_state_; i < inputs.size(); ++i) {
    const ModeledStrokeInput& current_input = inputs[i];

    ProcessSingleInput(input_modeler_state, current_input,
                       GetTravelDirection(inputs, i), previous_input,
                       last_modeled_tip_state_metrics);
    previous_input = &current_input;
  }
}

bool BrushTipModeler::HasUnfinishedTimeBehaviors(
    const StrokeInputModeler::State& input_modeler_state) const {
  return TimeSinceLastInput(input_modeler_state) <
         time_remaining_behavior_upper_bound_;
}

InputMetrics BrushTipModeler::CalculateMaxFixedInputMetrics(
    const StrokeInputModeler::State& input_modeler_state,
    absl::Span<const ModeledStrokeInput> inputs) const {
  if (input_modeler_state.stable_input_count == 0) {
    return {.traveled_distance = 0, .elapsed_time = Duration32::Zero()};
  }

  // Measure from the last stable input, because all unstable inputs in the
  // `StrokeInputModeler::State` may be removed in future updates. We can only
  // count on the last stable properties to be non-decreasing.
  const ModeledStrokeInput& last_stable_input =
      inputs[input_modeler_state.stable_input_count - 1];
  return {
      .traveled_distance =
          last_stable_input.traveled_distance -
          std::max(distance_remaining_behavior_upper_bound_,
                   distance_fraction_behavior_upper_bound_ *
                       input_modeler_state.complete_traveled_distance),
      .elapsed_time =
          last_stable_input.elapsed_time - time_remaining_behavior_upper_bound_,
  };
}

void BrushTipModeler::ProcessSingleInput(
    const StrokeInputModeler::State& input_modeler_state,
    const ModeledStrokeInput& current_input,
    std::optional<Angle> current_travel_direction,
    const ModeledStrokeInput* absl_nullable previous_input,
    std::optional<InputMetrics>& last_modeled_tip_state_metrics) {
  bool do_continuous_extrusion =
      particle_gap_metrics_.traveled_distance == 0 &&
      particle_gap_metrics_.elapsed_time == Duration32::Zero();

  if (do_continuous_extrusion || !last_modeled_tip_state_metrics.has_value()) {
    // This is either
    //   a) Continuous extrusion,
    // OR
    //   b) Particle extrusion, but no tip states have been modeled so far,
    //      which should always result in emitting a single particle.

    AddNewTipState(
        input_modeler_state, current_input, current_travel_direction,
        previous_input == nullptr
            ? std::nullopt
            : std::optional<InputMetrics>({
                  .traveled_distance = previous_input->traveled_distance,
                  .elapsed_time = previous_input->elapsed_time,
              }),
        last_modeled_tip_state_metrics);

    if (!do_continuous_extrusion) AppendParticleGapTipState();
    return;
  }

  // The remainder of the function handles emitting particles by interpolating
  // between the previous and current inputs.

  // If we have already modeled a tip state, we must have already had an input.
  ABSL_DCHECK(previous_input != nullptr);

  // Emit as many particles as can fit according to `particle_gap_metrics_`,
  // taking into account that there will usually be some budget left over from
  // the previous call to this function.  I.e. when emitting particles,
  // `*last_modeled_tip_state_metrics` will usually lag a little bit behind the
  // metrics of `*previous_input`.

  InputMetrics input_delta = {
      .traveled_distance =
          current_input.traveled_distance - previous_input->traveled_distance,
      .elapsed_time = current_input.elapsed_time - previous_input->elapsed_time,
  };

  while ((current_input.traveled_distance -
          last_modeled_tip_state_metrics->traveled_distance) >=
             particle_gap_metrics_.traveled_distance &&
         (current_input.elapsed_time -
          last_modeled_tip_state_metrics->elapsed_time) >=
             particle_gap_metrics_.elapsed_time) {
    // Calculate an interpolation value from the current input toward the
    // previous input for the new particle tip state.
    float t = 1.f;
    if (particle_gap_metrics_.traveled_distance != 0) {
      t = std::min(t, (current_input.traveled_distance -
                       last_modeled_tip_state_metrics->traveled_distance -
                       particle_gap_metrics_.traveled_distance) /
                          input_delta.traveled_distance);
    }
    if (particle_gap_metrics_.elapsed_time != Duration32::Zero()) {
      t = std::min(t, (current_input.elapsed_time -
                       last_modeled_tip_state_metrics->elapsed_time -
                       particle_gap_metrics_.elapsed_time) /
                          input_delta.elapsed_time);
    }
    ModeledStrokeInput lerped_input = Lerp(current_input, *previous_input, t);
    AddNewTipState(input_modeler_state, lerped_input, current_travel_direction,
                   // Use `last_modeled_tip_state_metrics` as the "previous
                   // input" metrics (copied by value):
                   last_modeled_tip_state_metrics,
                   // Mutable reference out-param to replace
                   // `last_modeled_tip_state_metrics` with updated metrics:
                   last_modeled_tip_state_metrics);

    AppendParticleGapTipState();
  }
}

void BrushTipModeler::AddNewTipState(
    const StrokeInputModeler::State& input_modeler_state,
    const ModeledStrokeInput& input, std::optional<Angle> travel_direction,
    std::optional<InputMetrics> previous_input_metrics,
    std::optional<InputMetrics>& last_modeled_tip_state_metrics) {
  ABSL_CHECK_NE(brush_tip_, nullptr);
  BehaviorNodeContext context = {
      .input_modeler_state = input_modeler_state,
      .current_input = input,
      .current_travel_direction = travel_direction,
      .brush_size = brush_size_,
      .previous_input_metrics = previous_input_metrics,
      .stack = behavior_stack_,
      .noise_generators = absl::MakeSpan(current_noise_generators_),
      .damped_values = absl::MakeSpan(current_damped_values_),
      .target_modifiers = absl::MakeSpan(current_target_modifiers_),
  };
  ABSL_DCHECK(behavior_stack_.empty());
  for (const BehaviorNodeImplementation& node : behavior_nodes_) {
    ProcessBehaviorNode(node, context);
  }
  ABSL_DCHECK(behavior_stack_.empty());

  saved_tip_states_.push_back(
      CreateTipState(input.position, travel_direction, *brush_tip_, brush_size_,
                     behavior_targets_, current_target_modifiers_));
  last_modeled_tip_state_metrics = {
      .traveled_distance = input.traveled_distance,
      .elapsed_time = input.elapsed_time,
  };
}

}  // namespace ink::strokes_internal
