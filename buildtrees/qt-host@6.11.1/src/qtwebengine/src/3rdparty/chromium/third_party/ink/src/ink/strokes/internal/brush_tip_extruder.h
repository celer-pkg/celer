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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extrusion.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/extrusion_points.h"
#include "ink/strokes/internal/stroke_outline.h"
#include "ink/strokes/internal/stroke_shape_update.h"

namespace ink::strokes_internal {

// Type responsible for generating extruded stroke geometry.
//
// Extrusion of a stroke:
//   * Incrementally takes a sequence of `BrushTipState`s, where each state
//     defines tip properties at one point along the stroke.
//   * For each tip state:
//       * Models the shape of the state when its width and height are both
//         greater than the brush epsilon value.
//       * Inserts a "break-point" in the geometry when the width and height of
//         the tip state are below the threshold. This allows an extruded stroke
//         to have gaps in geometry for dashes and dots.
//   * Connects the contiguous sequences of shapes with a triangle mesh, as
//     though each tip shape smoothly morphs into the next.
//
// The resulting geometry is added to a target mesh specified for the current
// stroke.
class BrushTipExtruder {
 public:
  BrushTipExtruder() = default;
  BrushTipExtruder(const BrushTipExtruder&) = delete;
  BrushTipExtruder& operator=(const BrushTipExtruder&) = delete;
  BrushTipExtruder(BrushTipExtruder&&) = default;
  BrushTipExtruder& operator=(BrushTipExtruder&&) = default;
  ~BrushTipExtruder() = default;

  // Starts a new stroke.
  //
  // The value of `brush_epsilon` must be greater than zero and represents the
  // minimum separation distance for two points to be considered distinct. It
  // will be used:
  //   * To determine the fidelity of the extruded mesh. Lower values result in
  //     a higher density of vertices used to approximate stroke curvature.
  //   * As a minimum threshold value for tip state width and height to be
  //     modeled into a shape and extruded. Otherwise, a break-point will be
  //     added as described above.
  //
  // `is_winding_texture_particle_brush` indicates whether the stroke is a
  // particle brush with a winding texture (which may be animated).
  //
  // Extruded mesh data will be added to the target `mesh`, the lifetime of
  // which must extend for all subsequent calls to `ExtendStroke()`
  // until this object is destroyed or `StartStroke()` is called again.
  //
  // This function must be called at least once after construction before
  // calling `ExtendStroke()`. Any previously extruded stroke data is cleared.
  void StartStroke(float brush_epsilon, bool is_winding_texture_particle_brush,
                   MutableMesh& mesh);

  // Extends the stroke by extruding geometry using new "fixed" and "volatile"
  // tip states.
  //
  // Tip states are referred to as "fixed" when they should be a permanent part
  // of the stroke, while "volatile" tip states should only be added until the
  // next call to `ExtendStroke()`.
  //
  // This function first reverts any past "volatile" extrusions. The returned
  // update covers both the reverted extruded geometry and changes based on the
  // new tip states.
  StrokeShapeUpdate ExtendStroke(
      absl::Span<const BrushTipState> new_fixed_states,
      absl::Span<const BrushTipState> volatile_states);

  // Returns the bounding region of positions extruded into the current mesh.
  const Envelope& GetBounds() const;

  // Returns the outlines for the current brush tip. This can include empty
  // outlines. (In particular, we greedily allocate the first outline, so
  // that is empty if the stroke is empty.)
  absl::Span<const StrokeOutline> GetOutlines() const;

 private:
  // Data used to incrementally update the bounds of geometry extruded into the
  // current mesh.
  struct Bounds {
    // The bounds of the portion of the mesh that is unlikely to change with
    // future extrusions, as we usually only grow the "fixed" portion of the
    // stroke. In the event that that portion of the mesh does change (i.e. if
    // we mutate or delete triangles or vertices), this cached value will no
    // longer be correct, and must be cleared by calling
    // `ClearCachedPartialBounds()`.
    //
    // We maintain this in order to speed up `GetBounds()`; it allows us to
    // avoid traversing the entire mesh in order to compute the bounds.
    Envelope cached_partial_bounds;
    // The number of indices at the start of the left and right sides of the
    // stroke whose positions have been used to calculate
    // `cached_partial_bounds`.
    uint32_t cached_partial_bounds_left_index_count = 0;
    uint32_t cached_partial_bounds_right_index_count = 0;
    // The complete current bounding region.
    Envelope current;
  };

