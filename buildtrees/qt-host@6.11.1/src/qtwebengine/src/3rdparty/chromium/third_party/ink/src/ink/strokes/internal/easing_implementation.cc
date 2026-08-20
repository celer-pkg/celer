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

#include "ink/strokes/internal/easing_implementation.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <variant>

#include "absl/container/inlined_vector.h"
#include "absl/functional/overload.h"
#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/strings/str_cat.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink::strokes_internal {
namespace {

Vec CubicBezierValue(Vec v1, Vec v2, float t) {
  float c1 = 3 * (1 - t) * (1 - t) * t;
  float c2 = 3 * (1 - t) * t * t;
  float c3 = t * t * t;
  return c1 * v1 + c2 * v2 + Vec({c3, c3});
}

EasingFunction::CubicBezier GetAsCubicBezierParameters(
    const EasingFunction::Predefined predefined) {
  switch (predefined) {
    case EasingFunction::Predefined::kEase:
      return {.x1 = 0.25, .y1 = 0.1, .x2 = 0.25, .y2 = 1};
    case EasingFunction::Predefined::kEaseIn:
      return {.x1 = 0.42, .y1 = 0, .x2 = 1, .y2 = 1};
    case EasingFunction::Predefined::kEaseOut:
      return {.x1 = 0, .y1 = 0, .x2 = 0.58, .y2 = 1};
    case EasingFunction::Predefined::kEaseInOut:
      return {.x1 = 0.42, .y1 = 0, .x2 = 0.58, .y2 = 1};
    case EasingFunction::Predefined::kLinear:
    case EasingFunction::Predefined::kStepStart:
    case EasingFunction::Predefined::kStepEnd:
      break;
  }
  ABSL_LOG(FATAL)
      << "Should only be possible for `predefined` to be a predefined "
         "cubic Bezier. Got "
      << absl::StrCat(predefined);
}

}  // namespace

EasingImplementation::EasingImplementation(const EasingFunction& ease) {
  std::visit([this](const auto& arg) { SetUpImplementationType(arg); },
             ease.parameters);
}

void EasingImplementation::SetUpImplementationType(
    EasingFunction::Predefined predefined) {
  if (predefined == EasingFunction::Predefined::kLinear) {
    implementation_type_ = Identity{};
    return;
  }
  if (predefined == EasingFunction::Predefined::kStepEnd) {
    SetUpImplementationType(EasingFunction::Steps{
        .step_count = 1,
        .step_position = EasingFunction::StepPosition::kJumpEnd});
    return;
  }
  if (predefined == EasingFunction::Predefined::kStepStart) {
    SetUpImplementationType(EasingFunction::Steps{
        .step_count = 1,
        .step_position = EasingFunction::StepPosition::kJumpStart});
    return;
  }
  SetUpImplementationType(GetAsCubicBezierParameters(predefined));
}

void EasingImplementation::SetUpImplementationType(
    const EasingFunction::CubicBezier& cubic_bezier) {
  ABSL_DCHECK_GE(cubic_bezier.x1, 0);
  ABSL_DCHECK_LE(cubic_bezier.x1, 1);
  ABSL_DCHECK_GE(cubic_bezier.x2, 0);
  ABSL_DCHECK_LE(cubic_bezier.x2, 1);

  CubicBezierApproximation cubic_bezier_approximation;
  Vec v1 = {cubic_bezier.x1, cubic_bezier.y1};
  Vec v2 = {cubic_bezier.x2, cubic_bezier.y2};

  // Given the restrictions of an easing-function and formula of the cubic
  // Bezier as B(t), it can be seen that dBx/dt, the x component of the
  // derivative with respect to t, will have a maximum value of 3.
  //
  // This follows from the fact that all control-point x values must be in the
  // range [0, 1]. See the formula for B'(t) at
  // https://en.wikipedia.org/wiki/B%C3%A9zier_curve#Cubic_B%C3%A9zier_curves.
  constexpr float kdBxdtLimit = 3.f;
  constexpr float kTableDeltaX =
      1 / (cubic_bezier_approximation.kTableSize - 1.f);
  // Any `kTStep` smaller than `kTableDeltaX / kdBxdtLimit` should guarantee
  // that incrementing t by `kTStep` passes at most one table x value, with
  // smaller values improving accuracy.
  constexpr float kTStep = 0.5f * kTableDeltaX / kdBxdtLimit;

  float t = 0;
  Vec last_value;
  Vec value = {0, 0};
  for (int i = 0; i < cubic_bezier_approximation.kTableSize; ++i) {
    float target_x = i * kTableDeltaX;
    do {
      t += kTStep;
      last_value = value;
      value = CubicBezierValue(v1, v2, t);
    } while (value.x < target_x);

    cubic_bezier_approximation.lookup_table[i] = geometry_internal::Lerp(
        last_value.y, value.y,
        geometry_internal::InverseLerp(last_value.x, value.x, target_x));
  }
  implementation_type_ = cubic_bezier_approximation;
}

