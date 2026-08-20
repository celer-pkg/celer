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

#include "ink/storage/stroke_input_batch.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/strokes/input/fuzz_domains.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/input/type_matchers.h"
#include "ink/types/duration.h"
#include "ink/types/iterator_range.h"
#include "ink/types/physical_distance.h"

namespace ink {
namespace {

using ::ink::proto::CodedStrokeInputBatch;
using ::testing::ElementsAre;
using ::testing::FloatEq;
using ::testing::FloatNear;

class StrokeInputBatchTest : public ::testing::Test {
 public:
  void SetUp() override {
    input_proto_.mutable_x_stroke_space()->set_offset(10);
    input_proto_.mutable_x_stroke_space()->add_deltas(0);
    input_proto_.mutable_x_stroke_space()->add_deltas(-3);
    input_proto_.mutable_x_stroke_space()->add_deltas(1);
    input_proto_.mutable_y_stroke_space()->set_offset(3);
    input_proto_.mutable_y_stroke_space()->add_deltas(0);
    input_proto_.mutable_y_stroke_space()->add_deltas(1);
    input_proto_.mutable_y_stroke_space()->add_deltas(-2);
    input_proto_.mutable_elapsed_time_seconds()->set_scale(
        1e-3);  // deltas are millis
    input_proto_.mutable_elapsed_time_seconds()->add_deltas(0);
    input_proto_.mutable_elapsed_time_seconds()->add_deltas(500);
    input_proto_.mutable_elapsed_time_seconds()->add_deltas(500);
    input_proto_.mutable_pressure()->set_offset(0.0f);
    input_proto_.mutable_pressure()->set_scale(0.1f);
    input_proto_.mutable_pressure()->add_deltas(2);
    input_proto_.mutable_pressure()->add_deltas(1);
    input_proto_.mutable_pressure()->add_deltas(3);
    input_proto_.mutable_tilt()->set_offset(0.0f);
    input_proto_.mutable_tilt()->set_scale(0.1f);
    input_proto_.mutable_tilt()->add_deltas(4);
    input_proto_.mutable_tilt()->add_deltas(0);
    input_proto_.mutable_tilt()->add_deltas(1);
    input_proto_.mutable_orientation()->set_offset(0.0f);
    input_proto_.mutable_orientation()->set_scale(0.1f);
    input_proto_.mutable_orientation()->add_deltas(5);
    input_proto_.mutable_orientation()->add_deltas(1);
    input_proto_.mutable_orientation()->add_deltas(1);
    input_proto_.set_tool_type(CodedStrokeInputBatch::STYLUS);
    input_proto_.set_stroke_unit_length_in_centimeters(0.1);

    absl::StatusOr<StrokeInputBatch> input_batch = StrokeInputBatch::Create(
        {{.tool_type = StrokeInput::ToolType::kStylus,
          .position = {10, 3},
          .elapsed_time = Duration32::Zero(),
          .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
          .pressure = 0.2,
          .tilt = Angle::Radians(0.4),
          .orientation = Angle::Radians(0.5)},
         {.tool_type = StrokeInput::ToolType::kStylus,
          .position = {7, 4},
          .elapsed_time = Duration32::Seconds(0.5f),
          .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
          .pressure = 0.3,
          .tilt = Angle::Radians(0.4),
          .orientation = Angle::Radians(0.6)},
         {.tool_type = StrokeInput::ToolType::kStylus,
          .position = {8, 2},
          .elapsed_time = Duration32::Seconds(1.0f),
          .stroke_unit_length = PhysicalDistance::Centimeters(0.1),
          .pressure = 0.6,
          .tilt = Angle::Radians(0.5),
          .orientation = Angle::Radians(0.7)}});
    ASSERT_EQ(input_batch.status(), absl::OkStatus());
    input_batch_ = *std::move(input_batch);
  }

