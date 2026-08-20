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

#include "ink/strokes/internal/brush_tip_state.h"

#include "ink/geometry/internal/algorithms.h"

namespace ink::strokes_internal {

BrushTipState BrushTipState::LerpShapeAttributes(const BrushTipState& a,
                                                 const BrushTipState& b,
                                                 float t) {
  using ::ink::geometry_internal::Lerp;
  using ::ink::geometry_internal::NormalizedAngleLerp;

  return {
      .position = b.position,
      .width = Lerp(a.width, b.width, t),
      .height = Lerp(a.height, b.height, t),
      .percent_radius = Lerp(a.percent_radius, b.percent_radius, t),
      .rotation = NormalizedAngleLerp(a.rotation, b.rotation, t),
      .slant = Lerp(a.slant, b.slant, t),
      .pinch = Lerp(a.pinch, b.pinch, t),
      .hue_offset_in_full_turns = b.hue_offset_in_full_turns,
      .saturation_multiplier = b.saturation_multiplier,
      .luminosity_shift = b.luminosity_shift,
      .opacity_multiplier = b.opacity_multiplier,
  };
}

}  // namespace ink::strokes_internal
