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

#include <iterator>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/fuzz_domains.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/input/type_matchers.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

TEST(StrokeInputBatchConstIteratorTest, EmptyBatch) {
  StrokeInputBatch inputs;

  StrokeInputBatch::ConstIterator iter = inputs.begin();
  EXPECT_EQ(iter, inputs.end());
}

TEST(StrokeInputBatchConstIteratorTest, MultipleElementBatch) {
  StrokeInputBatch inputs;
  StrokeInput input0 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {10, 20},
                        .elapsed_time = Duration32::Seconds(5.0f),
                        .pressure = 0.4,
                        .tilt = Angle::Radians(1),
                        .orientation = Angle::Radians(2)};
  StrokeInput input1 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {10, 23},
                        .elapsed_time = Duration32::Seconds(6.0f),
                        .pressure = 0.3,
                        .tilt = Angle::Radians(1),
                        .orientation = Angle::Radians(2)};
  StrokeInput input2 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {10, 23},
                        .elapsed_time = Duration32::Seconds(7.0f),
                        .pressure = 0.3,
                        .tilt = Angle::Radians(1),
                        .orientation = Angle::Radians(1.1)};
  std::vector<StrokeInput> input_vector = {input0, input1, input2};

  ASSERT_EQ(absl::OkStatus(), inputs.Append(input_vector));
  EXPECT_THAT(*(inputs.begin()), StrokeInputEq(input0));
  StrokeInputBatch::ConstIterator iter = inputs.begin();
  EXPECT_THAT(*iter, StrokeInputEq(input0));
  EXPECT_THAT(*iter++, StrokeInputEq(input0));
  EXPECT_THAT(*iter, StrokeInputEq(input1));
  EXPECT_THAT(*++iter, StrokeInputEq(input2));
  EXPECT_EQ(++iter, inputs.end());
}

TEST(StrokeInputBatchConstIteratorTest, SingleInputBatch) {
  StrokeInputBatch inputs;
  StrokeInput input0 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {10, 20},
                        .elapsed_time = Duration32::Seconds(5.0f),
                        .pressure = 0.4,
                        .tilt = Angle::Radians(1),
                        .orientation = Angle::Radians(2)};

  ASSERT_EQ(absl::OkStatus(), inputs.Append(input0));
  StrokeInputBatch::ConstIterator iter = inputs.begin();
  EXPECT_THAT(*iter, StrokeInputEq(input0));
  EXPECT_THAT(iter->position, PointEq({10, 20}));
  EXPECT_THAT(*iter++, StrokeInputEq(input0));
  EXPECT_NE(iter, inputs.begin());
  EXPECT_EQ(iter, inputs.end());
}

TEST(StrokeInputBatchConstIteratorTest, LazyInit) {
  StrokeInputBatch inputs;
  StrokeInput input0 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {10, 20},
                        .elapsed_time = Duration32::Seconds(5.0f),
                        .pressure = 0.4,
                        .tilt = Angle::Radians(1),
                        .orientation = Angle::Radians(2)};

  ASSERT_EQ(absl::OkStatus(), inputs.Append(input0));
  StrokeInputBatch::ConstIterator iter;
  iter = inputs.begin();
  EXPECT_EQ(iter, inputs.begin());
  EXPECT_THAT(*iter, StrokeInputEq(*inputs.begin()));
}

void IteratorDistanceIsSize(const StrokeInputBatch& batch) {
  EXPECT_EQ(std::distance(batch.begin(), batch.end()), batch.Size());
}
FUZZ_TEST(StrokeInputBatchConstIteratorTest, IteratorDistanceIsSize)
    .WithDomains(ArbitraryStrokeInputBatch());

void IteratorVisitsInputsInOrder(const StrokeInputBatch& batch) {
  int i = 0;
  for (StrokeInput input : batch) {
    ASSERT_LT(i, batch.Size());
    EXPECT_THAT(input, StrokeInputEq(batch.Get(i)));
    ++i;
  }
  EXPECT_EQ(i, batch.Size());
}
FUZZ_TEST(StrokeInputBatchConstIteratorTest, IteratorVisitsInputsInOrder)
    .WithDomains(ArbitraryStrokeInputBatch());

}  // namespace
}  // namespace ink
