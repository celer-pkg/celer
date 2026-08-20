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

#include "ink/strokes/internal/brush_tip_extruder.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_extrusion.h"
#include "ink/strokes/internal/brush_tip_shape.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/constrain_brush_tip_extrusion.h"
#include "ink/strokes/internal/extrusion_points.h"
#include "ink/strokes/internal/stroke_outline.h"
#include "ink/strokes/internal/stroke_shape_update.h"

namespace ink::strokes_internal {
namespace {

using ::ink::brush_tip_extruder_internal::Geometry;
using ::ink::brush_tip_extruder_internal::MutableMeshView;
using ::ink::brush_tip_extruder_internal::Side;

// TODO: b/289230108 - Define a clear relationship between brush epsilon and the
// max chord height / simplification threshold values. Probably at least one of
// these should not be a passthrough as the values should not be equal.
float GetMaxChordHeight(float brush_epsilon) { return brush_epsilon; }
float GetSimplificationThreshold(float brush_epsilon) { return brush_epsilon; }

}  // namespace

void BrushTipExtruder::StartStroke(float brush_epsilon,
                                   bool is_winding_texture_particle_brush,
                                   MutableMesh& mesh) {
  ABSL_CHECK_GT(brush_epsilon, 0);
  brush_epsilon_ = brush_epsilon;
  max_chord_height_ = GetMaxChordHeight(brush_epsilon);
  simplification_threshold_ = GetSimplificationThreshold(brush_epsilon);
  is_winding_texture_particle_brush_ = is_winding_texture_particle_brush;
  extrusions_.clear();
  saved_extrusion_data_count_ = 0;
  deleted_save_point_extrusions_.clear();
  geometry_.Reset(MutableMeshView(mesh));
  bounds_ = {};
  // Pre-allocate the first outline.
  num_outlines_ = 1;
  if (outlines_.empty()) {
    outlines_.resize(1);
  }
  // Clear all the outlines from the previous stroke.
  for (StrokeOutline& outline : outlines_) {
    outline.TruncateIndices({0, 0});
  }
}

namespace {

StrokeShapeUpdate ConstructUpdate(const Geometry& geometry,
                                  uint32_t triangle_count_before_update,
                                  uint32_t vertex_count_before_update) {
  StrokeShapeUpdate update{
      .region = geometry.CalculateVisuallyUpdatedRegion(),
  };

  const MutableMeshView& mesh_view = geometry.GetMeshView();
  if (mesh_view.FirstMutatedTriangle() != mesh_view.TriangleCount() ||
      mesh_view.TriangleCount() != triangle_count_before_update) {
    constexpr int kIndicesPerTriangle = 3;
    update.first_index_offset =
        kIndicesPerTriangle * mesh_view.FirstMutatedTriangle();
  }
  if (mesh_view.FirstMutatedVertex() != mesh_view.VertexCount() ||
      mesh_view.VertexCount() != vertex_count_before_update) {
    update.first_vertex_offset = mesh_view.FirstMutatedVertex();
  }
  return update;
}

}  // namespace

StrokeShapeUpdate BrushTipExtruder::ExtendStroke(
    absl::Span<const BrushTipState> new_fixed_states,
    absl::Span<const BrushTipState> volatile_states) {
  ABSL_CHECK_GT(brush_epsilon_, 0) << "`StartStroke()` has not been called";

  geometry_.ResetMutationTracking();
  uint32_t triangle_count_before_update =
      geometry_.GetMeshView().TriangleCount();
  uint32_t vertex_count_before_update = geometry_.GetMeshView().VertexCount();

  Restore();

  for (size_t i = 0; i < new_fixed_states.size(); ++i) {
    const BrushTipState& tip_state = new_fixed_states[i];
    Extrude(tip_state, /* is_volatile_state = */ false,
            volatile_states.empty() && i == new_fixed_states.size() - 1);
  }

  UpdateCachedPartialBounds();
  Save();

  for (size_t i = 0; i < volatile_states.size(); ++i) {
    const BrushTipState& tip_state = volatile_states[i];
    Extrude(tip_state, /* is_volatile_state = */ true,
            i == volatile_states.size() - 1);
  }

  ExtrudeBreakPoint();
  geometry_.UpdateMeshDerivatives();
  UpdateCurrentBounds();
  return ConstructUpdate(geometry_, triangle_count_before_update,
                         vertex_count_before_update);
}

void BrushTipExtruder::ClearCachedPartialBounds() {
  bounds_.cached_partial_bounds.Reset();
  bounds_.cached_partial_bounds_left_index_count = 0;
  bounds_.cached_partial_bounds_right_index_count = 0;
}

namespace {

void AddPositionsToEnvelope(const MutableMeshView& mesh_view,
                            absl::Span<const uint32_t> indices,
                            Envelope& envelope) {
  for (uint32_t index : indices) {
    envelope.Add(mesh_view.GetPosition(index));
  }
}

void UpdateCachedPartialBoundsForSide(
    const MutableMeshView& mesh_view, const Side& side,
    Envelope& cached_partial_bounds,
    uint32_t& cached_partial_bounds_side_index_count) {
  ABSL_CHECK_LE(cached_partial_bounds_side_index_count, side.indices.size());

  auto new_indices = absl::MakeSpan(side.indices)
                         .subspan(cached_partial_bounds_side_index_count);
  if (!new_indices.empty()) {
    // At this point, the last index of each side cannot be used for the cached
    // subregion. This is because its position may be "simplified" away on the
    // next extrusion
    // (https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm).
    // If we considered the last index for the cached subregion, it could make
    // the calculated complete bounds larger than the actual minimal bounds by
    // brush epsilon on each side. Instead, we will leave the last index to be
    // calculated as part of the complete bounds.
    new_indices.remove_suffix(1);
  }

  AddPositionsToEnvelope(mesh_view, new_indices, cached_partial_bounds);
  cached_partial_bounds_side_index_count += new_indices.size();
}

}  // namespace

void BrushTipExtruder::UpdateCachedPartialBounds() {
  // Note that updating the cached partial bounds is strictly a performance
  // optimization, and the effect of calling the following two functions would
  // only be observable through benchmarks. If they were not called,
  // `cached_partial_bounds_left_index_count` and
  // `cached_partial_bounds_right_index_count` would not be incremented. Then,
  // any positions not picked up here would be picked up later in
  // `UpdateCurrentBounds()`.
  UpdateCachedPartialBoundsForSide(
      geometry_.GetMeshView(), geometry_.LeftSide(),
      bounds_.cached_partial_bounds,
      bounds_.cached_partial_bounds_left_index_count);
  UpdateCachedPartialBoundsForSide(
      geometry_.GetMeshView(), geometry_.RightSide(),
      bounds_.cached_partial_bounds,
      bounds_.cached_partial_bounds_right_index_count);
}

void BrushTipExtruder::UpdateCurrentBounds() {
  bounds_.current = bounds_.cached_partial_bounds;

  AddPositionsToEnvelope(
      geometry_.GetMeshView(),
      absl::MakeSpan(geometry_.LeftSide().indices)
          .subspan(bounds_.cached_partial_bounds_left_index_count),
      bounds_.current);
  AddPositionsToEnvelope(
      geometry_.GetMeshView(),
      absl::MakeSpan(geometry_.RightSide().indices)
          .subspan(bounds_.cached_partial_bounds_right_index_count),
      bounds_.current);
}

void BrushTipExtruder::Save() {
  saved_extrusion_data_count_ = extrusions_.size();
  deleted_save_point_extrusions_.clear();
  geometry_.SetSavePoint();
}

void BrushTipExtruder::TruncateOutlines() {
  ABSL_DCHECK_LE(geometry_.ExtrusionBreakCount(), outlines_.size());
  // Prune the outline after the last break point to the first mutation.
  uint32_t max_num_outlines = geometry_.ExtrusionBreakCount() + 1;
  if (num_outlines_ >= max_num_outlines) {
    num_outlines_ = max_num_outlines;
    const Geometry::IndexCounts& last_extrusion_break_offset =
        geometry_.IndexCountsAtLastExtrusionBreak();
    ABSL_DCHECK_GE(geometry_.FirstMutatedLeftIndexOffsetInCurrentPartition(),
                   last_extrusion_break_offset.left);
    ABSL_DCHECK_GE(geometry_.FirstMutatedRightIndexOffsetInCurrentPartition(),
                   last_extrusion_break_offset.right);
    StrokeOutline& last_outline = outlines_[num_outlines_ - 1];
    last_outline.TruncateIndices({
        .left = geometry_.FirstMutatedLeftIndexOffsetInCurrentPartition() -
                last_extrusion_break_offset.left,
        .right = geometry_.FirstMutatedRightIndexOffsetInCurrentPartition() -
                 last_extrusion_break_offset.right,
    });
  }
  // Clear any later outlines.
  for (uint32_t i = max_num_outlines; i < outlines_.size(); i++) {
    outlines_[i].TruncateIndices({0, 0});
  }
}

void BrushTipExtruder::Restore() {
  extrusions_.resize(saved_extrusion_data_count_);
  absl::c_copy(deleted_save_point_extrusions_,
               extrusions_.end() - deleted_save_point_extrusions_.size());
  geometry_.RevertToSavePoint();
  TruncateOutlines();
}

void BrushTipExtruder::ClearSinceLastExtrusionBreak(
    std::vector<BrushTipExtrusion>::iterator first_extrusion_to_erase,
    bool triggered_by_volatile_extrusion) {
  ABSL_CHECK(first_extrusion_to_erase != extrusions_.end());

  // We're about to delete extrusions; if they contributed to the state when
  // we last called `Save()`, then we'll need to be able to replace them if
  // `Restore()` is called.
  //
  // If any of the following conditions are true, then we don't preserve the
  // extrusions:
  // - We are extruding fixed states; these occur after `Restore()` but before
  //   `Save()`, so there is no saved state
  // - We already have deleted extrusions; this means that we already cleared
  //   at least once since saving, so any extrusions currently past the last
  //   break were not present at the save point
  // - We are only deleting extrusions that are at indices greater than
  //   `saved_extrusion_data_count_`; these were not present at the save point
  //
  // NOMUTANTS -- removing the condition on `triggered_by_volatile_extrusion`
  // does not affect the correctness; `Save()` is called after the fixed states
  // are extruded, which clears `deleted_save_point_extrusions_`.
  if (triggered_by_volatile_extrusion &&
      deleted_save_point_extrusions_.empty() &&
      static_cast<size_t>(
          std::distance(extrusions_.begin(), first_extrusion_to_erase)) <
          saved_extrusion_data_count_) {
    std::copy(first_extrusion_to_erase,
              extrusions_.begin() + saved_extrusion_data_count_,
              std::back_inserter(deleted_save_point_extrusions_));
  }

  extrusions_.erase(first_extrusion_to_erase, extrusions_.end());
  geometry_.ClearSinceLastExtrusionBreak();
  TruncateOutlines();
  ClearCachedPartialBounds();
}

bool BrushTipExtruder::TryAppendNonBreakPointState(
    const BrushTipState& tip_state, bool is_volatile_state,
    bool is_last_state) {
  using ResultType =
      ::ink::strokes_internal::ConstrainedBrushTipExtrusion::ResultType;

  BrushTipExtrusion new_data(tip_state, brush_epsilon_);

  if (extrusions_.empty() || extrusions_.back().IsBreakPoint()) {
    // This extrusion does not interact with anything before it, either because
    // this is the first extrusion or because the last one was a break point. So
    // it's always OK to add.
    extrusions_.push_back(std::move(new_data));
    return true;
  }

  // The maximum number of iterations to perform when searching for an
  // intermediate tip state, if the given one doesn't have good tangents. This
  // gives us a result that is within ~0.78% of the optimal intermediate state.
  constexpr int kMaxIterations = 7;

  ConstrainedBrushTipExtrusion result = ConstrainBrushTipExtrusion(
      extrusions_.back(), new_data, brush_epsilon_, kMaxIterations);

  switch (result.result_type) {
    case ResultType::kProposedExtrusionIsValid:
      // The new tip shape is good, ship it.
      extrusions_.push_back(std::move(new_data));
      break;
    case ResultType::kConstrainedExtrusionFound: {
      // If multiple consecutive states are constrained such that the shape is
      // too similar to the previous shape, then the brush will appear to be
      // unable to "catch up" to the changes in the tip. To avoid this, we
      // reject constrained states whose interpolation value is too close to
      // zero; this gives the next state a little bit more space, which makes it
      // more likely that we can construct something closer to the desired
      // shape.
      // Note that we don't reject the state if it's the last one in the batch,
      // to prevent the stroke from lagging behind the input.
      // TODO: b/317366793 - This value may need to be tuned.
      constexpr float kMinimumLerpAmount = 0.1;
      if (!is_last_state && result.lerp_amount < kMinimumLerpAmount) {
        return false;
      }

      extrusions_.push_back(std::move(result.extrusion));
      break;
    }
    case ResultType::kProposedExtrusionContainsLastExtrusion: {
      // The last tip shape is contained within the new one; find which tip
      // shapes since the last extrusion break are also contained. We iterate
      // backwards because once we find any tip shape that isn't contained, we
      // can consider the rest to not be contained; even if they do loop back
      // around, we treat it as separate overlapping geometry.
      bool found_non_contained_tip_shape = false;
      auto it = extrusions_.rbegin() + 1;
      while (it != extrusions_.rend() && !it->IsBreakPoint()) {
        if (!new_data.GetShape().Contains(it->GetShape())) {
          // Some, but not all, of the states since the last extrusion break are
          // contained within the new tip shape; add a new break-point to
          // disconnect them.
          // TODO: b/317364849 - This might create more self-overlap than we
          // really want. Once we have more usage experience, we should revisit
          // whether there are cases in which we don't want to create a new
          // break-point.
          ExtrudeBreakPoint();
          found_non_contained_tip_shape = true;
          break;
        }

        ++it;
      }

      if (!found_non_contained_tip_shape) {
        // The entirety of the stroke since the last break-point is contained.
        // Clear everything added since the last break-point and restart.
        ClearSinceLastExtrusionBreak(it.base(), is_volatile_state);
      }
      extrusions_.push_back(std::move(new_data));
      break;
    }
    case ResultType::kLastExtrusionContainsProposedExtrusion:
      // The new tip shape is contained within the last one, so doesn't
      // contribute to the geometry. Reject this state.
      return false;
    case ResultType::kCannotFindValidIntermediateExtrusion:
      // We couldn't construct good tangents for the new tip shape, nor could we
      // find an intermediate shape with good tangents. Reject it.
      return false;
  }

  // TODO: b/317363625 - If we added a break-point because the new state
  // contained some prior ones, we should modify the opacity on vertices
  // belonging to contained tip shapes to give it a "fade out" effect instead
  // of a jarring overlap on semi-transparent strokes.

  return true;
}

namespace {

// This calculates a transform that maps from the vertex position to the texture
// surface UV-coordinates for the particle generated from `tip_state`. See also
// `StrokeVertex::surface_uv`.
AffineTransform ComputeParticleSurfaceUvTransform(
    const BrushTipState& tip_state) {
  // This transform takes tip size, position, and rotation into account, but
  // deliberately ignores tip slant, pinch, and corner rounding.
  return AffineTransform::Translate({0.5, 0.5}) *
         AffineTransform::Scale(1.0f / tip_state.width,
                                1.0f / tip_state.height) *
         AffineTransform::Rotate(-tip_state.rotation) *
         AffineTransform::Translate(
             {-tip_state.position.x, -tip_state.position.y});
}

// Appends and processes new "left" and "right" vertices in `geometry`.
void ExtrudeGeometry(const ExtrusionPoints& points,
                     const BrushTipState& tip_state,
                     float simplification_threshold,
                     bool apply_particle_surface_uv,
                     brush_tip_extruder_internal::Geometry& geometry) {
  // TODO: b/271837965 - Add calculation of winding texture coordinates.

  // TODO: b/271837965 - Investigate if we should interpolate per-vertex
  // color-shifts between adjacent tip states instead of feeding the same values
  // for every vertex per call to this function.

  // Calculate color "shift" values, each within the range [-1, 1] (for the sake
  // of simplier vertex packing). The color shift components that actually
  // represent [0, 2] multipliers will be decoded in the shader.
  float opacity_shift = tip_state.opacity_multiplier - 1.f;
  std::array<float, 3> hsl_shift = {tip_state.hue_offset_in_full_turns,
                                    tip_state.saturation_multiplier - 1.f,
                                    tip_state.luminosity_shift};

  AffineTransform position_to_particle_surface_uv =
      ComputeParticleSurfaceUvTransform(tip_state);

  auto compute_surface_uv =
      [apply_particle_surface_uv,
       &position_to_particle_surface_uv](Point p) -> Point {
    // If we don't need surface UVs, we default to (0, 0).
    if (!apply_particle_surface_uv) return {0, 0};

    Point transformed = position_to_particle_surface_uv.Apply(p);

    // Surface UVs must be lie in the interval [0, 1]; however, we may end up
    // with values outside of that due to floating-point precision loss, so we
    // clamp it to that interval.
    return {std::clamp(transformed.x, 0.f, 1.f),
            std::clamp(transformed.y, 0.f, 1.f)};
  };

  for (Point point : points.left) {
    geometry.AppendLeftVertex(point, opacity_shift, hsl_shift,
                              compute_surface_uv(point),
                              tip_state.texture_animation_progress_offset);
  }
  for (Point point : points.right) {
    geometry.AppendRightVertex(point, opacity_shift, hsl_shift,
                               compute_surface_uv(point),
                               tip_state.texture_animation_progress_offset);
  }
  geometry.ProcessNewVertices(simplification_threshold, tip_state);
}

}  // namespace

void BrushTipExtruder::Extrude(const BrushTipState& tip_state,
                               bool is_volatile_state, bool is_last_state) {
  if (tip_state.width < brush_epsilon_ && tip_state.height < brush_epsilon_) {
    ExtrudeBreakPoint();
    return;
  }

  if (!TryAppendNonBreakPointState(tip_state, is_volatile_state, is_last_state))
    return;

  auto end_iter = extrusions_.end();
  if (extrusions_.size() < 2 || (end_iter - 2)->IsBreakPoint()) {
    // There is nothing for this function to do with fewer than two
    // non-break-point extrusion data.
    return;
  }

  current_extrusion_points_.left.clear();
  current_extrusion_points_.right.clear();
  if (extrusions_.size() >= 3 && !(end_iter - 3)->IsBreakPoint()) {
    BrushTipShape::AppendTurnExtrusionPoints(
        (end_iter - 3)->GetShape(), (end_iter - 2)->GetShape(),
        (end_iter - 1)->GetShape(), max_chord_height_,
        current_extrusion_points_);
  } else {
    // The second to last extrusion data has a shape and either:
    //   A) `extrusions_.size()` is 2,
    // or
    //   B) `extrusions_.size()` 3 or more, but the third to last data is an
    //      extrusion break-point, which is equivalent to (A).
    BrushTipShape::AppendStartcapExtrusionPoints(
        (end_iter - 2)->GetShape(), (end_iter - 1)->GetShape(),
        max_chord_height_, current_extrusion_points_);
  }

  const BrushTipState& extruded_state = (end_iter - 2)->GetState();
  ExtrudeGeometry(current_extrusion_points_, extruded_state,
                  simplification_threshold_, is_winding_texture_particle_brush_,
                  geometry_);
}

void BrushTipExtruder::ExtrudeBreakPoint() {
  auto end_iter = extrusions_.end();
  if (extrusions_.empty() || (end_iter - 1)->IsBreakPoint()) {
    return;
  }

  current_extrusion_points_.left.clear();
  current_extrusion_points_.right.clear();
  if (extrusions_.size() > 1 && !(end_iter - 2)->IsBreakPoint()) {
    BrushTipShape::AppendEndcapExtrusionPoints(
        (end_iter - 2)->GetShape(), (end_iter - 1)->GetShape(),
        max_chord_height_, current_extrusion_points_);
  } else {
    // The last extrusion data has a shape and either:
    //   A) `extrusions_.size()` is 1,
    // or
    //   B) `extrusions_.size()` is 2 or more, but the second to last data
    //      is an extrusion break-point.
    BrushTipShape::AppendWholeShapeExtrusionPoints(
        (end_iter - 1)->GetShape(), max_chord_height_,
        /* forward_direction = */ {1, 0}, current_extrusion_points_);
  }

  ExtrudeGeometry(current_extrusion_points_, extrusions_.back().GetState(),
                  simplification_threshold_, is_winding_texture_particle_brush_,
                  geometry_);

  // If no new geometry was added after the last breakpoint, we don't need to
  // do anything.
  Geometry::IndexCounts counts_at_last_break =
      geometry_.IndexCountsAtLastExtrusionBreak();

  if (geometry_.LeftSide().indices.size() <= counts_at_last_break.left &&
      geometry_.RightSide().indices.size() <= counts_at_last_break.right) {
    return;
  }

  // Allocate more capacity if we need it.
  if (outlines_.size() == geometry_.ExtrusionBreakCount()) {
    outlines_.resize(outlines_.size() * 2);
  }

  // Add to the count of outlines if we're completing an entirely new outline
  // instead of refinishing an in-progress one.
  if (num_outlines_ == geometry_.ExtrusionBreakCount()) {
    num_outlines_++;
  }

  geometry_.AddExtrusionBreak();
  extrusions_.emplace_back(BrushTipExtrusion::BreakPoint{});

  StrokeOutline& outline = outlines_[num_outlines_ - 1];
  ABSL_DCHECK_EQ(geometry_.ExtrusionBreakCount(), num_outlines_);
  StrokeOutline::IndexCounts outline_counts = outline.GetIndexCounts();
  ABSL_DCHECK_GE(geometry_.LeftSide().indices.size(),
                 counts_at_last_break.left + outline_counts.left);
  absl::Span<const uint32_t> new_left_indices =
      absl::MakeSpan(geometry_.LeftSide().indices)
          .subspan(counts_at_last_break.left + outline_counts.left);
  ABSL_DCHECK_GE(geometry_.RightSide().indices.size(),
                 counts_at_last_break.right + outline_counts.right);
  absl::Span<const uint32_t> new_right_indices =
      absl::MakeSpan(geometry_.RightSide().indices)
          .subspan(counts_at_last_break.right + outline_counts.right);
  outline.AppendNewIndices(new_left_indices, new_right_indices);
}

}  // namespace ink::strokes_internal
