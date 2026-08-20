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

#ifndef INK_STROKES_INTERNAL_STROKE_OUTLINE_H_
#define INK_STROKES_INTERNAL_STROKE_OUTLINE_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "absl/base/nullability.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"

namespace ink::strokes_internal {

// Helper type used to efficiently build up a span of outline data.
//
// New outline indices can be added to the outline without having to reallocate
// memory for the entire outline on every update.
//
// The outline of a stroke is divided into two sides: "left" and "right". The
// label corresponds to the position of a stroke vertex when viewed from the
// positive z-axis in the direction of travel when the vertex was created.
class StrokeOutline {
 public:
  // A size-type for outline indices.
  struct IndexCounts {
    size_t left = 0;
    size_t right = 0;
  };

  StrokeOutline() = default;
  StrokeOutline(const StrokeOutline&) = delete;
  StrokeOutline(StrokeOutline&&) = default;
  StrokeOutline& operator=(const StrokeOutline&) = delete;
  StrokeOutline& operator=(StrokeOutline&&) = default;
  ~StrokeOutline() = default;

  // Adds new "left" and "right" indices to the outline at the end of the
  // stroke.
  void AppendNewIndices(absl::Span<const uint32_t> new_left_indices,
                        absl::Span<const uint32_t> new_right_indices);

  // Decreases the size of the outline to the values in `counts`.
  //
  // For each side, this operation is a no-op if it already has size smaller
  // than or equal to the relevant value in `counts`.
  void TruncateIndices(IndexCounts counts);

  // Returns the current outline indices.
  //
  // If not empty, the returned span begins at the end of the stroke on the
  // "right" side and ends at the end of the stroke on the "left" side.
  absl::Span<const uint32_t> GetIndices() const;

  // Returns the numbers of "left" and "right" indices in the outline.
  //
  // The sum of the values in the return type will equal the size of the span
  // returned by `GetIndices()`.
  IndexCounts GetIndexCounts() const;

 private:
  // The underlying storage for outline indices.
  //
  // This always has even capacity, 2N for some integer N. The "left" indices
  // are stored first-to-last at [N, N + # of left indices). The "right" indices
  // are stored last-to-first at [N - # of right indices, N). We expect to reuse
  // objects of this type, so we assume that we will need the same amount of
  // space for both "left" and "right" indices on average.
  //
  // We use a `unique_ptr<T[]>` instead of a `vector<T>` so that we can get
  // default-initialized unused capacity at both the start and end of the
  // allocated array.
  struct IndexStorage {
    absl_nonnull std::unique_ptr<uint32_t[]> data;
    size_t capacity = 0;
    IndexCounts used_counts;

    // Returns the number of elements at the start of `data` that are reserved
    // for "right" indices and are currently unused.
    size_t UnusedRightCapacity() const;
    // Returns the number of elements at the end of `data` that are reserved for
    // "left" indices and are currently unused.
    size_t UnusedLeftCapacity() const;

    // Returns the span of used indices according to `used_counts`.
    absl::Span<uint32_t> UsedSpan();
    absl::Span<const uint32_t> UsedSpan() const;
  };

  void GrowIndexStorage(IndexCounts new_index_counts);

  IndexStorage index_storage_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline void StrokeOutline::TruncateIndices(IndexCounts counts) {
  index_storage_.used_counts.left =
      std::min(index_storage_.used_counts.left, counts.left);
  index_storage_.used_counts.right =
      std::min(index_storage_.used_counts.right, counts.right);
}

inline absl::Span<const uint32_t> StrokeOutline::GetIndices() const {
  return index_storage_.UsedSpan();
}

inline StrokeOutline::IndexCounts StrokeOutline::GetIndexCounts() const {
  return index_storage_.used_counts;
}

inline size_t StrokeOutline::IndexStorage::UnusedLeftCapacity() const {
  ABSL_DCHECK_EQ(capacity % 2, 0u);
  ABSL_DCHECK_LE(used_counts.left, capacity / 2);
  return capacity / 2 - used_counts.left;
}

inline size_t StrokeOutline::IndexStorage::UnusedRightCapacity() const {
  ABSL_DCHECK_EQ(capacity % 2, 0u);
  ABSL_DCHECK_LE(used_counts.right, capacity / 2);
  return capacity / 2 - used_counts.right;
}

inline absl::Span<uint32_t> StrokeOutline::IndexStorage::UsedSpan() {
  ABSL_DCHECK_LE(UnusedRightCapacity() + used_counts.left + used_counts.right,
                 capacity);
  return absl::MakeSpan(data.get() + UnusedRightCapacity(),
                        used_counts.left + used_counts.right);
}

inline absl::Span<const uint32_t> StrokeOutline::IndexStorage::UsedSpan()
    const {
  ABSL_DCHECK_LE(UnusedRightCapacity() + used_counts.left + used_counts.right,
                 capacity);
  return absl::MakeSpan(data.get() + UnusedRightCapacity(),
                        used_counts.left + used_counts.right);
}

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_STROKE_OUTLINE_H_
