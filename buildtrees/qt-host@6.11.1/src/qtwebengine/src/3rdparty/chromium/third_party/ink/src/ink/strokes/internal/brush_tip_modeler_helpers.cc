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

#include "ink/strokes/internal/brush_tip_modeler_helpers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/modulo.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/easing_implementation.h"
#include "ink/strokes/internal/noise_generator.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::InverseLerp;
using ::ink::geometry_internal::Lerp;

bool IsToolTypeEnabled(BrushBehavior::EnabledToolTypes enabled_tool_types,
                       StrokeInput::ToolType tool_type) {
  switch (tool_type) {
    case StrokeInput::ToolType::kUnknown:
      return enabled_tool_types.unknown;
    case StrokeInput::ToolType::kMouse:
      return enabled_tool_types.mouse;
    case StrokeInput::ToolType::kTouch:
      return enabled_tool_types.touch;
    case StrokeInput::ToolType::kStylus:
      return enabled_tool_types.stylus;
  }
  return false;
}

std::optional<float> GetTiltX(Angle tilt, Angle orientation) {
  if (tilt == Angle()) return 0;
  // When tilt equals pi/2, tilt-x and tilt-y are indeterminate, so we return
  // std::nullopt.
  if (tilt == kQuarterTurn) return std::nullopt;
  return Atan(Cos(orientation) * Tan(tilt)).ValueInRadians();
}

std::optional<float> GetTiltY(Angle tilt, Angle orientation) {
  if (tilt == Angle()) return 0;
  // When tilt equals pi/2, tilt-x and tilt-y are indeterminate, so we return
  // std::nullopt.
  if (tilt == kQuarterTurn) return std::nullopt;
  return Atan(Sin(orientation) * Tan(tilt)).ValueInRadians();
}

float GetPredictedDistanceTraveledInStrokeUnits(
    const StrokeInputModeler::State& input_modeler_state,
    const ModeledStrokeInput& input) {
  return std::max(
      0.f, input.traveled_distance - input_modeler_state.total_real_distance);
}

Duration32 GetPredictedTimeElapsed(
    const StrokeInputModeler::State& input_modeler_state,
    const ModeledStrokeInput& input) {
  return std::max(
      Duration32::Zero(),
      input.elapsed_time - input_modeler_state.total_real_elapsed_time);
}

Duration32 GetTimeSinceInput(
    const StrokeInputModeler::State& input_modeler_state,
    const ModeledStrokeInput& input) {
  return input_modeler_state.complete_elapsed_time - input.elapsed_time;
}