void EasingImplementation::SetUpImplementationType(
    const EasingFunction::Linear& linear) {
  Linear linear_implementation;

  // Note: (0,0) and (1,1) are implicit on `EasingFunction::Linear` and will
  // also not be explicitly added to `linear_implementation`. However these
  // implicit endpoints still count toward repeated x values below.

  // If `linear.points` contains the same x value twice in a row, the
  // `EasingFunction` has a discontinuity. If linear.points contains the same x
  // value more than twice in a row, we are only keeping the first and the last
  // as the points in between wouldn't be used regardless. For x == 0 only the
  // last point is added, for x == 1 only the first point is added.
  for (size_t i = 0; i < linear.points.size(); ++i) {
    size_t linear_implementation_size = linear_implementation.points.size();
    if (i > 0 && linear.points[i].x == 0) {
      linear_implementation.points[0] = linear.points[i];
    } else if (linear.points[i].x == 1 && linear_implementation_size >= 1 &&
               linear_implementation.points.back().x == 1.f) {
      break;
    } else if (linear_implementation_size >= 2 &&
               linear_implementation.points[linear_implementation_size - 1].x ==
                   linear.points[i].x &&
               linear_implementation.points[linear_implementation_size - 2].x ==
                   linear.points[i].x) {
      linear_implementation.points.back() = linear.points[i];
    } else {
      linear_implementation.points.push_back(linear.points[i]);
    }
  }
  implementation_type_ = std::move(linear_implementation);
}

void EasingImplementation::SetUpImplementationType(
    const EasingFunction::Steps& steps) {
  ABSL_DCHECK_GT(steps.step_count, 0);
  Steps steps_implementation = {
      .step_count = static_cast<float>(steps.step_count),
      .step_position = steps.step_position};
  switch (steps.step_position) {
    case EasingFunction::StepPosition::kJumpEnd:
      steps_implementation.step_height = 1 / steps_implementation.step_count;
      steps_implementation.starting_y = 0;
      break;
    case EasingFunction::StepPosition::kJumpStart:
      steps_implementation.step_height = 1 / steps_implementation.step_count;
      steps_implementation.starting_y = steps_implementation.step_height;
      break;
    case EasingFunction::StepPosition::kJumpNone:
      ABSL_DCHECK_GT(steps.step_count, 1);
      steps_implementation.step_height =
          1 / (steps_implementation.step_count - 1);
      steps_implementation.starting_y = 0;
      break;
    case EasingFunction::StepPosition::kJumpBoth:
      steps_implementation.step_height =
          1 / (steps_implementation.step_count + 1);
      steps_implementation.starting_y = steps_implementation.step_height;
      break;
  }
  implementation_type_ = steps_implementation;
}

float EasingImplementation::GetY(float x) const {
  return std::visit([x](const auto& arg) { return arg.GetY(x); },
                    implementation_type_);
}

float EasingImplementation::Identity::GetY(float x) const { return x; }

