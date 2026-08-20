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

#ifndef INK_STROKES_INPUT_STROKE_INPUT_BATCH_H_
#define INK_STROKES_INPUT_STROKE_INPUT_BATCH_H_

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/types/duration.h"
#include "ink/types/internal/copy_on_write.h"
#include "ink/types/physical_distance.h"

namespace ink {

// A `StrokeInputBatch` allows for validating and storing one or more
// consecutive inputs from all or part of a stroke.
//
// The type is more memory efficient than a large array of `StrokeInput`, as it
// does not use extra memory when pressure, tilt, or orientation values are not
// reported.
//
// The `StrokeInputBatch` implements copy-on-write, making it cheap to copy
// independent of batch size. This design supports efficiently sharing the same
// input data between multiple `Stroke` objects.
//
// Validation requirements:
//
//   1) All floating point values are required to be finite and the format of
//      all inputs added to the same batch must be consistent. This means all
//      inputs added to a batch should report the same optional properties, and
//      have the same value of `StrokeInput::tool_type` and of
//      `StrokeInput::stroke_unit_length`. For example, if the first input
//      reports a value for pressure then all subsequent inputs must also report
//      pressure.
//   2) The sequence of inputs in the batch must not contain repeated x-y-t
//      triplets, and the elapsed time values must be non-decreasing.
//   3) Pressure, tilt, and orientation should either be set to corresponding
//      sentinel values indicating their absence, or be in the ranges of [0, 1],
//      [0, π/2], and [0, 2π) respectively.
//   4) `StrokeInput::tool_type` must be one of the enumerator values.
//
class StrokeInputBatch {
 public:
  // A constant iterator type to enable range-based for loop iteration. See the
  // complete class definition below for details.
  //
  // NOTE: Calling any non-const member function of `StrokeInputBatch` should be
  // assumed to cause all iterators to be invalidated.
  class ConstIterator;
  using value_type = StrokeInput;

  // Performs validation on `inputs` and returns the resulting batch or error.
  static absl::StatusOr<StrokeInputBatch> Create(
      absl::Span<const StrokeInput> inputs, uint32_t noise_seed = 0);

  StrokeInputBatch() = default;
  StrokeInputBatch(const StrokeInputBatch&) = default;
  StrokeInputBatch(StrokeInputBatch&&) = default;
  StrokeInputBatch& operator=(const StrokeInputBatch&) = default;
  StrokeInputBatch& operator=(StrokeInputBatch&&) = default;
  ~StrokeInputBatch() = default;

  ConstIterator begin() const;
  ConstIterator end() const;

  void Clear();

  size_t Size() const;
  bool IsEmpty() const;

  // Returns a copy of this `StrokeInputBatch` that initially has unique
  // ownership of its memory. See comment above regarding copy-on-write
  // behavior.
  StrokeInputBatch MakeDeepCopy() const;

  // Validates and sets the value of the i-th input.
  //
  // In the special case that this will overwrite the only held `StrokeInput`,
  // it is valid for the format of `input` to be different from the currently
  // held value.
  //
  // Returns an error and does not modify the batch if validation fails.
  absl::Status Set(size_t i, const StrokeInput& input);

  StrokeInput Get(size_t i) const;

  // Validates and appends a new `input`.
  //
  // Returns an error and does not modify the batch if validation fails.
  absl::Status Append(const StrokeInput& input);

  // Validates and appends a sequence of `inputs`. This batch's per-stroke seed
  // value is left unchanged, even when appending another batch with a different
  // seed value.
  //
  // Returns an error and does not modify the batch if validation fails.
  absl::Status Append(absl::Span<const StrokeInput> inputs);
  absl::Status Append(const StrokeInputBatch& inputs);

  // Erases `count` elements beginning at `start`.
  //
  // If `start` + `count` is greater than `Size()`, then all elements from
  // `start` until the end of the input batch are erased. CHECK-fails if `start`
  // is not less than or equal to `Size()`.
  void Erase(size_t start, size_t count = std::numeric_limits<size_t>::max());

  // Returns the current input tool type or `StrokeInput::ToolType::kUnknown`
  // if the batch is empty.
  StrokeInput::ToolType GetToolType() const;

  // Returns the physical distance that the pointer traveled in order to produce
  // an input motion of one stroke unit. For stylus/touch, this is the
  // real-world distance that the stylus/fingertip moved in physical space; for
  // mouse, this is the visual distance that the mouse pointer traveled along
  // the surface of the display.
  //
  // A value of `std::nullopt` indicates that the relationship between stroke
  // space and physical space is unknown or ill-defined. Otherwise, the value
  // will be finite and strictly positive.
  std::optional<PhysicalDistance> GetStrokeUnitLength() const;

