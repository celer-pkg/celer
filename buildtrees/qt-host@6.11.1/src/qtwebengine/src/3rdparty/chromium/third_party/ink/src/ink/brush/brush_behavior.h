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

#ifndef INK_STROKES_BRUSH_BRUSH_BEHAVIOR_H_
#define INK_STROKES_BRUSH_BRUSH_BEHAVIOR_H_

#include <array>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "ink/brush/easing_function.h"

namespace ink {

// A behavior describing how stroke input properties should affect the shape and
// color of the brush tip.
//
// The behavior is conceptually a graph made from the various node types defined
// below. Each edge of the graph represents passing a nullable floating point
// value between nodes, and each node in the graph fits into one of the
// following categories:
//   1. Leaf nodes generate an output value without graph inputs. For example,
//      they can create a value from properties of stroke input.
//   2. Filter nodes can conditionally toggle branches of the graph "on" by
//      outputting their input value, or "off" by outputting a null value.
//   3. Operator nodes take in one or more input values and generate an output.
//      For example, by mapping input to output with an easing function.
//   4. Terminal nodes apply one or more input values to chosen properties of
//      the brush tip.
//
// The behavior is specified by a single list of nodes that represents a
// flattened, post-order traversal of the graph. The simplest form of behavior
// consists of two nodes:
//
//                     +--------+      +--------+
//                     | Source | ---> | Target |
//                     +--------+      +--------+
//
// This behavior would be represented by the list {Source, Target}. An example
// of such a behavior defined in C++ would be:
//
//    BrushBeahvior behavior = {
//        .nodes = {
//            BrushBehavior::SourceNode{
//                .source = BrushBehavior::Source::kPressure,
//                .source_value_range = {0.2, 0.8},
//            },
//            BrushBehavior::TargetNode{
//                .target = BrushBehavior::Target::kSizeMultiplier,
//                .target_modifier_range = {0.75, 1.25},
//            },
//        }};
//
// A more complex behavior could use two source values for a single target:
//
//             +----------+      +-----+
//             | Source 1 | ---> |     |
//             +----------+      |     |      +--------+
//                               | Max | ---> | Target |
//             +----------+      |     |      +--------+
//             | Source 2 | ---> |     |
//             +----------+      +-----+
//
// This could be represented by the list {Source 1, Source 2, Max, Target}.
//
// For each input in a stroke, `BrushTip::behaviors` are applied as follows:
//   1. A target modifier for each tip property is accumulated from every
//     `BrushBehavior` present on the current `BrushTip`:
//        * Multiple behaviors can affect the same `Target`.
//        * Depending on the `Target`, modifiers from multiple behaviors will
//          stack either additively or multiplicatively, according to the
//          descriptions on that `BrushBehavior::Target`.
//        * Regardless, the order of specified behaviors does not affect the
//          result.
//   2. The modifiers are applied to the shape and color shift values of the
//      tip's state according to the descriptions on each `Target`. The
//      resulting tip property values are then clamped or normalized to within
//      their valid range of values. E.g. the final value of
//      `BrushTip::corner_rounding` will be clamped within [0, 1].
//      Generally:
//        * The affected shape values are those found in `BrushTip` members.
//        * The color shift values remain in the range -100% to +100%. Note that
//          when stored on a vertex, the color shift is encoded such that each
//          channel is in the range [0, 1], where 0.5 represents a 0% shift.
//
// Note that the accumulated tip shape property modifiers may be adjusted by the
// implementation before being applied: The rates of change of shape properties
// may be constrained to keep them from changing too rapidly with respect to
// distance traveled from one input to the next.
struct BrushBehavior {
  // List of input properties along with their units that can act as sources for
  // a `BrushBehavior`.
  //
  // This should match the enum in BrushBehavior.kt and
  // BrushFamilyExtensions.kt.
  enum class Source : int8_t {
    // Stylus or touch pressure with values reported in the range [0, 1].
    kNormalizedPressure,
    // Stylus tilt with values reported in the range [0, π/2] radians.
    kTiltInRadians,
    // Stylus tilt along the x and y axes in the range [-π/2, π/2], with a
    // positive value corresponding to tilt toward the respective positive axis.
    // In order for those values to be reported, both tilt and orientation have
    // to be populated on the StrokeInput.
    kTiltXInRadians,
    kTiltYInRadians,
    // Stylus orientation with values reported in the range [0, 2π).
    kOrientationInRadians,
    // Stylus orientation with values reported in the range (-π, π].
    kOrientationAboutZeroInRadians,
    // Pointer speed with values >= 0 in distance units per second, where one
    // distance unit is equal to the brush size.
    kSpeedInMultiplesOfBrushSizePerSecond,
    // Signed x and y components of pointer velocity in distance units per
    // second, where one distance unit is equal to the brush size.
    kVelocityXInMultiplesOfBrushSizePerSecond,
    kVelocityYInMultiplesOfBrushSizePerSecond,
    // The angle of the stroke's current direction of travel in stroke space,
    // normalized to the range [0, 2π). A value of 0 indicates the direction of
    // the positive X-axis in stroke space; a value of π/2 indicates the
    // direction of the positive Y-axis in stroke space.
    kDirectionInRadians,
    // The angle of the stroke's current direction of travel in stroke space,
    // normalized to the range (-π, π]. A value of 0 indicates the direction of
    // the positive X-axis in stroke space; a value of π/2 indicates the
    // direction of the positive Y-axis in stroke space.
    kDirectionAboutZeroInRadians,
    // Signed x and y components of the normalized travel direction, with values
    // in the range [-1, 1].
    kNormalizedDirectionX,
    kNormalizedDirectionY,
    // Distance traveled by the inputs of the current stroke, starting at 0 at
    // the first input, where one distance unit is equal to the brush size.
    kDistanceTraveledInMultiplesOfBrushSize,
    // The time elapsed from when the stroke started to when this part of the
    // stroke was drawn. The value remains fixed for any given part of the
    // stroke once drawn.
    kTimeOfInputInSeconds,
    kTimeOfInputInMillis,
    // Distance traveled by the inputs of the current prediction, starting at 0
    // at the last non-predicted input, where one distance unit is equal to the
    // brush size. For cases where prediction hasn't started yet, we don't
    // return a negative value, but clamp to a min of 0.
    kPredictedDistanceTraveledInMultiplesOfBrushSize,
    // Elapsed time of the prediction, starting at 0 at the last non-predicted
    // input. For cases where prediction hasn't started yet, we don't
    // return a negative value, but clamp to a min of 0.
    kPredictedTimeElapsedInSeconds,
    kPredictedTimeElapsedInMillis,
    // The distance left to be traveled from a given input to the current last
    // input of the stroke, where one distance unit is equal to the brush
    // size. This value changes for each input as the stroke is drawn.
    kDistanceRemainingInMultiplesOfBrushSize,
    // The amount of time that has elapsed since this part of the stroke was
    // drawn. This continues to increase even after all stroke inputs have
    // completed, and can be used to drive stroke animations. These enumerators
    // are only compatible with a `source_out_of_range_behavior` of `kClamp`, to
    // ensure that the animation will eventually end.
    kTimeSinceInputInSeconds,
    kTimeSinceInputInMillis,
    // Directionless pointer acceleration with values >= 0 in distance units per
    // second squared, where one distance unit is equal to the brush size.
    kAccelerationInMultiplesOfBrushSizePerSecondSquared,
    // Signed x and y components of pointer acceleration in distance units per
    // second squared, where one distance unit is equal to the brush size.
    kAccelerationXInMultiplesOfBrushSizePerSecondSquared,
    kAccelerationYInMultiplesOfBrushSizePerSecondSquared,
    // Pointer acceleration along the current direction of travel in distance
    // units per second squared, where one distance unit is equal to the brush
    // size. A positive value indicates that the pointer is accelerating along
    // the current direction of travel, while a negative value indicates that
    // the pointer is decelerating.
    kAccelerationForwardInMultiplesOfBrushSizePerSecondSquared,
    // Pointer acceleration perpendicular to the current direction of travel in
    // distance units per second squared, where one distance unit is equal to
    // the brush size. If the X- and Y-axes of stroke space were rotated so that
    // the positive X-axis points in the direction of stroke travel, then a
    // positive value for this source indicates acceleration along the positive
    // Y-axis (and a negative value indicates acceleration along the negative
    // Y-axis).
    kAccelerationLateralInMultiplesOfBrushSizePerSecondSquared,
    // The physical speed of the input pointer at the point in question, in
    // centimeters per second.
    kInputSpeedInCentimetersPerSecond,
    // Signed x and y components of the physical velocity of the input pointer
    // at the point in question, in centimeters per second.
    kInputVelocityXInCentimetersPerSecond,
    kInputVelocityYInCentimetersPerSecond,
    // The physical distance traveled by the input pointer from the start of the
    // stroke along the input path to the point in question, in centimeters.
    kInputDistanceTraveledInCentimeters,
    // The physical distance that the input pointer would have to travel from
    // its actual last real position along its predicted path to reach the
    // predicted point in question, in centimeters. For points on the stroke
    // before the predicted portion, this has a value of zero.
    kPredictedInputDistanceTraveledInCentimeters,
    // The directionless physical acceleration of the input pointer at the point
    // in question, with values >= 0, in centimeters per second squared.
    kInputAccelerationInCentimetersPerSecondSquared,
    // Signed x and y components of the physical acceleration of the input
    // pointer, in centimeters per second squared.
    kInputAccelerationXInCentimetersPerSecondSquared,
    kInputAccelerationYInCentimetersPerSecondSquared,
    // The physical acceleration of the input pointer along its current
    // direction of travel at the point in question, in centimeters per second
    // squared. A positive value indicates that the pointer is accelerating
    // along the current direction of travel, while a negative value indicates
    // that the pointer is decelerating.
    kInputAccelerationForwardInCentimetersPerSecondSquared,
    // The physical acceleration of the input pointer perpendicular to its
    // current direction of travel at the point in question, in centimeters per
    // second squared. If the X- and Y-axes of stroke space were rotated so that
    // the positive X-axis points in the direction of stroke travel, then a
    // positive value for this source indicates acceleration along the positive
    // Y-axis (and a negative value indicates acceleration along the negative
    // Y-axis).
    kInputAccelerationLateralInCentimetersPerSecondSquared,
    // The distance left to be traveled from a given input to the current last
    // input of the stroke, as a fraction of the current total length of the
    // stroke. This value changes for each input as the stroke is drawn.
    kDistanceRemainingAsFractionOfStrokeLength,
    // TODO: b/336565152 - Add kInputDistanceRemainingInCentimeters (this will
    // require some refactoring for the code that calculates
    // BrushTipModeler::distance_remaining_behavior_upper_bound_).
  };