 protected:
  CodedStrokeInputBatch input_proto_;
  StrokeInputBatch input_batch_;
};

TEST_F(StrokeInputBatchTest, DecodeInputs) {
  absl::StatusOr<StrokeInputBatch> decoded =
      DecodeStrokeInputBatch(input_proto_);
  ASSERT_EQ(decoded.status(), absl::OkStatus());
  EXPECT_THAT(*decoded, StrokeInputBatchEq(input_batch_));
}

TEST_F(StrokeInputBatchTest, DecodeEmptyInputs) {
  CodedStrokeInputBatch empty_proto;
  absl::StatusOr<StrokeInputBatch> input_batch =
      DecodeStrokeInputBatch(empty_proto);
  ASSERT_EQ(input_batch.status(), absl::OkStatus());
  EXPECT_THAT(*input_batch, StrokeInputBatchEq(StrokeInputBatch()));
}

TEST_F(StrokeInputBatchTest, EncodeInputs) {
  CodedStrokeInputBatch encoded_input_proto;
  EncodeStrokeInputBatch(input_batch_, encoded_input_proto);

  const float epsilon = 1e-3;
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(10, epsilon), FloatNear(7, epsilon),
                            FloatNear(8, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(3, epsilon), FloatNear(4, epsilon),
                            FloatNear(2, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.elapsed_time_seconds());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(0.0f, 0.5f, 1.0f));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.pressure());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(0.2f, epsilon), FloatNear(0.3f, epsilon),
                            FloatNear(0.6f, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.tilt());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(0.4f, epsilon), FloatNear(0.4f, epsilon),
                            FloatNear(0.5f, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.orientation());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(0.5f, epsilon), FloatNear(0.6f, epsilon),
                            FloatNear(0.7f, epsilon)));
  }

  EXPECT_NE(encoded_input_proto.x_stroke_space().scale(), 1.0f);
  EXPECT_NE(encoded_input_proto.y_stroke_space().scale(), 1.0f);
}

TEST_F(StrokeInputBatchTest, EncodeLargishPositions) {
  absl::StatusOr<StrokeInputBatch> input_batch =
      StrokeInputBatch::Create({{.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {0, 0},
                                 .elapsed_time = Duration32::Zero()},
                                {.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {4000, 0},
                                 .elapsed_time = Duration32::Seconds(0.5)},
                                {.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {0, 4000},
                                 .elapsed_time = Duration32::Seconds(1.0)}});
  ASSERT_EQ(input_batch.status(), absl::OkStatus());

  CodedStrokeInputBatch encoded_input_proto;
  EncodeStrokeInputBatch(*input_batch, encoded_input_proto);

  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(FloatEq(0), FloatEq(4000), FloatEq(0)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(encoded_input_proto.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(FloatEq(0), FloatEq(0), FloatEq(4000)));
  }
}

TEST_F(StrokeInputBatchTest, EncodeEmptyInputs) {
  // Start with a non-empty CodedStrokeInputBatch.
  CodedStrokeInputBatch input_proto;
  input_proto.mutable_x_stroke_space()->add_deltas(1);
  // Encoding an empty StrokeInputBatch should clear the CodedStrokeInputBatch.
  StrokeInputBatch empty_input_batch;
  EncodeStrokeInputBatch(empty_input_batch, input_proto);
  EXPECT_FALSE(input_proto.has_x_stroke_space());
}

TEST_F(StrokeInputBatchTest, EncodeEmptyInputsWithNoiseSeed) {
  CodedStrokeInputBatch input_proto;
  StrokeInputBatch empty_input_batch;
  empty_input_batch.SetNoiseSeed(12345);
  EncodeStrokeInputBatch(empty_input_batch, input_proto);
  EXPECT_FALSE(input_proto.has_x_stroke_space());
  EXPECT_EQ(input_proto.noise_seed(), 12345);
}