std::optional<float> GetSourceValue(
    const ModeledStrokeInput& input, std::optional<Angle> travel_direction,
    float brush_size, const StrokeInputModeler::State& input_modeler_state,
    BrushBehavior::Source source) {
  switch (source) {
    case BrushBehavior::Source::kNormalizedPressure:
      if (input.pressure == StrokeInput::kNoPressure) break;
      return input.pressure;
    case BrushBehavior::Source::kTiltInRadians:
      if (input.tilt == StrokeInput::kNoTilt) break;
      return input.tilt.ValueInRadians();
    case BrushBehavior::Source::kTiltXInRadians:
      if (input.tilt == StrokeInput::kNoTilt ||
          input.orientation == StrokeInput::kNoOrientation)
        break;
      return GetTiltX(input.tilt, input.orientation);
    case BrushBehavior::Source::kTiltYInRadians:
      if (input.tilt == StrokeInput::kNoTilt ||
          input.orientation == StrokeInput::kNoOrientation)
        break;
      return GetTiltY(input.tilt, input.orientation);
    case BrushBehavior::Source::kOrientationInRadians:
      if (input.orientation == StrokeInput::kNoOrientation ||
          input.tilt == Angle())
        break;
      return input.orientation.ValueInRadians();
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
      if (input.orientation == StrokeInput::kNoOrientation ||
          input.tilt == Angle())
        break;
      return input.orientation.NormalizedAboutZero().ValueInRadians();
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
      return input.velocity.Magnitude() / brush_size;
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
      return input.velocity.x / brush_size;
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
      return input.velocity.y / brush_size;
    case BrushBehavior::Source::kDirectionInRadians:
      if (!travel_direction.has_value()) break;
      return travel_direction->Normalized().ValueInRadians();
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
      if (!travel_direction.has_value()) break;
      return travel_direction->ValueInRadians();
    case BrushBehavior::Source::kNormalizedDirectionX:
      if (!travel_direction.has_value()) break;
      return Cos(*travel_direction);
    case BrushBehavior::Source::kNormalizedDirectionY:
      if (!travel_direction.has_value()) break;
      return Sin(*travel_direction);
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
      return input.traveled_distance / brush_size;
    case BrushBehavior::Source::kTimeOfInputInSeconds:
      return input.elapsed_time.ToSeconds();
    case BrushBehavior::Source::kTimeOfInputInMillis:
      return input.elapsed_time.ToMillis();
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
      return GetPredictedDistanceTraveledInStrokeUnits(input_modeler_state,
                                                       input) /
             brush_size;
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
      return GetPredictedTimeElapsed(input_modeler_state, input).ToSeconds();
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
      return GetPredictedTimeElapsed(input_modeler_state, input).ToMillis();
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
      return (input_modeler_state.complete_traveled_distance -
              input.traveled_distance) /
             brush_size;
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
      return GetTimeSinceInput(input_modeler_state, input).ToSeconds();
    case BrushBehavior::Source::kTimeSinceInputInMillis:
      return GetTimeSinceInput(input_modeler_state, input).ToMillis();
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
      return input.acceleration.Magnitude() / brush_size;
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
      return input.acceleration.x / brush_size;
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
      return input.acceleration.y / brush_size;
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
      return Vec::DotProduct(input.acceleration, input.velocity.AsUnitVec()) /
             brush_size;
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
      return Vec::DotProduct(input.acceleration,
                             input.velocity.AsUnitVec().Orthogonal()) /
             brush_size;
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.velocity.Magnitude() *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.velocity.x *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.velocity.y *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.traveled_distance *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return GetPredictedDistanceTraveledInStrokeUnits(input_modeler_state,
                                                       input) *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.acceleration.Magnitude() *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.acceleration.x *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return input.acceleration.y *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return Vec::DotProduct(input.acceleration, input.velocity.AsUnitVec()) *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
      if (!input_modeler_state.stroke_unit_length.has_value()) break;
      return Vec::DotProduct(input.acceleration,
                             input.velocity.AsUnitVec().Orthogonal()) *
             input_modeler_state.stroke_unit_length->ToCentimeters();
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      return input_modeler_state.complete_traveled_distance == 0.0f
                 ? 0.0f
                 : 1.0f - input.traveled_distance /
                              input_modeler_state.complete_traveled_distance;
  }
  return std::nullopt;
}

bool IsOptionalInputPropertyPresent(
    const BrushBehavior::OptionalInputProperty& property,
    const ModeledStrokeInput& input) {
  switch (property) {
    case BrushBehavior::OptionalInputProperty::kPressure:
      return input.pressure != StrokeInput::kNoPressure;
    case BrushBehavior::OptionalInputProperty::kTilt:
      return input.tilt != StrokeInput::kNoTilt;
    case BrushBehavior::OptionalInputProperty::kOrientation:
      return input.orientation != StrokeInput::kNoOrientation;
    case BrushBehavior::OptionalInputProperty::kTiltXAndY:
      return input.tilt != StrokeInput::kNoTilt &&
             input.orientation != StrokeInput::kNoOrientation;
  }
  return false;
}

