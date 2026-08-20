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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_H_

#include <array>
#include <cstdint>
#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {

// Helper type used to calculate properties of vertices in a `MutableMeshView`
// that consist of positional derivatives along with their related "margins".
//
// This type exists to cache allocations for per-vertex metadata used during
// each update.
class DerivativeCalculator {
 public:
  // Type used to accumulate an average derivative vector.
  //
  // Averaging is done by independently averaging the magnitudes and unit
  // vectors of each incoming vector. Anti-aliasing working correctly requires
  // the final vertex gradients to be non-zero, so incoming vectors with zero
  // magnitude are ignored for the average.
  class AverageDerivative {
   public:
    AverageDerivative() = default;
    AverageDerivative(const AverageDerivative&) = default;
    AverageDerivative& operator=(const AverageDerivative&) = default;
    ~AverageDerivative() = default;

    void Add(Vec v);
    void Add(const AverageDerivative& other);

    // Returns the average value based the values added since construction.
    Vec Value() const;

   private:
    float magnitude_sum_ = 0;
    Vec unit_vector_sum_ = {0, 0};
    int count_ = 0;
  };

  struct AverageVertexDerivatives {
    AverageDerivative side;
    AverageDerivative forward;
  };

  DerivativeCalculator() = default;
  DerivativeCalculator(const DerivativeCalculator&) = default;
  DerivativeCalculator(DerivativeCalculator&&) = default;
  DerivativeCalculator& operator=(const DerivativeCalculator&) = default;
  DerivativeCalculator& operator=(DerivativeCalculator&&) = default;
  ~DerivativeCalculator() = default;

  // Updates values of vertex derivative attributes in the stroke `mesh`.
  //
  // This type and function are optimized for the expected case that the values
  // needing to be updated lie near the end of the `mesh`.
  //
  // The values in `left_indices_to_update` and `right_indices_to_update` are
  // expected to be subranges of `brush_tip_extruder_internal::Side::indices`
  // for the "left" and "right" sides of the stroke.
  void UpdateMesh(absl::Span<const uint32_t> left_indices_to_update,
                  absl::Span<const uint32_t> right_indices_to_update,
                  MutableMeshView& mesh);

 private:
  // Prepares the tracked average derivatives and minimum margins for
  // calculating new values. The derivatives are zeroed out, and the margins are
  // set to `StrokeVertex::kMaximumMargin`.
  void ResetTrackedValues(absl::Span<const uint32_t> left_indices_to_update,
                          absl::Span<const uint32_t> right_indices_to_update,
                          const MutableMeshView& mesh);

  // The following helper functions update the relevant values in
  // `tracked_average_derivatives_` for the given `indices` of the current mesh.
  // The value will be ignored for any index less than `minimum_tracked_index_`.
  void SaveSideDerivative(const std::array<uint32_t, 3>& indices,
                          Vec derivative);
  void SaveForwardDerivative(const std::array<uint32_t, 3>& indices,
                             Vec derivative);

  // Iterates over `mesh` triangles, calculates derivatives, and adds them to
  // associated tracked average values.
  void AccumulateDerivatives(const MutableMeshView& mesh);

  // Calculates and saves the values of derivatives for a single triplet of
  // `triangle_indices` of the `mesh`.
  void AddDerivativesForTriangle(
      const MutableMeshView& mesh,
      const std::array<uint32_t, 3>& triangle_indices);

  // Updates the derivative attribute values in `mesh` for each vertex index in
  // `indices_to_update`.
  void UpdateMeshDerivatives(absl::Span<const uint32_t> indices_to_update,
                             MutableMeshView& mesh);

  // Iterates over `mesh` triangles, calculates margins, and updates the
  // associated tracked upper bounds.
  void AccumulateMargins(const MutableMeshView& mesh);

  // Calculates and saves the margin upper bounds for a single triplet of
  // `triangle_indices` of the `mesh`.
  void AddMarginUpperBoundsForTriangle(
      const MutableMeshView& mesh,
      const std::array<uint32_t, 3>& triangle_indices);

  // Updates `tracked_side_margin_upper_bounds_` when the new `margin` is
  // smaller that the current value stored at `index`. The new value will be
  // ignored if `index` is less than `minimum_tracked_index_`.
  void SaveSideMarginUpperBound(uint32_t index, float margin);

  // Updates the margin encoded into each label in `mesh` for vertices given by
  // `indices_to_update`.
  void UpdateMeshMargins(absl::Span<const uint32_t> indices_to_update,
                         MutableMeshView& mesh);

  // The lowest mesh index for which average derivatives are being calculated
  // and saved.
  uint32_t minimum_tracked_index_ = 0;
  // Derivative and side margin values tracked for vertices starting at
  // `minimum_tracked_index_` until the end of the mesh. We do not bother
  // tracking and calculating forward margins in this class, because of mesh
  // convexity at forward-exterior vertices.
  //
  // These are persisted as member variables to reuse allocations over
  // multiple updates. We save one derivative per outline segment connected to
  // each vertex, and there can up to two of these segments per vertex.
  std::vector<AverageVertexDerivatives> tracked_average_derivatives_;
  std::vector<float> tracked_side_margin_upper_bounds_;
};

}  // namespace ink::brush_tip_extruder_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_DERIVATIVE_CALCULATOR_H_
