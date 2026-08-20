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

#ifndef INK_STROKES_BRUSH_EASING_FUNCTION_H_
#define INK_STROKES_BRUSH_EASING_FUNCTION_H_

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "absl/status/status.h"
#include "ink/geometry/point.h"

namespace ink {

// An `EasingFunction` defines a curve between input x and output y "progress
// values" similar to the CSS easing function:
// https://www.w3.org/TR/css-easing-1/#easing-functions
//
// An easing function always passes through the (x, y) points (0, 0) and (1, 1).
// It typically acts to map x values in the [0, 1] interval to y values in
// [0, 1] by either one of the predefined or one of the parameterized curve
// types below. Depending on the type of curve, input and output values outside
// [0, 1] are possible.
//
// A default-constructed EasingFunction specifies a linear mapping (the simplest
// possible easing function).
struct EasingFunction {
  // Predefined functions. Only the named enumerators are valid for use.
  //
  // If this changes, also update platform enum in EasingFunction.kt.
  enum class Predefined {
    // The linear identity function: accepts and returns values outside [0, 1].
    kLinear,
    // Predefined cubic Bezier functions:
    // https://www.w3.org/TR/css-easing-1/#cubic-bezier-easing-functions
    // (see note on `CubicBezier` below about input values outside [0, 1])
    kEase,
    kEaseIn,
    kEaseOut,
    kEaseInOut,
    // Predefined step functions:
    // https://www.w3.org/TR/css-easing-1/#step-easing-functions
    kStepStart,
    kStepEnd,
  };

  // Parameters for a custom cubic Bezier easing function.
  //
  // A cubic Bezier is generally defined by four points, P0 - P3. In the case of
  // the easing function, P0 is defined to be the point (0, 0), and P3 is
  // defined to be the point (1, 1). The values of `x1` and `x2` are required to
  // be in the range [0, 1]. This guarantees that the resulting curve is a
  // function with respect to x and follows the CSS cubic Bezier specification:
  // https://www.w3.org/TR/css-easing-1/#cubic-bezier-easing-functions
  //
  // Valid parameters must have all finite values, and `x1` and `x2` must be in
  // the interval [0, 1].
  //
  // Input x values that are outside the interval [0, 1] will be clamped, but
  // output values will not. This is somewhat different from the w3c defined
  // cubic Bezier that allows extrapolated values outside x in [0, 1] by
  // following end-point tangents.
  // TODO: b/346774811 - Make the behavior above consistent with w3c.
  struct CubicBezier {
    float x1;
    float y1;
    float x2;
    float y2;

    bool operator==(const CubicBezier& other) const;
    bool operator!=(const CubicBezier& other) const;
  };

  // Parameters for a custom piecewise-linear easing function.
  //
  // A piecewise-linear function is defined by a sequence of points; the value
  // of the function at an x-position equal to one of those points is equal to
  // the y-position of that point, and the value of the function at an
  // x-position between two points is equal to the linear interpolation between
  // those points' y-positions. This easing function implicitly includes the
  // points (0, 0) and (1, 1), so the `points` field below need only include any
  // points between those. If `points` is empty, then this function is
  // equivalent to the predefined `kLinear` identity function.
  //
  // To be valid, all y-positions must be finite, and all x-positions must be in
  // the range [0, 1] and must be monotonicly non-decreasing. It is valid for
  // multiple points to have the same x-position, in order to create a
  // discontinuity in the function; in that case, the value of the function at
  // exactly that x-position is equal to the y-position of the last of these
  // points.
  //
  // If the input x-value is outside the interval [0, 1], the output will be
  // extrapolated from the first/last line segment.
  struct Linear {
    std::vector<Point> points;

    bool operator==(const Linear& other) const;
    bool operator!=(const Linear& other) const;
  };