// Applies the `out_of_range_behavior` to `x` to return a value in [0, 1].
float ApplyOutOfRangeBehavior(BrushBehavior::OutOfRange behavior, float x) {
  switch (behavior) {
    case BrushBehavior::OutOfRange::kClamp:
      return std::clamp(x, 0.f, 1.f);
    case BrushBehavior::OutOfRange::kRepeat:
      return x - std::floor(x);
    case BrushBehavior::OutOfRange::kMirror:
      return std::abs(x - 2 * std::round(0.5f * x));
  }
  return 0;
}

float DampOffsetTransition(float target_offset, float previous_offset,
                           float delta, float response_gap) {
  // Damp the transition via exponential decay. This models a critically damped
  // oscillator and allows us to perform the transition with only local
  // knowledge of the offsets. I.e. we do not need to know the original offset
  // when damping first started. We use the `response_gap` as the decay constant
  // `tau` (https://en.wikipedia.org/wiki/Exponential_decay). This means that
  // after a distance/time of `response_gap` has passed, the transition will be
  // (1 - e^-1), which is about 63% complete (which turns out to feel more
  // intuitive in practice for humans tuning the `response_gap` than it would if
  // we were to apply a multiplier such that the `response_gap` is when the
  // transition is, say, 99% complete).
  if (delta <= 0) return previous_offset;
  return Lerp(target_offset, previous_offset, std::exp(-delta / response_gap));
}

inline float DampOffsetTransition(float target_offset, float previous_offset,
                                  Duration32 time_delta,
                                  Duration32 response_time) {
  return DampOffsetTransition(target_offset, previous_offset,
                              time_delta.ToSeconds(),
                              response_time.ToSeconds());
}

inline float DampOffsetTransition(float target_offset, float previous_offset,
                                  PhysicalDistance distance_delta,
                                  PhysicalDistance response_distance) {
  return DampOffsetTransition(target_offset, previous_offset,
                              distance_delta.ToCentimeters(),
                              response_distance.ToCentimeters());
}

void ProcessBehaviorNodeImpl(const BrushBehavior::SourceNode& node,
                             const BehaviorNodeContext& context) {
  std::optional<float> source_value = GetSourceValue(
      context.current_input, context.current_travel_direction,
      context.brush_size, context.input_modeler_state, node.source);
  if (!source_value.has_value()) {
    context.stack.push_back(kNullBehaviorNodeValue);
    return;
  }

  context.stack.push_back(ApplyOutOfRangeBehavior(
      node.source_out_of_range_behavior,
      InverseLerp(node.source_value_range[0], node.source_value_range[1],
                  *source_value)));
}

void ProcessBehaviorNodeImpl(const BrushBehavior::ConstantNode& node,
                             const BehaviorNodeContext& context) {
  context.stack.push_back(node.value);
}

void ProcessBehaviorNodeImpl(const NoiseNodeImplementation& node,
                             const BehaviorNodeContext& context) {
  float advance_by = 0.0f;
  switch (node.vary_over) {
    case BrushBehavior::DampingSource::kDistanceInCentimeters: {
      PhysicalDistance period = PhysicalDistance::Centimeters(node.base_period);
      float previous_traveled_distance =
          context.previous_input_metrics.has_value()
              ? context.previous_input_metrics->traveled_distance
              : 0.0f;
      PhysicalDistance traveled_distance_delta =
          context.input_modeler_state.stroke_unit_length.has_value()
              ? *context.input_modeler_state.stroke_unit_length *
                    (context.current_input.traveled_distance -
                     previous_traveled_distance)
              : PhysicalDistance::Zero();
      advance_by = traveled_distance_delta / period;
    } break;
    case BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize: {
      float period = context.brush_size * node.base_period;
      float previous_traveled_distance =
          context.previous_input_metrics.has_value()
              ? context.previous_input_metrics->traveled_distance
              : 0.0f;
      float traveled_distance_delta =
          context.current_input.traveled_distance - previous_traveled_distance;
      advance_by = traveled_distance_delta / period;
    } break;
    case BrushBehavior::DampingSource::kTimeInSeconds: {
      Duration32 period = Duration32::Seconds(node.base_period);
      Duration32 previous_elapsed_time =
          context.previous_input_metrics.has_value()
              ? context.previous_input_metrics->elapsed_time
              : Duration32::Zero();
      Duration32 elapsed_time_delta =
          context.current_input.elapsed_time - previous_elapsed_time;
      advance_by = elapsed_time_delta / period;
    } break;
  }
  NoiseGenerator& generator = context.noise_generators[node.generator_index];
  generator.AdvanceInputBy(advance_by);
  context.stack.push_back(generator.CurrentOutputValue());
}

