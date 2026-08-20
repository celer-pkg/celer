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

#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "ink/brush/fuzz_domains.h"

namespace ink {
namespace {

using ::testing::HasSubstr;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();

TEST(EasingFunctionTest, StringifyPredefined) {
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kLinear), "kLinear");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kEase), "kEase");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kEaseIn), "kEaseIn");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kEaseOut), "kEaseOut");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kEaseInOut), "kEaseInOut");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kStepStart), "kStepStart");
  EXPECT_EQ(absl::StrCat(EasingFunction::Predefined::kStepEnd), "kStepEnd");
  EXPECT_EQ(absl::StrCat(static_cast<EasingFunction::Predefined>(99)),
            "Predefined(99)");
}

TEST(EasingFunctionTest, StringifyCubicBezier) {
  EXPECT_EQ(absl::StrCat(EasingFunction::CubicBezier{1, 2, -3, 4}),
            "CubicBezier{1, 2, -3, 4}");
  EXPECT_EQ(absl::StrCat(EasingFunction::CubicBezier{1.75, 0, kInfinity, kNan}),
            "CubicBezier{1.75, 0, inf, nan}");
}

TEST(EasingFunctionTest, StringifyLinear) {
  EXPECT_EQ(absl::StrCat(EasingFunction::Linear{}), "Linear{}");
  EXPECT_EQ(absl::StrCat(EasingFunction::Linear{{{0.5, 0.25}}}),
            "Linear{(0.5, 0.25)}");
  EXPECT_EQ(absl::StrCat(EasingFunction::Linear{{{0.25, 0}, {0.75, 1}}}),
            "Linear{(0.25, 0), (0.75, 1)}");
}

TEST(EasingFunctionTest, StringifySteps) {
  EXPECT_EQ(absl::StrCat(EasingFunction::Steps{
                .step_count = 3,
                .step_position = EasingFunction::StepPosition::kJumpEnd}),
            "Steps{3, kJumpEnd}");
  EXPECT_EQ(
      absl::StrCat(EasingFunction::Steps{
          .step_count = 3,
          .step_position = static_cast<EasingFunction::StepPosition>(99)}),
      "Steps{3, StepPosition(99)}");
}

TEST(EasingFunctionTest, StringifyEasingFunction) {
  EXPECT_EQ(absl::StrCat(EasingFunction{}), "kLinear");
  EXPECT_EQ(absl::StrCat(EasingFunction{EasingFunction::Predefined::kEaseIn}),
            "kEaseIn");
  EXPECT_EQ(
      absl::StrCat(EasingFunction{EasingFunction::CubicBezier{1, 2, -3, 4}}),
      "CubicBezier{1, 2, -3, 4}");
}

TEST(EasingFunctionTest, StringifyEasingParameter) {
  EXPECT_EQ(absl::StrCat(EasingFunction::Parameters{
                EasingFunction::Predefined::kLinear}),
            "kLinear");
  EXPECT_EQ(absl::StrCat(EasingFunction::Parameters{
                EasingFunction::Predefined::kEaseIn}),
            "kEaseIn");
  EXPECT_EQ(absl::StrCat(EasingFunction::Parameters{
                EasingFunction::CubicBezier{1, 2, -3, 4}}),
            "CubicBezier{1, 2, -3, 4}");
}

TEST(EasingFunctionTest, CubicBezierEqualAndNotEqual) {
  EasingFunction::CubicBezier cubic_bezier =
      EasingFunction::CubicBezier{1, 2, -3, 4};

  EXPECT_EQ(cubic_bezier, EasingFunction::CubicBezier({1, 2, -3, 4}));
  EXPECT_NE(cubic_bezier, EasingFunction::CubicBezier({9, 2, -3, 4}));
  EXPECT_NE(cubic_bezier, EasingFunction::CubicBezier({1, 9, -3, 4}));
  EXPECT_NE(cubic_bezier, EasingFunction::CubicBezier({1, 2, -9, 4}));
  EXPECT_NE(cubic_bezier, EasingFunction::CubicBezier({1, 2, -3, 9}));
}

TEST(EasingFunctionTest, LinearEqualAndNotEqual) {
  EasingFunction::Linear linear =
      EasingFunction::Linear{{{0.25, 0.5}, {0.75, 0.5}}};

  EXPECT_EQ(linear, EasingFunction::Linear({{{0.25, 0.5}, {0.75, 0.5}}}));
  EXPECT_NE(linear, EasingFunction::Linear({{{0.25, 0.5}}}));
  EXPECT_NE(linear, EasingFunction::Linear({{{0.75, 0.5}, {0.75, 0.5}}}));
  EXPECT_NE(linear, EasingFunction::Linear({{{0.25, 0.5}, {0.75, 0.4}}}));
  EXPECT_NE(linear,
            EasingFunction::Linear({{{0.25, 0.5}, {0.75, 0.5}, {0.9, 0.1}}}));
}

