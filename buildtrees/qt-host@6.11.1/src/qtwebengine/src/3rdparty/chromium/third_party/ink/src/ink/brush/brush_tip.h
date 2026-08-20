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

#ifndef INK_STROKES_BRUSH_BRUSH_TIP_H_
#define INK_STROKES_BRUSH_BRUSH_TIP_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "ink/brush/brush_behavior.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"
#include "ink/types/duration.h"

namespace ink {

// A `BrushTip` consists of parameters that control how stroke inputs are used
// to model the tip shape and color, and create vertices for the stroke mesh.
//
// The specification can be considered in two parts:
//   1. Parameters for the base shape of the tip as a function of `Brush` size.
//   2. An array of `BrushBehavior`s that allow dynamic properties of each input
//      to augment the tip shape and color.
//
// Depending on the combination of values, the tip can be shaped as a rounded
// parallelogram, circle, or stadium. Through `BrushBehavior`s, the tip can
// produce a per-vertex HSLA color shift that can be used to augment the `Brush`
// color when drawing. The default values below produce a static circular tip
// shape with diameter equal to the `Brush` size and no color shift.
struct BrushTip {
  // 2D scale used to calculate the initial width and height of the tip shape
  // relative to the brush size prior to applying `slant` and `rotation`.
  //
  // The base width and height of the tip will be equal to the brush size
  // multiplied by `scale.x` and `scale.y` respectively. Valid values must be
  // finite and non-negative, with at least one value greater than zero.
  Vec scale = {1, 1};
  // A normalized value in the range [0, 1] that is used to calculate the
  // initial radius of curvature for the tip's corners. A value of 0 results in
  // sharp corners and a value of 1 results in the maximum radius of curvature
  // given the current tip dimensions.
  float corner_rounding = 1;
  // Angle used to calculate the initial slant of the tip shape prior to
  // applying `rotation`.
  //
  // This property is similar to the single-arg CSS skew() transformation.
  // Unlike skew, slant tries to preserve the perimeter of the tip shape as
  // opposed to its area. This is akin to "pressing" a rectangle into a
  // parallelogram with non-right angles while preserving the side lengths.
  //
  // The value should be in the range [-π/2, π/2] radians, and represents the
  // angle by which "vertical" lines of the tip shape will appear rotated about
  // their intersection with the x-axis.
  Angle slant = Angle::Radians(0);
  // A unitless parameter in the range [0, 1] that controls the separation
  // between two of the shape's corners prior to applying `rotation`.
  //
  // The two corners affected lie toward the negative y-axis relative to the
  // center of the tip shape. I.e. the "upper edge" of the shape if positive y
  // is chosen to point "down" in stroke coordinates.
  //
  // If `scale.x` is not 0, different values of `pinch` produce the following
  // shapes:
  //   * A value of 0 will leave the corners unaffected as a rectangle or
  //     parallelogram.
  //   * Values between 0 and 1 will bring the corners closer together to result
  //     in a (possibly slanted) trapezoidal shape.
  //   * A value of 1 will make the two corners coincide and result in a
  //     triangular shape.
  float pinch = 0.f;
  // Angle specifying the initial rotation of the tip shape after applying
  // `scale`, `pinch`, and `slant`.
  Angle rotation = Angle::Radians(0);
  // Scales the opacity of the base brush color for this tip, independent of
  // `brush_behavior`s. A possible example application is a highlighter brush.
  //
  // The multiplier must be in the range [0, 2] and the value ultimately applied
  // can be modified by applicable `brush_behavior`s.
  float opacity_multiplier = 1.f;
  // Parameter controlling emission of particles as a function of distance
  // traveled by the stroke inputs. The value must be finite and non-negative.
  //
  // When this and `particle_gap_duration` are both zero, the stroke will be
  // continuous, unless gaps are introduced dynamically by `BrushBehavior`s.
  // Otherwise, the stroke will be made up of particles. A new particle will be
  // emitted after at least `particle_gap_distance_scale * brush_size` distance
  // has been traveled by the stoke inputs.
  float particle_gap_distance_scale = 0.f;
  // Parameter controlling emission of particles as a function of time elapsed
  // along the stroke. The value must be finite and non-negative.
  //
  // When this and `particle_gap_distance_scale` are both zero, the stroke will
  // be continuous, unless gaps are introduced dynamically by `BrushBehavior`s.
  // Otherwise, the stroke will be made up of particles. Particles will be
  // emitted at most once every `particle_gap_duration`.
  Duration32 particle_gap_duration = Duration32::Zero();

  std::vector<BrushBehavior> behaviors;

  bool operator==(const BrushTip& other) const;
  bool operator!=(const BrushTip& other) const;
};

namespace brush_internal {

// Determines whether the given BrushTip struct is valid to be used in a
// BrushFamily, and returns an error if not.
absl::Status ValidateBrushTip(const BrushTip& tip);

std::string ToFormattedString(const BrushTip& tip);

}  // namespace brush_internal

template <typename Sink>
void AbslStringify(Sink& sink, const BrushTip& tip) {
  sink.Append(brush_internal::ToFormattedString(tip));
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_TIP_H_