  // List of tip properties that can be modified by a `BrushBehavior`.
  //
  // This should match the enums in BrushBehavior.kt and
  // BrushFamilyExtensions.kt.
  enum class Target : int8_t {
    // `kWidthMultiplier` and `kHeightMultiplier` scale the brush-tip size along
    // one dimension, starting from the values calculated using
    // `BrushTip::scale`, while `kSizeMultiplier` is a convenience target that
    // affects both width and height at once. The final brush size is clamped to
    // a maximum of twice the base size along each dimension. If multiple
    // behaviors have one of these targets, they stack multiplicatively (thus
    // allowing one behavior to scale the size down to zero over time, "winning"
    // over all other size-modifying behaviors).
    kWidthMultiplier,
    kHeightMultiplier,
    kSizeMultiplier,
    // Adds the target modifier to `BrushTip::slant`. The final brush slant
    // value is clamped to [-π/2, π/2]. If multiple behaviors have this target,
    // they stack additively.
    kSlantOffsetInRadians,
    // Adds the target modifier to `BrushTip::pinch`. The final brush pinch
    // value is clamped to [0, 1]. If multiple behaviors have this target, they
    // stack additively.
    kPinchOffset,
    // Adds the target modifier to `BrushTip::rotation`. The final brush
    // rotation angle is effectively normalized (mod 2π). If multiple behaviors
    // have this target, they stack additively.
    kRotationOffsetInRadians,
    // Adds the target modifier to `BrushTip::corner_rounding`. The final brush
    // corner rounding value is clamped to [0, 1]. If multiple behaviors have
    // this target, they stack additively.
    kCornerRoundingOffset,
    // Adds the target modifier to the brush tip x/y position, where one
    // distance unit is equal to the brush size.
    kPositionOffsetXInMultiplesOfBrushSize,
    kPositionOffsetYInMultiplesOfBrushSize,
    // Moves the brush tip center forward (or backward, for negative values)
    // from the input position, in the current direction of stroke travel, where
    // one distance unit is equal to the brush size.
    kPositionOffsetForwardInMultiplesOfBrushSize,
    // Moves the brush tip center sideways from the input position, relative to
    // the direction of stroke travel, where one distance unit is equal to the
    // brush size. If the X- and Y-axes of stroke space were rotated so that the
    // positive X-axis points in the direction of stroke travel, then a positive
    // value for this offset moves the brush tip center towards the positive
    // Y-axis (and a negative value moves the brush tip center towards the
    // negative Y-axis).
    kPositionOffsetLateralInMultiplesOfBrushSize,
    // Adds the target modifier to the initial texture animation progress value
    // of the current particle (which is relevant only for strokes with an
    // animated texture). The final progress offset is not clamped, but is
    // effectively normalized (mod 1). If multiple behaviors have this target,
    // they stack additively.
    kTextureAnimationProgressOffset,

