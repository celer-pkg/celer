// Copyright 2024-2025 Google LLC
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

#include "ink/storage/brush.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/storage/color.h"
#include "ink/storage/proto/brush.pb.h"
#include "ink/storage/proto/brush_family.pb.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

proto::BrushBehavior::BinaryOp EncodeBrushBehaviorBinaryOp(
    BrushBehavior::BinaryOp binary_op) {
  switch (binary_op) {
    case BrushBehavior::BinaryOp::kProduct:
      return proto::BrushBehavior::BINARY_OP_PRODUCT;
    case BrushBehavior::BinaryOp::kSum:
      return proto::BrushBehavior::BINARY_OP_SUM;
  }
  return proto::BrushBehavior::BINARY_OP_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::BinaryOp> DecodeBrushBehaviorBinaryOp(
    proto::BrushBehavior::BinaryOp binary_op_proto) {
  switch (binary_op_proto) {
    case proto::BrushBehavior::BINARY_OP_PRODUCT:
      return BrushBehavior::BinaryOp::kProduct;
    case proto::BrushBehavior::BINARY_OP_SUM:
      return BrushBehavior::BinaryOp::kSum;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushBehavior.BinaryOp value: ", binary_op_proto));
  }
}

proto::BrushBehavior::DampingSource EncodeBrushBehaviorDampingSource(
    BrushBehavior::DampingSource damping_source) {
  switch (damping_source) {
    case BrushBehavior::DampingSource::kDistanceInCentimeters:
      return proto::BrushBehavior::DAMPING_SOURCE_DISTANCE_IN_CENTIMETERS;
    case BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          DAMPING_SOURCE_DISTANCE_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::DampingSource::kTimeInSeconds:
      return proto::BrushBehavior::DAMPING_SOURCE_TIME_IN_SECONDS;
  }
  return proto::BrushBehavior::DAMPING_SOURCE_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::DampingSource> DecodeBrushBehaviorDampingSource(
    proto::BrushBehavior::DampingSource damping_source_proto) {
  switch (damping_source_proto) {
    case proto::BrushBehavior::DAMPING_SOURCE_TIME_IN_SECONDS:
      return BrushBehavior::DampingSource::kTimeInSeconds;
    case proto::BrushBehavior::DAMPING_SOURCE_DISTANCE_IN_CENTIMETERS:
      return BrushBehavior::DampingSource::kDistanceInCentimeters;
    case proto::BrushBehavior::
        DAMPING_SOURCE_DISTANCE_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::DampingSource::kDistanceInMultiplesOfBrushSize;
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("invalid ink.proto.BrushBehavior.DampingSource value: ",
                       damping_source_proto));
  }
}

proto::BrushBehavior::Interpolation EncodeBrushBehaviorInterpolation(
    BrushBehavior::Interpolation interpolation) {
  switch (interpolation) {
    case BrushBehavior::Interpolation::kLerp:
      return proto::BrushBehavior::INTERPOLATION_LERP;
    case BrushBehavior::Interpolation::kInverseLerp:
      return proto::BrushBehavior::INTERPOLATION_INVERSE_LERP;
  }
  return proto::BrushBehavior::INTERPOLATION_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::Interpolation> DecodeBrushBehaviorInterpolation(
    proto::BrushBehavior::Interpolation interpolation_proto) {
  switch (interpolation_proto) {
    case proto::BrushBehavior::INTERPOLATION_LERP:
      return BrushBehavior::Interpolation::kLerp;
    case proto::BrushBehavior::INTERPOLATION_INVERSE_LERP:
      return BrushBehavior::Interpolation::kInverseLerp;
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("invalid ink.proto.BrushBehavior.Interpolation value: ",
                       interpolation_proto));
  }
}

proto::BrushBehavior::Source EncodeBrushBehaviorSource(
    BrushBehavior::Source source) {
  // NOLINTBEGIN(whitespace/line_length)
  switch (source) {
    case BrushBehavior::Source::kNormalizedPressure:
      return proto::BrushBehavior::SOURCE_NORMALIZED_PRESSURE;
    case BrushBehavior::Source::kTiltInRadians:
      return proto::BrushBehavior::SOURCE_TILT_IN_RADIANS;
    case BrushBehavior::Source::kTiltXInRadians:
      return proto::BrushBehavior::SOURCE_TILT_X_IN_RADIANS;
    case BrushBehavior::Source::kTiltYInRadians:
      return proto::BrushBehavior::SOURCE_TILT_Y_IN_RADIANS;
    case BrushBehavior::Source::kOrientationInRadians:
      return proto::BrushBehavior::SOURCE_ORIENTATION_IN_RADIANS;
    case BrushBehavior::Source::kOrientationAboutZeroInRadians:
      return proto::BrushBehavior::SOURCE_ORIENTATION_ABOUT_ZERO_IN_RADIANS;
    case BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond:
      return proto::BrushBehavior::
          SOURCE_SPEED_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND;
    case BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond:
      return proto::BrushBehavior::
          SOURCE_VELOCITY_X_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND;
    case BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond:
      return proto::BrushBehavior::
          SOURCE_VELOCITY_Y_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND;
    case BrushBehavior::Source::kDirectionInRadians:
      return proto::BrushBehavior::SOURCE_DIRECTION_IN_RADIANS;
    case BrushBehavior::Source::kDirectionAboutZeroInRadians:
      return proto::BrushBehavior::SOURCE_DIRECTION_ABOUT_ZERO_IN_RADIANS;
    case BrushBehavior::Source::kNormalizedDirectionX:
      return proto::BrushBehavior::SOURCE_NORMALIZED_DIRECTION_X;
    case BrushBehavior::Source::kNormalizedDirectionY:
      return proto::BrushBehavior::SOURCE_NORMALIZED_DIRECTION_Y;
    case BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          SOURCE_DISTANCE_TRAVELED_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Source::kTimeOfInputInSeconds:
      return proto::BrushBehavior::SOURCE_TIME_OF_INPUT_IN_SECONDS;
    case BrushBehavior::Source::kTimeOfInputInMillis:
      return proto::BrushBehavior::SOURCE_TIME_OF_INPUT_IN_MILLIS;
    case BrushBehavior::Source::
        kPredictedDistanceTraveledInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          SOURCE_PREDICTED_DISTANCE_TRAVELED_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Source::kPredictedTimeElapsedInSeconds:
      return proto::BrushBehavior::SOURCE_PREDICTED_TIME_ELAPSED_IN_SECONDS;
    case BrushBehavior::Source::kPredictedTimeElapsedInMillis:
      return proto::BrushBehavior::SOURCE_PREDICTED_TIME_ELAPSED_IN_MILLIS;
    case BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          SOURCE_DISTANCE_REMAINING_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Source::kTimeSinceInputInSeconds:
      return proto::BrushBehavior::SOURCE_TIME_SINCE_INPUT_IN_SECONDS;
    case BrushBehavior::Source::kTimeSinceInputInMillis:
      return proto::BrushBehavior::SOURCE_TIME_SINCE_INPUT_IN_MILLIS;
    case BrushBehavior::Source::
        kAccelerationInMultiplesOfBrushSizePerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_ACCELERATION_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kAccelerationXInMultiplesOfBrushSizePerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_ACCELERATION_X_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kAccelerationYInMultiplesOfBrushSizePerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_ACCELERATION_Y_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_ACCELERATION_FORWARD_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_ACCELERATION_LATERAL_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED;
    case BrushBehavior::Source::kInputSpeedInCentimetersPerSecond:
      return proto::BrushBehavior::SOURCE_INPUT_SPEED_IN_CENTIMETERS_PER_SECOND;
    case BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond:
      return proto::BrushBehavior::
          SOURCE_INPUT_VELOCITY_X_IN_CENTIMETERS_PER_SECOND;
    case BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond:
      return proto::BrushBehavior::
          SOURCE_INPUT_VELOCITY_Y_IN_CENTIMETERS_PER_SECOND;
    case BrushBehavior::Source::kInputDistanceTraveledInCentimeters:
      return proto::BrushBehavior::
          SOURCE_INPUT_DISTANCE_TRAVELED_IN_CENTIMETERS;
    case BrushBehavior::Source::kPredictedInputDistanceTraveledInCentimeters:
      return proto::BrushBehavior::
          SOURCE_PREDICTED_INPUT_DISTANCE_TRAVELED_IN_CENTIMETERS;
    case BrushBehavior::Source::kInputAccelerationInCentimetersPerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_INPUT_ACCELERATION_IN_CENTIMETERS_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kInputAccelerationXInCentimetersPerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_INPUT_ACCELERATION_X_IN_CENTIMETERS_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kInputAccelerationYInCentimetersPerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_INPUT_ACCELERATION_Y_IN_CENTIMETERS_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kInputAccelerationForwardInCentimetersPerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_INPUT_ACCELERATION_FORWARD_IN_CENTIMETERS_PER_SECOND_SQUARED;
    case BrushBehavior::Source::
        kInputAccelerationLateralInCentimetersPerSecondSquared:
      return proto::BrushBehavior::
          SOURCE_INPUT_ACCELERATION_LATERAL_IN_CENTIMETERS_PER_SECOND_SQUARED;
    case BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength:
      return proto::BrushBehavior::
          SOURCE_DISTANCE_REMAINING_AS_FRACTION_OF_STROKE_LENGTH;
  }
  // NOLINTEND(whitespace/line_length)
  return proto::BrushBehavior::SOURCE_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::Source> DecodeBrushBehaviorSource(
    proto::BrushBehavior::Source source_proto) {
  // NOLINTBEGIN(whitespace/line_length)
  switch (source_proto) {
    case proto::BrushBehavior::SOURCE_NORMALIZED_PRESSURE:
      return BrushBehavior::Source::kNormalizedPressure;
    case proto::BrushBehavior::SOURCE_TILT_IN_RADIANS:
      return BrushBehavior::Source::kTiltInRadians;
    case proto::BrushBehavior::SOURCE_TILT_X_IN_RADIANS:
      return BrushBehavior::Source::kTiltXInRadians;
    case proto::BrushBehavior::SOURCE_TILT_Y_IN_RADIANS:
      return BrushBehavior::Source::kTiltYInRadians;
    case proto::BrushBehavior::SOURCE_ORIENTATION_IN_RADIANS:
      return BrushBehavior::Source::kOrientationInRadians;
    case proto::BrushBehavior::SOURCE_ORIENTATION_ABOUT_ZERO_IN_RADIANS:
      return BrushBehavior::Source::kOrientationAboutZeroInRadians;
    case proto::BrushBehavior::
        SOURCE_SPEED_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND:
      return BrushBehavior::Source::kSpeedInMultiplesOfBrushSizePerSecond;
    case proto::BrushBehavior::
        SOURCE_VELOCITY_X_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND:
      return BrushBehavior::Source::kVelocityXInMultiplesOfBrushSizePerSecond;
    case proto::BrushBehavior::
        SOURCE_VELOCITY_Y_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND:
      return BrushBehavior::Source::kVelocityYInMultiplesOfBrushSizePerSecond;
    case proto::BrushBehavior::SOURCE_NORMALIZED_DIRECTION_X:
      return BrushBehavior::Source::kNormalizedDirectionX;
    case proto::BrushBehavior::SOURCE_NORMALIZED_DIRECTION_Y:
      return BrushBehavior::Source::kNormalizedDirectionY;
    case proto::BrushBehavior::
        SOURCE_DISTANCE_TRAVELED_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Source::kDistanceTraveledInMultiplesOfBrushSize;
    case proto::BrushBehavior::SOURCE_TIME_OF_INPUT_IN_SECONDS:
      return BrushBehavior::Source::kTimeOfInputInSeconds;
    case proto::BrushBehavior::SOURCE_TIME_OF_INPUT_IN_MILLIS:
      return BrushBehavior::Source::kTimeOfInputInMillis;
    case proto::BrushBehavior::
        SOURCE_PREDICTED_DISTANCE_TRAVELED_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Source::
          kPredictedDistanceTraveledInMultiplesOfBrushSize;
    case proto::BrushBehavior::SOURCE_PREDICTED_TIME_ELAPSED_IN_SECONDS:
      return BrushBehavior::Source::kPredictedTimeElapsedInSeconds;
    case proto::BrushBehavior::SOURCE_PREDICTED_TIME_ELAPSED_IN_MILLIS:
      return BrushBehavior::Source::kPredictedTimeElapsedInMillis;
    case proto::BrushBehavior::
        SOURCE_DISTANCE_REMAINING_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Source::kDistanceRemainingInMultiplesOfBrushSize;
    case proto::BrushBehavior::SOURCE_TIME_SINCE_INPUT_IN_SECONDS:
      return BrushBehavior::Source::kTimeSinceInputInSeconds;
    case proto::BrushBehavior::SOURCE_TIME_SINCE_INPUT_IN_MILLIS:
      return BrushBehavior::Source::kTimeSinceInputInMillis;
    case proto::BrushBehavior::SOURCE_DIRECTION_IN_RADIANS:
      return BrushBehavior::Source::kDirectionInRadians;
    case proto::BrushBehavior::SOURCE_DIRECTION_ABOUT_ZERO_IN_RADIANS:
      return BrushBehavior::Source::kDirectionAboutZeroInRadians;
    case proto::BrushBehavior::
        SOURCE_ACCELERATION_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kAccelerationInMultiplesOfBrushSizePerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_ACCELERATION_X_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kAccelerationXInMultiplesOfBrushSizePerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_ACCELERATION_Y_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kAccelerationYInMultiplesOfBrushSizePerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_ACCELERATION_FORWARD_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_ACCELERATION_LATERAL_IN_MULTIPLES_OF_BRUSH_SIZE_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared;
    case proto::BrushBehavior::SOURCE_INPUT_SPEED_IN_CENTIMETERS_PER_SECOND:
      return BrushBehavior::Source::kInputSpeedInCentimetersPerSecond;
    case proto::BrushBehavior::
        SOURCE_INPUT_VELOCITY_X_IN_CENTIMETERS_PER_SECOND:
      return BrushBehavior::Source::kInputVelocityXInCentimetersPerSecond;
    case proto::BrushBehavior::
        SOURCE_INPUT_VELOCITY_Y_IN_CENTIMETERS_PER_SECOND:
      return BrushBehavior::Source::kInputVelocityYInCentimetersPerSecond;
    case proto::BrushBehavior::SOURCE_INPUT_DISTANCE_TRAVELED_IN_CENTIMETERS:
      return BrushBehavior::Source::kInputDistanceTraveledInCentimeters;
    case proto::BrushBehavior::
        SOURCE_PREDICTED_INPUT_DISTANCE_TRAVELED_IN_CENTIMETERS:
      return BrushBehavior::Source::
          kPredictedInputDistanceTraveledInCentimeters;
    case proto::BrushBehavior::
        SOURCE_INPUT_ACCELERATION_IN_CENTIMETERS_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kInputAccelerationInCentimetersPerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_INPUT_ACCELERATION_X_IN_CENTIMETERS_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kInputAccelerationXInCentimetersPerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_INPUT_ACCELERATION_Y_IN_CENTIMETERS_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kInputAccelerationYInCentimetersPerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_INPUT_ACCELERATION_FORWARD_IN_CENTIMETERS_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kInputAccelerationForwardInCentimetersPerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_INPUT_ACCELERATION_LATERAL_IN_CENTIMETERS_PER_SECOND_SQUARED:
      return BrushBehavior::Source::
          kInputAccelerationLateralInCentimetersPerSecondSquared;
    case proto::BrushBehavior::
        SOURCE_DISTANCE_REMAINING_AS_FRACTION_OF_STROKE_LENGTH:
      return BrushBehavior::Source::kDistanceRemainingAsFractionOfStrokeLength;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushBehavior.Source value: ", source_proto));
  }
  // NOLINTEND(whitespace/line_length)
}

proto::BrushBehavior::Target EncodeBrushBehaviorTarget(
    BrushBehavior::Target target) {
  switch (target) {
    case BrushBehavior::Target::kWidthMultiplier:
      return proto::BrushBehavior::TARGET_WIDTH_MULTIPLIER;
    case BrushBehavior::Target::kHeightMultiplier:
      return proto::BrushBehavior::TARGET_HEIGHT_MULTIPLIER;
    case BrushBehavior::Target::kSizeMultiplier:
      return proto::BrushBehavior::TARGET_SIZE_MULTIPLIER;
    case BrushBehavior::Target::kSlantOffsetInRadians:
      return proto::BrushBehavior::TARGET_SLANT_OFFSET_IN_RADIANS;
    case BrushBehavior::Target::kPinchOffset:
      return proto::BrushBehavior::TARGET_PINCH_OFFSET;
    case BrushBehavior::Target::kRotationOffsetInRadians:
      return proto::BrushBehavior::TARGET_ROTATION_OFFSET_IN_RADIANS;
    case BrushBehavior::Target::kCornerRoundingOffset:
      return proto::BrushBehavior::TARGET_CORNER_ROUNDING_OFFSET;
    case BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          TARGET_POSITION_OFFSET_X_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          TARGET_POSITION_OFFSET_Y_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Target::kPositionOffsetForwardInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          TARGET_POSITION_OFFSET_FORWARD_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Target::kPositionOffsetLateralInMultiplesOfBrushSize:
      return proto::BrushBehavior::
          TARGET_POSITION_OFFSET_LATERAL_IN_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::Target::kTextureAnimationProgressOffset:
      return proto::BrushBehavior::TARGET_UNSPECIFIED;
    case BrushBehavior::Target::kHueOffsetInRadians:
      return proto::BrushBehavior::TARGET_HUE_OFFSET_IN_RADIANS;
    case BrushBehavior::Target::kSaturationMultiplier:
      return proto::BrushBehavior::TARGET_SATURATION_MULTIPLIER;
    case BrushBehavior::Target::kLuminosity:
      return proto::BrushBehavior::TARGET_LUMINOSITY;
    case BrushBehavior::Target::kOpacityMultiplier:
      return proto::BrushBehavior::TARGET_OPACITY_MULTIPLIER;
  }
  return proto::BrushBehavior::TARGET_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::Target> DecodeBrushBehaviorTarget(
    proto::BrushBehavior::Target target_proto) {
  switch (target_proto) {
    case proto::BrushBehavior::TARGET_WIDTH_MULTIPLIER:
      return BrushBehavior::Target::kWidthMultiplier;
    case proto::BrushBehavior::TARGET_HEIGHT_MULTIPLIER:
      return BrushBehavior::Target::kHeightMultiplier;
    case proto::BrushBehavior::TARGET_SIZE_MULTIPLIER:
      return BrushBehavior::Target::kSizeMultiplier;
    case proto::BrushBehavior::TARGET_SLANT_OFFSET_IN_RADIANS:
      return BrushBehavior::Target::kSlantOffsetInRadians;
    case proto::BrushBehavior::TARGET_PINCH_OFFSET:
      return BrushBehavior::Target::kPinchOffset;
    case proto::BrushBehavior::TARGET_ROTATION_OFFSET_IN_RADIANS:
      return BrushBehavior::Target::kRotationOffsetInRadians;
    case proto::BrushBehavior::TARGET_CORNER_ROUNDING_OFFSET:
      return BrushBehavior::Target::kCornerRoundingOffset;
    case proto::BrushBehavior::TARGET_HUE_OFFSET_IN_RADIANS:
      return BrushBehavior::Target::kHueOffsetInRadians;
    case proto::BrushBehavior::TARGET_SATURATION_MULTIPLIER:
      return BrushBehavior::Target::kSaturationMultiplier;
    case proto::BrushBehavior::TARGET_LUMINOSITY:
      return BrushBehavior::Target::kLuminosity;
    case proto::BrushBehavior::TARGET_OPACITY_MULTIPLIER:
      return BrushBehavior::Target::kOpacityMultiplier;
    case proto::BrushBehavior::
        TARGET_POSITION_OFFSET_X_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Target::kPositionOffsetXInMultiplesOfBrushSize;
    case proto::BrushBehavior::
        TARGET_POSITION_OFFSET_Y_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Target::kPositionOffsetYInMultiplesOfBrushSize;
    case proto::BrushBehavior::
        TARGET_POSITION_OFFSET_FORWARD_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Target::
          kPositionOffsetForwardInMultiplesOfBrushSize;
    case proto::BrushBehavior::
        TARGET_POSITION_OFFSET_LATERAL_IN_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::Target::
          kPositionOffsetLateralInMultiplesOfBrushSize;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushBehavior.Target value: ", target_proto));
  }
}

proto::BrushBehavior::PolarTarget EncodeBrushBehaviorPolarTarget(
    BrushBehavior::PolarTarget target) {
  switch (target) {
    case BrushBehavior::PolarTarget::
        kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize:
      return proto::BrushBehavior::
          POLAR_POSITION_OFFSET_ABSOLUTE_IN_RADIANS_AND_MULTIPLES_OF_BRUSH_SIZE;
    case BrushBehavior::PolarTarget::
        kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize:
      return proto::BrushBehavior::
          POLAR_POSITION_OFFSET_RELATIVE_IN_RADIANS_AND_MULTIPLES_OF_BRUSH_SIZE;
  }
  return proto::BrushBehavior::POLAR_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::PolarTarget> DecodeBrushBehaviorPolarTarget(
    proto::BrushBehavior::PolarTarget target_proto) {
  switch (target_proto) {
    case proto::BrushBehavior::
        POLAR_POSITION_OFFSET_ABSOLUTE_IN_RADIANS_AND_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::PolarTarget::
          kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize;
    case proto::BrushBehavior::
        POLAR_POSITION_OFFSET_RELATIVE_IN_RADIANS_AND_MULTIPLES_OF_BRUSH_SIZE:
      return BrushBehavior::PolarTarget::
          kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushBehavior.PolarTarget value: ", target_proto));
  }
}

proto::BrushBehavior::OutOfRange EncodeBrushBehaviorOutOfRange(
    BrushBehavior::OutOfRange out_of_range) {
  switch (out_of_range) {
    case BrushBehavior::OutOfRange::kClamp:
      return proto::BrushBehavior::OUT_OF_RANGE_CLAMP;
    case BrushBehavior::OutOfRange::kRepeat:
      return proto::BrushBehavior::OUT_OF_RANGE_REPEAT;
    case BrushBehavior::OutOfRange::kMirror:
      return proto::BrushBehavior::OUT_OF_RANGE_MIRROR;
  }
  return proto::BrushBehavior::OUT_OF_RANGE_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::OutOfRange> DecodeBrushBehaviorOutOfRange(
    proto::BrushBehavior::OutOfRange out_of_range_proto) {
  switch (out_of_range_proto) {
    case proto::BrushBehavior::OUT_OF_RANGE_CLAMP:
      return BrushBehavior::OutOfRange::kClamp;
    case proto::BrushBehavior::OUT_OF_RANGE_REPEAT:
      return BrushBehavior::OutOfRange::kRepeat;
    case proto::BrushBehavior::OUT_OF_RANGE_MIRROR:
      return BrushBehavior::OutOfRange::kMirror;
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("invalid ink.proto.BrushBehavior.OutOfRange value: ",
                       out_of_range_proto));
  }
}

uint32_t EncodeBrushBehaviorEnabledToolTypes(
    BrushBehavior::EnabledToolTypes types) {
  return (types.unknown ? 1u << proto::CodedStrokeInputBatch::UNKNOWN_TYPE
                        : 0u) |
         (types.mouse ? 1u << proto::CodedStrokeInputBatch::MOUSE : 0u) |
         (types.touch ? 1u << proto::CodedStrokeInputBatch::TOUCH : 0u) |
         (types.stylus ? 1u << proto::CodedStrokeInputBatch::STYLUS : 0u);
}

BrushBehavior::EnabledToolTypes DecodeBrushBehaviorEnabledToolTypes(
    uint32_t types) {
  return BrushBehavior::EnabledToolTypes{
      .unknown =
          (types & (1u << proto::CodedStrokeInputBatch::UNKNOWN_TYPE)) != 0,
      .mouse = (types & (1u << proto::CodedStrokeInputBatch::MOUSE)) != 0,
      .touch = (types & (1u << proto::CodedStrokeInputBatch::TOUCH)) != 0,
      .stylus = (types & (1u << proto::CodedStrokeInputBatch::STYLUS)) != 0,
  };
}

proto::BrushBehavior::OptionalInputProperty
EncodeBrushBehaviorOptionalInputProperty(
    BrushBehavior::OptionalInputProperty optional_input) {
  switch (optional_input) {
    case BrushBehavior::OptionalInputProperty::kPressure:
      return proto::BrushBehavior::OPTIONAL_INPUT_PRESSURE;
    case BrushBehavior::OptionalInputProperty::kTilt:
      return proto::BrushBehavior::OPTIONAL_INPUT_TILT;
    case BrushBehavior::OptionalInputProperty::kOrientation:
      return proto::BrushBehavior::OPTIONAL_INPUT_ORIENTATION;
    case BrushBehavior::OptionalInputProperty::kTiltXAndY:
      return proto::BrushBehavior::OPTIONAL_INPUT_TILT_X_AND_Y;
  }
  return proto::BrushBehavior::OPTIONAL_INPUT_UNSPECIFIED;
}

absl::StatusOr<BrushBehavior::OptionalInputProperty>
DecodeBrushBehaviorOptionalInputProperty(
    proto::BrushBehavior::OptionalInputProperty optional_input_proto) {
  switch (optional_input_proto) {
    case proto::BrushBehavior::OPTIONAL_INPUT_PRESSURE:
      return BrushBehavior::OptionalInputProperty::kPressure;
    case proto::BrushBehavior::OPTIONAL_INPUT_TILT:
      return BrushBehavior::OptionalInputProperty::kTilt;
    case proto::BrushBehavior::OPTIONAL_INPUT_ORIENTATION:
      return BrushBehavior::OptionalInputProperty::kOrientation;
    case proto::BrushBehavior::OPTIONAL_INPUT_TILT_X_AND_Y:
      return BrushBehavior::OptionalInputProperty::kTiltXAndY;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushBehavior.OptionalInputProperty value: ",
          optional_input_proto));
  }
}

proto::PredefinedEasingFunction EncodeEasingFunctionPredefined(
    EasingFunction::Predefined predefined) {
  switch (predefined) {
    case EasingFunction::Predefined::kLinear:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_LINEAR;
    case EasingFunction::Predefined::kEase:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE;
    case EasingFunction::Predefined::kEaseIn:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_IN;
    case EasingFunction::Predefined::kEaseOut:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_OUT;
    case EasingFunction::Predefined::kEaseInOut:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_IN_OUT;
    case EasingFunction::Predefined::kStepStart:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_STEP_START;
    case EasingFunction::Predefined::kStepEnd:
      return proto::PredefinedEasingFunction::PREDEFINED_EASING_STEP_END;
  }
  return proto::PredefinedEasingFunction::PREDEFINED_EASING_UNSPECIFIED;
}

absl::StatusOr<EasingFunction::Predefined> DecodeEasingFunctionPredefined(
    proto::PredefinedEasingFunction predefined_proto) {
  switch (predefined_proto) {
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_LINEAR:
      return EasingFunction::Predefined::kLinear;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE:
      return EasingFunction::Predefined::kEase;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_IN:
      return EasingFunction::Predefined::kEaseIn;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_OUT:
      return EasingFunction::Predefined::kEaseOut;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_EASE_IN_OUT:
      return EasingFunction::Predefined::kEaseInOut;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_STEP_START:
      return EasingFunction::Predefined::kStepStart;
    case proto::PredefinedEasingFunction::PREDEFINED_EASING_STEP_END:
      return EasingFunction::Predefined::kStepEnd;
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("invalid ink.proto.PredefinedEasingFunction value: ",
                       predefined_proto));
  }
}

void EncodeEasingFunctionCubicBezier(
    const EasingFunction::CubicBezier& cubic_bezier,
    proto::CubicBezierEasingFunction& cubic_bezier_proto_out) {
  cubic_bezier_proto_out.set_x1(cubic_bezier.x1);
  cubic_bezier_proto_out.set_y1(cubic_bezier.y1);
  cubic_bezier_proto_out.set_x2(cubic_bezier.x2);
  cubic_bezier_proto_out.set_y2(cubic_bezier.y2);
}

EasingFunction::CubicBezier DecodeEasingFunctionCubicBezier(
    proto::CubicBezierEasingFunction cubic_bezier_proto) {
  return EasingFunction::CubicBezier{
      .x1 = cubic_bezier_proto.x1(),
      .y1 = cubic_bezier_proto.y1(),
      .x2 = cubic_bezier_proto.x2(),
      .y2 = cubic_bezier_proto.y2(),
  };
}

void EncodeEasingFunctionLinear(const EasingFunction::Linear& linear,
                                proto::LinearEasingFunction& linear_proto_out) {
  linear_proto_out.clear_x();
  linear_proto_out.clear_y();
  linear_proto_out.mutable_x()->Reserve(linear.points.size());
  linear_proto_out.mutable_y()->Reserve(linear.points.size());
  for (Point point : linear.points) {
    linear_proto_out.add_x(point.x);
    linear_proto_out.add_y(point.y);
  }
}

absl::StatusOr<EasingFunction::Linear> DecodeEasingFunctionLinear(
    proto::LinearEasingFunction linear_proto) {
  if (linear_proto.x_size() != linear_proto.y_size()) {
    return absl::InvalidArgumentError(
        "x and y fields of LinearEasingFunction must have the same length");
  }
  std::vector<Point> points;
  points.reserve(linear_proto.x_size());
  for (int i = 0; i < linear_proto.x_size(); ++i) {
    points.push_back({linear_proto.x(i), linear_proto.y(i)});
  }
  return EasingFunction::Linear{std::move(points)};
}

proto::StepPosition EncodeStepPosition(
    EasingFunction::StepPosition step_position) {
  switch (step_position) {
    case EasingFunction::StepPosition::kJumpStart:
      return proto::StepPosition::STEP_POSITION_JUMP_START;
    case EasingFunction::StepPosition::kJumpEnd:
      return proto::StepPosition::STEP_POSITION_JUMP_END;
    case EasingFunction::StepPosition::kJumpNone:
      return proto::StepPosition::STEP_POSITION_JUMP_NONE;
    case EasingFunction::StepPosition::kJumpBoth:
      return proto::StepPosition::STEP_POSITION_JUMP_BOTH;
  }
  return proto::StepPosition::STEP_POSITION_UNSPECIFIED;
}

void EncodeEasingFunctionSteps(const EasingFunction::Steps& steps,
                               proto::StepsEasingFunction& steps_proto_out) {
  steps_proto_out.set_step_count(steps.step_count);
  steps_proto_out.set_step_position(EncodeStepPosition(steps.step_position));
}

absl::StatusOr<EasingFunction::StepPosition> DecodeStepPosition(
    proto::StepPosition step_position_proto) {
  switch (step_position_proto) {
    case proto::StepPosition::STEP_POSITION_JUMP_END:
      return EasingFunction::StepPosition::kJumpEnd;
    case proto::StepPosition::STEP_POSITION_JUMP_START:
      return EasingFunction::StepPosition::kJumpStart;
    case proto::StepPosition::STEP_POSITION_JUMP_NONE:
      return EasingFunction::StepPosition::kJumpNone;
    case proto::StepPosition::STEP_POSITION_JUMP_BOTH:
      return EasingFunction::StepPosition::kJumpBoth;
    case proto::StepPosition::STEP_POSITION_UNSPECIFIED:
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.StepPosition value: ", step_position_proto));
  }
}

absl::StatusOr<EasingFunction::Steps> DecodeEasingFunctionSteps(
    proto::StepsEasingFunction steps_proto) {
  auto step_position = DecodeStepPosition(steps_proto.step_position());
  if (!step_position.ok()) {
    return step_position.status();
  }
  return EasingFunction::Steps{
      .step_count = steps_proto.step_count(),
      .step_position = *step_position,
  };
}

void EncodeEasingFunction(EasingFunction::Predefined predefined,
                          proto::BrushBehavior::ResponseNode& node_proto_out) {
  node_proto_out.set_predefined_response_curve(
      EncodeEasingFunctionPredefined(predefined));
}

void EncodeEasingFunction(const EasingFunction::CubicBezier& cubic_bezier,
                          proto::BrushBehavior::ResponseNode& node_proto_out) {
  EncodeEasingFunctionCubicBezier(
      cubic_bezier, *node_proto_out.mutable_cubic_bezier_response_curve());
}

void EncodeEasingFunction(const EasingFunction::Linear& linear,
                          proto::BrushBehavior::ResponseNode& node_proto_out) {
  EncodeEasingFunctionLinear(linear,
                             *node_proto_out.mutable_linear_response_curve());
}

void EncodeEasingFunction(const EasingFunction::Steps& steps,
                          proto::BrushBehavior::ResponseNode& node_proto_out) {
  EncodeEasingFunctionSteps(steps,
                            *node_proto_out.mutable_steps_response_curve());
}

void EncodeBrushBehaviorNode(const BrushBehavior::SourceNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::SourceNode* source_node_proto =
      node_proto_out.mutable_source_node();
  source_node_proto->set_source(EncodeBrushBehaviorSource(node.source));
  source_node_proto->set_source_out_of_range_behavior(
      EncodeBrushBehaviorOutOfRange(node.source_out_of_range_behavior));
  source_node_proto->set_source_value_range_start(node.source_value_range[0]);
  source_node_proto->set_source_value_range_end(node.source_value_range[1]);
}

void EncodeBrushBehaviorNode(const BrushBehavior::ConstantNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  node_proto_out.mutable_constant_node()->set_value(node.value);
}

void EncodeBrushBehaviorNode(const BrushBehavior::NoiseNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::NoiseNode* noise_node_proto =
      node_proto_out.mutable_noise_node();
  noise_node_proto->set_seed(node.seed);
  noise_node_proto->set_vary_over(
      EncodeBrushBehaviorDampingSource(node.vary_over));
  noise_node_proto->set_base_period(node.base_period);
}

void EncodeBrushBehaviorNode(const BrushBehavior::FallbackFilterNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  node_proto_out.mutable_fallback_filter_node()->set_is_fallback_for(
      EncodeBrushBehaviorOptionalInputProperty(node.is_fallback_for));
}

void EncodeBrushBehaviorNode(const BrushBehavior::ToolTypeFilterNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  node_proto_out.mutable_tool_type_filter_node()->set_enabled_tool_types(
      EncodeBrushBehaviorEnabledToolTypes(node.enabled_tool_types));
}

void EncodeBrushBehaviorNode(const BrushBehavior::DampingNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::DampingNode* damping_node_proto =
      node_proto_out.mutable_damping_node();
  damping_node_proto->set_damping_source(
      EncodeBrushBehaviorDampingSource(node.damping_source));
  damping_node_proto->set_damping_gap(node.damping_gap);
}

void EncodeBrushBehaviorNode(const BrushBehavior::ResponseNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  std::visit(
      [&node_proto_out](const auto& params) {
        EncodeEasingFunction(params, *node_proto_out.mutable_response_node());
      },
      node.response_curve.parameters);
}

void EncodeBrushBehaviorNode(const BrushBehavior::BinaryOpNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  node_proto_out.mutable_binary_op_node()->set_operation(
      EncodeBrushBehaviorBinaryOp(node.operation));
}

void EncodeBrushBehaviorNode(const BrushBehavior::InterpolationNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::InterpolationNode* interpolation_node_proto =
      node_proto_out.mutable_interpolation_node();
  interpolation_node_proto->set_interpolation(
      EncodeBrushBehaviorInterpolation(node.interpolation));
}

void EncodeBrushBehaviorNode(const BrushBehavior::TargetNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::TargetNode* target_node_proto =
      node_proto_out.mutable_target_node();
  target_node_proto->set_target(EncodeBrushBehaviorTarget(node.target));
  target_node_proto->set_target_modifier_range_start(
      node.target_modifier_range[0]);
  target_node_proto->set_target_modifier_range_end(
      node.target_modifier_range[1]);
}

void EncodeBrushBehaviorNode(const BrushBehavior::PolarTargetNode& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  proto::BrushBehavior::PolarTargetNode* target_node_proto =
      node_proto_out.mutable_polar_target_node();
  target_node_proto->set_target(EncodeBrushBehaviorPolarTarget(node.target));
  target_node_proto->set_angle_range_start(node.angle_range[0]);
  target_node_proto->set_angle_range_end(node.angle_range[1]);
  target_node_proto->set_magnitude_range_start(node.magnitude_range[0]);
  target_node_proto->set_magnitude_range_end(node.magnitude_range[1]);
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorSourceNode(
    const proto::BrushBehavior::SourceNode& node_proto) {
  absl::StatusOr<BrushBehavior::Source> source =
      DecodeBrushBehaviorSource(node_proto.source());
  if (!source.ok()) return source.status();

  absl::StatusOr<BrushBehavior::OutOfRange> source_out_of_range_behavior =
      DecodeBrushBehaviorOutOfRange(node_proto.source_out_of_range_behavior());
  if (!source_out_of_range_behavior.ok()) {
    return source_out_of_range_behavior.status();
  }

  return BrushBehavior::SourceNode{
      .source = *source,
      .source_out_of_range_behavior = *source_out_of_range_behavior,
      .source_value_range = {node_proto.source_value_range_start(),
                             node_proto.source_value_range_end()},
  };
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorConstantNode(
    const proto::BrushBehavior::ConstantNode& node_proto) {
  return BrushBehavior::ConstantNode{.value = node_proto.value()};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorNoiseNode(
    const proto::BrushBehavior::NoiseNode& node_proto) {
  absl::StatusOr<BrushBehavior::DampingSource> vary_over =
      DecodeBrushBehaviorDampingSource(node_proto.vary_over());
  if (!vary_over.ok()) return vary_over.status();
  return BrushBehavior::NoiseNode{
      .seed = node_proto.seed(),
      .vary_over = *vary_over,
      .base_period = node_proto.base_period(),
  };
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorResponseNode(
    const proto::BrushBehavior::ResponseNode& node_proto) {
  EasingFunction function;
  switch (node_proto.response_curve_case()) {
    case proto::BrushBehavior::ResponseNode::RESPONSE_CURVE_NOT_SET:
      return absl::InvalidArgumentError(
          "ink.proto.BrushBehavior.ResponseNode must specify a response_curve");
    case proto::BrushBehavior::ResponseNode::kPredefinedResponseCurve: {
      absl::StatusOr<EasingFunction::Predefined> predefined =
          DecodeEasingFunctionPredefined(
              node_proto.predefined_response_curve());
      if (!predefined.ok()) return predefined.status();
      function.parameters = *predefined;
    } break;
    case proto::BrushBehavior::ResponseNode::kCubicBezierResponseCurve:
      function.parameters = DecodeEasingFunctionCubicBezier(
          node_proto.cubic_bezier_response_curve());
      break;
    case proto::BrushBehavior::ResponseNode::kLinearResponseCurve: {
      absl::StatusOr<EasingFunction::Linear> linear =
          DecodeEasingFunctionLinear(node_proto.linear_response_curve());
      if (!linear.ok()) return linear.status();
      function.parameters = *linear;
    } break;
    case proto::BrushBehavior::ResponseNode::kStepsResponseCurve: {
      absl::StatusOr<EasingFunction::Steps> steps =
          DecodeEasingFunctionSteps(node_proto.steps_response_curve());
      if (!steps.ok()) return steps.status();
      function.parameters = *steps;
    } break;
  }
  return BrushBehavior::ResponseNode{.response_curve = std::move(function)};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorFallbackFilterNode(
    const proto::BrushBehavior::FallbackFilterNode& node_proto) {
  absl::StatusOr<BrushBehavior::OptionalInputProperty> property =
      DecodeBrushBehaviorOptionalInputProperty(node_proto.is_fallback_for());
  if (!property.ok()) return property.status();
  return BrushBehavior::FallbackFilterNode{.is_fallback_for = *property};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorToolTypeFilterNode(
    const proto::BrushBehavior::ToolTypeFilterNode& node_proto) {
  return BrushBehavior::ToolTypeFilterNode{
      .enabled_tool_types =
          DecodeBrushBehaviorEnabledToolTypes(node_proto.enabled_tool_types())};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorDampingNode(
    const proto::BrushBehavior::DampingNode& node_proto) {
  absl::StatusOr<BrushBehavior::DampingSource> damping_source =
      DecodeBrushBehaviorDampingSource(node_proto.damping_source());
  if (!damping_source.ok()) return damping_source.status();

  return BrushBehavior::DampingNode{
      .damping_source = *damping_source,
      .damping_gap = node_proto.damping_gap(),
  };
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorBinaryOpNode(
    proto::BrushBehavior::BinaryOpNode node_proto) {
  absl::StatusOr<BrushBehavior::BinaryOp> binary_op =
      DecodeBrushBehaviorBinaryOp(node_proto.operation());
  if (!binary_op.ok()) return binary_op.status();
  return BrushBehavior::BinaryOpNode{.operation = *binary_op};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorInterpolationNode(
    const proto::BrushBehavior::InterpolationNode& node_proto) {
  absl::StatusOr<BrushBehavior::Interpolation> interpolation =
      DecodeBrushBehaviorInterpolation(node_proto.interpolation());
  if (!interpolation.ok()) return interpolation.status();
  return BrushBehavior::InterpolationNode{.interpolation = *interpolation};
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorTargetNode(
    const proto::BrushBehavior::TargetNode& node_proto) {
  absl::StatusOr<BrushBehavior::Target> target =
      DecodeBrushBehaviorTarget(node_proto.target());
  if (!target.ok()) return target.status();

  return BrushBehavior::TargetNode{
      .target = *target,
      .target_modifier_range = {node_proto.target_modifier_range_start(),
                                node_proto.target_modifier_range_end()},
  };
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorPolarTargetNode(
    const proto::BrushBehavior::PolarTargetNode& node_proto) {
  absl::StatusOr<BrushBehavior::PolarTarget> target =
      DecodeBrushBehaviorPolarTarget(node_proto.target());
  if (!target.ok()) return target.status();

  return BrushBehavior::PolarTargetNode{
      .target = *target,
      .angle_range = {node_proto.angle_range_start(),
                      node_proto.angle_range_end()},
      .magnitude_range = {node_proto.magnitude_range_start(),
                          node_proto.magnitude_range_end()},
  };
}

void EncodeBrushBehavior(const BrushBehavior& behavior,
                         proto::BrushBehavior& behavior_proto_out) {
  behavior_proto_out.clear_nodes();
  for (const BrushBehavior::Node& node : behavior.nodes) {
    EncodeBrushBehaviorNode(node, *behavior_proto_out.add_nodes());
  }
}

absl::StatusOr<BrushBehavior> DecodeBrushBehavior(
    const proto::BrushBehavior& behavior_proto) {
  std::vector<BrushBehavior::Node> nodes;
  nodes.reserve(behavior_proto.nodes_size());
  for (const proto::BrushBehavior::Node& node_proto : behavior_proto.nodes()) {
    absl::StatusOr<BrushBehavior::Node> node =
        DecodeBrushBehaviorNode(node_proto);
    if (!node.ok()) return node.status();
    nodes.push_back(*std::move(node));
  }
  return BrushBehavior{.nodes = std::move(nodes)};
}

proto::BrushPaint::TextureLayer::Mapping EncodeBrushPaintTextureMapping(
    BrushPaint::TextureMapping mapping) {
  switch (mapping) {
    case BrushPaint::TextureMapping::kWinding:
      return proto::BrushPaint::TextureLayer::MAPPING_WINDING;
    case BrushPaint::TextureMapping::kTiling:
      return proto::BrushPaint::TextureLayer::MAPPING_TILING;
  }
  return proto::BrushPaint::TextureLayer::MAPPING_UNSPECIFIED;
}

absl::StatusOr<BrushPaint::TextureMapping> DecodeBrushPaintTextureMapping(
    proto::BrushPaint::TextureLayer::Mapping mapping_proto) {
  switch (mapping_proto) {
    case proto::BrushPaint::TextureLayer::MAPPING_WINDING:
      return BrushPaint::TextureMapping::kWinding;
    case proto::BrushPaint::TextureLayer::MAPPING_TILING:
      return BrushPaint::TextureMapping::kTiling;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushPaint.TextureLayer.mapping value: ",
          mapping_proto));
  }
}

proto::BrushPaint::TextureLayer::Origin EncodeBrushPaintTextureOrigin(
    BrushPaint::TextureOrigin origin) {
  switch (origin) {
    case BrushPaint::TextureOrigin::kStrokeSpaceOrigin:
      return proto::BrushPaint::TextureLayer::ORIGIN_STROKE_SPACE_ORIGIN;
    case BrushPaint::TextureOrigin::kFirstStrokeInput:
      return proto::BrushPaint::TextureLayer::ORIGIN_FIRST_STROKE_INPUT;
    case BrushPaint::TextureOrigin::kLastStrokeInput:
      return proto::BrushPaint::TextureLayer::ORIGIN_LAST_STROKE_INPUT;
  }
  return proto::BrushPaint::TextureLayer::ORIGIN_UNSPECIFIED;
}

absl::StatusOr<BrushPaint::TextureOrigin> DecodeBrushPaintTextureOrigin(
    proto::BrushPaint::TextureLayer::Origin origin_proto) {
  switch (origin_proto) {
    case proto::BrushPaint::TextureLayer::ORIGIN_STROKE_SPACE_ORIGIN:
      return BrushPaint::TextureOrigin::kStrokeSpaceOrigin;
    case proto::BrushPaint::TextureLayer::ORIGIN_FIRST_STROKE_INPUT:
      return BrushPaint::TextureOrigin::kFirstStrokeInput;
    case proto::BrushPaint::TextureLayer::ORIGIN_LAST_STROKE_INPUT:
      return BrushPaint::TextureOrigin::kLastStrokeInput;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushPaint.TextureLayer.origin value: ",
          origin_proto));
  }
}

proto::BrushPaint::TextureLayer::SizeUnit EncodeBrushPaintSizeUnit(
    BrushPaint::TextureSizeUnit size_unit) {
  switch (size_unit) {
    case BrushPaint::TextureSizeUnit::kBrushSize:
      return proto::BrushPaint::TextureLayer::SIZE_UNIT_BRUSH_SIZE;
    case BrushPaint::TextureSizeUnit::kStrokeCoordinates:
      return proto::BrushPaint::TextureLayer::SIZE_UNIT_STROKE_COORDINATES;
    case BrushPaint::TextureSizeUnit::kStrokeSize:
      return proto::BrushPaint::TextureLayer::SIZE_UNIT_UNSPECIFIED;
  }
  return proto::BrushPaint::TextureLayer::SIZE_UNIT_UNSPECIFIED;
}

absl::StatusOr<BrushPaint::TextureSizeUnit> DecodeBrushPaintSizeUnit(
    proto::BrushPaint::TextureLayer::SizeUnit size_unit_proto) {
  switch (size_unit_proto) {
    case proto::BrushPaint::TextureLayer::SIZE_UNIT_BRUSH_SIZE:
      return BrushPaint::TextureSizeUnit::kBrushSize;
    case proto::BrushPaint::TextureLayer::SIZE_UNIT_STROKE_COORDINATES:
      return BrushPaint::TextureSizeUnit::kStrokeCoordinates;
    case proto::BrushPaint::TextureLayer::SIZE_UNIT_UNSPECIFIED:
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushPaint.TextureLayer.size_unit value: ",
          size_unit_proto));
  }
}

proto::BrushPaint::TextureLayer::Wrap EncodeBrushPaintWrap(
    BrushPaint::TextureWrap wrap) {
  switch (wrap) {
    case BrushPaint::TextureWrap::kRepeat:
      return proto::BrushPaint::TextureLayer::WRAP_REPEAT;
    case BrushPaint::TextureWrap::kMirror:
      return proto::BrushPaint::TextureLayer::WRAP_MIRROR;
    case BrushPaint::TextureWrap::kClamp:
      return proto::BrushPaint::TextureLayer::WRAP_CLAMP;
  }
  return proto::BrushPaint::TextureLayer::WRAP_UNSPECIFIED;
}

absl::StatusOr<BrushPaint::TextureWrap> DecodeBrushPaintWrap(
    proto::BrushPaint::TextureLayer::Wrap wrap_proto) {
  switch (wrap_proto) {
    case proto::BrushPaint::TextureLayer::WRAP_REPEAT:
      return BrushPaint::TextureWrap::kRepeat;
    case proto::BrushPaint::TextureLayer::WRAP_MIRROR:
      return BrushPaint::TextureWrap::kMirror;
    case proto::BrushPaint::TextureLayer::WRAP_CLAMP:
      return BrushPaint::TextureWrap::kClamp;
    case proto::BrushPaint::TextureLayer::WRAP_UNSPECIFIED:
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("invalid ink.proto.BrushPaint.TextureLayer.wrap value: ",
                       wrap_proto));
  }
}

proto::BrushPaint::TextureLayer::BlendMode EncodeBrushPaintBlendMode(
    BrushPaint::BlendMode blend_mode) {
  switch (blend_mode) {
    case BrushPaint::BlendMode::kModulate:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_MODULATE;
    case BrushPaint::BlendMode::kDstIn:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_DST_IN;
    case BrushPaint::BlendMode::kDstOut:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_DST_OUT;
    case BrushPaint::BlendMode::kSrcAtop:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_ATOP;
    case BrushPaint::BlendMode::kSrcIn:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_IN;
    case BrushPaint::BlendMode::kSrcOver:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_OVER;
    case BrushPaint::BlendMode::kDstOver:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_DST_OVER;
    case BrushPaint::BlendMode::kSrc:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_SRC;
    case BrushPaint::BlendMode::kDst:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_DST;
    case BrushPaint::BlendMode::kSrcOut:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_OUT;
    case BrushPaint::BlendMode::kDstAtop:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_DST_ATOP;
    case BrushPaint::BlendMode::kXor:
      return proto::BrushPaint::TextureLayer::BLEND_MODE_XOR;
  }
  return proto::BrushPaint::TextureLayer::BLEND_MODE_UNSPECIFIED;
}

absl::StatusOr<BrushPaint::BlendMode> DecodeBrushPaintBlendMode(
    proto::BrushPaint::TextureLayer::BlendMode blend_mode_proto) {
  switch (blend_mode_proto) {
    case proto::BrushPaint::TextureLayer::BLEND_MODE_MODULATE:
      return BrushPaint::BlendMode::kModulate;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_DST_IN:
      return BrushPaint::BlendMode::kDstIn;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_DST_OUT:
      return BrushPaint::BlendMode::kDstOut;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_ATOP:
      return BrushPaint::BlendMode::kSrcAtop;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_IN:
      return BrushPaint::BlendMode::kSrcIn;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_OVER:
      return BrushPaint::BlendMode::kSrcOver;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_DST_OVER:
      return BrushPaint::BlendMode::kDstOver;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_SRC:
      return BrushPaint::BlendMode::kSrc;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_DST:
      return BrushPaint::BlendMode::kDst;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_SRC_OUT:
      return BrushPaint::BlendMode::kSrcOut;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_DST_ATOP:
      return BrushPaint::BlendMode::kDstAtop;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_XOR:
      return BrushPaint::BlendMode::kXor;
    case proto::BrushPaint::TextureLayer::BLEND_MODE_UNSPECIFIED:
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.BrushPaint.TextureLayer.blend_mode value: ",
          blend_mode_proto));
  }
}

void EncodeBrushPaintTextureLayer(
    const BrushPaint::TextureLayer& layer,
    proto::BrushPaint::TextureLayer& layer_proto_out) {
  layer_proto_out.set_client_texture_id(layer.client_texture_id);
  layer_proto_out.set_size_unit(EncodeBrushPaintSizeUnit(layer.size_unit));
  layer_proto_out.set_wrap_x(EncodeBrushPaintWrap(layer.wrap_x));
  layer_proto_out.set_wrap_y(EncodeBrushPaintWrap(layer.wrap_y));
  layer_proto_out.set_size_x(layer.size.x);
  layer_proto_out.set_size_y(layer.size.y);
  layer_proto_out.set_mapping(EncodeBrushPaintTextureMapping(layer.mapping));
  layer_proto_out.set_origin(EncodeBrushPaintTextureOrigin(layer.origin));
  layer_proto_out.set_offset_x(layer.offset.x);
  layer_proto_out.set_offset_y(layer.offset.y);
  layer_proto_out.set_rotation_in_radians(layer.rotation.ValueInRadians());
  layer_proto_out.set_opacity(layer.opacity);
  layer_proto_out.set_blend_mode(EncodeBrushPaintBlendMode(layer.blend_mode));
}

absl::StatusOr<BrushPaint::TextureLayer> DecodeBrushPaintTextureLayer(
    const proto::BrushPaint::TextureLayer& layer_proto,
    ClientTextureIdProvider get_client_texture_id) {
  auto mapping = DecodeBrushPaintTextureMapping(layer_proto.mapping());
  if (!mapping.ok()) {
    return mapping.status();
  }
  auto origin = DecodeBrushPaintTextureOrigin(layer_proto.origin());
  if (!origin.ok()) {
    return origin.status();
  }
  auto size_unit = DecodeBrushPaintSizeUnit(layer_proto.size_unit());
  if (!size_unit.ok()) {
    return size_unit.status();
  }
  auto wrap_x = DecodeBrushPaintWrap(layer_proto.wrap_x());
  if (!wrap_x.ok()) {
    return wrap_x.status();
  }
  auto wrap_y = DecodeBrushPaintWrap(layer_proto.wrap_y());
  if (!wrap_y.ok()) {
    return wrap_y.status();
  }
  auto blend_mode = DecodeBrushPaintBlendMode(layer_proto.blend_mode());
  if (!blend_mode.ok()) {
    return blend_mode.status();
  }

  absl::StatusOr<std::string> client_texture_id =
      get_client_texture_id(layer_proto.client_texture_id());
  if (!client_texture_id.ok()) {
    return client_texture_id.status();
  }
  BrushPaint::TextureLayer texture_layer{
      .client_texture_id = *client_texture_id,
      .mapping = *mapping,
      .origin = *origin,
      .size_unit = *size_unit,
      .wrap_x = *wrap_x,
      .wrap_y = *wrap_y,
      .size = {layer_proto.size_x(), layer_proto.size_y()},
      .offset = {layer_proto.offset_x(), layer_proto.offset_y()},
      .rotation = Angle::Radians(layer_proto.rotation_in_radians()),
      .opacity = layer_proto.opacity(),
      .blend_mode = *blend_mode};
  if (absl::Status status =
          brush_internal::ValidateBrushPaintTextureLayer(texture_layer);
      !status.ok()) {
    return status;
  }
  return std::move(texture_layer);
}

void EncodeBrushFamilyInputModel(
    const BrushFamily::SpringModel& model,
    proto::BrushFamily::InputModel& model_proto_out) {
  model_proto_out.mutable_spring_model();  // no fields to set
}

void EncodeBrushFamilyInputModel(
    const BrushFamily::ExperimentalRawPositionModel& model,
    proto::BrushFamily::InputModel& model_proto_out) {
  model_proto_out
      .mutable_experimental_raw_position_model();  // no fields to set
}

void EncodeBrushFamilyInputModel(
    const BrushFamily::InputModel& input_model,
    proto::BrushFamily::InputModel& model_proto_out) {
  std::visit(
      [&model_proto_out](const auto& model) {
        EncodeBrushFamilyInputModel(model, model_proto_out);
      },
      input_model);
}

absl::StatusOr<BrushFamily::InputModel> DecodeBrushFamilyInputModel(
    const proto::BrushFamily::InputModel& model_proto) {
  switch (model_proto.input_model_case()) {
    case proto::BrushFamily::InputModel::kSpringModel:
      return BrushFamily::SpringModel{};
    case proto::BrushFamily::InputModel::kExperimentalRawPositionModel:
      return BrushFamily::ExperimentalRawPositionModel{};
    case proto::BrushFamily::InputModel::INPUT_MODEL_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "ink.proto.BrushFamily.InputModel must specify an input model");
}

}  // namespace

void EncodeBrushBehaviorNode(const BrushBehavior::Node& node,
                             proto::BrushBehavior::Node& node_proto_out) {
  std::visit(
      [&node_proto_out](const auto& node) {
        EncodeBrushBehaviorNode(node, node_proto_out);
      },
      node);
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorNodeUnvalidated(
    const proto::BrushBehavior::Node& node_proto) {
  switch (node_proto.node_case()) {
    case proto::BrushBehavior::Node::kSourceNode:
      return DecodeBrushBehaviorSourceNode(node_proto.source_node());
    case proto::BrushBehavior::Node::kConstantNode:
      return DecodeBrushBehaviorConstantNode(node_proto.constant_node());
    case proto::BrushBehavior::Node::kNoiseNode:
      return DecodeBrushBehaviorNoiseNode(node_proto.noise_node());
    case proto::BrushBehavior::Node::kFallbackFilterNode:
      return DecodeBrushBehaviorFallbackFilterNode(
          node_proto.fallback_filter_node());
    case proto::BrushBehavior::Node::kToolTypeFilterNode:
      return DecodeBrushBehaviorToolTypeFilterNode(
          node_proto.tool_type_filter_node());
    case proto::BrushBehavior::Node::kDampingNode:
      return DecodeBrushBehaviorDampingNode(node_proto.damping_node());
    case proto::BrushBehavior::Node::kResponseNode:
      return DecodeBrushBehaviorResponseNode(node_proto.response_node());
    case proto::BrushBehavior::Node::kBinaryOpNode:
      return DecodeBrushBehaviorBinaryOpNode(node_proto.binary_op_node());
    case proto::BrushBehavior::Node::kInterpolationNode:
      return DecodeBrushBehaviorInterpolationNode(
          node_proto.interpolation_node());
    case proto::BrushBehavior::Node::kTargetNode:
      return DecodeBrushBehaviorTargetNode(node_proto.target_node());
    case proto::BrushBehavior::Node::kPolarTargetNode:
      return DecodeBrushBehaviorPolarTargetNode(node_proto.polar_target_node());
    case proto::BrushBehavior::Node::NODE_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "ink.proto.BrushBehavior.Node must specify a node");
}

absl::StatusOr<BrushBehavior::Node> DecodeBrushBehaviorNode(
    const proto::BrushBehavior::Node& node_proto) {
  absl::StatusOr<BrushBehavior::Node> node =
      DecodeBrushBehaviorNodeUnvalidated(node_proto);
  if (!node.ok()) {
    return node.status();
  }
  if (absl::Status status = brush_internal::ValidateBrushBehaviorNode(*node);
      !status.ok()) {
    return status;
  }
  return *node;
}

void EncodeBrushPaint(const BrushPaint& paint,
                      proto::BrushPaint& paint_proto_out) {
  paint_proto_out.mutable_texture_layers()->Clear();
  paint_proto_out.mutable_texture_layers()->Reserve(
      paint.texture_layers.size());
  for (const BrushPaint::TextureLayer& layer : paint.texture_layers) {
    EncodeBrushPaintTextureLayer(layer, *paint_proto_out.add_texture_layers());
  }
}

absl::StatusOr<BrushPaint> DecodeBrushPaint(
    const proto::BrushPaint& paint_proto,
    ClientTextureIdProvider get_client_texture_id) {
  std::vector<BrushPaint::TextureLayer> layers;
  layers.reserve(paint_proto.texture_layers_size());
  for (const proto::BrushPaint::TextureLayer& layer_proto :
       paint_proto.texture_layers()) {
    absl::StatusOr<BrushPaint::TextureLayer> layer =
        DecodeBrushPaintTextureLayer(layer_proto, get_client_texture_id);
    if (!layer.ok()) {
      return layer.status();
    }
    layers.push_back(*std::move(layer));
  }
  BrushPaint paint{.texture_layers = std::move(layers)};
  if (absl::Status status = brush_internal::ValidateBrushPaintTopLevel(paint);
      !status.ok()) {
    return status;
  }
  return std::move(paint);
}

void EncodeBrushTip(const BrushTip& tip, proto::BrushTip& tip_proto_out) {
  tip_proto_out.set_scale_x(tip.scale.x);
  tip_proto_out.set_scale_y(tip.scale.y);
  tip_proto_out.set_corner_rounding(tip.corner_rounding);
  tip_proto_out.set_slant_radians(tip.slant.ValueInRadians());
  tip_proto_out.set_pinch(tip.pinch);
  tip_proto_out.set_rotation_radians(tip.rotation.ValueInRadians());
  tip_proto_out.set_opacity_multiplier(tip.opacity_multiplier);
  tip_proto_out.set_particle_gap_distance_scale(
      tip.particle_gap_distance_scale);
  tip_proto_out.set_particle_gap_duration_seconds(
      tip.particle_gap_duration.ToSeconds());

  tip_proto_out.mutable_behaviors()->Clear();
  tip_proto_out.mutable_behaviors()->Reserve(tip.behaviors.size());
  for (const BrushBehavior& behavior : tip.behaviors) {
    EncodeBrushBehavior(behavior, *tip_proto_out.add_behaviors());
  }
}

absl::StatusOr<BrushTip> DecodeBrushTip(const proto::BrushTip& tip_proto) {
  std::vector<BrushBehavior> behaviors;
  behaviors.reserve(tip_proto.behaviors_size());
  for (const proto::BrushBehavior& behavior_proto : tip_proto.behaviors()) {
    absl::StatusOr<BrushBehavior> behavior =
        DecodeBrushBehavior(behavior_proto);
    if (!behavior.ok()) {
      return behavior.status();
    }
    behaviors.push_back(*std::move(behavior));
  }
  BrushTip tip = {.behaviors = std::move(behaviors)};
  if (tip_proto.has_scale_x()) {
    tip.scale.x = tip_proto.scale_x();
  }
  if (tip_proto.has_scale_y()) {
    tip.scale.y = tip_proto.scale_y();
  }
  if (tip_proto.has_corner_rounding()) {
    tip.corner_rounding = tip_proto.corner_rounding();
  }
  if (tip_proto.has_slant_radians()) {
    tip.slant = Angle::Radians(tip_proto.slant_radians());
  }
  if (tip_proto.has_pinch()) {
    tip.pinch = tip_proto.pinch();
  }
  if (tip_proto.has_rotation_radians()) {
    tip.rotation = Angle::Radians(tip_proto.rotation_radians());
  }
  if (tip_proto.has_opacity_multiplier()) {
    tip.opacity_multiplier = tip_proto.opacity_multiplier();
  }
  if (tip_proto.has_particle_gap_distance_scale()) {
    tip.particle_gap_distance_scale = tip_proto.particle_gap_distance_scale();
  }
  if (tip_proto.has_particle_gap_duration_seconds()) {
    tip.particle_gap_duration =
        Duration32::Seconds(tip_proto.particle_gap_duration_seconds());
  }
  if (absl::Status status = brush_internal::ValidateBrushTip(tip);
      !status.ok()) {
    return status;
  }
  return tip;
}

void EncodeBrushCoat(const BrushCoat& coat, proto::BrushCoat& coat_proto_out) {
  EncodeBrushTip(coat.tip, *coat_proto_out.mutable_tip());
  EncodeBrushPaint(coat.paint, *coat_proto_out.mutable_paint());
}

absl::StatusOr<BrushCoat> DecodeBrushCoat(
    const proto::BrushCoat& coat_proto,
    ClientTextureIdProvider get_client_texture_id) {
  absl::StatusOr<BrushTip> tip = DecodeBrushTip(coat_proto.tip());
  if (!tip.ok()) {
    return tip.status();
  }
  absl::StatusOr<BrushPaint> paint =
      DecodeBrushPaint(coat_proto.paint(), get_client_texture_id);
  if (!paint.ok()) {
    return paint.status();
  }
  // There's no further validation to be done here if the paint and tip are
  // valid.
  return BrushCoat{.tip = *std::move(tip), .paint = *std::move(paint)};
}

void EncodeBrushFamilyTextureMap(
    const BrushFamily& family,
    ::google::protobuf::Map<std::string, std::string>& texture_id_to_bitmap_out,
    TextureBitmapProvider get_bitmap) {
  texture_id_to_bitmap_out.clear();
  // The set of texture ids for which we have already called get_bitmap().
  std::unordered_set<std::string> seen_ids = {};
  for (const BrushCoat& coat : family.GetCoats()) {
    for (const BrushPaint::TextureLayer& layer : coat.paint.texture_layers) {
      if (seen_ids.find(layer.client_texture_id) != seen_ids.end()) {
        continue;
      }

      std::optional<std::string> bitmap = get_bitmap(layer.client_texture_id);
      seen_ids.insert(layer.client_texture_id);
      if (!bitmap.has_value()) {
        continue;
      }

      texture_id_to_bitmap_out.insert({layer.client_texture_id, *bitmap});
    }
  }
}

void EncodeBrushFamily(const BrushFamily& family,
                       proto::BrushFamily& family_proto_out,
                       TextureBitmapProvider get_bitmap) {
  family_proto_out.Clear();
  EncodeBrushFamilyTextureMap(
      family, *family_proto_out.mutable_texture_id_to_bitmap(), get_bitmap);
  absl::Span<const BrushCoat> coats = family.GetCoats();
  family_proto_out.mutable_coats()->Reserve(coats.size());
  for (const BrushCoat& coat : coats) {
    EncodeBrushCoat(coat, *family_proto_out.add_coats());
  }

  if (family.GetClientBrushFamilyId().empty()) {
    family_proto_out.clear_client_brush_family_id();
  } else {
    family_proto_out.set_client_brush_family_id(
        family.GetClientBrushFamilyId());
  }

  EncodeBrushFamilyInputModel(family.GetInputModel(),
                              *family_proto_out.mutable_input_model());
}

absl::StatusOr<std::vector<BrushCoat>> DecodeBrushFamilyCoats(
    const proto::BrushFamily& family_proto,
    ClientTextureIdProvider get_client_texture_id) {
  std::vector<BrushCoat> coats;

  coats.reserve(family_proto.coats_size());
  for (const proto::BrushCoat& coat_proto : family_proto.coats()) {
    absl::StatusOr<BrushCoat> coat =
        DecodeBrushCoat(coat_proto, get_client_texture_id);
    if (!coat.ok()) {
      return coat.status();
    }
    coats.push_back(*std::move(coat));
  }
  return std::move(coats);
}

absl::StatusOr<BrushFamily> DecodeBrushFamily(
    const proto::BrushFamily& family_proto,
    ClientTextureIdProviderAndBitmapReceiver get_client_texture_id) {
  // ID map that also serves as a record of the IDs for which we've already
  // called `get_client_texture_id`.
  std::map<std::string, std::string> old_to_new_id = {};

  ClientTextureIdProvider texture_callback =
      [&family_proto, &old_to_new_id, &get_client_texture_id](
          const std::string& old_id) -> absl::StatusOr<std::string> {
    if (auto it = old_to_new_id.find(old_id); it != old_to_new_id.end()) {
      // No need to call `get_client_texture_id` again.
      return it->second;
    }

    absl::StatusOr<std::string> new_id;
    if (auto bitmap_it = family_proto.texture_id_to_bitmap().find(old_id);
        bitmap_it != family_proto.texture_id_to_bitmap().end()) {
      new_id = get_client_texture_id(old_id, bitmap_it->second);
    } else {
      new_id = get_client_texture_id(old_id, std::string());
    }
    if (!new_id.ok()) {
      return new_id.status();
    }
    old_to_new_id.insert({old_id, *new_id});
    return *new_id;
  };

  absl::StatusOr<std::vector<BrushCoat>> coats =
      DecodeBrushFamilyCoats(family_proto, texture_callback);
  if (!coats.ok()) {
    return coats.status();
  }
  absl::StatusOr<BrushFamily::InputModel> input_model =
      BrushFamily::SpringModel{};
  if (family_proto.has_input_model()) {
    input_model = DecodeBrushFamilyInputModel(family_proto.input_model());
  }
  if (!input_model.ok()) {
    return input_model.status();
  }
  // BrushFamily::Create() validates the BrushFamily.
  return BrushFamily::Create(absl::MakeConstSpan(*coats),
                             family_proto.client_brush_family_id(),
                             *input_model);
}

void EncodeBrush(const Brush& brush, proto::Brush& brush_proto_out,
                 TextureBitmapProvider get_bitmap) {
  EncodeColor(brush.GetColor(), *brush_proto_out.mutable_color());
  brush_proto_out.set_size_stroke_space(brush.GetSize());
  brush_proto_out.set_epsilon_stroke_space(brush.GetEpsilon());
  EncodeBrushFamily(brush.GetFamily(), *brush_proto_out.mutable_brush_family(),
                    get_bitmap);
}

absl::StatusOr<Brush> DecodeBrush(
    const proto::Brush& brush_proto,
    ClientTextureIdProviderAndBitmapReceiver get_client_texture_id) {
  absl::StatusOr<BrushFamily> brush_family =
      DecodeBrushFamily(brush_proto.brush_family(), get_client_texture_id);
  if (!brush_family.ok()) {
    return brush_family.status();
  }
  // Brush::Create() validates the brush.
  return Brush::Create(
      *std::move(brush_family), DecodeColor(brush_proto.color()),
      brush_proto.size_stroke_space(), brush_proto.epsilon_stroke_space());
}

}  // namespace ink