TEST(EasingFunctionTest, StepsEqualAndNotEqual) {
  EasingFunction::Steps steps = EasingFunction::Steps{
      .step_count = 3, .step_position = EasingFunction::StepPosition::kJumpEnd};

  EXPECT_EQ(steps,
            EasingFunction::Steps(
                {.step_count = 3,
                 .step_position = EasingFunction::StepPosition::kJumpEnd}));
  EXPECT_NE(steps,
            EasingFunction::Steps(
                {.step_count = 9,
                 .step_position = EasingFunction::StepPosition::kJumpEnd}));
  EXPECT_NE(steps,
            EasingFunction::Steps(
                {.step_count = 3,
                 .step_position = EasingFunction::StepPosition::kJumpStart}));
}

TEST(EasingFunctionTest, EasingFunctionEqualAndNotEqual) {
  EasingFunction cubic_bezier{.parameters =
                                  EasingFunction::CubicBezier{1, 2, -3, 4}};
  EasingFunction steps{
      .parameters = EasingFunction::Steps{
          .step_count = 3,
          .step_position = EasingFunction::StepPosition::kJumpEnd}};
  EasingFunction predefined{.parameters = EasingFunction::Predefined::kEase};

  EXPECT_EQ(cubic_bezier,
            EasingFunction({EasingFunction::CubicBezier({1, 2, -3, 4})}));
  EXPECT_EQ(steps, EasingFunction({EasingFunction::Steps{
                       3, EasingFunction::StepPosition::kJumpEnd}}));
  EXPECT_EQ(predefined, EasingFunction({EasingFunction::Predefined::kEase}));

  EXPECT_NE(cubic_bezier, steps);
  EXPECT_NE(cubic_bezier, predefined);
  EXPECT_NE(steps, predefined);

  EXPECT_NE(cubic_bezier,
            EasingFunction({EasingFunction::CubicBezier({5, 6, -7, 8})}));
  EXPECT_NE(steps, EasingFunction({EasingFunction::Steps{
                       7, EasingFunction::StepPosition::kJumpStart}}));
  EXPECT_NE(predefined, EasingFunction({EasingFunction::Predefined::kEaseOut}));
}

TEST(EasingFunctionTest, InvalidPredefined) {
  absl::Status invalid_predefined = brush_internal::ValidateEasingFunction(
      {static_cast<EasingFunction::Predefined>(-1)});
  EXPECT_EQ(invalid_predefined.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(invalid_predefined.message(), HasSubstr("Predefined"));
}

TEST(BrushFamilyTest, InvalidCubicBezier) {
  // X value < 0:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = -1, .y1 = 0, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = -1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  // X value > 1:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 2, .y1 = 0, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = 2, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  // Infinite X or Y value:
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::CubicBezier{
            .x1 = kInfinity, .y1 = 0, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::CubicBezier{
            .x1 = 0, .y1 = kInfinity, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::CubicBezier{
            .x1 = 0, .y1 = 0, .x2 = kInfinity, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::CubicBezier{
            .x1 = 0, .y1 = 0, .x2 = 1, .y2 = kInfinity}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  // NaN X or Y value:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = kNan, .y1 = 0, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 0, .y1 = kNan, .x2 = 1, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = kNan, .y2 = 1}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = 1, .y2 = kNan}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
  }
}

TEST(EasingFunctionTest, InvalidLinear) {
  // Non-finite Y-position:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{0, kNan}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{0, kInfinity}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{0, -kInfinity}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  // Non-finite X-position:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{kNan, 0}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{kInfinity, 0}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{-kInfinity, 0}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  // X-position out of range:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{-0.1, 0}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{1.1, 0}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  // X-positions that aren't monotonicly non-decreasing:
  {
    absl::Status status = brush_internal::ValidateEasingFunction(
        {EasingFunction::Linear{{{0.75, 0}, {0.25, 1}}}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("monotonic"));
  }
}

TEST(EasingFunctionTest, InvalidSteps) {
  // Step count < 1:
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::Steps{
            .step_count = 0,
            .step_position = EasingFunction::StepPosition::kJumpEnd}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Steps"));
  }
  // Invalid StepPosition value:
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::Steps{
            .step_count = 1,
            .step_position = static_cast<EasingFunction::StepPosition>(-1)}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Steps"));
  }
  // Step count < 2 with StepPosition::kJumpNone:
  {
    absl::Status status =
        brush_internal::ValidateEasingFunction({EasingFunction::Steps{
            .step_count = 1,
            .step_position = EasingFunction::StepPosition::kJumpNone}});
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Steps"));
  }
}

void CanValidateValidEasingFunction(const EasingFunction& easing_function) {
  EXPECT_EQ(absl::OkStatus(),
            brush_internal::ValidateEasingFunction(easing_function));
}
FUZZ_TEST(EasingFunctionTest, CanValidateValidEasingFunction)
    .WithDomains(ValidEasingFunction());

}  // namespace
}  // namespace ink
