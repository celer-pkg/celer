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

#include <cstddef>
#include <optional>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/types/iterator_range.h"

namespace ink {

CodedStrokeInputBatchIterator::CodedStrokeInputBatchIterator(
    CodedNumericRunIterator<float> x_stroke_space,
    CodedNumericRunIterator<float> y_stroke_space,
    CodedNumericRunIterator<float> elapsed_time_seconds,
    CodedNumericRunIterator<float> pressure,
    CodedNumericRunIterator<float> tilt,
    CodedNumericRunIterator<float> orientation)
    : x_stroke_space_(x_stroke_space),
      y_stroke_space_(y_stroke_space),
      elapsed_time_seconds_(elapsed_time_seconds),
      pressure_(pressure),
      tilt_(tilt),
      orientation_(orientation) {
  UpdateValue();
}

CodedStrokeInputBatchIterator& CodedStrokeInputBatchIterator::operator++() {
  ++x_stroke_space_;
  ++y_stroke_space_;
  ++elapsed_time_seconds_;
  if (pressure_.HasValue()) {
    ++pressure_;
  }
  if (tilt_.HasValue()) {
    ++tilt_;
  }
  if (orientation_.HasValue()) {
    ++orientation_;
  }
  UpdateValue();
  return *this;
}

void CodedStrokeInputBatchIterator::UpdateValue() {
  if (!x_stroke_space_.HasValue()) return;
  value_.position_stroke_space.x = *x_stroke_space_;
  value_.position_stroke_space.y = *y_stroke_space_;
  value_.elapsed_time = Duration32::Seconds(*elapsed_time_seconds_);
  if (pressure_.HasValue()) {
    value_.pressure = *pressure_;
  }
  if (tilt_.HasValue()) {
    value_.tilt = *tilt_;
  }
  if (orientation_.HasValue()) {
    value_.orientation = *orientation_;
  }
}

absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>>
DecodeStrokeInputBatchProto(const proto::CodedStrokeInputBatch& input) {
  size_t num_input_points = input.x_stroke_space().deltas_size();
  if (input.y_stroke_space().deltas_size() != num_input_points ||
      input.elapsed_time_seconds().deltas_size() != num_input_points ||
      (input.has_pressure() &&
       input.pressure().deltas_size() != num_input_points) ||
      (input.has_tilt() && input.tilt().deltas_size() != num_input_points) ||
      (input.has_orientation() &&
       input.orientation().deltas_size() != num_input_points)) {
    return absl::InvalidArgumentError(
        "invalid StrokeInputBatch: mismatched numeric run lengths");
  }

  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
      x_stroke_space = DecodeFloatNumericRun(input.x_stroke_space());
  if (!x_stroke_space.ok()) {
    return x_stroke_space.status();
  }
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
      y_stroke_space = DecodeFloatNumericRun(input.y_stroke_space());
  if (!y_stroke_space.ok()) {
    return y_stroke_space.status();
  }
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
      elapsed_time_seconds =
          DecodeFloatNumericRun(input.elapsed_time_seconds());
  if (!elapsed_time_seconds.ok()) {
    return elapsed_time_seconds.status();
  }

  iterator_range<CodedNumericRunIterator<float>> pressure;
  if (input.has_pressure()) {
    auto result = DecodeFloatNumericRun(input.pressure());
    if (!result.ok()) {
      return result.status();
    }
    pressure = *result;
  }

  iterator_range<CodedNumericRunIterator<float>> tilt;
  if (input.has_tilt()) {
    auto result = DecodeFloatNumericRun(input.tilt());
    if (!result.ok()) {
      return result.status();
    }
    tilt = *result;
  }

  iterator_range<CodedNumericRunIterator<float>> orientation;
  if (input.has_orientation()) {
    auto result = DecodeFloatNumericRun(input.orientation());
    if (!result.ok()) {
      return result.status();
    }
    orientation = *result;
  }

  return iterator_range<CodedStrokeInputBatchIterator>{
      CodedStrokeInputBatchIterator(
          x_stroke_space->begin(), y_stroke_space->begin(),
          elapsed_time_seconds->begin(), pressure.begin(), tilt.begin(),
          orientation.begin()),
      CodedStrokeInputBatchIterator(x_stroke_space->end(),
                                    y_stroke_space->end(),
                                    elapsed_time_seconds->end(), pressure.end(),
                                    tilt.end(), orientation.end())};
}

}  // namespace ink
