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

#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/container/inlined_vector.h"
#include "ink/brush/easing_function.h"
#include "ink/brush/fuzz_domains.h"

namespace ink::strokes_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::FloatNear;
using ::testing::IsEmpty;
using ::testing::IsNan;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();

TEST(EasingImplementationTest, TestPredefinedLinear) {
  EasingFunction::Predefined predefined = EasingFunction::Predefined::kLinear;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), -kInfinity);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), -0.1f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.1f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.6f), 0.6f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.2f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), kInfinity);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedEase) {
  EasingFunction::Predefined predefined = EasingFunction::Predefined::kEase;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.102f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.6f), FloatNear(0.884f, 0.001f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedEaseIn) {
  EasingFunction::Predefined predefined = EasingFunction::Predefined::kEaseIn;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.019f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.6f), FloatNear(0.430f, 0.001f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedEaseOut) {
  EasingFunction::Predefined predefined = EasingFunction::Predefined::kEaseOut;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.160f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.6f), FloatNear(0.784f, 0.001f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedEaseInOut) {
  EasingFunction::Predefined predefined =
      EasingFunction::Predefined::kEaseInOut;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.022f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.6f), FloatNear(0.667f, 0.001f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedStepStart) {
  EasingFunction::Predefined predefined =
      EasingFunction::Predefined::kStepStart;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.6f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestPredefinedStepEnd) {
  EasingFunction::Predefined predefined = EasingFunction::Predefined::kStepEnd;

  EasingImplementation easing_function({predefined});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.6f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestCubicBezierYBelowOne) {
  EasingFunction::CubicBezier cubic_bezier = {
      .x1 = 1.f, .y1 = 0.15f, .x2 = 0.01f, .y2 = 0.82f};

  EasingImplementation easing_function({cubic_bezier});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.019f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.6f), FloatNear(0.824f, 0.001f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestCubicBezierYAboveOne) {
  EasingFunction::CubicBezier cubic_bezier = {
      .x1 = 0.7f, .y1 = 1.8f, .x2 = 0.5, .y2 = -0.75f};

  EasingImplementation easing_function({cubic_bezier});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_THAT(easing_function.GetY(0.1f), FloatNear(0.240f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.4f), FloatNear(0.670f, 0.001f));
  EXPECT_THAT(easing_function.GetY(0.7f), FloatNear(0.361f, 0.005f));
  EXPECT_THAT(easing_function.GetY(0.85f), FloatNear(0.550f, 0.005f));
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearEmpty) {
  EasingFunction::Linear linear = {.points = {}};

  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), -kInfinity);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), -0.1f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.1f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.6f), 0.6f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.2f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), kInfinity);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearOneInternalPoint) {
  EasingFunction::Linear linear = {.points = {{0.5, 0.25}}};

  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), -kInfinity);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.25f), -0.125f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.25f), 0.125f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.5f), 0.25f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.75f), 0.625f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.25f), 1.375f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), kInfinity);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearDiscontinuityInMiddle) {
  EasingFunction::Linear linear = {.points = {{0.25, 0}, {0.25, 1}}};

  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.24f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.25f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.26f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.1f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearDiscontinuityAtStart) {
  EasingFunction::Linear linear = {.points = {{0, 1}}};

  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.5f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.5f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.5f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearDiscontinuityAtEnd) {
  EasingFunction::Linear linear = {.points = {{1, 0}}};

  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.5f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.5f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.5f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestLinearMultiplePointsWithSameX) {
  EasingFunction::Linear linear = {
      .points = {{0.25, 0}, {0.25, 0.75}, {0.25, 1}}};
  EasingImplementation easing_function({linear});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.24f), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.25f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.26f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.1f), 1.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestStepJumpStart) {
  EasingFunction::Steps steps = {
      .step_count = 4,
      .step_position = EasingFunction::StepPosition::kJumpStart};

  EasingImplementation easing_function({steps});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.0f), 0.25);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.25);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.4f), 0.5);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.7f), 0.75);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.85f), 1.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestStepJumpEnd) {
  EasingFunction::Steps steps = {
      .step_count = 4, .step_position = EasingFunction::StepPosition::kJumpEnd};

  EasingImplementation easing_function({steps});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.0f), 0.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.25f), 0.25);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.4f), 0.25);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.7f), 0.5);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.85f), 0.75);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestStepJumpNone) {
  EasingFunction::Steps steps = {
      .step_count = 5,
      .step_position = EasingFunction::StepPosition::kJumpNone};

  EasingImplementation easing_function({steps});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.0f), 0.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.3f), 0.25);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.4f), 0.5);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.7f), 0.75);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.85f), 1.0);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, TestStepJumpBoth) {
  EasingFunction::Steps steps = {
      .step_count = 4,
      .step_position = EasingFunction::StepPosition::kJumpBoth};

  EasingImplementation easing_function({steps});

  EXPECT_FLOAT_EQ(easing_function.GetY(-kInfinity), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(-0.1f), 0.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.0f), 0.2);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.1f), 0.2);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.3f), 0.4f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.4f), 0.4f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.7f), 0.6f);
  EXPECT_FLOAT_EQ(easing_function.GetY(0.85f), 0.8f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.0f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(1.2f), 1.0f);
  EXPECT_FLOAT_EQ(easing_function.GetY(kInfinity), 1.0f);
  EXPECT_THAT(easing_function.GetY(kNan), IsNan());
}

