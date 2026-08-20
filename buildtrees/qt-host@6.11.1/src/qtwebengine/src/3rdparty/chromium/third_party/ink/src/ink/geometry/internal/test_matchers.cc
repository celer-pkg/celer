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

#include "ink/geometry/internal/test_matchers.h"

#include "gmock/gmock.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/type_matchers.h"

namespace ink::geometry_internal {
namespace {

using ::testing::AllOf;
using ::testing::ExplainMatchResult;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::PrintToString;
using ::testing::Property;

MATCHER_P(CircleEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Center (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Property("Center", &Circle::Center, PointEq(expected.Center())),
            Property("Radius", &Circle::Radius, FloatEq(expected.Radius()))),
      arg, result_listener);
}

MATCHER_P2(CircleNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Center (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Property("Center", &Circle::Center,
                     PointNear(expected.Center(), tolerance)),
            Property("Radius", &Circle::Radius,
                     FloatNear(expected.Radius(), tolerance))),
      arg, result_listener);
}

}  // namespace

Matcher<Circle> CircleEq(const Circle& expected) {
  return CircleEqMatcher(expected);
}

Matcher<Circle> CircleNear(const Circle& expected, float tolerance) {
  return CircleNearMatcher(expected, tolerance);
}

}  // namespace ink::geometry_internal