void ProcessBehaviorNodeImpl(const BrushBehavior::FallbackFilterNode& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK(!context.stack.empty());
  if (IsOptionalInputPropertyPresent(node.is_fallback_for,
                                     context.current_input)) {
    context.stack.back() = kNullBehaviorNodeValue;
  }
}

void ProcessBehaviorNodeImpl(const BrushBehavior::ToolTypeFilterNode& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK(!context.stack.empty());
  if (!IsToolTypeEnabled(node.enabled_tool_types,
                         context.input_modeler_state.tool_type)) {
    context.stack.back() = kNullBehaviorNodeValue;
  }
}

void ProcessBehaviorNodeImpl(const DampingNodeImplementation& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK(!context.stack.empty());
  float& damped_value = context.damped_values[node.damping_index];
  float input = context.stack.back();
  if (IsNullBehaviorNodeValue(input)) {
    // Input is null, so use previous damped value unchanged.
  } else if (IsNullBehaviorNodeValue(damped_value) ||
             node.damping_gap == 0.0f) {
    // Input is non-null.  If previous damped value is null, then this is the
    // first non-null input, so snap the damped value to the input.  Or, if the
    // damping_gap is zero, then there's no damping to be done, so also snap the
    // damped value to the input.
    damped_value = input;
  } else {
    // Input and previous damped value are both non-null, so move the damped
    // value towards the input according to the damping settings.  Note that a
    // non-null previous damped value implies that there was at least one
    // previous input, and thus `context.previous_input_metrics` is present.
    ABSL_DCHECK(context.previous_input_metrics.has_value());
    switch (node.damping_source) {
      case BrushBehavior::DampingSource::kDistanceInCentimeters: {
        // If no mapping from stroke units to physical units is available, then
        // don't perform any damping (i.e. snap damped value to input).
        if (!context.input_modeler_state.stroke_unit_length.has_value()) {
          damped_value = input;
          break;
        }
        PhysicalDistance damping_distance =
            PhysicalDistance::Centimeters(node.damping_gap);
        PhysicalDistance traveled_distance_delta =
            *context.input_modeler_state.stroke_unit_length *
            (context.current_input.traveled_distance -
             context.previous_input_metrics->traveled_distance);
        damped_value = DampOffsetTransition(
            input, damped_value, traveled_distance_delta, damping_distance);
      } break;
      case BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize: {
        float damping_distance = context.brush_size * node.damping_gap;
        float traveled_distance_delta =
            context.current_input.traveled_distance -
            context.previous_input_metrics->traveled_distance;
        damped_value = DampOffsetTransition(
            input, damped_value, traveled_distance_delta, damping_distance);
      } break;
      case BrushBehavior::DampingSource::kTimeInSeconds: {
        Duration32 damping_time = Duration32::Seconds(node.damping_gap);
        Duration32 elapsed_time_delta =
            context.current_input.elapsed_time -
            context.previous_input_metrics->elapsed_time;
        damped_value = DampOffsetTransition(input, damped_value,
                                            elapsed_time_delta, damping_time);
      } break;
    }
  }
  context.stack.back() = damped_value;
}

void ProcessBehaviorNodeImpl(const EasingImplementation& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK(!context.stack.empty());
  context.stack.back() = node.GetY(context.stack.back());
}

