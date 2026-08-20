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

#include "ink/brush/brush_behavior.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <variant>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "ink/brush/easing_function.h"

namespace ink {

bool BrushBehavior::EnabledToolTypes::HasAnyTypes() const {
  return unknown || mouse || touch || stylus;
}

bool BrushBehavior::EnabledToolTypes::HasAllTypes() const {
  return unknown && mouse && touch && stylus;
}

bool operator==(const BrushBehavior::EnabledToolTypes& lhs,
                const BrushBehavior::EnabledToolTypes& rhs) {
  return lhs.unknown == rhs.unknown && lhs.mouse == rhs.mouse &&
         lhs.touch == rhs.touch && lhs.stylus == rhs.stylus;
}

bool operator!=(const BrushBehavior::EnabledToolTypes& lhs,
                const BrushBehavior::EnabledToolTypes& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::SourceNode& lhs,
                const BrushBehavior::SourceNode& rhs) {
  return lhs.source == rhs.source &&
         lhs.source_out_of_range_behavior == rhs.source_out_of_range_behavior &&
         lhs.source_value_range == rhs.source_value_range;
}

bool operator!=(const BrushBehavior::SourceNode& lhs,
                const BrushBehavior::SourceNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::ConstantNode& lhs,
                const BrushBehavior::ConstantNode& rhs) {
  return lhs.value == rhs.value;
}

bool operator!=(const BrushBehavior::ConstantNode& lhs,
                const BrushBehavior::ConstantNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::NoiseNode& lhs,
                const BrushBehavior::NoiseNode& rhs) {
  return lhs.seed == rhs.seed && lhs.vary_over == rhs.vary_over &&
         lhs.base_period == rhs.base_period;
}

bool operator!=(const BrushBehavior::NoiseNode& lhs,
                const BrushBehavior::NoiseNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::FallbackFilterNode& lhs,
                const BrushBehavior::FallbackFilterNode& rhs) {
  return lhs.is_fallback_for == rhs.is_fallback_for;
}

bool operator!=(const BrushBehavior::FallbackFilterNode& lhs,
                const BrushBehavior::FallbackFilterNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::ToolTypeFilterNode& lhs,
                const BrushBehavior::ToolTypeFilterNode& rhs) {
  return lhs.enabled_tool_types == rhs.enabled_tool_types;
}

bool operator!=(const BrushBehavior::ToolTypeFilterNode& lhs,
                const BrushBehavior::ToolTypeFilterNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::DampingNode& lhs,
                const BrushBehavior::DampingNode& rhs) {
  return lhs.damping_source == rhs.damping_source &&
         lhs.damping_gap == rhs.damping_gap;
}

bool operator!=(const BrushBehavior::DampingNode& lhs,
                const BrushBehavior::DampingNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::ResponseNode& lhs,
                const BrushBehavior::ResponseNode& rhs) {
  return lhs.response_curve == rhs.response_curve;
}

bool operator!=(const BrushBehavior::ResponseNode& lhs,
                const BrushBehavior::ResponseNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::BinaryOpNode& lhs,
                const BrushBehavior::BinaryOpNode& rhs) {
  return lhs.operation == rhs.operation;
}

bool operator!=(const BrushBehavior::BinaryOpNode& lhs,
                const BrushBehavior::BinaryOpNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::InterpolationNode& lhs,
                const BrushBehavior::InterpolationNode& rhs) {
  return lhs.interpolation == rhs.interpolation;
}

bool operator!=(const BrushBehavior::InterpolationNode& lhs,
                const BrushBehavior::InterpolationNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::TargetNode& lhs,
                const BrushBehavior::TargetNode& rhs) {
  return lhs.target == rhs.target &&
         lhs.target_modifier_range == rhs.target_modifier_range;
}

bool operator!=(const BrushBehavior::TargetNode& lhs,
                const BrushBehavior::TargetNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior::PolarTargetNode& lhs,
                const BrushBehavior::PolarTargetNode& rhs) {
  return lhs.target == rhs.target && lhs.angle_range == rhs.angle_range &&
         lhs.magnitude_range == rhs.magnitude_range;
}

bool operator!=(const BrushBehavior::PolarTargetNode& lhs,
                const BrushBehavior::PolarTargetNode& rhs) {
  return !(lhs == rhs);
}

bool operator==(const BrushBehavior& lhs, const BrushBehavior& rhs) {
  return lhs.nodes == rhs.nodes;
}

bool operator!=(const BrushBehavior& lhs, const BrushBehavior& rhs) {
  return !(lhs == rhs);
}

namespace brush_internal {
namespace {

bool IsValidBehaviorSource(BrushBehavior::Source source) {
  switch (source) {
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
      return true;
  }
  return false;
}

absl::Status ValidateSourceAndOutOfRangeCombination(
    BrushBehavior::Source source, BrushBehavior::OutOfRange out_of_range) {
  switch (source) {
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
    case BrushBehavior::Source::kTimeSinceInputInMillis:
      if (out_of_range != BrushBehavior::OutOfRange::kClamp) {
        return absl::InvalidArgumentError(
            "`Source::kTimeSinceInput*` must only be used with "
            "`source_out_of_range_behavior` of `kClamp`.");
      }
      break;
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
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kTimeOfInputInSeconds:
    case BrushBehavior::Source::kTimeOfInputInMillis:
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
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
      break;
  }
  return absl::OkStatus();
}

bool IsValidBehaviorTarget(BrushBehavior::Target target) {
  switch (target) {
    case BrushBehavior::Target::kWidthMultiplier:
    case BrushBehavior::Target::kHeightMultiplier:
    case BrushBehavior::Target::kSizeMultiplier:
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
    case BrushBehavior::Target::kSaturationMultiplier:
    case BrushBehavior::Target::kLuminosity:
    case BrushBehavior::Target::kOpacityMultiplier:
      return true;
  }
  return false;
}

bool IsValidBehaviorPolarTarget(BrushBehavior::PolarTarget target) {
  switch (target) {
    case BrushBehavior::PolarTarget::
        kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize:
    case BrushBehavior::PolarTarget::
        kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize:
      return true;
  }
  return false;
}

bool IsValidBehaviorOutOfRange(BrushBehavior::OutOfRange out_of_range) {
  switch (out_of_range) {
    case BrushBehavior::OutOfRange::kClamp:
    case BrushBehavior::OutOfRange::kRepeat:
    case BrushBehavior::OutOfRange::kMirror:
      return true;
  }
  return false;
}

bool IsRangeValid(std::array<float, 2> range) {
  return range.size() == 2 && std::isfinite(range[0]) &&
         std::isfinite(range[1]) && range[0] != range[1];
}

bool IsValidOptionalInputProperty(
    BrushBehavior::OptionalInputProperty unreported_source) {
  switch (unreported_source) {
    case BrushBehavior::OptionalInputProperty::kPressure:
    case BrushBehavior::OptionalInputProperty::kTilt:
    case BrushBehavior::OptionalInputProperty::kOrientation:
    case BrushBehavior::OptionalInputProperty::kTiltXAndY:
      return true;
  }
  return false;
}

bool IsValidBehaviorBinaryOp(BrushBehavior::BinaryOp operation) {
  switch (operation) {
    case BrushBehavior::BinaryOp::kProduct:
    case BrushBehavior::BinaryOp::kSum:
      return true;
  }
  return false;
}

bool IsValidBehaviorDampingSource(BrushBehavior::DampingSource damping_source) {
  switch (damping_source) {
    case BrushBehavior::DampingSource::kDistanceInCentimeters:
    case BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize:
    case BrushBehavior::DampingSource::kTimeInSeconds:
      return true;
  }
  return false;
}

bool IsValidBehaviorInterpolation(BrushBehavior::Interpolation interpolation) {
  switch (interpolation) {
    case BrushBehavior::Interpolation::kLerp:
    case BrushBehavior::Interpolation::kInverseLerp:
      return true;
  }
  return false;
}

// Returns the number of input values that a given `Node` consumes.
int NodeInputCount(const BrushBehavior::SourceNode& node) { return 0; }
int NodeInputCount(const BrushBehavior::ConstantNode& node) { return 0; }
int NodeInputCount(const BrushBehavior::NoiseNode& node) { return 0; }
int NodeInputCount(const BrushBehavior::FallbackFilterNode& node) { return 1; }
int NodeInputCount(const BrushBehavior::ToolTypeFilterNode& node) { return 1; }
int NodeInputCount(const BrushBehavior::DampingNode& node) { return 1; }
int NodeInputCount(const BrushBehavior::ResponseNode& node) { return 1; }
int NodeInputCount(const BrushBehavior::BinaryOpNode& node) { return 2; }
int NodeInputCount(const BrushBehavior::InterpolationNode& node) { return 3; }
int NodeInputCount(const BrushBehavior::TargetNode& node) { return 1; }
int NodeInputCount(const BrushBehavior::PolarTargetNode& node) { return 2; }
int NodeInputCount(const BrushBehavior::Node& node) {
  return std::visit([](const auto& node) { return NodeInputCount(node); },
                    node);
}

// Returns the number of output values that a given `Node` produces (0 for
// terminal nodes, 1 for value nodes).
int NodeOutputCount(const BrushBehavior::Node& node) {
  if (std::holds_alternative<BrushBehavior::TargetNode>(node) ||
      std::holds_alternative<BrushBehavior::PolarTargetNode>(node)) {
    return 0;
  }
  return 1;
}

absl::Status ValidateNode(const BrushBehavior::SourceNode& node) {
  if (!IsValidBehaviorSource(node.source)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`SourceNode::source` holds non-enumerator value %d",
                        static_cast<int>(node.source)));
  }
  if (!IsValidBehaviorOutOfRange(node.source_out_of_range_behavior)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`SourceNode::source_out_of_range_behavior` holds non-enumerator "
        "value %d",
        static_cast<int>(node.source_out_of_range_behavior)));
  }
  if (auto status = ValidateSourceAndOutOfRangeCombination(
          node.source, node.source_out_of_range_behavior);
      !status.ok()) {
    return status;
  }
  if (!IsRangeValid(node.source_value_range)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`SourceNode::source_value_range` must hold 2 finite and distinct "
        "values. Got {%f, %f}",
        node.source_value_range[0], node.source_value_range[1]));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::ConstantNode& node) {
  if (!std::isfinite(node.value)) {
    return absl::InvalidArgumentError(
        absl::StrCat("`ConstantNode::value` must be finite. Got ", node.value));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::NoiseNode& node) {
  if (!IsValidBehaviorDampingSource(node.vary_over)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`NoiseNode::vary_over` holds non-enumerator value %d",
                        static_cast<int>(node.vary_over)));
  }
  if (!std::isfinite(node.base_period) || node.base_period <= 0.f) {
    return absl::InvalidArgumentError(absl::StrCat(
        "`NoiseNode::base_period` must be finite and positive. Got ",
        node.base_period));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::FallbackFilterNode& node) {
  if (!IsValidOptionalInputProperty(node.is_fallback_for)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`FallbackFilterNode::is_fallback_for` holds non-enumerator value %d",
        static_cast<int>(node.is_fallback_for)));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::ToolTypeFilterNode& node) {
  if (!node.enabled_tool_types.HasAnyTypes()) {
    return absl::InvalidArgumentError(
        "`BrushBehavior::enabled_tool_types` must enable at least one tool "
        "type.");
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::DampingNode& node) {
  if (!IsValidBehaviorDampingSource(node.damping_source)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`DampingNode::damping_source` holds non-enumerator value %d",
        static_cast<int>(node.damping_source)));
  }
  if (!std::isfinite(node.damping_gap) || node.damping_gap < 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("`DampingNode::damping_gap` must be finite and "
                     "non-negative. Got ",
                     node.damping_gap));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::ResponseNode& node) {
  return ValidateEasingFunction(node.response_curve);
}

