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

#include "ink/strokes/input/stroke_input_batch.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/strokes/input/internal/stroke_input_validation_helpers.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/types/duration.h"
#include "ink/types/internal/copy_on_write.h"
#include "ink/types/physical_distance.h"

namespace ink {

StrokeInputBatch::ConstIterator& StrokeInputBatch::ConstIterator::operator++() {
  ABSL_DCHECK(!batch_subdata_.empty())
      << "Attempted to dereference singular or past-the-end iterator";
  ABSL_DCHECK_EQ(batch_subdata_.size() % FloatsPerInput(value_), 0);

  batch_subdata_.remove_prefix(FloatsPerInput(value_));
  if (!batch_subdata_.empty()) {
    auto iter = batch_subdata_.begin();
    value_.position = {.x = *iter++, .y = *iter++};
    value_.elapsed_time = Duration32::Seconds(*iter++);
    if (value_.HasPressure()) value_.pressure = *iter++;
    if (value_.HasTilt()) value_.tilt = Angle::Radians(*iter++);
    if (value_.HasOrientation()) value_.orientation = Angle::Radians(*iter++);
  }
  return *this;
}

StrokeInputBatch::ConstIterator StrokeInputBatch::ConstIterator::operator++(
    int) {
  ConstIterator retval = *this;
  ++(*this);
  return retval;
}

void StrokeInputBatch::Clear() {
  if (data_.IsShared()) {
    data_.Reset();
  } else if (data_.HasValue()) {
    data_.MutableValue().clear();
  }

  size_ = 0;
  tool_type_ = StrokeInput::ToolType::kUnknown;
  stroke_unit_length_ = StrokeInput::kNoStrokeUnitLength;
  noise_seed_ = 0;
  has_pressure_ = false;
  has_tilt_ = false;
  has_orientation_ = false;
}

StrokeInputBatch StrokeInputBatch::MakeDeepCopy() const {
  StrokeInputBatch new_batch(*this);
  if (new_batch.data_.HasValue()) {
    new_batch.data_.Emplace(new_batch.data_->begin(), new_batch.data_->end());
  }
  return new_batch;
}

namespace {

using ::ink::stroke_input_internal::ValidateConsecutiveInputs;

// Validates properties of a single `StrokeInput`.
//
// This includes checking that the `input`:
//   * Has a valid `tool_type`
//   * Has all finite floating point values
//   * If they are reported, `pressure`, `tilt` and `orientation` are in their
//     respective valid range
absl::Status ValidateSingleInput(const StrokeInput& input) {
  if (input.tool_type != StrokeInput::ToolType::kUnknown &&
      input.tool_type != StrokeInput::ToolType::kMouse &&
      input.tool_type != StrokeInput::ToolType::kStylus &&
      input.tool_type != StrokeInput::ToolType::kTouch) {
    return absl::InvalidArgumentError(absl::Substitute(
        "`tool_type` must be one of the named enumerators. Got value $0",
        static_cast<int>(input.tool_type)));
  }

  if (!std::isfinite(input.position.x) || !std::isfinite(input.position.y)) {
    return absl::InvalidArgumentError(absl::Substitute(
        "`StrokeInput::position` must be finite. Got: $0", input.position));
  }

  if (!input.elapsed_time.IsFinite() ||
      input.elapsed_time < Duration32::Zero()) {
    return absl::InvalidArgumentError(absl::Substitute(
        "`StrokeInput::elapsed_time` must be finite and non-negative. Got: $0",
        input.elapsed_time.ToSeconds()));
  }

  if (input.HasStrokeUnitLength() &&
      !(input.stroke_unit_length.IsFinite() &&
        input.stroke_unit_length > PhysicalDistance::Zero())) {
    return absl::InvalidArgumentError(
        absl::Substitute("If present, `StrokeInput::stroke_unit_length` must "
                         "be finite and strictly positive. Got: $0",
                         input.stroke_unit_length));
  }

  if (!std::isfinite(input.pressure) ||
      (input.HasPressure() && !(input.pressure >= 0 && input.pressure <= 1))) {
    return absl::InvalidArgumentError(
        absl::Substitute("`StrokeInput::pressure` must be -1 or in the range "
                         "[0, 1]. Got: $0",
                         input.pressure));
  }

  if (!std::isfinite(input.tilt.ValueInRadians()) ||
      (input.HasTilt() &&
       !(input.tilt >= Angle() && input.tilt <= kQuarterTurn))) {
    return absl::InvalidArgumentError(
        absl::Substitute("`StrokeInput::tilt` must be -1 or in the range [0, "
                         "pi / 2]. Got: $0",
                         input.tilt));
  }

  if (!std::isfinite(input.orientation.ValueInRadians()) ||
      (input.HasOrientation() &&
       !(input.orientation >= Angle() && input.orientation <= kFullTurn))) {
    return absl::InvalidArgumentError(
        absl::Substitute("`StrokeInput::orientation` must be -1 or in the "
                         "range [0, 2 * pi). Got: $0",
                         input.orientation));
  }

  return absl::OkStatus();
}

absl::Status ValidateInputSequence(absl::Span<const StrokeInput> inputs) {
  if (inputs.empty()) return absl::OkStatus();
  absl::Status status = ValidateSingleInput(inputs[0]);
  if (!status.ok()) {
    return status;
  }
  for (size_t i = 1; i < inputs.size(); ++i) {
    if (status = ValidateSingleInput(inputs[i]); !status.ok()) {
      return status;
    }
    if (status = ValidateConsecutiveInputs(inputs[i - 1], inputs[i]);
        !status.ok()) {
      return status;
    }
  }

  return absl::OkStatus();
}

}  // namespace

void StrokeInputBatch::SetInlineFormatMetadata(const StrokeInput& input) {
  ABSL_DCHECK(IsEmpty());
  tool_type_ = input.tool_type;
  stroke_unit_length_ = input.stroke_unit_length;
  has_pressure_ = input.HasPressure();
  has_tilt_ = input.HasTilt();
  has_orientation_ = input.HasOrientation();
}

namespace {

void AppendInputToFloatVector(const StrokeInput& input,
                              std::vector<float>& data) {
  data.push_back(input.position.x);
  data.push_back(input.position.y);
  data.push_back(input.elapsed_time.ToSeconds());
  if (input.HasPressure()) data.push_back(input.pressure);
  if (input.HasTilt()) data.push_back(input.tilt.ValueInRadians());
  if (input.HasOrientation()) {
    data.push_back(input.orientation.ValueInRadians());
  }
}

}  // namespace

absl::Status StrokeInputBatch::Set(size_t i, const StrokeInput& input) {
  ABSL_CHECK_LT(i, Size());
  absl::Status status = ValidateSingleInput(input);
  if (!status.ok()) {
    return status;
  }

  if (Size() == 1) {
    Clear();
    if (!data_.HasValue()) data_.Emplace();
    SetInlineFormatMetadata(input);
    AppendInputToFloatVector(input, data_.MutableValue());
    size_ = 1;
    return absl::OkStatus();
  }

  if (i > 0) {
    if (status = ValidateConsecutiveInputs(Get(i - 1), input); !status.ok()) {
      return status;
    }
  }
  if (i + 1 < Size()) {
    if (status = ValidateConsecutiveInputs(input, Get(i + 1)); !status.ok()) {
      return status;
    }
  }

  auto data =
      absl::MakeSpan(data_.MutableValue()).subspan(i * FloatsPerInput());
  auto iter = data.begin();
  *iter++ = input.position.x;
  *iter++ = input.position.y;
  *iter++ = input.elapsed_time.ToSeconds();
  if (HasPressure()) *iter++ = input.pressure;
  if (HasTilt()) *iter++ = input.tilt.ValueInRadians();
  if (HasOrientation()) *iter++ = input.orientation.ValueInRadians();

  return absl::OkStatus();
}

StrokeInput StrokeInputBatch::Get(size_t i) const {
  ABSL_CHECK_LT(i, Size());

  auto data = absl::MakeSpan(data_.Value()).subspan(i * FloatsPerInput());
  auto iter = data.begin();
  return {.tool_type = tool_type_,
          .position = {.x = *iter++, .y = *iter++},
          .elapsed_time = Duration32::Seconds(*iter++),
          .stroke_unit_length = stroke_unit_length_,
          .pressure = HasPressure() ? *iter++ : StrokeInput::kNoPressure,
          .tilt = HasTilt() ? Angle::Radians(*iter++) : StrokeInput::kNoTilt,
          .orientation = HasOrientation() ? Angle::Radians(*iter++)
                                          : StrokeInput::kNoOrientation};
}

absl::Status StrokeInputBatch::Append(const StrokeInput& input) {
  absl::Status status = ValidateSingleInput(input);
  if (!status.ok()) {
    return status;
  }

  if (!IsEmpty()) {
    if (status = ValidateConsecutiveInputs(Get(Size() - 1), input);
        !status.ok()) {
      return status;
    }
  } else {
    if (!data_.HasValue()) data_.Emplace();
    SetInlineFormatMetadata(input);
  }

  AppendInputToFloatVector(input, data_.MutableValue());
  ++size_;

  return absl::OkStatus();
}

absl::Status StrokeInputBatch::Append(absl::Span<const StrokeInput> inputs) {
  if (inputs.empty()) {
    return absl::OkStatus();
  }

  absl::Status status = ValidateInputSequence(inputs);
  if (!status.ok()) {
    return status;
  }
  if (!IsEmpty()) {
    if (status = ValidateConsecutiveInputs(Get(Size() - 1), inputs.front());
        !status.ok()) {
      return status;
    }
  } else {
    if (!data_.HasValue()) data_.Emplace();
    SetInlineFormatMetadata(inputs.front());
  }

  // We don't call `vector::reserve` on purpose. Depending on the STL
  // implementation, it could degrade performance given the expectation that
  // this function will be called repeatedly with relatively small batches of
  // new inputs.

  std::vector<float>& data = data_.MutableValue();
  for (const StrokeInput& input : inputs) {
    AppendInputToFloatVector(input, data);
  }
  size_ += inputs.size();

  return absl::OkStatus();
}

absl::StatusOr<StrokeInputBatch> StrokeInputBatch::Create(
    absl::Span<const StrokeInput> inputs, uint32_t noise_seed) {
  StrokeInputBatch batch;

  if (!inputs.empty()) {
    batch.data_.Emplace().reserve(inputs.size() *
                                  FloatsPerInput(inputs.front()));
    if (absl::Status status = batch.Append(inputs); !status.ok()) {
      return status;
    }
  }

  batch.SetNoiseSeed(noise_seed);
  return batch;
}

absl::Status StrokeInputBatch::Append(const StrokeInputBatch& inputs) {
  if (inputs.IsEmpty()) return absl::OkStatus();

  if (IsEmpty()) {
    *this = inputs;
    return absl::OkStatus();
  }

  if (absl::Status status =
          ValidateConsecutiveInputs(Get(Size() - 1), inputs.Get(0));
      !status.ok()) {
    return status;
  }

  // We don't call `vector::reserve` on purpose. Depending on the STL
  // implementation, it could degrade performance given the expectation that
  // this function will be called repeatedly with relatively small batches of
  // new inputs.

  std::vector<float>& data = data_.MutableValue();
  const std::vector<float>& append_data = inputs.data_.Value();
  data.insert(data.end(), append_data.begin(), append_data.end());
  size_ += inputs.Size();

  return absl::OkStatus();
}

void StrokeInputBatch::Erase(size_t start, size_t count) {
  ABSL_CHECK_LE(start, Size());

  count = std::min(count, Size() - start);
  if (count == 0) return;
  if (start == 0 && count == Size()) {
    Clear();
    return;
  }

  std::vector<float>& data = data_.MutableValue();
  int stride = FloatsPerInput();
  data.erase(data.begin() + start * stride,
             data.begin() + (start + count) * stride);
  size_ -= count;
}

Duration32 StrokeInputBatch::GetDuration() const {
  if (IsEmpty()) return Duration32::Zero();
  return Get(Size() - 1).elapsed_time - Get(0).elapsed_time;
}

void StrokeInputBatch::Transform(const AffineTransform& transform,
                                 TransformInvariant invariant) {
  if (IsEmpty()) return;
  // TODO: b/278536966 - add PreservingVelocity invariant and function when
  // ready
  TransformPreservingDuration(transform);
}

void StrokeInputBatch::TransformPreservingDuration(
    const AffineTransform& transform) {
  auto data = absl::MakeSpan(data_.MutableValue());
  int stride = FloatsPerInput();
  for (auto iter = data.begin(); iter != data.end(); iter += stride) {
    Point old_position = {*iter, *(iter + 1)};
    Point new_position = transform.Apply(old_position);
    *iter = new_position.x;
    *(iter + 1) = new_position.y;
  }
}

std::string StrokeInputBatch::ToFormattedString() const {
  return absl::StrCat("StrokeInputBatch[", absl::StrJoin(begin(), end(), ", "),
                      "]");
}

}  // namespace ink