  // Clears the value of `bounds_.cached_partial_bounds`.
  //
  // This must be called if any of the vertices that contributed to the cached
  // partial bounds were mutated or deleted.
  void ClearCachedPartialBounds();

  // Updates the value of `bounds_.cached_partial_bounds`.
  //
  // This function expects to be called right after `ExtendStroke()` has
  // finished extruding `new_fixed_states`, but before it extrudes any
  // `volatile_states`.
  void UpdateCachedPartialBounds();

  // Updates the value of `bounds_.current`.
  //
  // This function expects to be called at the end of `ExtendStroke()`, once all
  // `new_fixed_states` and `volatile_states` have been extruded.
  void UpdateCurrentBounds();

  // Saves the current state of the extruder so that volatile changes can be
  // reverted.
  void Save();

  // Restores the saved state of the extruder.
  void Restore();

  // Truncate outlines to match the current geometry.
  void TruncateOutlines();

  // Clears the geometry, extrusions, and outline indices since the last break
  // in extrusion. This is either since the last explicitly added break-point,
  // or since the implicit break-point at start of the stroke.
  //
  // `first_extrusion_to_clear` must be the first extrusion since the most
  // recent break-point or the first extrusion in the stroke (i.e. if there
  // has never been an explicitly added break-point).
  //
  // `triggered_by_volatile_extrusion` must be true if this was called during
  // extrusion of a volatile state, or false if during extrusion of a fixed
  // state.
  void ClearSinceLastExtrusionBreak(
      std::vector<BrushTipExtrusion>::iterator first_extrusion_to_erase,
      bool triggered_by_volatile_extrusion);

  // Attempts to create and append new `BrushTipExtrusion` based on `tip_state`;
  // returns `true` on success, and false if `tip_state` was rejected. See
  // `Extrude` for definition of `is_volatile_state` and `is_last_state`.
  bool TryAppendNonBreakPointState(const BrushTipState& tip_state,
                                   bool is_volatile_state, bool is_last_state);

  // Extrudes stroke geometry based on `tip_state`. `is_volatile_state`
  // indicates whether `tip_state` is volatile (see `ExtendStroke`).
  // `is_last_state` indicates whether `tip_state` is the last state in the
  // batch passed to `ExtendStroke`, which may be either a fixed or volatile
  // state.
  void Extrude(const BrushTipState& tip_state, bool is_volatile_state,
               bool is_last_state);

  // Extrudes the end of a contiguous section of the stroke from up to the last
  // two non-break-point elements in `extrusion_data_`, appends a new
  // break-point as needed.
  //
  // If `extrusion_data_` is empty or the last element is already a break-point,
  // this function is a no-op.
  void ExtrudeBreakPoint();

  // The tip data and extrusion break-points for the stroke. Will not contain
  // more than one break-point in a row.
  //
  // TODO: b/268209721 - We will likely not need to store every state and shape
  // for the entire stroke, but we'll start out doing this. Extrusion needs to
  // "constrain" tip state dynamics more so than stamping would, and that logic
  // would live inside this type and use older tip data.
  std::vector<BrushTipExtrusion> extrusions_;
  // The size of `extrusion_data_` at the last call to `Save()` or
  // `StartStroke()`.
  size_t saved_extrusion_data_count_ = 0;
  // The list of extrusions that were present when `Save()` was last called and
  // have since been deleted.
  std::vector<BrushTipExtrusion> deleted_save_point_extrusions_;

  float brush_epsilon_ = 0;
  // Parameter controlling the number of points created to approximate arcs.
  float max_chord_height_ = 0;
  // Parameter used by `geometry_` to remove points along the outline that do
  // not meaningfully contribute to curvature.
  float simplification_threshold_ = 0;
  // Indicates whether this is stroke is being extruded with a particle brush
  // with a winding texture (which may be animated).
  bool is_winding_texture_particle_brush_;

  ExtrusionPoints current_extrusion_points_;
  brush_tip_extruder_internal::Geometry geometry_;
  Bounds bounds_;

  // Store a separate count of the number of used outlines so that storage can
  // be reused when outlines are discarded.
  uint32_t num_outlines_ = 1;
  absl::InlinedVector<StrokeOutline, 1> outlines_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline const Envelope& BrushTipExtruder::GetBounds() const {
  return bounds_.current;
}

inline absl::Span<const StrokeOutline> BrushTipExtruder::GetOutlines() const {
  return absl::MakeSpan(outlines_).subspan(0, num_outlines_);
}

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_H_