absl::Status ValidateNode(const BrushBehavior::BinaryOpNode& node) {
  if (!IsValidBehaviorBinaryOp(node.operation)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`BinaryOpNode::operation` holds non-enumerator value %d",
        static_cast<int>(node.operation)));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::InterpolationNode& node) {
  if (!IsValidBehaviorInterpolation(node.interpolation)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`InterpolationNode::interpolation` holds non-enumerator value %d",
        static_cast<int>(node.interpolation)));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::TargetNode& node) {
  if (!IsValidBehaviorTarget(node.target)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`TargetNode::target` holds non-enumerator value %d",
                        static_cast<int>(node.target)));
  }
  if (!IsRangeValid(node.target_modifier_range)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`TargetNode::target_modifier_range` must hold 2 finite and "
        "distinct values. Got {%f, %f}",
        node.target_modifier_range[0], node.target_modifier_range[1]));
  }
  return absl::OkStatus();
}

absl::Status ValidateNode(const BrushBehavior::PolarTargetNode& node) {
  if (!IsValidBehaviorPolarTarget(node.target)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`PolarTargetNode::target` holds non-enumerator value %d",
        static_cast<int>(node.target)));
  }
  if (!IsRangeValid(node.angle_range)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`PolarTargetNode::angle_range` must hold 2 finite and "
                        "distinct values. Got {%f, %f}",
                        node.angle_range[0], node.angle_range[1]));
  }
  if (!IsRangeValid(node.magnitude_range)) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`PolarTargetNode::magnitude_range` must hold 2 finite and "
        "distinct values. Got {%f, %f}",
        node.magnitude_range[0], node.magnitude_range[1]));
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status ValidateBrushBehaviorNode(const BrushBehavior::Node& node) {
  return std::visit([](const auto& node) { return ValidateNode(node); }, node);
}