    // The following are targets for tip color adjustments, including opacity.
    // Renderers can apply them to the brush color when a stroke is drawn to
    // contribute to the local color of each part of the stroke.
    //
    // TODO: b/344839538 - Rename and re-document kLuminosity once we decide how
    // that target should stack.

    // Shifts the hue of the base brush color.  A positive offset shifts around
    // the hue wheel from red towards orange, while a negative offset shifts the
    // other way, from red towards violet. The final hue offset is not clamped,
    // but is effectively normalized (mod 2π). If multiple behaviors have this
    // target, they stack additively.
    kHueOffsetInRadians,
    // Scales the saturation of the base brush color.  If multiple behaviors
    // have one of these targets, they stack multiplicatively.  The final
    // saturation multiplier is clamped to [0, 2].
    kSaturationMultiplier,
    // Target the luminosity of the color. An offset of +/-100% corresponds to
    // changing the luminosity by up to +/-100%.
    kLuminosity,
    // Scales the opacity of the base brush color.  If multiple behaviors have
    // one of these targets, they stack multiplicatively.  The final opacity
    // multiplier is clamped to [0, 2].
    kOpacityMultiplier,
  };

  // List of vector tip properties that can be modified by a `BrushBehavior`.
  //
  // This should match the enums in BrushBehavior.kt and
  // BrushFamilyExtensions.kt.
  enum class PolarTarget : int8_t {
    // Adds the vector to the brush tip's absolute x/y position in stroke space,
    // where the angle input is measured in radians and the magnitude input is
    // measured in units equal to the brush size. An angle of zero indicates an
    // offset in the direction of the positive X-axis in stroke space; an angle
    // of π/2 indicates the direction of the positive Y-axis in stroke space.
    kPositionOffsetAbsoluteInRadiansAndMultiplesOfBrushSize,
    // Adds the vector to the brush tip's forward/lateral position relative to
    // the current direction of input travel, where the angle input is measured
    // in radians and the magnitude input is measured in units equal to the
    // brush size. An angle of zero indicates a forward offset in the current
    // direction of input travel, while an angle of π indicates a backwards
    // offset. Meanwhile, if the X- and Y-axes of stroke space were rotated so
    // that the positive X-axis points in the direction of stroke travel, then
    // an angle of π/2 would indicate a lateral offset towards the positive
    // Y-axis, and an angle of -π/2 would indicate a lateral offset towards the
    // negative Y-axis.
    kPositionOffsetRelativeInRadiansAndMultiplesOfBrushSize,
  };

