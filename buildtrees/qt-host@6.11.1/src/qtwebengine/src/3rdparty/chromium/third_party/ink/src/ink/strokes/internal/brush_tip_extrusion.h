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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUSION_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUSION_H_

#include <optional>
#include <utility>

#include "ink/strokes/internal/brush_tip_shape.h"
#include "ink/strokes/internal/brush_tip_state.h"

namespace ink::strokes_internal {

// Helper type representing either a `BrushTipState` with associated
// `BrushTipShape`, or a "break-point" in extrusion.
class BrushTipExtrusion {
 public:
  // Tag-type representing a break-point.
  //
  // `BrushTipExtruder` adds a break-point when extrusion comes to one or more
  // tip states that introduce a gap in the extrusion by having width and height
  // less than the brush epsilon value.
  struct BreakPoint {};

  BrushTipExtrusion() = default;
  explicit BrushTipExtrusion(BreakPoint) : BrushTipExtrusion() {}
  BrushTipExtrusion(const BrushTipState& state,
                    float min_nonzero_radius_and_separation)
      : tip_state_and_shape_(std::make_pair(
            state, BrushTipShape(state, min_nonzero_radius_and_separation))) {}

  bool IsBreakPoint() const { return !tip_state_and_shape_.has_value(); }
  const BrushTipState& GetState() const { return tip_state_and_shape_->first; }
  const BrushTipShape& GetShape() const { return tip_state_and_shape_->second; }

  // Evaluates whether we can construct tangents between `first` and `second`,
  // and whether those tangents are "good"; i.e. whether the shape formed by
  // connecting `first` and `second` covers all of `first` and `second`.
  enum class TangentQuality {
    // We can't construct tangents, because `first` contains `second`.
    kNoTangentsFirstContainsSecond,
    // We can't construct tangents, because `second` contains `first`.
    kNoTangentsSecondContainsFirst,
    // We can construct tangents, but the joined shape doesn't cover all of
    // `first` and `second`.
    kBadTangentsJoinedShapeDoesNotCoverInputShapes,
    // We can construct tangents, and the joined shape covers `first` and
    // `second`.
    kGoodTangents,
  };
  // `travel_threshold` is used to determine if the centers of the extrusions
  // are sufficiently close to be considered not moving.
  static TangentQuality EvaluateTangentQuality(const BrushTipExtrusion& first,
                                               const BrushTipExtrusion& second,
                                               float travel_threshold);

 private:
  std::optional<std::pair<BrushTipState, BrushTipShape>> tip_state_and_shape_;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUSION_H_