absl::Status ValidateBrushBehaviorTopLevel(const BrushBehavior& behavior) {
  int stack_depth = 0;
  for (size_t i = 0; i < behavior.nodes.size(); ++i) {
    const BrushBehavior::Node& node = behavior.nodes[i];
    int input_count = NodeInputCount(node);
    if (stack_depth < input_count) {
      return absl::InvalidArgumentError(
          absl::StrFormat("Insufficient inputs into `BrushBehavior::Node` (%v) "
                          "at index=%u. Requires %d inputs, but got %d",
                          node, i, input_count, stack_depth));
    }
    stack_depth -= input_count;
    stack_depth += NodeOutputCount(node);
  }
  if (stack_depth > 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("A `BrushBehavior::Node` list must consume all generated "
                     "values, but there were ",
                     stack_depth, " values remaining."));
  }
  return absl::OkStatus();
}

absl::Status ValidateBrushBehavior(const BrushBehavior& behavior) {
  for (size_t i = 0; i < behavior.nodes.size(); ++i) {
    if (absl::Status status = ValidateBrushBehaviorNode(behavior.nodes[i]);
        !status.ok()) {
      return status;
    }
  }
  if (absl::Status status = ValidateBrushBehaviorTopLevel(behavior);
      !status.ok()) {
    return status;
  }
  return absl::OkStatus();
}

