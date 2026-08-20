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

#ifndef INK_TYPES_SMALL_ARRAY_H_
#define INK_TYPES_SMALL_ARRAY_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>

#include "absl/algorithm/container.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"

namespace ink {

// A container that does not allocate, stores its data contiguously, supports
// random access, and has a cap on the number of elements it may hold.
//
// The number of elements must be between 0 and `N`, inclusive. A `SmallArray`
// is always allocated with space for `N` elements, even if it actually contains
// fewer than that. However, since `N` is known at compile-time, this allows the
// `SmallArray` to be entirely stack-allocated if the stored type `T` does not
// allocate any memory.
//
// This differs from `std::array` in that `std::array` is always considered to
// have `N` elements; this may be used when the maximum number of elements is
// known at compile-time, but the actual number of elements is not.
template <typename T, uint8_t N>
class SmallArray {
 public:
  // Constructs an empty container.
  SmallArray() = default;

  // Constructs a container with `count` elements, all initialized to `value`.
  // DCHECK-fails if `count` > `N`, which results in undefined behavior in prod.
  explicit SmallArray(uint8_t count, const T& value = T()) : size_(count) {
    ABSL_DCHECK_LE(count, N);
    std::fill(data_.begin(), data_.begin() + size_, value);
  }

  // Constructs a container populated by `values`. DCHECK-fails if
  // `values.size()` > `N`, which results in undefined behavior in prod.
  SmallArray(std::initializer_list<T> values) : size_(values.size()) {
    ABSL_DCHECK_LE(values.size(), N);
    std::copy(values.begin(), values.end(), data_.begin());
  }
  explicit SmallArray(absl::Span<const T> values) : size_(values.size()) {
    ABSL_DCHECK_LE(values.size(), N);
    std::copy(values.begin(), values.end(), data_.begin());
  }

  SmallArray(const SmallArray&) = default;
  SmallArray(SmallArray&&) = default;
  SmallArray& operator=(const SmallArray&) = default;
  SmallArray& operator=(SmallArray&&) = default;
  ~SmallArray() = default;

  // Resizes the container to hold `new_size` elements.
  //
  // If `new_size` is greater than the previous size, new elements will be
  // initialized to `value`. Otherwise, if `new_size` is smaller, any excess
  // elements will be reassigned to `T()`.
  //
  // DCHECK-fails if `new_size` > N, which results in undefined behavior in
  // prod.
  void Resize(uint8_t new_size, const T& value = T()) {
    ABSL_DCHECK_LE(new_size, N);
    if (new_size == size_) return;
    if (new_size > size_) {
      std::fill(data_.begin() + size_, data_.begin() + new_size, value);
    } else {
      std::fill(data_.begin() + new_size, data_.begin() + size_, T());
    }
    size_ = new_size;
  }

  // Fetches a span of the elements in the container.
  absl::Span<T> Values() { return absl::MakeSpan(data_.data(), size_); }
  absl::Span<const T> Values() const {
    return absl::MakeSpan(data_.data(), size_);
  }

  // Random access to the elements in the container. DCHECK-fails if `index` >=
  // `Size()`, which results in undefined behavior in prod.
  T& operator[](uint8_t index) {
    ABSL_DCHECK_LT(index, size_);
    return data_[index];
  }
  const T& operator[](uint8_t index) const {
    ABSL_DCHECK_LT(index, size_);
    return data_[index];
  }

  // Returns the number of elements in the container.
  uint8_t Size() const { return size_; }

  // Returns the maximum number of elements the container may hold, which is
  // equal to `N`.
  uint8_t MaxSize() const { return N; }

 private:
  uint8_t size_ = 0;
  // We use a `std::array` for the underlying storage instead of an aligned
  // array of `char`. This keeps the implementation simpler and also this type
  // will generally be used with very small `N` and trivial `T`.
  std::array<T, N> data_;
};

template <typename T, uint8_t N>
bool operator==(SmallArray<T, N> lhs, SmallArray<T, N> rhs) {
  return absl::c_equal(lhs.Values(), rhs.Values());
}

}  // namespace ink

#endif  // INK_TYPES_SMALL_ARRAY_H_