  // Setting to determine the desired output value of the first and last
  // step of [0, 1) for the Steps EasingFunction, see below for more context.
  //
  // If this changes, also update platform enum in EasingFunction.kt.
  enum class StepPosition {
    // The step function "jumps" at the end of [0, 1):
    //   * for x in [0, 1/step_count)      =>     y = 0
    //   * for x in [1 - 1/step_count, 1)  =>     y = 1 - 1/step_count
    kJumpEnd,
    // The step function "jumps" at the start of [0, 1):
    //   * for x in [0, 1/step_count)      =>     y = 1/step_count
    //   * for x in [1 - 1/step_count, 1)  =>     y = 1
    kJumpStart,
    // The step function "jumps" at both the start and the end:
    //   * for x in [0, 1/step_count)      =>     y = 1/(step_count + 1)
    //   * for x in [1 - 1/step_count, 1)  =>     y = 1 - 1/(step_count + 1)
    kJumpBoth,
    // The step function does not "jump" at either boundary:
    //   * for x in [0, 1/step_count)      =>     y = 0
    //   * for x in [1 - 1/step_count, 1)  =>     y = 1
    kJumpNone,
  };

  // Parameters for a custom step easing function.
  //
  // A step function is defined by the number of equal-sized steps into which
  // the [0, 1) interval of input-x is split and the behavior at the extremes.
  // When x < 0, the output will always be 0. When x >= 1, the output will
  // always be 1. The output of the first and last steps is governed by the
  // `StepPosition`.
  //
  // The behavior and naming follows the CSS steps() specification at:
  // https://www.w3.org/TR/css-easing-1/#step-easing-functions
  struct Steps {
    // The number of steps.
    //
    // Must always be greater than 0, and must be greater than 1 if
    // `step_position` is `kJumpNone`.
    int32_t step_count;
    StepPosition step_position;

    bool operator==(const Steps& other) const;
    bool operator!=(const Steps& other) const;
  };

  // Union of possible easing function parameters.
  using Parameters = std::variant<Predefined, CubicBezier, Linear, Steps>;

  Parameters parameters = Predefined::kLinear;

  bool operator==(const EasingFunction& other) const;
  bool operator!=(const EasingFunction& other) const;
};

namespace brush_internal {

// Determines whether the given EasingFunction struct is valid to be used in a
// BrushFamily, and returns an error if not.
absl::Status ValidateEasingFunction(const EasingFunction& easing_function);

std::string ToFormattedString(const EasingFunction& easing_function);
std::string ToFormattedString(EasingFunction::Predefined predefined);
std::string ToFormattedString(const EasingFunction::CubicBezier& cubic_bezier);
std::string ToFormattedString(const EasingFunction::Linear& linear);
std::string ToFormattedString(EasingFunction::StepPosition step_position);
std::string ToFormattedString(const EasingFunction::Steps& steps);
std::string ToFormattedString(const EasingFunction::Parameters& parameters);

}  // namespace brush_internal

template <typename Sink>
void AbslStringify(Sink& sink, const EasingFunction& easing_function) {
  sink.Append(brush_internal::ToFormattedString(easing_function));
}

template <typename Sink>
void AbslStringify(Sink& sink, EasingFunction::Predefined predefined) {
  sink.Append(brush_internal::ToFormattedString(predefined));
}

template <typename Sink>
void AbslStringify(Sink& sink,
                   const EasingFunction::CubicBezier& cubic_bezier) {
  sink.Append(brush_internal::ToFormattedString(cubic_bezier));
}

template <typename Sink>
void AbslStringify(Sink& sink, const EasingFunction::Linear& linear) {
  sink.Append(brush_internal::ToFormattedString(linear));
}

template <typename Sink>
void AbslStringify(Sink& sink, EasingFunction::StepPosition step_position) {
  sink.Append(brush_internal::ToFormattedString(step_position));
}

template <typename Sink>
void AbslStringify(Sink& sink, const EasingFunction::Steps& steps) {
  sink.Append(brush_internal::ToFormattedString(steps));
}

template <typename Sink>
void AbslStringify(Sink& sink, const EasingFunction::Parameters& parameters) {
  sink.Append(brush_internal::ToFormattedString(parameters));
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_EASING_FUNCTION_H_