std::string ToFormattedString(BrushBehavior::Source source) {
  switch (source) {
    case BrushBehavior::Source::kNormalizedPressure:
      return "kNormalizedPressure";
    case BrushBehavior::Source::kTiltInRadians:
      return "kTiltInRadians";
    case BrushBehavior::Source::kTiltXInRadians:
      return "kTiltXInRadians";
    case BrushBehavior::Source::kTiltYInRadians:
      return "kTiltYInRadians";
    case BrushBehavior::Source::kOrientationInRadians:
      return "kOrientationInRadians";
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
      return "kOrientationAboutZeroInRadians";
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
      return "kSpeedInMultiplesOfBrushSizePerSecond";
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
      return "kVelocityXInMultiplesOfBrushSizePerSecond";
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
      return "kVelocityYInMultiplesOfBrushSizePerSecond";
    case BrushBehavior::Source::kDirectionInRadians:
      return "kDirectionInRadians";
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
      return "kDirectionAboutZeroInRadians";
    case BrushBehavior::Source::kNormalizedDirectionX:
      return "kNormalizedDirectionX";
    case BrushBehavior::Source::kNormalizedDirectionY:
      return "kNormalizedDirectionY";
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
      return "kDistanceTraveledInMultiplesOfBrushSize";
    case BrushBehavior::Source::kTimeOfInputInSeconds:
      return "kTimeOfInputInSeconds";
    case BrushBehavior::Source::kTimeOfInputInMillis:
      return "kTimeOfInputInMillis";
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
      return "kPredictedDistanceTraveledInMultiplesOfBrushSize";
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
      return "kPredictedTimeElapsedInSeconds";
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
      return "kPredictedTimeElapsedInMillis";
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
      return "kDistanceRemainingInMultiplesOfBrushSize";
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
      return "kTimeSinceInputInSeconds";
    case BrushBehavior::Source::kTimeSinceInputInMillis:
      return "kTimeSinceInputInMillis";
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
      return "kAccelerationInMultiplesOfBrushSizePerSecondSquared";
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
      return "kAccelerationXInMultiplesOfBrushSizePerSecondSquared";
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
      return "kAccelerationYInMultiplesOfBrushSizePerSecondSquared";
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
      return "kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared";
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
      return "kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared";
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
      return "kInputSpeedInCentimetersPerSecond";
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
      return "kInputVelocityXInCentimetersPerSecond";
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
      return "kInputVelocityYInCentimetersPerSecond";
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
      return "kInputDistanceTraveledInCentimeters";
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
      return "kPredictedInputDistanceTraveledInCentimeters";
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
      return "kInputAccelerationInCentimetersPerSecondSquared";
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
      return "kInputAccelerationXInCentimetersPerSecondSquared";
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
      return "kInputAccelerationYInCentimetersPerSecondSquared";
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
      return "kInputAccelerationForwardInCentimetersPerSecondSquared";
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
      return "kInputAccelerationLateralInCentimetersPerSecondSquared";
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      return "kDistanceRemainingAsFractionOfStrokeLength";
  }
  return absl::StrCat("Source(", static_cast<int>(source), ")");
}

