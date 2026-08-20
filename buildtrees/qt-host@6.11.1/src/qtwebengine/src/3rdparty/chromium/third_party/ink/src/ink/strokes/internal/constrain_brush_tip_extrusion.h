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

#ifndef INK_STROKES_INTERNAL_CONSTRAIN_BRUSH_TIP_EXTRUSION_H_
#define INK_STROKES_INTERNAL_CONSTRAIN_BRUSH_TIP_EXTRUSION_H_

#include "ink/strokes/internal/brush_tip_extrusion.h"

namespace ink::strokes_internal {

// The result of `ConstrainBrushTipExtrusion`.
struct ConstrainedBrushTipExtrusion {
  // The category of result.
  enum class ResultType {
    // We can construct good tangents to the proposed extrusion, no change is
    // necessary.
    kProposedExtrusionIsValid,
    // We could not construct good tangents to the proposed extrusion, so we
    // found an intermediate extrusion that we could construct good tangents to.
    kConstrainedExtrusionFound,
    // The last extrusion is entirely contained in the proposed extrusion, so we
    // can't construct good tangents. Note that a valid intermediate extrusion
    // may actually exist; however, we get a better stroke shape by adding an
    // extrusion break-point (which is handled in `BrushTipExtruder`).
    kProposedExtrusionContainsLastExtrusion,
    // The proposed extrusion is entirely contained in the last extrusion, so we
    // can't construct good tangents. Note that a valid intermediate extrusion
    // may actually exist; however, we get a better stroke shape by discarding
    // the proposed extrusion (which is handled in `BrushTipExtruder`).
    kLastExtrusionContainsProposedExtrusion,
    // The intermediate extrusion at `lerp_amount` = 0 is invalid; a valid
    // intermediate extrusion does not exist.
    kCannotFindValidIntermediateExtrusion,
  };
  ResultType result_type;
  // If `result_type` == `kConstrainedExtrusionFound`, this will be the
  // interpolation amount between `last_extrusion` and `proposed_extrusion` that
  // was used to construct the intermediate extrusion. Otherwise, this should be
  // be ignored, and will be set to -1.
  float lerp_amount;
  // If `result_type` == `kConstrainedExtrusionFound`, this will be the
  // intermediate extrusion with good tangents. It will be created by
  // interpolating between `last_extrusion` and `proposed_extrusion` using
  // `BrushTipShape::LerpShapeAttributes`.
  // If `result_type` != `kConstrainedExtrusionFound`, this should be ignored,
  // and will be a default constructed `BrushTipExtrusion`.
  BrushTipExtrusion extrusion;
};

// Given the last extrusion and the proposed next one, checks whether we can
// construct "good" tangents between them, and if not, attempts to find an
// intermediate extrusion shape such that we can find "good" tangents. Having
// "good" tangents means that when we connect the two extrusion shapes by their
// tangents, all of the perimeter circles of the shapes are fully contained with
// with the joined shape.
//
// `last_extrusion` and `proposed_extrusion` are the `BrushTipExtrusion`s that
// we're attempting to find tangents for. `min_nonzero_radius_and_separation`
// should be the epsilon value used to construct `last_extrusion` and
// `proposed_extrusion`, and will be used to construct any intermediate
// extrusions. `max_iterations` indicates how many iterations to perform when
// searching for an intermediate shape; the maximum error between the returned
// `ConstrainedBrushTipExtrusion::lerp_amount` and the optimal result is
// 0.5^`max_iterations`.
//
// This is used in the `BrushTipExtruder` to constrain the outline geometry
// before triangulation, to reduce the occurance of bad self-intersections and
// incorrectly-winding triangles.
ConstrainedBrushTipExtrusion ConstrainBrushTipExtrusion(
    const BrushTipExtrusion& last_extrusion,
    const BrushTipExtrusion& proposed_extrusion,
    float min_nonzero_radius_and_separation, int max_iterations);

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_CONSTRAIN_BRUSH_TIP_EXTRUSION_H_
