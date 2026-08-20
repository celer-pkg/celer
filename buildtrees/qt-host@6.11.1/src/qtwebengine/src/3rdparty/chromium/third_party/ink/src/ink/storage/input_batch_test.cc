// Copyright 2024-2025 Google LLC
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

#include "ink/storage/input_batch.h"

#include <limits>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/geometry/type_matchers.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/types/iterator_range.h"
#include "ink/types/numbers.h"
#include "ink/types/type_matchers.h"
#include "google/protobuf/text_format.h"

namespace ink {
namespace {

using ::ink::numbers::kPi;
using ::google::protobuf::TextFormat;
using ::testing::ElementsAre;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::HasSubstr;
using ::testing::Optional;

TEST(CodedStrokeInputBatchIteratorTest, DecodeStylusStrokeInputBatch) {
  proto::CodedStrokeInputBatch coded;
  ASSERT_TRUE(TextFormat::ParseFromString(R"pb(
                                            x_stroke_space {
                                              scale: 2.5
                                              deltas: [ 1, 2, 1 ]
                                            }
                                            y_stroke_space {
                                              scale: 2.5
                                              deltas: [ 1, -1, 1 ]
                                            }
                                            elapsed_time_seconds {
                                              scale: 0.001
                                              deltas: [ 0, 30, 30 ]
                                            }
                                            pressure {
                                              scale: 0.01
                                              deltas: [ 25, 50, -25 ]
                                            }
                                            tilt {
                                              scale: 0.0314159
                                              deltas: [ 10, 10, 10 ]
                                            }
                                            orientation {
                                              scale: 0.0314159
                                              deltas: [ 25, -50, 25 ]
                                            }
                                          )pb",
                                          &coded));

  absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>> range =
      DecodeStrokeInputBatchProto(coded);
  ASSERT_EQ(range.status(), absl::OkStatus());
  std::vector<CodedStrokeInputBatchIterator::ValueType> values(range->begin(),
                                                               range->end());
  ASSERT_EQ(values.size(), 3);

  EXPECT_THAT(values[0].position_stroke_space, PointEq({2.5f, 2.5f}));
  EXPECT_THAT(values[1].position_stroke_space, PointEq({7.5f, 0.0f}));
  EXPECT_THAT(values[2].position_stroke_space, PointEq({10.0f, 2.5f}));

  EXPECT_THAT(values[0].elapsed_time, Duration32Seconds(FloatEq(0.000f)));
  EXPECT_THAT(values[1].elapsed_time, Duration32Seconds(FloatEq(0.030f)));
  EXPECT_THAT(values[2].elapsed_time, Duration32Seconds(FloatEq(0.060f)));

  EXPECT_THAT(values[0].pressure, Optional(FloatEq(0.25f)));
  EXPECT_THAT(values[1].pressure, Optional(FloatEq(0.75f)));
  EXPECT_THAT(values[2].pressure, Optional(FloatEq(0.50f)));

  EXPECT_THAT(values[0].tilt, Optional(FloatNear(1 * kPi / 10, 1e-5f)));
  EXPECT_THAT(values[1].tilt, Optional(FloatNear(2 * kPi / 10, 1e-5f)));
  EXPECT_THAT(values[2].tilt, Optional(FloatNear(3 * kPi / 10, 1e-5f)));

  EXPECT_THAT(values[0].orientation, Optional(FloatNear(kPi / 4, 1e-5f)));
  EXPECT_THAT(values[1].orientation, Optional(FloatNear(-kPi / 4, 1e-5f)));
  EXPECT_THAT(values[2].orientation, Optional(FloatNear(0.0f, 1e-5f)));
}

TEST(CodedStrokeInputBatchIteratorTest, DecodeMouseStrokeInputBatch) {
  // Pressure/tilt/orientation are optional and may be omitted (e.g. for mouse
  // input).
  proto::CodedStrokeInputBatch coded;
  ASSERT_TRUE(TextFormat::ParseFromString(R"pb(
                                            x_stroke_space {
                                              scale: 2.5
                                              deltas: [ 1, 2, 1 ]
                                            }
                                            y_stroke_space {
                                              scale: 2.5
                                              deltas: [ 1, -1, 1 ]
                                            }
                                            elapsed_time_seconds {
                                              scale: 0.001
                                              deltas: [ 0, 30, 30 ]
                                            }
                                          )pb",
                                          &coded));

  absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>> range =
      DecodeStrokeInputBatchProto(coded);
  ASSERT_EQ(range.status(), absl::OkStatus());
  std::vector<CodedStrokeInputBatchIterator::ValueType> values(range->begin(),
                                                               range->end());
  ASSERT_EQ(values.size(), 3);

  EXPECT_THAT(values[0].position_stroke_space, PointEq({2.5f, 2.5f}));
  EXPECT_THAT(values[1].position_stroke_space, PointEq({7.5f, 0.0f}));
  EXPECT_THAT(values[2].position_stroke_space, PointEq({10.0f, 2.5f}));

  EXPECT_THAT(values[0].elapsed_time, Duration32Seconds(FloatEq(0.000f)));
  EXPECT_THAT(values[1].elapsed_time, Duration32Seconds(FloatEq(0.030f)));
  EXPECT_THAT(values[2].elapsed_time, Duration32Seconds(FloatEq(0.060f)));

  for (const auto& value : values) {
    EXPECT_EQ(value.pressure, std::nullopt);
    EXPECT_EQ(value.tilt, std::nullopt);
    EXPECT_EQ(value.orientation, std::nullopt);
  }
}

TEST(CodedStrokeInputBatchIteratorTest, IteratorPostIncrement) {
  proto::CodedStrokeInputBatch coded;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        x_stroke_space { deltas: [ 1, 2, 1 ] }
        y_stroke_space { deltas: [ 1, -1, 1 ] }
        elapsed_time_seconds { deltas: [ 0, 1, 1 ] }
      )pb",
      &coded));

  absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>> range =
      DecodeStrokeInputBatchProto(coded);
  ASSERT_EQ(range.status(), absl::OkStatus());
  CodedStrokeInputBatchIterator iter = range->begin();
  const CodedStrokeInputBatchIterator iter0 = iter++;
  const CodedStrokeInputBatchIterator iter1 = iter++;
  const CodedStrokeInputBatchIterator iter2 = iter++;

  EXPECT_THAT(iter0->position_stroke_space, PointEq({1, 1}));
  EXPECT_THAT(iter1->position_stroke_space, PointEq({3, 0}));
  EXPECT_THAT(iter2->position_stroke_space, PointEq({4, 1}));
  EXPECT_EQ(iter, range->end());
}

TEST(CodedStrokeInputBatchIteratorTest, DecodeEmptyStrokeInputBatch) {
  proto::CodedStrokeInputBatch coded;
  absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>> batch =
      DecodeStrokeInputBatchProto(coded);
  ASSERT_EQ(batch.status(), absl::OkStatus());
  EXPECT_THAT(*batch, ElementsAre());
}

TEST(CodedStrokeInputBatchIteratorTest,
     DecodeStrokeInputBatchWithMismatchedLengths) {
  proto::CodedStrokeInputBatch coded;
  coded.mutable_pressure()->add_deltas(1);
  absl::Status mismatched_lengths = DecodeStrokeInputBatchProto(coded).status();
  EXPECT_EQ(mismatched_lengths.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(mismatched_lengths.message(),
              HasSubstr("mismatched numeric run lengths"));
}

TEST(CodedStrokeInputBatchIteratorTest,
     DecodeStrokeInputBatchWithInvalidPressure) {
  proto::CodedStrokeInputBatch coded;
  coded.mutable_x_stroke_space()->add_deltas(0);
  coded.mutable_y_stroke_space()->add_deltas(0);
  coded.mutable_elapsed_time_seconds()->add_deltas(0);
  coded.mutable_pressure()->add_deltas(0);
  coded.mutable_pressure()->set_scale(std::numeric_limits<float>::infinity());
  absl::Status non_finite_scale = DecodeStrokeInputBatchProto(coded).status();
  EXPECT_EQ(non_finite_scale.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(non_finite_scale.message(), HasSubstr("non-finite scale"));
}

}  // namespace
}  // namespace ink