  // The desired behavior when an input value is outside the bounds of
  // `source_value_range`.
  //
  // This should match the enum in BrushBehavior.kt and
  // BrushFamilyExtensions.kt.
  enum class OutOfRange : int8_t {
    // Values outside the range will be clamped to not exceed the bounds.
    kClamp,
    // Values will be shifted by an integer multiple of the range size so that
    // they fall within the bounds.
    //
    // In this case, the range will be treated as a half-open interval, with a
    // value exactly at `source_value_range[1]` being treated as though it was
    // `source_value_range[0]`.
    kRepeat,
    // Similar to `kRepeat`, but every other repetition of the bounds will be
    // mirrored, as though the two elements of `source_value_range` were
    // swapped. This means the range does not need to be treated as a half-open
    // interval like in the case of `kRepeat`.
    kMirror,
  };

  // Flags allowing behaviors to be active for a limited subset of tool types.
  struct EnabledToolTypes {
    bool unknown = false;
    bool mouse = false;
    bool touch = false;
    bool stylus = false;

    bool HasAnyTypes() const;
    bool HasAllTypes() const;
  };

  static constexpr EnabledToolTypes kAllToolTypes = {
      .unknown = true, .mouse = true, .touch = true, .stylus = true};

  // List of input properties that might not be reported by `StrokeInput`.
  //
  // This should match the enums in BrushBehavior.kt and
  // BrushFamilyExtensions.kt.
  enum OptionalInputProperty : int8_t {
    kPressure,
    kTilt,
    kOrientation,
    // Tilt-x and tilt-y require both tilt and orientation to be reported.
    kTiltXAndY,
  };

