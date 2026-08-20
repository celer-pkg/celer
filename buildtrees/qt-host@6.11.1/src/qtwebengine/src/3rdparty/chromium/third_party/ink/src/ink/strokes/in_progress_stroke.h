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

#ifndef INK_STROKES_IN_PROGRESS_STROKE_H_
#define INK_STROKES_IN_PROGRESS_STROKE_H_

#include <cstddef>
#include <cstdint>
#include <optional>

#include "absl/base/nullability.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "ink/brush/brush.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/stroke_shape_builder.h"
#include "ink/strokes/stroke.h"
#include "ink/types/duration.h"

namespace ink {

// An `InProgressStroke` can be used to efficiently build a stroke over multiple
// rendering frames with incremental inputs.
//
// An object of this type incrementally builds a `MutableMesh` that can be used
// to draw the stroke with a triangle mesh renderer. Depending on the `Brush`,
// it also builds one or more outlines that can be used to draw the stroke with
// a path-based renderer.
//
// Using an object of this type typically consists of:
//   1. Beginning a stroke by calling `Start()` with a chosen `Brush`.
//   2. Repeatedly updating the stroke by:
//       a) Calling `EnqueueInputs()` with any new real and predicted stroke
//          inputs.
//       b) Calling `UpdateShape()` when `NeedsUpdate()` is true and new
//          geometry is needed for rendering.
//       c) Rendering the current stroke mesh or outlines either via a provided
//          renderer that accepts an `InProgressStroke` or using the various
//          getters on this type with a custom renderer.
//   3. Calling `FinishInputs()` once there are no more inputs for this stroke
//      (e.g. the user lifts the stylus from the screen).
//   4. Continuing to call `UpdateShape()` and render after `FinishInputs()`
//      until `NeedsUpdate()` returns false (to allow any lingering brush
//      animations to complete).
//   5. Extracting the completed stroke by calling `CopyToStroke()`.
//   6. Preferably, reusing the allocations in this object by persisting it and
//      going back to step 1.
class InProgressStroke {
 public:
  // Defines a policy for which mesh attributes should be retained when
  // constructing the final stroke.
  enum class RetainAttributes {
    // Retain all mesh attributes, even ones that are not needed for the current
    // brush. This allows the brush paint for the stroke to be changed later
    // without needing to regenerate the mesh.
    kAll,
    // Retain only the mesh attributes that are needed for the current
    // brush. This saves on memory, but means that the mesh may need to be
    // regenerated if the brush paint is changed.
    kUsedByThisBrush,
  };

  InProgressStroke() = default;
  InProgressStroke(InProgressStroke&&) = default;
  InProgressStroke(const InProgressStroke&) = delete;
  InProgressStroke& operator=(const InProgressStroke&) = delete;
  InProgressStroke& operator=(InProgressStroke&&) = default;
  ~InProgressStroke() = default;

  // Clears the in progress stroke without starting a new one.
  //
  // This includes clearing or resetting any existing inputs, mesh data, and
  // updated region.  This is functionally equivalent to replacing this
  // `InProgressStroke` with a newly-constructed one, except that using
  // `Clear()` allows existing allocations within the `InProgressStroke` to be
  // reused, making it slightly more efficient.
  void Clear();

  // Clears and starts a new stroke with the given `brush`.
  //
  // This includes clearing or resetting any existing inputs, mesh data, and
  // updated region. This method must be called at least once after construction
  // before starting to call `EnqueueInputs()` or `UpdateShape()`.
  void Start(const Brush& brush, uint32_t noise_seed = 0);

  // Enqueues the incremental `real_inputs` and sets the prediction to
  // `predicted_inputs`, overwriting any previous prediction. Queued inputs will
  // be processed on the next call to `UpdateShape()`.
  //
  // This method requires that:
  //   * `Start()` has been previously called to set the current `Brush`.
  //   * `FinishInputs()` has not been called since the last call to `Start()`.
  //   * `real_inputs` and `predicted_inputs` must form a valid stroke input
  //     sequence together with previously added real input. See comments on
  //     `StrokeInputBatch::Append(const StrokeInputBatch&)` for details.
  //
  // If the above requirements are not satisfied, an error is returned and this
  // object is left in the state it had prior to the call.
  //
  // Either one or both of `real_inputs` and `predicted_inputs` may be empty.
  absl::Status EnqueueInputs(const StrokeInputBatch& real_inputs,
                             const StrokeInputBatch& predicted_inputs);