TEST_F(StrokeInputBatchTest, EncodeSingleInputPosition) {
  // Create a stroke with a single input point.
  absl::StatusOr<StrokeInputBatch> single_input_batch =
      StrokeInputBatch::Create(
          {{.position = {1.75, 2.25}, .elapsed_time = Duration32::Zero()}});
  ASSERT_EQ(single_input_batch.status(), absl::OkStatus());

  // Encode the input.
  CodedStrokeInputBatch input_proto;
  EncodeStrokeInputBatch(*single_input_batch, input_proto);
  // The CodedStrokeInputBatch should be valid even though the envelope of input
  // positions is empty (we should avoid calculating an infinite scale).
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(input_proto.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(1.75f));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(input_proto.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(2.25f));
  }

  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(input_proto.elapsed_time_seconds());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(0.f));
  }
  EXPECT_EQ(input_proto.x_stroke_space().scale(), 1.0f);
  EXPECT_EQ(input_proto.y_stroke_space().scale(), 1.0f);
}

TEST_F(StrokeInputBatchTest, EncodeLargeTimeValues) {
  absl::StatusOr<StrokeInputBatch> input_batch =
      StrokeInputBatch::Create({{.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {10, 3},
                                 .elapsed_time = Duration32::Zero(),
                                 .pressure = 0.4,
                                 .tilt = Angle::Radians(1.1),
                                 .orientation = Angle::Radians(2)},
                                {.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {7, 4},
                                 .elapsed_time = Duration32::Seconds(20000.f),
                                 .pressure = 0.3,
                                 .tilt = Angle::Radians(0.7),
                                 .orientation = Angle::Radians(0.9)},
                                {.tool_type = StrokeInput::ToolType::kStylus,
                                 .position = {8, 2},
                                 .elapsed_time = Duration32::Seconds(40000.f),
                                 .pressure = 0.7,
                                 .tilt = Angle::Radians(0.9),
                                 .orientation = Angle::Radians(1.1)}});
  ASSERT_EQ(input_batch.status(), absl::OkStatus());

  CodedStrokeInputBatch input_proto;
  EncodeStrokeInputBatch(*input_batch, input_proto);

  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
      DecodeFloatNumericRun(input_proto.elapsed_time_seconds());
  ASSERT_EQ(float_run.status(), absl::OkStatus());
  EXPECT_THAT(*float_run, ElementsAre(0.f, 20000.f, 40000.f));
}

// Regression test for b/349965543
TEST_F(StrokeInputBatchTest, RoundTripInputsThatQuantizeTheSame) {
  // Create an input batch with all positions the same, but with distinct
  // timestamps, with the first two timestamps very close to each other, and the
  // third very far away.
  StrokeInput input1 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {0, 0},
                        .elapsed_time = Duration32::Seconds(0.f),
                        .pressure = 0.5};
  StrokeInput input2 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {0, 0},
                        .elapsed_time = Duration32::Seconds(1e-30f),
                        .pressure = 0.25};
  StrokeInput input3 = {.tool_type = StrokeInput::ToolType::kStylus,
                        .position = {0, 0},
                        .elapsed_time = Duration32::Seconds(1e30f),
                        .pressure = 0.75};
  absl::StatusOr<StrokeInputBatch> input_batch =
      StrokeInputBatch::Create({input1, input2, input3});
  ASSERT_EQ(input_batch.status(), absl::OkStatus());

  CodedStrokeInputBatch input_proto;
  EncodeStrokeInputBatch(*input_batch, input_proto);

  // Because of quantization in the encoding, the first two timestamps will be
  // encoded as the same value.
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
      DecodeFloatNumericRun(input_proto.elapsed_time_seconds());
  ASSERT_EQ(float_run.status(), absl::OkStatus());
  EXPECT_THAT(*float_run, ElementsAre(0.f, 0.f, 1e30f));

  // Duplicate XYT triples are forbidden by `StrokeInputBatch::Create` (it will
  // return an error), so in order to decode the whole `CodedStrokeInputBatch`
  // successfully, we instead drop the duplicate entry (even though it has a
  // different pressure value). This is unfortunate, but it's better than
  // completely failing to decode a proto that originated from a valid
  // `StrokeInputBatch`, and the encoding is already lossy.
  absl::StatusOr<StrokeInputBatch> decoded =
      DecodeStrokeInputBatch(input_proto);
  ASSERT_EQ(decoded.status(), absl::OkStatus());
  EXPECT_THAT(*decoded,
              ElementsAre(StrokeInputEq(input1), StrokeInputEq(input3)));
}