  // A binary operation for combining two values in a `BinaryOpNode`.
  // LINT.IfChange(binary_op)
  enum class BinaryOp {
    kProduct,  // A * B, or null if either is null
    kSum,      // A + B, or null if either is null
  };
  // LINT.ThenChange(
  //   fuzz_domains.cc:binary_op,
  //   ../storage/proto/brush_family.proto:binary_op,
  // )

  // Dimensions/units for measuring the `damping_gap` field of a
  // `DampingNode`.
  // LINT.IfChange(damping_source)
  enum class DampingSource {
    // Value damping occurs over distance traveled by the input pointer, and the
    // `damping_gap` is measured in centimeters. If the input data does not
    // indicate the relationship between stroke units and physical units
    // (e.g. as may be the case for programmatically-generated inputs), then no
    // damping will be performed (i.e. the `damping_gap` will be treated as
    // zero).
    kDistanceInCentimeters,
    // Value damping occurs over distance traveled by the input pointer, and the
    // `damping_gap` is measured in multiples of the brush size.
    kDistanceInMultiplesOfBrushSize,
    // Value damping occurs over time, and the `damping_gap` is measured in
    // seconds.
    kTimeInSeconds,
  };
  // LINT.ThenChange(
  //   fuzz_domains.cc:damping_source,
  //   ../storage/proto/brush_family.proto:damping_source,
  // )

  // An interpolation function for combining three values in an
  // `InterpolationNode`.
  // LINT.IfChange(interpolation)
  enum class Interpolation {
    // Linear interpolation. Uses parameter A to interpolate between B (when
    // A=0) and C (when A=1).
    kLerp,
    // Inverse linear interpolation. Outputs 0 when A=B and 1 when A=C,
    // interpolating linearly in between. Outputs null if B=C.
    kInverseLerp,
  };
  // LINT.ThenChange(
  //   fuzz_domains.cc:interpolation,
  //   ../storage/proto/brush_family.proto:interpolation,
  // )