std::string ToFormattedString(BrushBehavior::Target target) {
  switch (target) {
    case BrushBehavior::Target::kWidthMultiplier:
      return "kWidthMultiplier";
    case BrushBehavior::Target::kHeightMultiplier:
      return "kHeightMultiplier";
    case BrushBehavior::Target::kSizeMultiplier:
      return "kSizeMultiplier";
    case BrushBehavior::Target::kSlantOffsetInRadians:
      return "kSlantOffsetInRadians";
    case BrushBehavior::Target::kPinchOffset:
      return "kPinchOffset";
    case BrushBehavior::Target::kRotationOffsetInRadians:
      return "kRotationOffsetInRadians";
    case BrushBehavior::Target::kCornerRoundingOffset:
      return "kCornerRoundingOffset";
    case BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize:
      return "kPositionOffsetXInMultiplesOfBrushSize";
    case BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize:
      return "kPositionOffsetYInMultiplesOfBrushSize";
    case BrushBehavior::Target::kPositionOffsetForwardInMultiplesOfBrushSize:
      return "kPositionOffsetForwardInMultiplesOfBrushSize";
    case BrushBehavior::Target::kPositionOffsetLateralInMultiplesOfBrushSize:
      return "kPositionOffsetLateralInMultiplesOfBrushSize";
    case BrushBehavior::Target::kTextureAnimationProgressOffset:
      return "kTextureAnimationProgressOffset";
    case BrushBehavior::Target::kHueOffsetInRadians:
      return "kHueOffsetInRadians";
    case BrushBehavior::Target::kSaturationMultiplier:
      return "kSaturationMultiplier";
    case BrushBehavior::Target::kLuminosity:
      return "kLuminosity";
    case BrushBehavior::Target::kOpacityMultiplier:
      return "kOpacityMultiplier";
  }
  return absl::StrCat("Target(", static_cast<int>(target), ")");
}

std::string ToFormattedString(BrushBehavior::PolarTarget target) {
  switch (target) {
    case BrushBehavior::PolarTarget::
        kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize:
      return "kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize";
    case BrushBehavior::PolarTarget::
        kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize:
      return "kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize";
  }
  return absl::StrCat("PolarTarget(", static_cast<int>(target), ")");
}

std::string ToFormattedString(BrushBehavior::OutOfRange out_of_range) {
  switch (out_of_range) {
    case BrushBehavior::OutOfRange::kClamp:
      return "kClamp";
    case BrushBehavior::OutOfRange::kRepeat:
      return "kRepeat";
    case BrushBehavior::OutOfRange::kMirror:
      return "kMirror";
  }
  return absl::StrCat("OutOfRange(", static_cast<int>(out_of_range), ")");
}

std::string ToFormattedString(BrushBehavior::EnabledToolTypes enabled) {
  if (enabled.HasAllTypes()) {
    return "all";
  }
  if (!enabled.HasAnyTypes()) {
    return "none";
  }
  std::string formatted;
  if (enabled.unknown) {
    formatted = "unknown";
  }
  if (enabled.mouse) {
    if (!formatted.empty()) formatted.push_back('/');
    formatted += "mouse";
  }
  if (enabled.touch) {
    if (!formatted.empty()) formatted.push_back('/');
    formatted += "touch";
  }
  if (enabled.stylus) {
    if (!formatted.empty()) formatted.push_back('/');
    formatted += "stylus";
  }
  return formatted;
}

std::string ToFormattedString(BrushBehavior::OptionalInputProperty input) {
  switch (input) {
    case BrushBehavior::OptionalInputProperty::kPressure:
      return "kPressure";
    case BrushBehavior::OptionalInputProperty::kTilt:
      return "kTilt";
    case BrushBehavior::OptionalInputProperty::kOrientation:
      return "kOrientation";
    case BrushBehavior::OptionalInputProperty::kTiltXAndY:
      return "kTiltXAndY";
  }
  return absl::StrCat("OptionalInputProperty(", static_cast<int>(input), ")");
}

