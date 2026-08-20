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

#ifndef INK_TYPES_TYPE_MATCHERS_H_
#define INK_TYPES_TYPE_MATCHERS_H_

#include "gtest/gtest.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {

// Matches a Duration32 by applying the given matcher to value of the duration's
// ToSeconds() method.
::testing::Matcher<Duration32> Duration32Seconds(
    ::testing::Matcher<float> seconds_matcher);

// Matchers for Duration32 by comparing to the given duration with
// ::testing::FloatEq() or ::testing::FloatNear().
::testing::Matcher<Duration32> Duration32Eq(Duration32 expected);
::testing::Matcher<Duration32> Duration32Near(Duration32 expected,
                                              float tolerance);

// Matcher for PhysicalDistance by comparing to the given distance with
// ::testing::FloatEq().
::testing::Matcher<PhysicalDistance> PhysicalDistanceEq(
    PhysicalDistance expected);

}  // namespace ink

#endif  // INK_TYPES_TYPE_MATCHERS_H_