  ////////////////////////
  /// LEAF VALUE NODES ///
  ////////////////////////

  // Value node for getting data from the stroke input batch.
  // Inputs: 0
  // Output: The value of the source after inverse-lerping from the specified
  //     value range and applying the specified out-of-range behavior, or null
  //     if the source value is indeterminate (e.g. because the stroke input
  //     batch is missing that property).
  // To be valid:
  //   - `source` must be a valid `Source` enumerator.
  //   - `source_out_of_range_behavior` must be a valid `OutOfRange` enumerator.
  //   - The endpoints of `source_value_range` must be finite and distinct.
  struct SourceNode {
    Source source;
    OutOfRange source_out_of_range_behavior = OutOfRange::kClamp;
    std::array<float, 2> source_value_range;
  };

  // Value node for producing a constant value.
  // Inputs: 0
  // Output: The specified constant value.
  // To be valid: `value` must be finite.
  struct ConstantNode {
    float value;
  };

  // Value node for producing a continuous random noise function with values
  // between 0 to 1.
  // Inputs: 0
  // Output: The current random value.
  // To be valid:
  //   - `vary_over` must be a valid `DampingSource` enumerator.
  //   - `base_period` must be finite and strictly positive.
  struct NoiseNode {
    uint32_t seed;
    DampingSource vary_over;
    float base_period;
  };

  //////////////////////////
  /// FILTER VALUE NODES ///
  //////////////////////////

  // Value node for filtering out a branch of a behavior graph unless a
  // particular stroke input property is missing.
  // Inputs: 1
  // Output: Null if the specified property is present in the stroke input
  //     batch, otherwise the input value.
  // To be valid:
  //   - `is_fallback_for` must be a valid `OptionalInputProperty` enumerator.
  struct FallbackFilterNode {
    OptionalInputProperty is_fallback_for;
  };

  // Value node for filtering out a branch of a behavior graph unless this
  // stroke's tool type is in the specified set.
  // Inputs: 1
  // Output: Null if this stroke's tool type is not in the specified set,
  //     otherwise the input value.
  // To be valid: At least one tool type must be enabled.
  struct ToolTypeFilterNode {
    EnabledToolTypes enabled_tool_types;
  };

  ////////////////////////////
  /// OPERATOR VALUE NODES ///
  ////////////////////////////

  // Value node for damping changes in an input value, causing the output value
  // to slowly follow changes in the input value over a specified time or
  // distance.
  // Inputs: 1
  // Output: The damped input value. If the input value becomes null, this node
  //     continues to emit its previous output value.  If the input value starts
  //     out null, the output value is null until the first non-null input.
  // To be valid:
  //   - `damping_source` must be a valid `DampingSource` enumerator.
  //   - `damping_gap` must be finite and non-negative.
  struct DampingNode {
    DampingSource damping_source;
    float damping_gap;
  };

  // Value node for mapping a value through a response curve.
  // Inputs: 1

  // Output: The result of the easing function when applied to the input value,
  //     or null if the input value is null.
  // To be valid: `function` must be a valid `EasingFunction`.
  struct ResponseNode {
    EasingFunction response_curve;
  };

  // Value node for combining two other values with a binary operation.
  // Inputs: 2
  // Output: The result of the specified operation on the two input values. See
  //     comments on `BinaryOp` for details on how each operator handles null
  //     input values.
  // To be valid: `operation` must be a valid `BinaryOp` enumerator.
  struct BinaryOpNode {
    BinaryOp operation;
  };

  // Value node for interpolating to/from a range of two values.
  // Inputs: 3
  // Output: The result of using the first input value as an interpolation
  //     parameter between the second and third input values, using the
  //     specified interpolation function, or null if any input value is null.
  // To be valid: `interpolation` must be a valid `Interpolation` enumerator.
  struct InterpolationNode {
    Interpolation interpolation;
  };

