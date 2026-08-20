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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/type_matchers.h"

namespace ink::strokes_internal {
namespace {

TEST(BrushTipStateTest, LerpShapeAttributesAtZero) {
  BrushTipState a{.position = {0, 0},
                  .width = 1,
                  .height = 2,
                  .percent_radius = 0,
                  .rotation = Angle::Radians(1),
                  .slant = Angle::Radians(0),
                  .pinch = 0.2,
                  .hue_offset_in_full_turns = 0.5,
                  .saturation_multiplier = 1.1,
                  .luminosity_shift = 0.2,
                  .opacity_multiplier = 1.7};
  BrushTipState b{.position = {5, 4},
                  .width = 3,
                  .height = 0.5,
                  .percent_radius = 0.7,
                  .rotation = Angle::Radians(0),
                  .slant = Angle::Radians(2),
                  .pinch = 0.7,
                  .hue_offset_in_full_turns = 0.1,
                  .saturation_multiplier = 1.6,
                  .luminosity_shift = 0.5,
                  .opacity_multiplier = 1.4};
  // Recall that only `width`. `height`, `percent_radius`, `rotation`, and
  // `slant` are interpolated; the rest are copied from `b`.
  EXPECT_THAT(
      BrushTipState::LerpShapeAttributes(a, b, 0),
      BrushTipStateEq({.position = b.position,
                       .width = a.width,
                       .height = a.height,
                       .percent_radius = a.percent_radius,
                       .rotation = a.rotation,
                       .slant = a.slant,
                       .pinch = a.pinch,
                       .hue_offset_in_full_turns = b.hue_offset_in_full_turns,
                       .saturation_multiplier = b.saturation_multiplier,
                       .luminosity_shift = b.luminosity_shift,
                       .opacity_multiplier = b.opacity_multiplier}));
}

TEST(BrushTipStateTest, LerpShapeAttributesAtOne) {
  BrushTipState a{.position = {0, 0},
                  .width = 1,
                  .height = 2,
                  .percent_radius = 0,
                  .rotation = Angle::Radians(1),
                  .slant = Angle::Radians(0),
                  .pinch = 0.2,
                  .hue_offset_in_full_turns = 0.5,
                  .saturation_multiplier = 1.1,
                  .luminosity_shift = 0.2,
                  .opacity_multiplier = 1.7};
  BrushTipState b{.position = {5, 4},
                  .width = 3,
                  .height = 0.5,
                  .percent_radius = 0.7,
                  .rotation = Angle::Radians(0),
                  .slant = Angle::Radians(2),
                  .pinch = 0.7,
                  .hue_offset_in_full_turns = 0.1,
                  .saturation_multiplier = 1.6,
                  .luminosity_shift = 0.5,
                  .opacity_multiplier = 1.4};
  EXPECT_THAT(BrushTipState::LerpShapeAttributes(a, b, 1), BrushTipStateEq(b));
}

TEST(BrushTipStateTest, LerpShapeAttributesBetweenZeroAndOne) {
  BrushTipState a{.position = {0, 0},
                  .width = 1,
                  .height = 2,
                  .percent_radius = 0,
                  .rotation = Angle::Radians(1),
                  .slant = Angle::Radians(0),
                  .pinch = 0.2,
                  .hue_offset_in_full_turns = 0.5,
                  .saturation_multiplier = 1.1,
                  .luminosity_shift = 0.2,
                  .opacity_multiplier = 1.7};
  BrushTipState b{.position = {5, 4},
                  .width = 3,
                  .height = 0.5,
                  .percent_radius = 0.7,
                  .rotation = Angle::Radians(0),
                  .slant = Angle::Radians(2),
                  .pinch = 0.7,
                  .hue_offset_in_full_turns = 0.1,
                  .saturation_multiplier = 1.6,
                  .luminosity_shift = 0.5,
                  .opacity_multiplier = 1.4};
  // Recall that only `width`. `height`, `percent_radius`, `rotation`, and
  // `slant` are interpolated; the rest are copied from `b`.
  EXPECT_THAT(
      BrushTipState::LerpShapeAttributes(a, b, 0.5),
      BrushTipStateEq({.position = b.position,
                       .width = 2,
                       .height = 1.25,
                       .percent_radius = 0.35,
                       .rotation = Angle::Radians(0.5),
                       .slant = Angle::Radians(1),
                       .pinch = 0.45,
                       .hue_offset_in_full_turns = b.hue_offset_in_full_turns,
                       .saturation_multiplier = b.saturation_multiplier,
                       .luminosity_shift = b.luminosity_shift,
                       .opacity_multiplier = b.opacity_multiplier}));
}

TEST(BrushTipStateTest, LerpShapeAttributesLessThanZero) {
  BrushTipState a{.position = {0, 0},
                  .width = 1,
                  .height = 2,
                  .percent_radius = 0,
                  .rotation = Angle::Radians(1),
                  .slant = Angle::Radians(0),
                  .pinch = 0.2,
                  .hue_offset_in_full_turns = 0.5,
                  .saturation_multiplier = 1.1,
                  .luminosity_shift = 0.2,
                  .opacity_multiplier = 1.7};
  BrushTipState b{.position = {5, 4},
                  .width = 3,
                  .height = 0.5,
                  .percent_radius = 0.7,
                  .rotation = Angle::Radians(0),
                  .slant = Angle::Radians(2),
                  .pinch = 0.7,
                  .hue_offset_in_full_turns = 0.1,
                  .saturation_multiplier = 1.6,
                  .luminosity_shift = 0.5,
                  .opacity_multiplier = 1.4};
  // Recall that only `width`. `height`, `percent_radius`, `rotation`, and
  // `slant` are interpolated; the rest are copied from `b`.
  EXPECT_THAT(
      BrushTipState::LerpShapeAttributes(a, b, -2),
      BrushTipStateEq({.position = b.position,
                       .width = -3,
                       .height = 5,
                       .percent_radius = -1.4,
                       .rotation = Angle::Radians(3),
                       .slant = Angle::Radians(-4),
                       .pinch = -0.8,
                       .hue_offset_in_full_turns = b.hue_offset_in_full_turns,
                       .saturation_multiplier = b.saturation_multiplier,
                       .luminosity_shift = b.luminosity_shift,
                       .opacity_multiplier = b.opacity_multiplier}));
}

TEST(BrushTipStateTest, LerpShapeAttributesGreaterThanOne) {
  BrushTipState a{.position = {0, 0},
                  .width = 1,
                  .height = 2,
                  .percent_radius = 0,
                  .rotation = Angle::Radians(1),
                  .slant = Angle::Radians(0),
                  .pinch = 0.2,
                  .hue_offset_in_full_turns = 0.5,
                  .saturation_multiplier = 1.1,
                  .luminosity_shift = 0.2,
                  .opacity_multiplier = 1.7};
  BrushTipState b{.position = {5, 4},
                  .width = 3,
                  .height = 0.5,
                  .percent_radius = 0.7,
                  .rotation = Angle::Radians(0),
                  .slant = Angle::Radians(2),
                  .pinch = 0.7,
                  .hue_offset_in_full_turns = 0.1,
                  .saturation_multiplier = 1.6,
                  .luminosity_shift = 0.5,
                  .opacity_multiplier = 1.4};
  // Recall that only `width`. `height`, `percent_radius`, `rotation`, and
  // `slant` are interpolated; the rest are copied from `b`.
  EXPECT_THAT(
      BrushTipState::LerpShapeAttributes(a, b, 2),
      BrushTipStateEq({.position = b.position,
                       .width = 5,
                       .height = -1,
                       .percent_radius = 1.4,
                       .rotation = kFullTurn - Angle::Radians(1),
                       .slant = Angle::Radians(4),
                       .pinch = 1.2,
                       .hue_offset_in_full_turns = b.hue_offset_in_full_turns,
                       .saturation_multiplier = b.saturation_multiplier,
                       .luminosity_shift = b.luminosity_shift,
                       .opacity_multiplier = b.opacity_multiplier}));
}

TEST(BrushTipStateTest, LerpShapeAttributesRotationAboutBoundary) {
  BrushTipState a{.rotation = kFullTurn / 8};
  BrushTipState b{.rotation = 7 * kFullTurn / 8};

  // Interpolation should go through the shorter path that crosses zero.
  EXPECT_THAT(BrushTipState::LerpShapeAttributes(a, b, 0.5).rotation,
              AngleNear(Angle::Radians(0), 0.001));
}

}  // namespace
}  // namespace ink::strokes_internal
