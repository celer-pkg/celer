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

#include "ink/strokes/internal/stroke_outline.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/numeric/bits.h"
#include "absl/types/span.h"

namespace ink::strokes_internal {

void StrokeOutline::GrowIndexStorage(IndexCounts new_index_counts) {
  StrokeOutline::IndexCounts current_counts = index_storage_.used_counts;
  size_t minimum_new_capacity =
      2 * std::max(current_counts.left + new_index_counts.left,
                   current_counts.right + new_index_counts.right);
  // Grow the storage to the next power-of-2 bytes that fits the new minimum.
  size_t new_capacity = absl::bit_ceil(minimum_new_capacity);

  IndexStorage new_storage = {
      .data = std::make_unique<uint32_t[]>(new_capacity),
//      .data = std::make_unique_for_overwrite<uint32_t[]>(new_capacity),
      .capacity = new_capacity,
      .used_counts = index_storage_.used_counts,
  };

  absl::c_copy(index_storage_.UsedSpan(), new_storage.UsedSpan().begin());
  index_storage_ = std::move(new_storage);
}

void StrokeOutline::AppendNewIndices(
    absl::Span<const uint32_t> new_left_indices,
    absl::Span<const uint32_t> new_right_indices) {
  if (index_storage_.UnusedLeftCapacity() < new_left_indices.size() ||
      index_storage_.UnusedRightCapacity() < new_right_indices.size()) {
    StrokeOutline::IndexCounts new_index_counts = {
        .left = new_left_indices.size(), .right = new_right_indices.size()};
    GrowIndexStorage(new_index_counts);
  }

  // Increment the `used_counts` first so that `index_storage_.UsedSpan().end()`
  // and `.rend()` line up with the boundaries of where to copy the new data.
  index_storage_.used_counts.left += new_left_indices.size();
  index_storage_.used_counts.right += new_right_indices.size();

  absl::c_copy(new_left_indices,
               index_storage_.UsedSpan().end() - new_left_indices.size());
  absl::c_copy(new_right_indices,
               index_storage_.UsedSpan().rend() - new_right_indices.size());
}

}  // namespace ink::strokes_internal
