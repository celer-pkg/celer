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

#include "ink/brush/easing_function.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <variant>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "ink/geometry/point.h"

namespace ink {

bool EasingFunction::operator==(const EasingFunction& other) const {
  return parameters == other.parameters;
}

bool EasingFunction::operator!=(const EasingFunction& other) const {
  return !(*this == other);
}

bool EasingFunction::CubicBezier::operator==(const CubicBezier& other) const {
  return x1 == other.x1 && y1 == other.y1 && x2 == other.x2 && y2 == other.y2;
}

bool EasingFunction::CubicBezier::operator!=(
    const EasingFunction::CubicBezier& other) const {
  return !(*this == other);
}

bool EasingFunction::Linear::operator==(const Linear& other) const {
  return points == other.points;
}

bool EasingFunction::Linear::operator!=(
    const EasingFunction::Linear& other) const {
  return !(*this == other);
}

bool EasingFunction::Steps::operator==(const Steps& other) const {
  return step_count == other.step_count && step_position == other.step_position;
}

bool EasingFunction::Steps::operator!=(
    const EasingFunction::Steps& other) const {
  return !(*this == other);
}

namespace brush_internal {
namespace {

bool IsValidPredefinedEasingFunction(EasingFunction::Predefined predefined) {
  switch (predefined) {
    case EasingFunction::Predefined::kLinear:
    case EasingFunction::Predefined::kEase:
    case EasingFunction::Predefined::kEaseIn:
    case EasingFunction::Predefined::kEaseOut:
    case EasingFunction::Predefined::kEaseInOut:
    case EasingFunction::Predefined::kStepStart:
    case EasingFunction::Predefined::kStepEnd:
      return true;
  }
  return false;
}

absl::Status ValidateEasingFunctionParameters(
    EasingFunction::Predefined params) {
  if (!IsValidPredefinedEasingFunction(params)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`EasingFunction::parameters` with type `Predefined` "
                        "holds non-enumerator value %d",
                        static_cast<int>(params)));
  }
  return absl::OkStatus();
}

absl::Status ValidateEasingFunctionParameters(
    const EasingFunction::CubicBezier& params) {
  if (!(std::isfinite(params.x1) && std::isfinite(params.y1) &&
        std::isfinite(params.x2) && std::isfinite(params.y2)) ||
      params.x1 < 0 || params.x1 > 1 || params.x2 < 0 || params.x2 > 1) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`EasingFunction::parameters` with type `CubicBezier` holds "
        "invalid values. All values must be finite and all x values "
        "must be in the interval [0, 1], got: x1: %f, y1: %f, x2: %f,"
        "y2: %f",
        params.x1, params.y1, params.x2, params.y2));
  }
  return absl::OkStatus();
}

absl::Status ValidateEasingFunctionParameters(
    const EasingFunction::Linear& params) {
  for (Point point : params.points) {
    if (!std::isfinite(point.x) || point.x < 0.f || point.x > 1.f) {
      return absl::InvalidArgumentError(
          absl::StrFormat("EasingFunction::Linear::points must have "
                          "x-positions in [0, 1], but found x=%f",
                          point.x));
    }
    if (!std::isfinite(point.y)) {
      return absl::InvalidArgumentError(
          absl::StrFormat("EasingFunction::Linear::points must have finite "
                          "y-positions, but found y=%f",
                          point.y));
    }
  }
  for (size_t i = 1; i < params.points.size(); ++i) {
    float prev_x = params.points[i - 1].x;
    float this_x = params.points[i].x;
    if (prev_x > this_x) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "EasingFunction::Linear::points must have monotonically "
          "non-decreasing x-positions, but found x=%f before x=%f",
          prev_x, this_x));
    }
  }
  return absl::OkStatus();
}

bool IsValidStepPosition(EasingFunction::StepPosition step_position) {
  switch (step_position) {
    case EasingFunction::StepPosition::kJumpEnd:
    case EasingFunction::StepPosition::kJumpStart:
    case EasingFunction::StepPosition::kJumpNone:
    case EasingFunction::StepPosition::kJumpBoth:
      return true;
  }
  return false;
}

absl::Status ValidateEasingFunctionParameters(
    const EasingFunction::Steps& steps) {
  if (!IsValidStepPosition(steps.step_position)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("`EasingFunction::parameters` with type `Steps` holds "
                        "invalid values. The `step_position` must be a valid "
                        "enumerator value, got: step_position: %d",
                        static_cast<int>(steps.step_position)));
  }
  if (steps.step_position == EasingFunction::StepPosition::kJumpNone &&
      steps.step_count < 2) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`EasingFunction::parameters` with type `Steps` holds "
        "invalid values. The value for `step_count` must be greater than 1 if "
        "`step_position` kJumpNone is selected, got: step_count: %d",
        steps.step_count));
  }
  if (steps.step_count < 1) {
    return absl::InvalidArgumentError(absl::StrFormat(
        "`EasingFunction::parameters` with type `Steps` holds "
        "invalid values. The value for `step_count` must be greater "
        "than 0, got: step_count: %d",
        steps.step_count));
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status ValidateEasingFunction(const EasingFunction& easing_function) {
  return std::visit(
      [](auto&& params) { return ValidateEasingFunctionParameters(params); },
      easing_function.parameters);
}

std::string ToFormattedString(const EasingFunction& easing_function) {
  return ToFormattedString(easing_function.parameters);
}

std::string ToFormattedString(EasingFunction::Predefined predefined) {
  switch (predefined) {
    case EasingFunction::Predefined::kLinear:
      return "kLinear";
    case EasingFunction::Predefined::kEase:
      return "kEase";
    case EasingFunction::Predefined::kEaseIn:
      return "kEaseIn";
    case EasingFunction::Predefined::kEaseOut:
      return "kEaseOut";
    case EasingFunction::Predefined::kEaseInOut:
      return "kEaseInOut";
    case ink::EasingFunction::Predefined::kStepStart:
      return "kStepStart";
    case ink::EasingFunction::Predefined::kStepEnd:
      return "kStepEnd";
  }
  return absl::StrCat("Predefined(", static_cast<int>(predefined), ")");
}

std::string ToFormattedString(const EasingFunction::CubicBezier& cubic_bezier) {
  return absl::StrCat("CubicBezier{", cubic_bezier.x1, ", ", cubic_bezier.y1,
                      ", ", cubic_bezier.x2, ", ", cubic_bezier.y2, "}");
}

std::string ToFormattedString(const EasingFunction::Linear& linear) {
  return absl::StrCat("Linear{", absl::StrJoin(linear.points, ", "), "}");
}

std::string ToFormattedString(EasingFunction::StepPosition step_position) {
  switch (step_position) {
    case EasingFunction::StepPosition::kJumpEnd:
      return "kJumpEnd";
    case EasingFunction::StepPosition::kJumpStart:
      return "kJumpStart";
    case EasingFunction::StepPosition::kJumpNone:
      return "kJumpNone";
    case EasingFunction::StepPosition::kJumpBoth:
      return "kJumpBoth";
  }
  return absl::StrCat("StepPosition(", static_cast<int>(step_position), ")");
}

std::string ToFormattedString(const EasingFunction::Steps& steps) {
  return absl::StrCat("Steps{", steps.step_count, ", ",
                      ToFormattedString(steps.step_position), "}");
}

std::string ToFormattedString(const EasingFunction::Parameters& parameters) {
  return std::visit([](const auto& arg) { return ToFormattedString(arg); },
                    parameters);
}

}  // namespace brush_internal
}  // namespace ink