  // Indicates that the inputs for the current stroke are finished. After
  // calling this, it is an error to call `EnqueueInputs()` until `Start()` is
  // called again to start a new stroke. This method is idempotent; it has no
  // effect if `Start()` was never called, or if this method has already been
  // called since the last call to `Start()`.
  void FinishInputs();

  // Updates the stroke geometry up to the given duration since the start of the
  // stroke. This will will consume any inputs queued up by calls to
  // `EnqueueInputs()`, and cause brush animations (if any) to progress up to
  // the specified time. Any stroke geometry resulting from previously-predicted
  // input from before the previous call to this method will be cleared.
  //
  // This method requires that:
  //   * `Start()` has been previously called to set the current `Brush`.
  //   * The value of `current_elapsed_time` passed into this method over the
  //     course of a single stroke must be non-decreasing and non-negative.
  //
  // If the above requirements are not satisfied, an error is returned and this
  // object is left in the state it had prior to the call.
  absl::Status UpdateShape(Duration32 current_elapsed_time);

  // Returns true if `FinishInputs()` has been called since the last call to
  // `Start()`, or if `Start()` hasn't been called yet. If this returns true, it
  // is an error to call `EnqueueInputs()`.
  bool InputsAreFinished() const;

  // Returns true if calling `UpdateShape()` would have any effect on the stroke
  // (and should thus be called before the next render), or false if no calls to
  // `UpdateShape()` are currently needed. Specifically:
  //   * If the brush has one or more timed animation behavior that are still
  //     active (which can be true even after inputs are finished), returns
  //     true.
  //   * If there are no active animation behaviors, but there are pending
  //     inputs from an `EnqueueInputs()` call that have not yet been consumed
  //     by a call to `UpdateShape()`, returns true.
  //   * Otherwise, returns false.
  //
  // Once `InputsAreFinished()` returns true and this method returns false, the
  // stroke is considered "dry", and will not change any further until the next
  // call to `Start()`.
  bool NeedsUpdate() const;

  // Returns a pointer to the current brush, or `nullptr` if `Start()` has not
  // been called.
  const Brush* absl_nullable GetBrush() const;

  // Returns the number of `BrushCoats` for the current brush, or zero if
  // `Start()` has not been called. The returned value is an exclusive upper
  // bound to the `coat_index` parameters that may be passed to the `GetMesh()`
  // and `GetMeshBounds()` methods.
  uint32_t BrushCoatCount() const;

  // Returns all of the current inputs in the stroke that have been processed by
  // a call to `UpdateShape()` (and are thus reflected in the current stroke
  // geometry). This includes all of the real inputs followed by the most
  // recently-processed sequence of predicted inputs, but does *not* include any
  // inputs that have been passed to `EnqueueInputs()` since the last call to
  // `UpdateShape()`.
  const StrokeInputBatch& GetInputs() const;

  // Returns the count of all current inputs processed in the stroke. This
  // includes all of the real inputs as well as the most-recently-processed
  // sequence of predicted inputs.
  size_t InputCount() const { return processed_inputs_.Size(); }

  // Returns the count of current inputs excluding predicted inputs.
  size_t RealInputCount() const { return real_input_count_; }

  // Returns the count of the most-recently-processed sequence of predicted
  // inputs.
  size_t PredictedInputCount() const { return InputCount() - RealInputCount(); }

  // Returns the currently-generated mesh for the specified coat of paint, which
  // includes geometry generated from all of the real inputs and the current
  // predicted inputs as of the last call to `Start()` or `UpdateShape()`. This
  // geometry will *not* reflect any inputs that have been passed to
  // `EnqueueInputs()` since the last call to `UpdateShape()`.
  //
  // TODO: b/295166196 - Once `MutableMesh` always uses 16-bit indices, rename
  // this method to `GetMeshes` and change it to return an `absl::Span<const
  // MutableMesh>`.
  const MutableMesh& GetMesh(uint32_t coat_index) const;

  // Returns the bounding region of the current positions in the mesh for the
  // specified coat of paint.
  const Envelope& GetMeshBounds(uint32_t coat_index) const;

  // Returns zero or more non-empty spans of vertex indices, one for each of the
  // stroke outlines for the specified coat of paint. There will be at least one
  // outline for each brush tip if the stroke is non-empty. Stroke coats with
  // discontinuous geometry will always have multiple outlines, but even
  // continuous geometry may be drawn with multiple overlapping outlines when
  // this improves rendering quality or performance.
  //
  // Every returned index value can be used to get an outline position from the
  // `MutableMesh` returned by `GetMesh()`.
  //
  // The first and last elements in each span reference the vertices at the end
  // of the stroke outline. The indices traverse the mesh such that the outline
  // has a negative winding number when viewed from the positive z-axis. That
  // is, the outline positions are in clockwise order if the y-axis points up,
  // counter-clockwise order if the y-axis points down.
  absl::Span<const absl::Span<const uint32_t>> GetCoatOutlines(
      uint32_t coat_index) const;