TEST_F(StrokeInputBatchTest, ClearPressureTiltOrientation) {
  // Start the CodedStrokeInputBatch with non-empty pressure/tilt/orientation,
  // so we can test that those get cleared.
  CodedStrokeInputBatch input_batch;
  input_batch.mutable_pressure()->add_deltas(1);
  input_batch.mutable_tilt()->add_deltas(1);
  input_batch.mutable_orientation()->add_deltas(1);

  // Encode an StrokeInputBatch with no pressure/tilt/orientation data.
  absl::StatusOr<StrokeInputBatch> inputs =
      StrokeInputBatch::Create({{.position = {1.75, 2.25},
                                 .elapsed_time = Duration32::Zero(),
                                 .pressure = StrokeInput::kNoPressure,
                                 .tilt = StrokeInput::kNoTilt,
                                 .orientation = StrokeInput::kNoOrientation}});
  ASSERT_EQ(inputs.status(), absl::OkStatus());
  EncodeStrokeInputBatch(*inputs, input_batch);

  // The CodedStrokeInputBatch should now have position data, but the
  // pressure/tilt/orientation data should have been cleared.
  EXPECT_TRUE(input_batch.has_x_stroke_space());
  EXPECT_TRUE(input_batch.has_y_stroke_space());
  EXPECT_TRUE(input_batch.has_elapsed_time_seconds());
  EXPECT_FALSE(input_batch.has_pressure());
  EXPECT_FALSE(input_batch.has_tilt());
  EXPECT_FALSE(input_batch.has_orientation());
}

void DecodeStrokeInputBatchDoesNotCrashOnArbitraryInput(
    const CodedStrokeInputBatch& proto) {
  DecodeStrokeInputBatch(proto).IgnoreError();
}
FUZZ_TEST(StrokeInputBatchFuzzTest,
          DecodeStrokeInputBatchDoesNotCrashOnArbitraryInput);

void EncodeStrokeInputBatchDoesNotCrashOnArbitraryInput(
    const StrokeInputBatch& inputs) {
  CodedStrokeInputBatch proto;
  EncodeStrokeInputBatch(inputs, proto);
}
FUZZ_TEST(StrokeInputBatchFuzzTest,
          EncodeStrokeInputBatchDoesNotCrashOnArbitraryInput)
    .WithDomains(ArbitraryStrokeInputBatch());

void StrokeInputBatchRoundTrip(const StrokeInputBatch& inputs) {
  CodedStrokeInputBatch proto;
  EncodeStrokeInputBatch(inputs, proto);

  // Any proto that is the result of encoding a valid `StrokeInputBatch` should
  // decode successfully.
  absl::StatusOr<StrokeInputBatch> decoded = DecodeStrokeInputBatch(proto);
  ASSERT_EQ(decoded.status(), absl::OkStatus());
  // Because of quantization, the decoded batch is not guaranteed to exactly
  // match the original.  However, some data should match exactly:
  EXPECT_EQ(decoded->GetToolType(), inputs.GetToolType());
  EXPECT_EQ(decoded->GetStrokeUnitLength(), inputs.GetStrokeUnitLength());
  EXPECT_EQ(decoded->GetNoiseSeed(), inputs.GetNoiseSeed());
}
FUZZ_TEST(StrokeInputBatchFuzzTest, StrokeInputBatchRoundTrip)
    // TODO: b/349965543 - Currently, extreme input position values sometimes
    // get quantized to infinity on encoding, and then don't decode
    // successfully. Once that gets fixed, change this to use
    // `ArbitraryStrokeInputBatch()`.
    .WithDomains(StrokeInputBatchInRect(
        Rect::FromCenterAndDimensions(kOrigin, 1e30f, 1e30f)));

}  // namespace
}  // namespace ink