  //////////////////////
  /// TERMINAL NODES ///
  //////////////////////

  // Terminal node that consumes a single input value to modify a scalar brush
  // tip property.
  // Inputs: 1
  // Effect: Applies a modifier to the specified target equal to the input value
  //     lerped to the specified range. If the input becomes null, the target
  //     continues to apply its previous effect from the most recent non-null
  //     input (if any).
  // To be valid:
  //   - `target` must be a valid `Target` enumerator.
  //   - The endpoints of `target_modifier_range` must be finite and distinct.
  struct TargetNode {
    Target target;
    std::array<float, 2> target_modifier_range;
  };

  // Terminal node that consumes two input values (angle and magnitude), forming
  // a polar vector to modify a vector brush tip property.
  // Inputs: 2
  // Effect: Applies a vector modifier to the specified target equal to the
  //     polar vector formed by lerping the first input value to the specified
  //     angle range, and the second input to the specified magnitude range. If
  //     either input becomes null, the target continues to apply its previous
  //     effect from the most recent non-null inputs (if any).
  // To be valid:
  //   - `target` must be a valid `PolarTarget` enumerator.
  //   - The endpoints of `angle_range` and of `magnitude_range` must be finite
  //     and distinct.
  struct PolarTargetNode {
    PolarTarget target;
    std::array<float, 2> angle_range;
    std::array<float, 2> magnitude_range;
  };

  // A single node in a behavior's graph.  Each node type is either a "value
  // node" which consumes zero or more input values and produces a single output
  // value, or a "terminal node" which consumes one or more input values and
  // applies some effect to the brush tip (but does not produce any output
  // value).
  using Node =
      std::variant<SourceNode, ConstantNode, NoiseNode, FallbackFilterNode,
                   ToolTypeFilterNode, DampingNode, ResponseNode, BinaryOpNode,
                   InterpolationNode, TargetNode, PolarTargetNode>;