  // Returns the bounding rectangle of mesh positions added, modified, or
  // removed by calls to `UpdateShape()` since the most recent call to `Start()`
  // or `ResetUpdatedRegion()`.
  const Envelope& GetUpdatedRegion() const;
  void ResetUpdatedRegion();

  // Copies the current input, brush, and geometry as of the last call to
  // `Start()` or `UpdateShape()` to a new `Stroke`.
  //
  // The resulting `Stroke` will not be modified if further inputs are added
  // to this `InProgressStroke`, and a `Stroke` created by another call to
  // this method will not modify or be connected in any way to the first
  // `Stroke`.
  Stroke CopyToStroke(
      RetainAttributes retain_attributes = RetainAttributes::kAll) const;

 private:
  absl::Status ValidateNewInputs(
      const StrokeInputBatch& real_inputs,
      const StrokeInputBatch& predicted_inputs) const;

  absl::Status ValidateNewElapsedTime(Duration32 current_elapsed_time) const;

  std::optional<Brush> brush_;
  // Real and predicted inputs that have been queued by calls to
  // `EnqueueInputs()` since the last call to `UpdateShape()`.
  StrokeInputBatch queued_real_inputs_;
  StrokeInputBatch queued_predicted_inputs_;
  // Inputs (combined real and predicted) that have already been processed by a
  // call to `UpdateShape()`, and are reflected in the current
  // `StrokeShapeBuilder` geometry.
  StrokeInputBatch processed_inputs_;
  // The number of inputs in `processed_inputs_`, starting from the beginning,
  // that are real inputs; the rest (if any) are predicted inputs.
  size_t real_input_count_ = 0;
  // The largest elapsed time passed to `UpdateShape()` since the last call to
  // `Start()`.
  Duration32 current_elapsed_time_ = Duration32::Zero();
  // A vector with at least one `StrokeShapeBuilder` for each `BrushCoat` in the
  // current brush (and potentially more; in order to cache allocations, we
  // never shrink this vector).
  absl::InlinedVector<strokes_internal::StrokeShapeBuilder, 1> shape_builders_;
  // The region updated by `UpdateShape()` since the last call to `Start()` or
  // `ResetUpdatedRegion()`.
  Envelope updated_region_;
  // True if `FinishInputs()` has been called since the last call to `Start()`,
  // or if `Start()` hasn't been called yet.
  bool inputs_are_finished_ = true;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline void InProgressStroke::FinishInputs() {
  inputs_are_finished_ = true;
  queued_predicted_inputs_.Clear();
}

inline bool InProgressStroke::InputsAreFinished() const {
  return inputs_are_finished_;
}

inline const Brush* absl_nullable InProgressStroke::GetBrush() const {
  return brush_.has_value() ? &*brush_ : nullptr;
}

inline uint32_t InProgressStroke::BrushCoatCount() const {
  return brush_.has_value() ? brush_->CoatCount() : 0;
}

inline const StrokeInputBatch& InProgressStroke::GetInputs() const {
  return processed_inputs_;
}

inline const MutableMesh& InProgressStroke::GetMesh(uint32_t coat_index) const {
  ABSL_CHECK_LT(coat_index, BrushCoatCount());
  return shape_builders_[coat_index].GetMesh();
}

inline const Envelope& InProgressStroke::GetMeshBounds(
    uint32_t coat_index) const {
  ABSL_CHECK_LT(coat_index, BrushCoatCount());
  return shape_builders_[coat_index].GetMeshBounds();
}

inline absl::Span<const absl::Span<const uint32_t>>
InProgressStroke::GetCoatOutlines(uint32_t coat_index) const {
  ABSL_CHECK_LT(coat_index, BrushCoatCount());
  return shape_builders_[coat_index].GetOutlines();
}

inline const Envelope& InProgressStroke::GetUpdatedRegion() const {
  return updated_region_;
}

inline void InProgressStroke::ResetUpdatedRegion() { updated_region_.Reset(); }

}  // namespace ink

#endif  // INK_STROKES_IN_PROGRESS_STROKE_H_