float EasingImplementation::CubicBezierApproximation::GetY(float x) const {
  if (std::isnan(x)) return x;
  if (x <= 0) return lookup_table.front();
  if (x >= 1) return lookup_table.back();

  x *= (kTableSize - 1);
  int index = static_cast<int>(x);
  float t = x - index;

  return geometry_internal::Lerp(lookup_table[index], lookup_table[index + 1],
                                 t);
}

float EasingImplementation::Linear::GetY(float x) const {
  if (std::isnan(x)) return x;

  // It is assumed that the EasingFunction used to create this
  // EasingImplementation has gone through validation, and therefore that the
  // x-positions of `points` are monotonically non-decreasing.  Therefore,
  // `std::upper_bound` will return an iterator to the first point whose
  // x-position is strictly greater than `x`, if any.
  auto iter =
      std::upper_bound(points.begin(), points.end(), x,
                       [](float x, Point point) { return x < point.x; });

  Point prev = {0, 0};
  Point next = {1, 1};
  if (iter == points.end()) {
    if (!points.empty()) {
      prev = points.back();
    }
  } else {
    next = *iter;
    if (iter != points.begin()) {
      --iter;
      prev = *iter;
    }
  }

  if (prev.y == next.y) {
    return next.y;
  }
  if (prev.x == next.x) {
    return x < next.x ? prev.y : next.y;
  }
  float t = (x - prev.x) / (next.x - prev.x);
  return geometry_internal::Lerp(prev.y, next.y, t);
}

float EasingImplementation::Steps::GetY(float x) const {
  if (x < 0) return 0;
  if (x >= 1) return 1;

  return starting_y + step_height * std::floor(step_count * x);
}

void EasingImplementation::AppendUnitIntervalCriticalPoints(
    absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
    const {
  std::visit(absl::Overload(
                 [](const Identity& arg) {},
                 [&critical_points](const CubicBezierApproximation& arg) {
                   return arg.AppendUnitIntervalCriticalPoints(critical_points);
                 },
                 [&critical_points](const Linear& arg) {
                   return arg.AppendUnitIntervalCriticalPoints(critical_points);
                 },
                 [&critical_points](const Steps& arg) {
                   return arg.AppendUnitIntervalCriticalPoints(critical_points);
                 }),
             implementation_type_);
}

namespace {

// Returns the https://en.wikipedia.org/wiki/Sign_function of x.
int SignFunction(float x) { return (x > 0) - (x < 0); }

}  // namespace

void EasingImplementation::CubicBezierApproximation::
    AppendUnitIntervalCriticalPoints(
        absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
        const {
  constexpr float kTableDeltaX = 1.f / (kTableSize - 1);
  int delta_y_sign = SignFunction(lookup_table[1] - lookup_table[0]);
  for (int i = 1; i + 1 < kTableSize; ++i) {
    int next_sign = SignFunction(lookup_table[i + 1] - lookup_table[i]);
    if (delta_y_sign != next_sign) {
      delta_y_sign = next_sign;
      critical_points.push_back(i * kTableDeltaX);
    }
  }
}

void EasingImplementation::Linear::AppendUnitIntervalCriticalPoints(
    absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
    const {
  for (const Point& point : points) {
    if (!critical_points.empty() && point.x == critical_points.back()) {
      critical_points.back() = std::nexttoward(point.x, 0);
    }
    critical_points.push_back(point.x);
  }
}

void EasingImplementation::Steps::AppendUnitIntervalCriticalPoints(
    absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
    const {
  if (step_position == EasingFunction::StepPosition::kJumpStart ||
      step_position == EasingFunction::StepPosition::kJumpBoth) {
    critical_points.push_back(0);
  }
  float step = 1 / step_count;
  for (int i = 1; i < step_count; ++i) {
    critical_points.push_back(std::nexttoward(i * step, 0));
    critical_points.push_back(i * step);
  }
  if (step_position == EasingFunction::StepPosition::kJumpEnd ||
      step_position == EasingFunction::StepPosition::kJumpBoth ||
      step_position == EasingFunction::StepPosition::kJumpNone) {
    critical_points.push_back(std::nexttoward(1.f, 0));
    critical_points.push_back(1.f);
  }
}

}  // namespace ink::strokes_internal
