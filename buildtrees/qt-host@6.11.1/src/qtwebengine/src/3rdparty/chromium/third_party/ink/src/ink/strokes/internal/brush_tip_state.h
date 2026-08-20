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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_STATE_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_STATE_H_

#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"

namespace ink::strokes_internal {

// Collection of properties describing the modeled state of the `BrushTip` at a
// particular point along the stroke.
struct BrushTipState {
  // The current absolute position of the brush tip center within stroke space.
  Point position;
  // The current width/height of the brush tip shape, measured in stroke units.
  float width;
  float height;
  // The current corner rounding of the brush tip shape, from 0 (fully square)
  // to 1 (fully circular).
  float percent_radius;
  // The current rotation angle of the brush tip shape.
  Angle rotation;
  // The current slant of the brush tip shape, in the range [-π/2, π/2] radians.
  Angle slant;
  // A unitless parameter in the range [0, 1] that controls the separation
  // between two of the brush tip shape's corners prior to applying `rotation`.
  float pinch;
  // An offset to the initial texture animation progress value
  // of the current particle (which is relevant only for strokes with an
  // animated texture) in the range [0, 1).
  float texture_animation_progress_offset = 0.f;
  // The hue angle offset, measured in full turns in the range [0, 1), to
  // add to the brush color at this tip position.
  float hue_offset_in_full_turns = 0.f;
  // The saturation multiplier, in the range [0, 2], to apply to the brush color
  // at this tip position.
  float saturation_multiplier = 1.f;
  // The luminosity shift, in the range [-1, 1], to apply to the brush color at
  // this tip position.
  //
  // TODO: b/344839538 - Once we decide if per-vertex luminosity shift is going
  // to become a multiplier or stay an additive offset, rename this to either
  // `luminosity_multiplier` or `luminosity_offset`.
  float luminosity_shift = 0.f;
  // The opacity multiplier, in the range [0, 2], to apply to the brush color at
  // this tip position.
  float opacity_multiplier = 1.f;

  // TODO: b/271837965 - It may be useful for winding texture coordinates to
  // also add distance traveled.

  // Returns a `BrushTipState` whose `width`, `height`, `percent_radius`,
  // `rotation`, and `slant` are linearly interpolated between 'a' and 'b'; all
  // other fields are copied from `b`. The interpolation of `rotation` will be
  // in the direction of the shortest path around a circle.
  //
  // If `t` is outside the interval [0, 1], then this will extrapolate for those
  // fields. Note that this may result in values that are not valid (e.g. a
  // negative value for `width`), even if `a` and `b` are both valid
  // `BrushTipState`s.
  static BrushTipState LerpShapeAttributes(const BrushTipState& a,
                                           const BrushTipState& b, float t);
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_STATE_H_