  // Returns the duration between the first and last input.
  Duration32 GetDuration() const;

  bool HasStrokeUnitLength() const;
  bool HasPressure() const;
  bool HasTilt() const;
  bool HasOrientation() const;

  // Returns the seed value that should be used for seeding any noise generators
  // for brush behaviors when a full stroke is regenerated with this input
  // batch. If no seed value has yet been set for this input batch, returns the
  // default seed of zero.
  uint32_t GetNoiseSeed() const;

  // Sets the per-stroke seed value that should be used when regenerating a
  // stroke from this input batch.
  void SetNoiseSeed(uint32_t seed);

  // Which properties of the stroke should be preserved over transforms.
  enum class TransformInvariant {
    kPreserveDuration = 0,
    //  `kPreserveVelocity` will be added once it's supported
  };

  // Applies the transformation in the `transform` to the points in the
  // `StrokeInputBatch` in-place.
  //
  // If `invariant` == `kPreserveDuration` the transformed points will have the
  // original `elapsed_time` for each `StrokeInput` in the `StrokeInputBatch`.
  // If `invariant` == `kPreserveVelocity` each `StrokeInput::elapsed_time` will
  // be adjusted to the value it would be if the transformed inputs had the same
  // velocity, resulting in a shorter or longer overall duration.
  // TODO: b/278536966 - Add support for `kPreserveVelocity`
  void Transform(
      const AffineTransform& transform,
      TransformInvariant invariant = TransformInvariant::kPreserveDuration);

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const StrokeInputBatch& batch) {
    sink.Append(batch.ToFormattedString());
  }

 private:
  void DebugCheckSizeAndFormatAreConsistent() const {
    ABSL_DCHECK_EQ(size_ * FloatsPerInput(),
                   data_.HasValue() ? data_->size() : 0);
  }

  // The following helpers return the number of floats needed to store the
  // numeric properties of a single `StrokeInput` when the missing optional
  // properties are skipped instead of being stored as sentinel values.
  //
  // The returned value will be at least 3 - the case when storing only position
  // and elapsed time. It will be no more than 6, which is the number required
  // to store an input with all of pressure, tilt, and orientation present.
  static int FloatsPerInput(bool has_pressure, bool has_tilt,
                            bool has_orientation);
  static int FloatsPerInput(const StrokeInput& input);
  int FloatsPerInput() const;

  // Transforms the input points in place, applying the `AffineTransform` while
  // keeping the stroke total elapsed time the same.
  void TransformPreservingDuration(const AffineTransform& transform);

  // Updates the inline member variables that store the "format" of the inputs
  // (i.e. tool type and whether pressure, tilt, and orientation are present).
  // This function should only be called when the batch is empty.
  void SetInlineFormatMetadata(const StrokeInput& input);

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  // Input property data stored as raw floats, using `FloatsPerInput()` values
  // per `StrokeInput`.
  //
  // Values for each input are stored at adjacent indices in the following
  // order:
  //   * position x
  //   * position y
  //   * elapsed time in seconds
  //   * pressure, only if `has_pressure_`
  //   * tilt in radians, only if `has_tilt_`
  //   * orientation in radians, only if `has_orientation_`
  //
  // TODO: b/295885521 - Replace with either `CopyOnWrite<T[]>` or a
  // `CopyOnWriteArray` to remove the extra indirection.
  ink_internal::CopyOnWrite<std::vector<float>> data_;

  // Store metadata inline so that simple getters do not need an extra branch
  // and pointer indirection:
  size_t size_ = 0;
  StrokeInput::ToolType tool_type_ = StrokeInput::ToolType::kUnknown;
  PhysicalDistance stroke_unit_length_ = StrokeInput::kNoStrokeUnitLength;
  uint32_t noise_seed_ = 0;
  bool has_pressure_ = false;
  bool has_tilt_ = false;
  bool has_orientation_ = false;
};

// Constant iterator type conforms to the named requirements of
// LegacyInputIterator
// (https://en.cppreference.com/w/cpp/named_req/InputIterator).
//
// Note that this is a proxy iterator. The value does not live in the
// `StrokeInputBatch`, but within the iterator, and so its lifetime is tied to
// the lifetime of the iterator itself.
//
// To help prevent lifetime issues, `reference` is an alias to the `const
// value_type`, not a true reference. This allows the return value of
// `operator*()` to partake in reference lifetime extension in order to
//   a) outlive the iterator
//   b) be unaffected by subsequent calls to e.g. `operator++()`
class StrokeInputBatch::ConstIterator {
 public:
  using value_type = StrokeInput;
  using pointer = const value_type*;
  using reference = const value_type;
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;