std::string ToFormattedString(BrushBehavior::BinaryOp operation) {
  switch (operation) {
    case BrushBehavior::BinaryOp::kProduct:
      return "kProduct";
    case BrushBehavior::BinaryOp::kSum:
      return "kSum";
  }
  return absl::StrCat("BinaryOp(", static_cast<int>(operation), ")");
}

std::string ToFormattedString(BrushBehavior::DampingSource damping_source) {
  switch (damping_source) {
    case BrushBehavior::DampingSource::kDistanceInCentimeters:
      return "kDistanceInCentimeters";
    case BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize:
      return "kDistanceInMultiplesOfBrushSize";
    case BrushBehavior::DampingSource::kTimeInSeconds:
      return "kTimeInSeconds";
  }
  return absl::StrCat("DampingSource(", static_cast<int>(damping_source), ")");
}

std::string ToFormattedString(BrushBehavior::Interpolation interpolation) {
  switch (interpolation) {
    case BrushBehavior::Interpolation::kLerp:
      return "kLerp";
    case BrushBehavior::Interpolation::kInverseLerp:
      return "kInverseLerp";
  }
  return absl::StrCat("Interpolation(", static_cast<int>(interpolation), ")");
}

namespace {

std::string ToFormattedString(const BrushBehavior::SourceNode& node) {
  std::string formatted = absl::StrCat("SourceNode{source=", node.source);
  if (node.source_out_of_range_behavior != BrushBehavior::OutOfRange::kClamp) {
    absl::StrAppend(&formatted, ", source_out_of_range_behavior=",
                    node.source_out_of_range_behavior);
  }
  absl::StrAppend(&formatted, ", source_value_range={",
                  node.source_value_range[0], ", ", node.source_value_range[1],
                  "}}");
  return formatted;
}

std::string ToFormattedString(const BrushBehavior::ConstantNode& node) {
  return absl::StrCat("ConstantNode{", node.value, "}");
}

std::string ToFormattedString(const BrushBehavior::NoiseNode& node) {
  return absl::StrCat(
      "NoiseNode{seed=0x", absl::Hex(node.seed, absl::kZeroPad8),
      ", vary_over=", node.vary_over, ", base_period=", node.base_period, "}");
}

std::string ToFormattedString(const BrushBehavior::FallbackFilterNode& node) {
  return absl::StrCat("FallbackFilterNode{", node.is_fallback_for, "}");
}

std::string ToFormattedString(const BrushBehavior::ToolTypeFilterNode& node) {
  return absl::StrCat("ToolTypeFilterNode{", node.enabled_tool_types, "}");
}

std::string ToFormattedString(const BrushBehavior::DampingNode& node) {
  return absl::StrCat("DampingNode{damping_source=", node.damping_source,
                      ", damping_gap=", node.damping_gap, "}");
}

std::string ToFormattedString(const BrushBehavior::ResponseNode& node) {
  return absl::StrCat("ResponseNode{", node.response_curve, "}");
}

std::string ToFormattedString(const BrushBehavior::BinaryOpNode& node) {
  return absl::StrCat("BinaryOpNode{", node.operation, "}");
}

std::string ToFormattedString(const BrushBehavior::InterpolationNode& node) {
  return absl::StrCat("InterpolationNode{", node.interpolation, "}");
}

std::string ToFormattedString(const BrushBehavior::TargetNode& node) {
  return absl::StrCat(
      "TargetNode{target=", node.target, ", target_modifier_range={",
      node.target_modifier_range[0], ", ", node.target_modifier_range[1], "}}");
}

std::string ToFormattedString(const BrushBehavior::PolarTargetNode& node) {
  return absl::StrCat("PolarTargetNode{target=", node.target, ", angle_range={",
                      node.angle_range[0], ", ", node.angle_range[1],
                      "}, magnitude_range={", node.magnitude_range[0], ", ",
                      node.magnitude_range[1], "}}");
}

}  // namespace

std::string ToFormattedString(const BrushBehavior::Node& node) {
  return std::visit([](const auto& node) { return ToFormattedString(node); },
                    node);
}

std::string ToFormattedString(const BrushBehavior& behavior) {
  return absl::StrCat("BrushBehavior{nodes={",
                      absl::StrJoin(behavior.nodes, ", "), "}}");
}

}  // namespace brush_internal
}  // namespace ink
