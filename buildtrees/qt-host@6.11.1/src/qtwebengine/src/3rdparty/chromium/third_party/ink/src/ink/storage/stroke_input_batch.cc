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

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/rect.h"
#include "ink/storage/input_batch.h"
#include "ink/storage/proto/coded_numeric_run.pb.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/iterator_range.h"
#include "ink/types/physical_distance.h"

namespace ink {

using ::ink::proto::CodedNumericRun;
using ::ink::proto::CodedStrokeInputBatch;

// These values were picked out of a hat, and should probably be tuned in the
// future.
constexpr float kInverseEnvelopeXScale = 4096;
constexpr float kInverseEnvelopeYScale = 4096;
constexpr float kDefaultInverseTimeScale = 1e6;  // microsecond resolution
constexpr float kInversePressureScale = 4096;
constexpr float kInverseTiltScale = 4096;
constexpr float kInverseOrientationScale = 4096;

namespace {

CodedStrokeInputBatch::ToolType ToProtoToolType(StrokeInput::ToolType type) {
  switch (type) {
    case StrokeInput::ToolType::kMouse:
      return CodedStrokeInputBatch::MOUSE;
    case StrokeInput::ToolType::kTouch:
      return CodedStrokeInputBatch::TOUCH;
    case StrokeInput::ToolType::kStylus:
      return CodedStrokeInputBatch::STYLUS;
    case StrokeInput::ToolType::kUnknown:
    default:
      return CodedStrokeInputBatch::UNKNOWN_TYPE;
  }
}

}  // namespace

void EncodeStrokeInputBatch(const StrokeInputBatch& input_batch,
                            CodedStrokeInputBatch& input_proto) {
  if (input_batch.Size() == 0) {
    input_proto.Clear();
    input_proto.set_noise_seed(input_batch.GetNoiseSeed());
    return;
  }
  // Determine the envelope for the input positions and the maximum input time
  // value.
  Rect stroke_space_bounds =
      Rect::FromCenterAndDimensions(input_batch.Get(0).position, 0, 0);
  float elapsed_time_seconds_max = 0.0f;
  for (auto input : input_batch) {
    stroke_space_bounds.Join(input.position);
    elapsed_time_seconds_max =
        std::fmax(elapsed_time_seconds_max, input.elapsed_time.ToSeconds());
  }

  // The encoded x-positions are offset and scaled relative to the envelope
  // calculated above, since the envelope's absolute size and position can vary
  // wildly depending on the client's definition of stroke space.
  //
  // We take some care here to avoid float overflow while calculating
  // inverse_x_scale (this is enforced by fuzz testing).
  CodedNumericRun* x_stroke_space = input_proto.mutable_x_stroke_space();
  const float bounds_semi_width = stroke_space_bounds.SemiWidth();
  const float inverse_x_scale =
      bounds_semi_width > 0.f
          ? (0.5f * kInverseEnvelopeXScale) / bounds_semi_width
          : 1.f;
  x_stroke_space->set_scale(1.f / inverse_x_scale);
  x_stroke_space->set_offset(stroke_space_bounds.XMin());
  x_stroke_space->mutable_deltas()->Clear();
  x_stroke_space->mutable_deltas()->Reserve(input_batch.Size());

  // Likewise, the encoded Y-positions are also offset and scaled to the
  // envelope calculated above.
  CodedNumericRun* y_stroke_space = input_proto.mutable_y_stroke_space();
  const float bounds_semi_height = stroke_space_bounds.SemiHeight();
  const float inverse_y_scale =
      bounds_semi_height > 0.f
          ? (0.5f * kInverseEnvelopeYScale) / bounds_semi_height
          : 1.f;
  y_stroke_space->set_scale(1.f / inverse_y_scale);
  y_stroke_space->set_offset(stroke_space_bounds.YMin());
  y_stroke_space->mutable_deltas()->Clear();
  y_stroke_space->mutable_deltas()->Reserve(input_batch.Size());

  // In most cases, we can use a fixed offset/scale for time, since the
  // stroke-relative input times should always start at zero, and in practice
  // will almost always have an upper bound somewhere within a fraction of a
  // second to a minute (i.e. a variation of a couple of orders of magnitude at
  // most, rather than an arbitrary number).
  //
  // However, if for some reason the maximum time value is larger than expected,
  // we need to calculate a less-precise scaling factor to prevent float-to-int
  // conversion from overflowing.  (Using INT_MAX in the below calculation
  // doesn't prevent the overflow problem because of float rounding, so instead
  // we use INT_MAX/2 to give ourselves some headroom there.)
  float inverse_time_scale =
      elapsed_time_seconds_max > 0.f
          ? std::fmin(
                kDefaultInverseTimeScale,
                static_cast<float>(std::numeric_limits<int32_t>::max() / 2) /
                    elapsed_time_seconds_max)
          : kDefaultInverseTimeScale;
  CodedNumericRun* elapsed_time_seconds =
      input_proto.mutable_elapsed_time_seconds();
  elapsed_time_seconds->set_scale(1.f / inverse_time_scale);
  elapsed_time_seconds->clear_offset();
  elapsed_time_seconds->mutable_deltas()->Clear();
  elapsed_time_seconds->mutable_deltas()->Reserve(input_batch.Size());

  // If the input_batch doesn't have pressure data, then we can omit pressure
  // data from the CodedStrokeInputBatch, clearing possible existing data.
  // Otherwise, set up for recording the pressure data.
  CodedNumericRun* pressure = nullptr;
  if (!input_batch.HasPressure()) {
    input_proto.clear_pressure();
  } else {
    pressure = input_proto.mutable_pressure();
    // Pressure values always range from 0 to 1, so we can just use a fixed
    // offset/scale for the CodedNumericRun.
    pressure->set_scale(1.f / kInversePressureScale);
    pressure->clear_offset();
    pressure->mutable_deltas()->Clear();
    pressure->mutable_deltas()->Reserve(input_batch.Size());
  }

  // If the input_batch doesn't have tilt data, then we can omit tilt
  // data from the CodedStrokeInputBatch, clearing possible existing data.
  // Otherwise, set up for recording the tilt data.
  CodedNumericRun* tilt = nullptr;
  if (!input_batch.HasTilt()) {
    input_proto.clear_tilt();
  } else {
    tilt = input_proto.mutable_tilt();
    // Tilt values always range from 0 to pi/2, so we can just use a fixed
    // offset/scale for the CodedNumericRun.
    tilt->set_scale(1.f / kInverseTiltScale);
    tilt->clear_offset();
    tilt->mutable_deltas()->Clear();
    tilt->mutable_deltas()->Reserve(input_batch.Size());
  }

  // If the input_batch doesn't have orientation data, then we can omit
  // orientation data from the CodedStrokeInputBatch, clearing possible existing
  // data. Otherwise, set up for recording the orientation data.
  CodedNumericRun* orientation = nullptr;
  if (!input_batch.HasOrientation()) {
    input_proto.clear_orientation();
  } else {
    orientation = input_proto.mutable_orientation();
    // Orientation values always range from 0 to 2pi, so we can just use a fixed
    // offset/scale for the CodedNumericRun.
    orientation->set_scale(1.f / kInverseOrientationScale);
    orientation->clear_offset();
    orientation->mutable_deltas()->Clear();
    orientation->mutable_deltas()->Reserve(input_batch.Size());
  }

  // Make another pass over the input data, delta-encoding the positions and
  // times for the input points and, where applicable, pressure, tilt and
  // orientation.
  const float scaled_x_origin = stroke_space_bounds.XMin() * inverse_x_scale;
  const float scaled_y_origin = stroke_space_bounds.YMin() * inverse_y_scale;
  int last_int_x = 0;
  int last_int_y = 0;
  int32_t last_int_time = 0;
  int last_int_pressure = 0;
  int last_int_tilt = 0;
  int last_int_orientation = 0;
  for (auto input : input_batch) {
    int int_x =
        static_cast<int>(input.position.x * inverse_x_scale - scaled_x_origin);
    int int_y =
        static_cast<int>(input.position.y * inverse_y_scale - scaled_y_origin);
    x_stroke_space->add_deltas(int_x - last_int_x);
    y_stroke_space->add_deltas(int_y - last_int_y);
    last_int_x = int_x;
    last_int_y = int_y;

    int32_t int_time = static_cast<int32_t>(input.elapsed_time.ToSeconds() *
                                            inverse_time_scale);
    elapsed_time_seconds->add_deltas(int_time - last_int_time);
    last_int_time = int_time;

    if (input_batch.HasPressure()) {
      int int_pressure =
          static_cast<int>(input.pressure * kInversePressureScale);
      pressure->add_deltas(int_pressure - last_int_pressure);
      last_int_pressure = int_pressure;
    }
    if (input_batch.HasTilt()) {
      int int_tilt =
          static_cast<int>(input.tilt.ValueInRadians() * kInverseTiltScale);
      tilt->add_deltas(int_tilt - last_int_tilt);
      last_int_tilt = int_tilt;
    }
    if (input_batch.HasOrientation()) {
      int int_orientation = static_cast<int>(
          input.orientation.ValueInRadians() * kInverseOrientationScale);
      orientation->add_deltas(int_orientation - last_int_orientation);
      last_int_orientation = int_orientation;
    }
  }

  input_proto.set_tool_type(ToProtoToolType(input_batch.GetToolType()));
  if (std::optional<PhysicalDistance> stroke_unit_length =
          input_batch.GetStrokeUnitLength();
      stroke_unit_length.has_value()) {
    input_proto.set_stroke_unit_length_in_centimeters(
        stroke_unit_length->ToCentimeters());
  }
  input_proto.set_noise_seed(input_batch.GetNoiseSeed());
}

namespace {

StrokeInput::ToolType ToStrokeInputToolType(
    CodedStrokeInputBatch::ToolType type) {
  switch (type) {
    case CodedStrokeInputBatch::MOUSE:
      return StrokeInput::ToolType::kMouse;
    case CodedStrokeInputBatch::TOUCH:
      return StrokeInput::ToolType::kTouch;
    case CodedStrokeInputBatch::STYLUS:
      return StrokeInput::ToolType::kStylus;
    case CodedStrokeInputBatch::UNKNOWN_TYPE:
    default:
      return StrokeInput::ToolType::kUnknown;
  }
}

}  // namespace

absl::StatusOr<StrokeInputBatch> DecodeStrokeInputBatch(
    const CodedStrokeInputBatch& input_proto) {
  absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>> range =
      DecodeStrokeInputBatchProto(input_proto);
  if (!range.ok()) {
    return range.status();
  }

  StrokeInput::ToolType tool_type =
      ToStrokeInputToolType(input_proto.tool_type());
  PhysicalDistance stroke_unit_length = PhysicalDistance::Centimeters(
      input_proto.stroke_unit_length_in_centimeters());

  StrokeInputBatch batch;
  // TODO: b/355637257 - Add a StrokeInputBatch::Reserve member function.
  for (const auto& input : *range) {
    if (!batch.IsEmpty()) {
      StrokeInput previous = batch.Get(batch.Size() - 1);
      if (input.position_stroke_space == previous.position &&
          input.elapsed_time == previous.elapsed_time) {
        continue;
      }
    }

    absl::Status status = batch.Append(
        {.tool_type = tool_type,
         .position = input.position_stroke_space,
         .elapsed_time = input.elapsed_time,
         .stroke_unit_length = stroke_unit_length,
         .pressure = input.pressure.value_or(StrokeInput::kNoPressure),
         .tilt = input.tilt.has_value() ? Angle::Radians(*input.tilt)
                                        : StrokeInput::kNoTilt,
         .orientation = input.orientation.has_value()
                            ? Angle::Radians(*input.orientation)
                            : StrokeInput::kNoOrientation});
    if (!status.ok()) {
      return status;
    }
  }
  batch.SetNoiseSeed(input_proto.noise_seed());
  return batch;
}

}  // namespace ink
