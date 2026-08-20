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

#include "ink/types/type_matchers.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {
namespace {

using ::testing::DescribeMatcher;
using ::testing::ExplainMatchResult;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::Matches;
using ::testing::Property;

MATCHER_P(Duration32EqMatcher, expected,
          absl::StrCat(negation ? "isn't" : "is", " approximately ",
                       expected)) {
  return Matches(FloatEq(expected.ToSeconds()))(arg.ToSeconds());
}

MATCHER_P2(Duration32NearMatcher, expected, tolerance,
           absl::StrCat(negation ? "isn't" : "is", " within ", tolerance,
                        " of ", expected)) {
  return Matches(FloatNear(expected.ToSeconds(), tolerance))(arg.ToSeconds());
}

MATCHER_P(Duration32SecondsMatcher, seconds_matcher,
          absl::StrCat("`ToSeconds` ",
                       DescribeMatcher<float>(seconds_matcher, negation))) {
  return ExplainMatchResult(
      Property("ToSeconds", &Duration32::ToSeconds, seconds_matcher), arg,
      result_listener);
}

MATCHER_P(PhysicalDistanceEqMatcher, expected,
          absl::StrCat(negation ? "isn't" : "is", " approximately ",
                       expected)) {
  return Matches(FloatEq(expected.ToCentimeters()))(arg.ToCentimeters());
}

}  // namespace

Matcher<Duration32> Duration32Seconds(Matcher<float> seconds_matcher) {
  return Duration32SecondsMatcher(std::move(seconds_matcher));
}

Matcher<Duration32> Duration32Eq(Duration32 expected) {
  return Duration32EqMatcher(expected);
}

Matcher<Duration32> Duration32Near(Duration32 expected, float tolerance) {
  return Duration32NearMatcher(expected, tolerance);
}

Matcher<PhysicalDistance> PhysicalDistanceEq(PhysicalDistance expected) {
  return PhysicalDistanceEqMatcher(expected);
}

}  // namespace ink
