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

#ifndef INK_STROKES_INTERNAL_EASING_IMPLEMENTATION_H_
#define INK_STROKES_INTERNAL_EASING_IMPLEMENTATION_H_

#include <array>
#include <variant>

#include "absl/container/inlined_vector.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/point.h"

namespace ink::strokes_internal {

// Implementation for an `EasingFunction` based on a constant-sized inline
// look-up table for constant-time x->y mapping.
class EasingImplementation {
 public:
  // Creates an easing function implementation based on `ease`, which is
  // expected to have gone through validation by `BrushFamily`.
  explicit EasingImplementation(const EasingFunction& ease);

  EasingImplementation(const EasingImplementation&) = default;
  EasingImplementation& operator=(const EasingImplementation&) = default;
  ~EasingImplementation() = default;

  // Though the number of critical points could be higher for step functions,
  // this seems like a reasonable starting point.
  static constexpr int kInlineCriticalPointCount = 8;

  float GetY(float x) const;

  // Appends `critical_points` with critical points, i.e. the x values where the
  // derivative is zero or undefined, in the unit interval. Zero and one are
  // only appended where they actually constitute critical points for that
  // easing function.
  void AppendUnitIntervalCriticalPoints(
      absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
      const;

 private:
  struct Identity {
    float GetY(float x) const;
  };
  struct CubicBezierApproximation {
    // This is set to the largest possible value that still allows the below
    // implementation_type_ variant to fit in 64 bytes.
    static constexpr int kTableSize = 14;

    float GetY(float x) const;
    void AppendUnitIntervalCriticalPoints(
        absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
        const;

    std::array<float, kTableSize> lookup_table;
  };
  struct Linear {
    // This is set to the largest possible value that still allows the below
    // implementation_type_ variant to fit in 64 bytes.
    static constexpr int kInlineSize = 6;

    float GetY(float x) const;
    void AppendUnitIntervalCriticalPoints(
        absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
        const;

    absl::InlinedVector<Point, kInlineSize> points;
  };
  struct Steps {
    float GetY(float x) const;
    void AppendUnitIntervalCriticalPoints(
        absl::InlinedVector<float, kInlineCriticalPointCount>& critical_points)
        const;

    float step_count;
    float step_height;
    float starting_y;
    EasingFunction::StepPosition step_position;
  };

  void SetUpImplementationType(EasingFunction::Predefined predefined);
  void SetUpImplementationType(const EasingFunction::CubicBezier& cubic_bezier);
  void SetUpImplementationType(const EasingFunction::Linear& linear);
  void SetUpImplementationType(const EasingFunction::Steps& steps);

  std::variant<Identity, CubicBezierApproximation, Linear, Steps>
      implementation_type_;
  // We expect implementation_type_ to fit in one 64-byte cache line.
  static_assert(sizeof(implementation_type_) <= 64);
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_EASING_IMPLEMENTATION_H_
