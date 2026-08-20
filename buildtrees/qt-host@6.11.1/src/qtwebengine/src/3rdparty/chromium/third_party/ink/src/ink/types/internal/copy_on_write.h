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

#ifndef INK_TYPES_INTERNAL_COPY_ON_WRITE_H_
#define INK_TYPES_INTERNAL_COPY_ON_WRITE_H_

#include <memory>
#include <type_traits>
#include <utility>

#include "absl/base/nullability.h"
#include "absl/log/absl_check.h"

namespace ink_internal {

// Container that manages an optional value of copy-constructible type `T` and
// provides copy-on-write semantics.
//
// The API resembles that of `std::optional` with the following key
// distinctions:
//   * The managed object is heap-allocated.
//   * The usual accessors always return read-only references and pointers to
//     the managed object.
//   * Getting a mutable reference to an already held value requires calling
//     `MutableValue()`.
//   * The moved-from state is guaranteed to be empty and equivalent to the
//     default-constructed state.
//
// Similarly to `std::optional`, a mutable reference to a newly created value is
// returned by `Emplace()`.
//
template <typename T>
class CopyOnWrite {
 public:
  static_assert(std::is_copy_constructible_v<T>);

  // Constructs a `CopyOnWrite` holding the given `value`.
  explicit CopyOnWrite(const T& value) : value_(std::make_shared<T>(value)) {}
  explicit CopyOnWrite(T&& value)
      : value_(std::make_shared<T>(std::move(value))) {}

  CopyOnWrite() = default;
  CopyOnWrite(const CopyOnWrite&) = default;
  CopyOnWrite(CopyOnWrite&&) = default;
  CopyOnWrite& operator=(const CopyOnWrite&) = default;
  CopyOnWrite& operator=(CopyOnWrite&&) = default;
  ~CopyOnWrite() = default;

  // Allocates a new managed object that is direct-initialized with `args`, and
  // returns a mutable reference to the newly managed object.
  template <typename... Args>
  T& Emplace(Args&&... args) {
    value_ = std::make_shared<T>(std::forward<Args>(args)...);
    return *value_;
  }

  // Resets the `CopyOnWrite` to the empty state. Any managed object is
  // destroyed if and only if it is not shared.
  void Reset() { value_.reset(); }

  bool HasValue() const { return value_ != nullptr; }

  // Returns true if the `CopyOnWrite` contains a value and the managed object
  // is shared with at least one other `CopyOnWrite` object.
  bool IsShared() const { return value_.use_count() > 1; }

  // Returns a mutable reference to the managed object.
  //
  // Check-fails if `HasValue()` is `false`. If `IsShared()` is `true`, this
  // function first creates a new copy of the managed object.
  T& MutableValue() {
    ABSL_CHECK_NE(value_, nullptr);
    if (IsShared()) Emplace(*value_);
    return *value_;
  }

  // Returns a read-only reference to the managed object. Check-fails if
  // `HasValue()` is `false`.
  const T& Value() const {
    ABSL_CHECK(HasValue());
    return *value_;
  }

  // Returns a read-only reference to the managed object. Behavior is undefined
  // if `HasValue()` returns `false`; CHECK-fails in debug.
  const T& operator*() const {
    ABSL_DCHECK(HasValue());
    return *value_;
  }

  // Returns a read-only pointer to the managed object. Behavior is undefined
  // if `HasValue()` returns `false`; CHECK-fails in debug.
  const T* operator->() const {
    ABSL_DCHECK(HasValue());
    return value_.get();
  }

 private:
  absl_nullable std::shared_ptr<T> value_;
};

}  // namespace ink_internal

#endif  // INK_TYPES_INTERNAL_COPY_ON_WRITE_H_