void ProcessBehaviorNodeImpl(const BrushBehavior::BinaryOpNode& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK_GE(context.stack.size(), 2);
  float second_input = context.stack.back();
  context.stack.pop_back();
  float first_input = context.stack.back();
  float* result = &context.stack.back();
  switch (node.operation) {
    case BrushBehavior::BinaryOp::kProduct:
      // kNullBehaviorNodeValue is NaN, so if either input value is null (NaN),
      // the result will be null (NaN).
      *result = first_input * second_input;
      break;
    case BrushBehavior::BinaryOp::kSum:
      // kNullBehaviorNodeValue is NaN, so if either input value is null (NaN),
      // the result will be null (NaN).
      *result = first_input + second_input;
      break;
  }
  // If any of the above operations resulted in a non-finite value
  // (e.g. overflow to infinity), treat the result as null.
  if (!std::isfinite(*result)) {
    *result = kNullBehaviorNodeValue;
  }
}

void ProcessBehaviorNodeImpl(const BrushBehavior::InterpolationNode& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK_GE(context.stack.size(), 3);
  float range_end = context.stack.back();
  context.stack.pop_back();
  float range_start = context.stack.back();
  context.stack.pop_back();
  float param = context.stack.back();
  float* result = &context.stack.back();
  if (IsNullBehaviorNodeValue(range_start) ||
      IsNullBehaviorNodeValue(range_end) || IsNullBehaviorNodeValue(param)) {
    *result = kNullBehaviorNodeValue;
    return;
  }
  switch (node.interpolation) {
    case BrushBehavior::Interpolation::kLerp:
      *result = Lerp(range_start, range_end, param);
      break;
    case BrushBehavior::Interpolation::kInverseLerp:
      if (range_start == range_end) {
        *result = kNullBehaviorNodeValue;
        return;
      }
      *result = InverseLerp(range_start, range_end, param);
      break;
  }
  // If any of the above resulted in a non-finite value (e.g. overflow to
  // infinity), treat the result as null.
  if (!std::isfinite(*result)) {
    *result = kNullBehaviorNodeValue;
  }
}

void ProcessBehaviorNodeImpl(const TargetNodeImplementation& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK(!context.stack.empty());
  float input = context.stack.back();
  context.stack.pop_back();
  if (IsNullBehaviorNodeValue(input)) return;
  context.target_modifiers[node.target_index] =
      Lerp(node.target_modifier_range[0], node.target_modifier_range[1], input);
}

void ProcessBehaviorNodeImpl(const PolarTargetNodeImplementation& node,
                             const BehaviorNodeContext& context) {
  ABSL_DCHECK_GE(context.stack.size(), 2);
  float magnitude_input = context.stack.back();
  context.stack.pop_back();
  float angle_input = context.stack.back();
  context.stack.pop_back();
  if (IsNullBehaviorNodeValue(angle_input) ||
      IsNullBehaviorNodeValue(magnitude_input)) {
    return;
  }
  Vec modifier = Vec::FromDirectionAndMagnitude(
      Angle::Radians(
          Lerp(node.angle_range[0], node.angle_range[1], angle_input)),
      Lerp(node.magnitude_range[0], node.magnitude_range[1], magnitude_input));
  context.target_modifiers[node.target_x_index] = modifier.x;
  context.target_modifiers[node.target_y_index] = modifier.y;
}

}  // namespace

void ProcessBehaviorNode(const BehaviorNodeImplementation& node,
                         const BehaviorNodeContext& context) {
  std::visit(
      [&context](const auto& node) { ProcessBehaviorNodeImpl(node, context); },
      node);
}

