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

#include "ink/strokes/input/type_matchers.h"

#include <cstddef>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/type_matchers.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatEq;

MATCHER_P(StrokeInputEqMatcher, expected,
          absl::StrCat(negation ? "isn't" : "is", " approximately ",
                       expected)) {
  return ExplainMatchResult(
      AllOf(
          Field("tool_type", &StrokeInput::tool_type, Eq(expected.tool_type)),
          Field("position", &StrokeInput::position, PointEq(expected.position)),
          Field("elapsed_time", &StrokeInput::elapsed_time,
                Duration32Eq(expected.elapsed_time)),
          Field("stroke_unit_length", &StrokeInput::stroke_unit_length,
                PhysicalDistanceEq(expected.stroke_unit_length)),
          Field("pressure", &StrokeInput::pressure, FloatEq(expected.pressure)),
          Field("tilt", &StrokeInput::tilt, AngleEq(expected.tilt)),
          Field("orientation", &StrokeInput::orientation,
                AngleEq(expected.orientation))),
      arg, result_listener);
}

MATCHER_P(StrokeInputBatchIsArrayMatcher, expected, "") {
  if (!ExplainMatchResult(Eq(expected.size()), arg.Size(), result_listener)) {
    return false;
  }

  for (size_t i = 0; i < arg.Size(); ++i) {
    if (!ExplainMatchResult(StrokeInputEq(expected[i]), arg.Get(i),
                            result_listener)) {
      *result_listener << " array index of first inequality = " << i;
      return false;
    }
  }

  return true;
}

MATCHER_P(StrokeInputBatchEqMatcher, expected, "") {
  if (!ExplainMatchResult(Eq(expected.Size()), arg.Size(), result_listener)) {
    return false;
  }

  for (size_t i = 0; i < arg.Size(); ++i) {
    if (!ExplainMatchResult(StrokeInputEq(expected.Get(i)), arg.Get(i),
                            result_listener)) {
      *result_listener << " array index of first inequality = " << i;
      return false;
    }
  }

  return ExplainMatchResult(Eq(expected.GetNoiseSeed()), arg.GetNoiseSeed(),
                            result_listener);
}

}  // namespace

::testing::Matcher<StrokeInput> StrokeInputEq(const StrokeInput& expected) {
  return StrokeInputEqMatcher(expected);
}

::testing::Matcher<StrokeInputBatch> StrokeInputBatchIsArray(
    absl::Span<const StrokeInput> expected) {
  return StrokeInputBatchIsArrayMatcher(expected);
}

::testing::Matcher<StrokeInputBatch> StrokeInputBatchEq(
    const StrokeInputBatch& expected) {
  return StrokeInputBatchEqMatcher(expected);
}

}  // namespace ink
