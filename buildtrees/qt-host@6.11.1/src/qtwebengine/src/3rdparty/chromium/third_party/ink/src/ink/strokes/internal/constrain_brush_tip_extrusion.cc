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

#include "ink/strokes/internal/constrain_brush_tip_extrusion.h"

#include <utility>

#include "absl/log/absl_check.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/strokes/internal/brush_tip_extrusion.h"
#include "ink/strokes/internal/brush_tip_state.h"

namespace ink::strokes_internal {

using ::ink::geometry_internal::Lerp;
using ResultType =
    ::ink::strokes_internal::ConstrainedBrushTipExtrusion::ResultType;
using TangentQuality =
    ::ink::strokes_internal::BrushTipExtrusion::TangentQuality;

ConstrainedBrushTipExtrusion ConstrainBrushTipExtrusion(
    const BrushTipExtrusion& last_extrusion,
    const BrushTipExtrusion& proposed_extrusion,
    float min_nonzero_radius_and_separation, int max_iterations) {
  // The tolerance used to determine if the centers are the extrusions are
  // sufficiently close to be considered not moving.
  // TODO: b/317366793 - This value may need to be tuned.
  const float kStationaryTol = 0.1 * min_nonzero_radius_and_separation;
  TangentQuality tangent_quality = BrushTipExtrusion::EvaluateTangentQuality(
      last_extrusion, proposed_extrusion, kStationaryTol);

  if (tangent_quality == TangentQuality::kNoTangentsFirstContainsSecond) {
    return {.result_type = ResultType::kLastExtrusionContainsProposedExtrusion,
            .lerp_amount = -1};
  }

  if (tangent_quality == TangentQuality::kNoTangentsSecondContainsFirst) {
    // TODO: b/317364849 - We may want to consider not giving up in some cases,
    // and attempting to find an intermediate shape, to avoid having too much
    // overlap from adding many nearby break-points. We'll need to figure out
    // how much is "too much", and what heuristic we can use to get the amount
    // we want.
    return {.result_type = ResultType::kProposedExtrusionContainsLastExtrusion,
            .lerp_amount = -1};
  }

  if (tangent_quality == TangentQuality::kGoodTangents) {
    // We can construct good tangents to `proposed_shape`, use it as is.
    return {.result_type = ResultType::kProposedExtrusionIsValid,
            .lerp_amount = -1};
  }

  // If the brush tip has not moved, then we expect that we won't be able to
  // find an intermediate shape with good tangents. This is because all of the
  // shape paramters except `pinch` are symmetric, so a change that results in
  // one corner leaving `last_shape` also results in a corner leaving
  // `last_shape` on the opposite side.
  // Because `pinch` is not symmetric (at least, not symmetric along the
  // y-axis), it's possible that there exist cases in which we can find
  // intermediate shapes with good tangents; but I haven't found an example of
  // this yet.
  if (Distance(last_extrusion.GetState().position,
               proposed_extrusion.GetState().position) < kStationaryTol) {
    return {.result_type = ResultType::kCannotFindValidIntermediateExtrusion,
            .lerp_amount = -1};
  }

  // We can't construct good tangents to `proposed_shape`, so we iteratively
  // search for an intermediate shape that we can construct good tangents for.
  // We know that the the interpolated extrusion at t = 0 will have good
  // tangents (because it's the same shape, just offset), and that the
  // interpolated extrusion at t = 1 has bad tangents (because we just checked).
  // Somewhere in that interval, it must change from good to bad, so we use the
  // bisection method to hone in on where that change is.

  auto lerp_extrusion = [&last_extrusion, &proposed_extrusion,
                         min_nonzero_radius_and_separation](float lerp_amount) {
    return BrushTipExtrusion{BrushTipState::LerpShapeAttributes(
                                 last_extrusion.GetState(),
                                 proposed_extrusion.GetState(), lerp_amount),
                             min_nonzero_radius_and_separation};
  };

  float lower_bound = 0;
  float upper_bound = 1;
  BrushTipExtrusion current_best_guess = lerp_extrusion(0);
  if (BrushTipExtrusion::EvaluateTangentQuality(
          last_extrusion, current_best_guess, kStationaryTol) !=
      TangentQuality::kGoodTangents) {
    // TODO: b/323763534 - Find a repro test case for this branch.
    return {.result_type = ResultType::kCannotFindValidIntermediateExtrusion,
            .lerp_amount = -1};
  }

  for (int iter = 0; iter < max_iterations; ++iter) {
    // Consistency check; these should be guaranteed by the logic below.
    ABSL_DCHECK(BrushTipExtrusion::EvaluateTangentQuality(
                    last_extrusion, current_best_guess, kStationaryTol) ==
                TangentQuality::kGoodTangents);
    ABSL_DCHECK_LE(lower_bound, upper_bound);

    float next_lerp_amount = Lerp(lower_bound, upper_bound, 0.5);
    BrushTipExtrusion next_guess = lerp_extrusion(next_lerp_amount);

    if (BrushTipExtrusion::EvaluateTangentQuality(last_extrusion, next_guess,
                                                  kStationaryTol) ==
        TangentQuality::kGoodTangents) {
      lower_bound = next_lerp_amount;
      current_best_guess = std::move(next_guess);
    } else {
      upper_bound = next_lerp_amount;
    }
  }

  // We want to return a result that definitely has good tangents, so we always
  // return the lower bound.
  return {.result_type = ResultType::kConstrainedExtrusionFound,
          .lerp_amount = lower_bound,
          .extrusion = std::move(current_best_guess)};
}

}  // namespace ink::strokes_internal