namespace {

// Percentage shifts for each `BrushBehavior::Target` of a `BrushTipState`.
struct BrushTipStateModifiers {
  Vec position_offset_in_stroke_units;
  float width_multiplier = 1;
  float height_multiplier = 1;
  Angle slant_offset;
  float corner_rounding_offset = 0;
  Angle rotation_offset;
  float pinch_offset = 0;
  float texture_animation_progress_offset = 0;
  Angle hue_offset;
  float saturation_multiplier = 1;
  float luminosity = 0;
  float opacity_multiplier = 1;
};

// Adds `modifier` to the appropriate member of `tip_state_modifiers` according
// to the `target` enum.
void ApplyModifierToTarget(float modifier, BrushBehavior::Target target,
                           std::optional<Angle> travel_direction,
                           float brush_size,
                           BrushTipStateModifiers& tip_state_modifiers) {
  ABSL_DCHECK(std::isfinite(modifier));
  switch (target) {
    case BrushBehavior::Target::kWidthMultiplier:
      tip_state_modifiers.width_multiplier *= modifier;
      break;
    case BrushBehavior::Target::kHeightMultiplier:
      tip_state_modifiers.height_multiplier *= modifier;
      break;
    case BrushBehavior::Target::kSizeMultiplier:
      tip_state_modifiers.width_multiplier *= modifier;
      tip_state_modifiers.height_multiplier *= modifier;
      break;
    case BrushBehavior::Target::kSlantOffsetInRadians:
      tip_state_modifiers.slant_offset += Angle::Radians(modifier);
      break;
    case BrushBehavior::Target::kPinchOffset:
      tip_state_modifiers.pinch_offset += modifier;
      break;
    case BrushBehavior::Target::kRotationOffsetInRadians:
      tip_state_modifiers.rotation_offset += Angle::Radians(modifier);
      break;
    case BrushBehavior::Target::kCornerRoundingOffset:
      tip_state_modifiers.corner_rounding_offset += modifier;
      break;
    case BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize:
      tip_state_modifiers.position_offset_in_stroke_units.x +=
          modifier * brush_size;
      break;
    case BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize:
      tip_state_modifiers.position_offset_in_stroke_units.y +=
          modifier * brush_size;
      break;
    case BrushBehavior::Target::kPositionOffsetForwardInMultiplesOfBrushSize:
      if (travel_direction.has_value()) {
        tip_state_modifiers.position_offset_in_stroke_units +=
            Vec::FromDirectionAndMagnitude(*travel_direction,
                                           modifier * brush_size);
      }
      break;
    case BrushBehavior::Target::kPositionOffsetLateralInMultiplesOfBrushSize:
      if (travel_direction.has_value()) {
        tip_state_modifiers.position_offset_in_stroke_units +=
            Vec::FromDirectionAndMagnitude(*travel_direction + kQuarterTurn,
                                           modifier * brush_size);
      }
      break;
    case BrushBehavior::Target::kTextureAnimationProgressOffset:
      tip_state_modifiers.texture_animation_progress_offset += modifier;
      break;
    case BrushBehavior::Target::kHueOffsetInRadians:
      tip_state_modifiers.hue_offset += Angle::Radians(modifier);
      break;
    case BrushBehavior::Target::kSaturationMultiplier:
      tip_state_modifiers.saturation_multiplier *= modifier;
      break;
    case BrushBehavior::Target::kLuminosity:
      tip_state_modifiers.luminosity += modifier;
      break;
    case BrushBehavior::Target::kOpacityMultiplier:
      tip_state_modifiers.opacity_multiplier *= modifier;
      break;
  }
}

void ApplyModifiersToTipState(const BrushTipStateModifiers& modifiers,
                              BrushTipState& tip_state) {
  tip_state.position += modifiers.position_offset_in_stroke_units;
  if (modifiers.width_multiplier != 1) {
    tip_state.width *= std::clamp(modifiers.width_multiplier, 0.f, 2.f);
  }
  if (modifiers.height_multiplier != 1) {
    tip_state.height *= std::clamp(modifiers.height_multiplier, 0.f, 2.f);
  }
  if (modifiers.slant_offset != Angle()) {
    tip_state.slant = std::clamp(tip_state.slant + modifiers.slant_offset,
                                 -kQuarterTurn, kQuarterTurn);
  }
  if (modifiers.pinch_offset != 0) {
    tip_state.pinch =
        std::clamp(tip_state.pinch + modifiers.pinch_offset, 0.f, 1.f);
  }
  if (modifiers.rotation_offset != Angle()) {
    tip_state.rotation =
        (tip_state.rotation + modifiers.rotation_offset).Normalized();
  }
  if (modifiers.corner_rounding_offset != 0) {
    tip_state.percent_radius = std::clamp(
        tip_state.percent_radius + modifiers.corner_rounding_offset, 0.f, 1.f);
  }
  if (modifiers.texture_animation_progress_offset != 0) {
    tip_state.texture_animation_progress_offset =
        geometry_internal::FloatModulo(
            tip_state.texture_animation_progress_offset +
                modifiers.texture_animation_progress_offset,
            1);
  }
  if (modifiers.hue_offset != Angle()) {
    tip_state.hue_offset_in_full_turns =
        modifiers.hue_offset.Normalized() / kFullTurn;
  }
  if (modifiers.saturation_multiplier != 1) {
    tip_state.saturation_multiplier =
        std::clamp(modifiers.saturation_multiplier, 0.f, 2.f);
  }
  if (modifiers.luminosity != 0) {
    tip_state.luminosity_shift = std::clamp(modifiers.luminosity, -1.f, 1.f);
  }
  if (modifiers.opacity_multiplier != 1) {
    tip_state.opacity_multiplier = std::clamp(
        tip_state.opacity_multiplier * modifiers.opacity_multiplier, 0.f, 2.f);
  }
}

}  // namespace