  ConstIterator() = default;

  // NOLINTNEXTLINE - Suppress ClangTidy const-return-type.
  reference operator*() const;
  pointer operator->() const;
  ConstIterator& operator++();
  ConstIterator operator++(int);

  friend bool operator==(const ConstIterator& lhs, const ConstIterator& rhs);
  friend bool operator!=(const ConstIterator& lhs, const ConstIterator& rhs);

 private:
  friend class StrokeInputBatch;

  ConstIterator(const StrokeInputBatch& inputs, size_t index) {
    if (!inputs.data_.HasValue()) return;
    batch_subdata_ = absl::MakeSpan(inputs.data_.Value())
                         .subspan(index * inputs.FloatsPerInput());
    if (index < inputs.Size()) value_ = inputs.Get(index);
  }

  // Subspan of batch `data_` from the current position of the iterator to the
  // end of the container.
  absl::Span<const float> batch_subdata_;

  // In order to have operator-> work in a sensible manner, it needs to return
  // a pointer to the value type. To accomplish this, since the value type is
  // not the same as the underlying stored type, we maintain a cached copy of
  // the value so that we may return a pointer to it.
  StrokeInput value_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline size_t StrokeInputBatch::Size() const {
  DebugCheckSizeAndFormatAreConsistent();
  return size_;
}

inline bool StrokeInputBatch::IsEmpty() const { return Size() == 0; }

inline StrokeInputBatch::ConstIterator StrokeInputBatch::begin() const {
  return StrokeInputBatch::ConstIterator(*this, 0);
}

inline StrokeInputBatch::ConstIterator StrokeInputBatch::end() const {
  return StrokeInputBatch::ConstIterator(*this, Size());
}

inline StrokeInput::ToolType StrokeInputBatch::GetToolType() const {
  return tool_type_;
}

inline std::optional<PhysicalDistance> StrokeInputBatch::GetStrokeUnitLength()
    const {
  if (!HasStrokeUnitLength()) return std::nullopt;
  return stroke_unit_length_;
}

inline uint32_t StrokeInputBatch::GetNoiseSeed() const { return noise_seed_; }

inline void StrokeInputBatch::SetNoiseSeed(uint32_t seed) {
  noise_seed_ = seed;
}

inline bool StrokeInputBatch::HasStrokeUnitLength() const {
  return stroke_unit_length_ != StrokeInput::kNoStrokeUnitLength;
}

inline bool StrokeInputBatch::HasPressure() const {
  DebugCheckSizeAndFormatAreConsistent();
  return has_pressure_;
}

inline bool StrokeInputBatch::HasTilt() const {
  DebugCheckSizeAndFormatAreConsistent();
  return has_tilt_;
}

inline bool StrokeInputBatch::HasOrientation() const {
  DebugCheckSizeAndFormatAreConsistent();
  return has_orientation_;
}

inline StrokeInputBatch::ConstIterator::pointer
StrokeInputBatch::ConstIterator::operator->() const {
  ABSL_DCHECK(!batch_subdata_.empty())
      << "Attempted to dereference singular or past-the-end iterator";
  return &value_;
}

// NOLINTNEXTLINE - Suppress ClangTidy const-return-type.
inline StrokeInputBatch::ConstIterator::reference
StrokeInputBatch::ConstIterator::operator*() const {
  return *operator->();
}

inline bool operator==(const StrokeInputBatch::ConstIterator& lhs,
                       const StrokeInputBatch::ConstIterator& rhs) {
  return lhs.batch_subdata_.data() == rhs.batch_subdata_.data();
}

inline bool operator!=(const StrokeInputBatch::ConstIterator& lhs,
                       const StrokeInputBatch::ConstIterator& rhs) {
  return !(lhs == rhs);
}

inline int StrokeInputBatch::FloatsPerInput(bool has_pressure, bool has_tilt,
                                            bool has_orientation) {
  // Minimum of 3 floats (two for position and one elapsed time) plus one for
  // each present optional property:
  return 3 + static_cast<int>(has_pressure) + static_cast<int>(has_tilt) +
         static_cast<int>(has_orientation);
}

inline int StrokeInputBatch::FloatsPerInput(const StrokeInput& input) {
  return FloatsPerInput(input.HasPressure(), input.HasTilt(),
                        input.HasOrientation());
}

inline int StrokeInputBatch::FloatsPerInput() const {
  return FloatsPerInput(has_pressure_, has_tilt_, has_orientation_);
}

}  // namespace ink

#endif  // INK_STROKES_INPUT_STROKE_INPUT_BATCH_H_
