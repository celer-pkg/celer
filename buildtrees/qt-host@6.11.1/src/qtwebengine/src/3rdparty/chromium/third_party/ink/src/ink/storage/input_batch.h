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

#ifndef INK_STORAGE_INPUT_BATCH_H_
#define INK_STORAGE_INPUT_BATCH_H_

#include <cstddef>
#include <iterator>
#include <optional>

#include "absl/status/statusor.h"
#include "ink/geometry/point.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/types/duration.h"
#include "ink/types/iterator_range.h"

namespace ink {

// An iterator over the sequence of input points represented by a
// CodedStrokeInputBatch proto. It is expected to be used via the
// DecodeStrokeInputBatchProto function below.
//
// This class conforms to the named requirements of LegacyInputIterator
// (https://en.cppreference.com/w/cpp/named_req/InputIterator).
//
// Note that this is a proxy iterator. The decoded value does not live in the
// CodedStrokeInputBatch, but within the iterator, and so its lifetime is tied
// to the lifetime of the iterator itself. To prevent lifetime issues,
// `reference` is an alias to the value type, not a true reference.
class CodedStrokeInputBatchIterator {
 public:
  struct ValueType {
    Point position_stroke_space;
    Duration32 elapsed_time;
    std::optional<float> pressure;
    std::optional<float> tilt;
    std::optional<float> orientation;
  };

  using difference_type = ptrdiff_t;
  using value_type = ValueType;
  using pointer = const ValueType*;
  using reference = const ValueType;
  using iterator_category = std::input_iterator_tag;

  // NOLINTNEXTLINE - Suppress ClangTidy const-return-type.
  reference operator*() const { return value_; }
  pointer operator->() const { return &value_; }

  CodedStrokeInputBatchIterator() = default;
  CodedStrokeInputBatchIterator(const CodedStrokeInputBatchIterator&) = default;
  CodedStrokeInputBatchIterator& operator=(
      const CodedStrokeInputBatchIterator&) = default;

  CodedStrokeInputBatchIterator& operator++();
  CodedStrokeInputBatchIterator operator++(int) {
    CodedStrokeInputBatchIterator retval = *this;
    ++(*this);
    return retval;
  }

  friend bool operator==(const CodedStrokeInputBatchIterator& lhs,
                         const CodedStrokeInputBatchIterator& rhs) {
    return lhs.x_stroke_space_ == rhs.x_stroke_space_;
  }
  friend bool operator!=(const CodedStrokeInputBatchIterator& lhs,
                         const CodedStrokeInputBatchIterator& rhs) {
    return lhs.x_stroke_space_ != rhs.x_stroke_space_;
  }

 private:
  CodedStrokeInputBatchIterator(
      CodedNumericRunIterator<float> x_stroke_space,
      CodedNumericRunIterator<float> y_stroke_space,
      CodedNumericRunIterator<float> elapsed_time_seconds,
      CodedNumericRunIterator<float> pressure,
      CodedNumericRunIterator<float> tilt,
      CodedNumericRunIterator<float> orientation);

  void UpdateValue();

  CodedNumericRunIterator<float> x_stroke_space_;
  CodedNumericRunIterator<float> y_stroke_space_;
  CodedNumericRunIterator<float> elapsed_time_seconds_;
  CodedNumericRunIterator<float> pressure_;
  CodedNumericRunIterator<float> tilt_;
  CodedNumericRunIterator<float> orientation_;
  ValueType value_;

  friend absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>>
  DecodeStrokeInputBatchProto(const proto::CodedStrokeInputBatch& input);
};

// Given a CodedStrokeInputBatch proto, returns an iterator range over the
// decoded sequence of input points.  The proto object must outlive the returned
// range. Returns an error if the proto is invalid (e.g. if any of its
// constituent numeric runs are invalid or of unequal lengths).
absl::StatusOr<iterator_range<CodedStrokeInputBatchIterator>>
DecodeStrokeInputBatchProto(const proto::CodedStrokeInputBatch& input);

}  // namespace ink

#endif  // INK_STORAGE_INPUT_BATCH_H_