TEST(EasingImplementationTest, CriticalPointsIdentity) {
  EasingImplementation easing_implementation(
      {EasingFunction::Predefined::kLinear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, IsEmpty());
}

TEST(EasingImplementationTest, CriticalPointsPredefinedCubicEaseIn) {
  EasingImplementation easing_implementation(
      {EasingFunction::Predefined::kEaseIn});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, IsEmpty());
}

TEST(EasingImplementationTest, CriticalPointsPredefinedCubicWithLocalMinMax) {
  EasingFunction::CubicBezier cubic_bezier = {
      .x1 = 0.5f, .y1 = 1.8f, .x2 = 0.75, .y2 = 0.75f};
  EasingImplementation easing_implementation({cubic_bezier});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.61, 0.01), FloatNear(0.92, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsLinearWithNoPoints) {
  EasingFunction::Linear linear = {.points = {}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, IsEmpty());
}

TEST(EasingImplementationTest, CriticalPointsLinearWithOnePoint) {
  EasingFunction::Linear linear = {.points = {{0.5, 0.25}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, ElementsAre(FloatNear(0.5, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsLinearWithMultiplePoints) {
  EasingFunction::Linear linear = {.points = {{0.25, 0}, {0.75, 1}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.25, 0.01), FloatNear(0.75, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsLinearWithDisjointPoints) {
  EasingFunction::Linear linear = {.points = {{0.25, 0}, {0.25, 1}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.2499, 0.01), FloatNear(0.25, 0.01)));
}

TEST(EasingImplementationTest,
     CriticalPointsLinearWithMoreThanThreeDisjointPoints) {
  EasingFunction::Linear linear = {.points = {{0.25, 0}, {0.25, 7}, {0.25, 1}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.2499, 0.01), FloatNear(0.25, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsLinearWithMultipleZeroX) {
  EasingFunction::Linear linear = {.points = {{0, 0.1}, {0, .7}, {0, 1}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, ElementsAre(FloatNear(0, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsLinearWithMultipleOneX) {
  EasingFunction::Linear linear = {.points = {{1, 0}, {1, 7}, {1, 1}}};
  EasingImplementation easing_implementation({linear});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points, ElementsAre(FloatNear(1, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsPredefinedStepEnd) {
  EasingImplementation easing_implementation(
      {EasingFunction::Predefined::kStepEnd});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.9999, 0.01), FloatNear(1.f, 0.01)));
}

TEST(EasingImplementationTest, CriticalPointsStepsMoreThanOneStep) {
  EasingFunction::Steps steps = {
      .step_count = 4,
      .step_position = EasingFunction::StepPosition::kJumpBoth};
  EasingImplementation easing_implementation({steps});
  absl::InlinedVector<float, EasingImplementation::kInlineCriticalPointCount>
      points;
  easing_implementation.AppendUnitIntervalCriticalPoints(points);

  EXPECT_THAT(points,
              ElementsAre(FloatNear(0.f, 0.01), FloatNear(0.2499, 0.01),
                          FloatNear(0.25, 0.01), FloatNear(0.4999, 0.01),
                          FloatNear(0.5, 0.01), FloatNear(0.7499, 0.01),
                          FloatNear(0.75, 0.01), FloatNear(0.9999, 0.01),
                          FloatNear(1.f, 0.01)));
}

void EasingImplementationDoesNotCrash(const EasingFunction& easing_function,
                                      float x) {
  EasingImplementation easing_implementation(easing_function);
  easing_implementation.GetY(x);
}
FUZZ_TEST(EasingImplementationTest, EasingImplementationDoesNotCrash)
    .WithDomains(ValidEasingFunction(), fuzztest::Arbitrary<float>());

}  // namespace
}  // namespace ink::strokes_internal
