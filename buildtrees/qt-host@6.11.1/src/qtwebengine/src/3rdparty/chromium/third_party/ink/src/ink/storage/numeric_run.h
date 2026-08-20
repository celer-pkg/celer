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

#ifndef INK_STORAGE_NUMERIC_RUN_H_
#define INK_STORAGE_NUMERIC_RUN_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>

#include "absl/status/statusor.h"
#include "ink/storage/proto/coded_numeric_run.pb.h"
#include "ink/types/iterator_range.h"

namespace ink {

// An iterator over the sequence of values represented by a CodedNumericRun
// proto. It is expected to be used via the DecodeFloatNumericRun and
// DecodeIntNumericRun functions below.
//
// This class conforms to the named requirements of LegacyInputIterator
// (https://en.cppreference.com/w/cpp/named_req/InputIterator).
//
// Note that this is a proxy iterator. The decoded value does not live in the
// CodedNumericRun, but within the iterator, and so its lifetime is tied to the
// lifetime of the iterator itself. To prevent lifetime issues, `reference` is
// an alias to the value type, not a true reference.
template <typename T>
class CodedNumericRunIterator {
 public:
  using difference_type = ptrdiff_t;
  using value_type = T;
  using pointer = const T*;
  using reference = const T;
  using iterator_category = std::input_iterator_tag;

  // NOLINTNEXTLINE - Suppress ClangTidy const-return-type.
  reference operator*() const { return value_; }

  CodedNumericRunIterator() : CodedNumericRunIterator(nullptr, 0) {}
  CodedNumericRunIterator(const CodedNumericRunIterator&) = default;
  CodedNumericRunIterator& operator=(const CodedNumericRunIterator&) = default;

  CodedNumericRunIterator& operator++() {
    ++index_;
    UpdateDeltaAndValue();
    return *this;
  }

  CodedNumericRunIterator operator++(int) {
    CodedNumericRunIterator retval = *this;
    ++(*this);
    return retval;
  }

  friend bool operator==(const CodedNumericRunIterator& lhs,
                         const CodedNumericRunIterator& rhs) {
    return lhs.index_ == rhs.index_ && lhs.run_ == rhs.run_;
  }
  friend bool operator!=(const CodedNumericRunIterator& lhs,
                         const CodedNumericRunIterator& rhs) {
    return !(lhs == rhs);
  }

  // Returns true iff this iterator can be dereferenced.
  bool HasValue() const {
    return run_ != nullptr && index_ < run_->deltas_size();
  }

 private:
  CodedNumericRunIterator(const proto::CodedNumericRun* run, size_t index)
      : run_(run), index_(index), value_() {
    UpdateDeltaAndValue();
  }

  void UpdateDeltaAndValue() {
    if (!HasValue()) return;
    cumulative_delta_ += run_->deltas(index_);
    if constexpr (std::numeric_limits<T>::is_integer) {
      int64_t value = static_cast<int64_t>(run_->offset()) +
                      static_cast<int64_t>(run_->scale()) * cumulative_delta_;
      value_ = static_cast<T>(std::clamp<int64_t>(
          value, static_cast<int64_t>(std::numeric_limits<T>::min()),
          static_cast<int64_t>(std::numeric_limits<T>::max())));
    } else {
      value_ =
          static_cast<T>(run_->offset()) +
          static_cast<T>(run_->scale()) * static_cast<T>(cumulative_delta_);
    }
  }

  const proto::CodedNumericRun* run_;
  int64_t cumulative_delta_ = 0;
  size_t index_;
  T value_;

  friend absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
  DecodeFloatNumericRun(const proto::CodedNumericRun& run);
  friend absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>>
  DecodeIntNumericRun(const proto::CodedNumericRun& run);
};

// Given a CodedNumericRun proto representing a sequence of floating point
// numbers, returns an iterator range over the decoded sequence.  The proto
// object must outlive the returned range.  Returns an error if the proto is
// invalid (e.g. if it has non-finite offset/scale values).
absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
DecodeFloatNumericRun(const proto::CodedNumericRun& run);

// Given a CodedNumericRun proto representing a sequence of integers, returns an
// iterator range over the decoded sequence.  The proto object must outlive the
// returned range.  Returns an error if the proto is invalid (e.g. if it has
// non-integral offset/scale values).
absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>>
DecodeIntNumericRun(const proto::CodedNumericRun& run);

// Given a pair of iterators defining a sequence of integers, populates the
// given CodedNumericRun proto to encode that sequence.
template <typename InputIter>
void EncodeIntNumericRun(InputIter begin, InputIter end,
                         proto::CodedNumericRun* out) {
  out->clear_offset();
  out->clear_scale();
  out->clear_deltas();
  out->mutable_deltas()->Reserve(std::distance(begin, end));
  int32_t previous = 0;
  while (begin != end) {
    int32_t value = *begin;
    out->add_deltas(value - previous);
    previous = value;
    ++begin;
  }
}

}  // namespace ink

#endif  // INK_STORAGE_NUMERIC_RUN_H_