  std::vector<Node> nodes;
};

bool operator==(const BrushBehavior::EnabledToolTypes& lhs,
                const BrushBehavior::EnabledToolTypes& rhs);
bool operator!=(const BrushBehavior::EnabledToolTypes& lhs,
                const BrushBehavior::EnabledToolTypes& rhs);

bool operator==(const BrushBehavior::SourceNode& lhs,
                const BrushBehavior::SourceNode& rhs);
bool operator!=(const BrushBehavior::SourceNode& lhs,
                const BrushBehavior::SourceNode& rhs);

bool operator==(const BrushBehavior::ConstantNode& lhs,
                const BrushBehavior::ConstantNode& rhs);
bool operator!=(const BrushBehavior::ConstantNode& lhs,
                const BrushBehavior::ConstantNode& rhs);

bool operator==(const BrushBehavior::NoiseNode& lhs,
                const BrushBehavior::NoiseNode& rhs);
bool operator!=(const BrushBehavior::NoiseNode& lhs,
                const BrushBehavior::NoiseNode& rhs);

bool operator==(const BrushBehavior::FallbackFilterNode& lhs,
                const BrushBehavior::FallbackFilterNode& rhs);
bool operator!=(const BrushBehavior::FallbackFilterNode& lhs,
                const BrushBehavior::FallbackFilterNode& rhs);

bool operator==(const BrushBehavior::ToolTypeFilterNode& lhs,
                const BrushBehavior::ToolTypeFilterNode& rhs);
bool operator!=(const BrushBehavior::ToolTypeFilterNode& lhs,
                const BrushBehavior::ToolTypeFilterNode& rhs);

bool operator==(const BrushBehavior::DampingNode& lhs,
                const BrushBehavior::DampingNode& rhs);
bool operator!=(const BrushBehavior::DampingNode& lhs,
                const BrushBehavior::DampingNode& rhs);

bool operator==(const BrushBehavior::ResponseNode& lhs,
                const BrushBehavior::ResponseNode& rhs);
bool operator!=(const BrushBehavior::ResponseNode& lhs,
                const BrushBehavior::ResponseNode& rhs);

bool operator==(const BrushBehavior::BinaryOpNode& lhs,
                const BrushBehavior::BinaryOpNode& rhs);
bool operator!=(const BrushBehavior::BinaryOpNode& lhs,
                const BrushBehavior::BinaryOpNode& rhs);

bool operator==(const BrushBehavior::InterpolationNode& lhs,
                const BrushBehavior::InterpolationNode& rhs);
bool operator!=(const BrushBehavior::InterpolationNode& lhs,
                const BrushBehavior::InterpolationNode& rhs);

bool operator==(const BrushBehavior::TargetNode& lhs,
                const BrushBehavior::TargetNode& rhs);
bool operator!=(const BrushBehavior::TargetNode& lhs,
                const BrushBehavior::TargetNode& rhs);

bool operator==(const BrushBehavior::PolarTargetNode& lhs,
                const BrushBehavior::PolarTargetNode& rhs);
bool operator!=(const BrushBehavior::PolarTargetNode& lhs,
                const BrushBehavior::PolarTargetNode& rhs);

bool operator==(const BrushBehavior& lhs, const BrushBehavior& rhs);
bool operator!=(const BrushBehavior& lhs, const BrushBehavior& rhs);

namespace brush_internal {

// Determines whether the given BrushBehavior struct is valid to be used in a
// BrushFamily, and returns an error if not. Validates both the top-level
// structure and the individual nodes.
absl::Status ValidateBrushBehavior(const BrushBehavior& behavior);

// Validates the top-level structure of a BrushBehavior, but not the individual
// nodes. This can be used to validate a BrushBehavior if the nodes are already
// validated.
absl::Status ValidateBrushBehaviorTopLevel(const BrushBehavior& behavior);

absl::Status ValidateBrushBehaviorNode(const BrushBehavior::Node& node);

std::string ToFormattedString(BrushBehavior::Source source);
std::string ToFormattedString(BrushBehavior::Target target);
std::string ToFormattedString(BrushBehavior::PolarTarget target);
std::string ToFormattedString(BrushBehavior::OutOfRange out_of_range);
std::string ToFormattedString(BrushBehavior::EnabledToolTypes enabled);
std::string ToFormattedString(BrushBehavior::OptionalInputProperty input);
std::string ToFormattedString(BrushBehavior::BinaryOp operation);
std::string ToFormattedString(BrushBehavior::DampingSource damping_source);
std::string ToFormattedString(BrushBehavior::Interpolation interpolation);
std::string ToFormattedString(const BrushBehavior::Node& node);
std::string ToFormattedString(const BrushBehavior& behavior);

}  // namespace brush_internal

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::Source source) {
  sink.Append(brush_internal::ToFormattedString(source));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::Target target) {
  sink.Append(brush_internal::ToFormattedString(target));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::PolarTarget target) {
  sink.Append(brush_internal::ToFormattedString(target));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::OutOfRange out_of_range) {
  sink.Append(brush_internal::ToFormattedString(out_of_range));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::EnabledToolTypes enabled) {
  sink.Append(brush_internal::ToFormattedString(enabled));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::OptionalInputProperty input) {
  sink.Append(brush_internal::ToFormattedString(input));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::BinaryOp operation) {
  sink.Append(brush_internal::ToFormattedString(operation));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::DampingSource damping_source) {
  sink.Append(brush_internal::ToFormattedString(damping_source));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushBehavior::Interpolation interpolation) {
  sink.Append(brush_internal::ToFormattedString(interpolation));
}

template <typename Sink>
void AbslStringify(Sink& sink, const BrushBehavior::Node& node) {
  sink.Append(brush_internal::ToFormattedString(node));
}

template <typename Sink>
void AbslStringify(Sink& sink, const BrushBehavior& behavior) {
  sink.Append(brush_internal::ToFormattedString(behavior));
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_BEHAVIOR_H_