BrushTipState CreateTipState(Point position, std::optional<Angle> direction,
                             const BrushTip& brush_tip, float brush_size,
                             absl::Span<const BrushBehavior::Target> targets,
                             absl::Span<const float> target_modifiers) {
  ABSL_DCHECK_EQ(targets.size(), target_modifiers.size());

  BrushTipStateModifiers tip_state_modifiers = {};
  for (size_t i = 0; i < targets.size(); ++i) {
    ApplyModifierToTarget(target_modifiers[i], targets[i], direction,
                          brush_size, tip_state_modifiers);
  }

  BrushTipState tip_state = {
      .position = position,
      .width = brush_size * brush_tip.scale.x,
      .height = brush_size * brush_tip.scale.y,
      .percent_radius = brush_tip.corner_rounding,
      .rotation = brush_tip.rotation,
      .slant = brush_tip.slant,
      .pinch = brush_tip.pinch,
      .opacity_multiplier = brush_tip.opacity_multiplier,
  };
  ApplyModifiersToTipState(tip_state_modifiers, tip_state);
  return tip_state;
}

ModeledStrokeInput Lerp(const ModeledStrokeInput& a,
                        const ModeledStrokeInput& b, float t) {
  return {.position = geometry_internal::Lerp(a.position, b.position, t),
          .velocity = geometry_internal::Lerp(a.velocity, b.velocity, t),
          .acceleration =
              geometry_internal::Lerp(a.acceleration, b.acceleration, t),
          .traveled_distance = geometry_internal::Lerp(a.traveled_distance,
                                                       b.traveled_distance, t),
          .elapsed_time = Duration32::Millis(geometry_internal::Lerp(
              a.elapsed_time.ToMillis(), b.elapsed_time.ToMillis(), t)),
          .pressure = geometry_internal::Lerp(a.pressure, b.pressure, t),
          .tilt = geometry_internal::Lerp(a.tilt, b.tilt, t),
          .orientation = geometry_internal::NormalizedAngleLerp(
              a.orientation, b.orientation, t)};
}

}  // namespace ink::strokes_internal
