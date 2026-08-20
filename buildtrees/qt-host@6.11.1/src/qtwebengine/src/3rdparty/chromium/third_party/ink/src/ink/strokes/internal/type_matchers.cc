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

#include "ink/strokes/internal/type_matchers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/inlined_vector.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/internal/test_matchers.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_shape.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/types/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ::testing::AllOf;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::PrintToString;

MATCHER_P(BrushTipStateEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushTipState (expected: ", PrintToString(expected),
                       ")")) {
  return ExplainMatchResult(
      AllOf(
          Field("position", &BrushTipState::position,
                PointEq(expected.position)),
          Field("width", &BrushTipState::width, FloatEq(expected.width)),
          Field("height", &BrushTipState::height, FloatEq(expected.height)),
          Field("percent_radius", &BrushTipState::percent_radius,
                FloatEq(expected.percent_radius)),
          Field("rotation", &BrushTipState::rotation,
                AngleEq(expected.rotation)),
          Field("slant", &BrushTipState::slant, AngleEq(expected.slant)),
          Field("pinch", &BrushTipState::pinch, FloatEq(expected.pinch)),
          Field("hue_offset_in_full_turns",
                &BrushTipState::hue_offset_in_full_turns,
                FloatEq(expected.hue_offset_in_full_turns)),
          Field("saturation_multiplier", &BrushTipState::saturation_multiplier,
                FloatEq(expected.saturation_multiplier)),
          Field("luminosity_shift", &BrushTipState::luminosity_shift,
                FloatEq(expected.luminosity_shift)),
          Field("opacity_multiplier", &BrushTipState::opacity_multiplier,
                FloatEq(expected.opacity_multiplier))),
      arg, result_listener);
}
MATCHER_P2(BrushTipStateNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximate equal"
                                 : "approximately equals",
                        " BrushTipState (expected: ", PrintToString(expected),
                        ", tolerance: ", tolerance, ")")) {
  return ExplainMatchResult(
      AllOf(
          Field("position", &BrushTipState::position,
                PointNear(expected.position, tolerance)),
          Field("width", &BrushTipState::width,
                FloatNear(expected.width, tolerance)),
          Field("height", &BrushTipState::height,
                FloatNear(expected.height, tolerance)),
          Field("percent_radius", &BrushTipState::percent_radius,
                FloatNear(expected.percent_radius, tolerance)),
          Field("rotation", &BrushTipState::rotation,
                AngleNear(expected.rotation, tolerance)),
          Field("slant", &BrushTipState::slant,
                AngleNear(expected.slant, tolerance)),
          Field("pinch", &BrushTipState::pinch,
                FloatNear(expected.pinch, tolerance)),
          Field("hue_offset_in_full_turns",
                &BrushTipState::hue_offset_in_full_turns,
                FloatNear(expected.hue_offset_in_full_turns, tolerance)),
          Field("saturation_multiplier", &BrushTipState::saturation_multiplier,
                FloatNear(expected.saturation_multiplier, tolerance)),
          Field("luminosity_shift", &BrushTipState::luminosity_shift,
                FloatNear(expected.luminosity_shift, tolerance)),
          Field("opacity_multiplier", &BrushTipState::opacity_multiplier,
                FloatNear(expected.opacity_multiplier, tolerance))),
      arg, result_listener);
}

MATCHER_P(BrushTipShapeEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushTipShape (expected: ", PrintToString(expected),
                       ")")) {
  absl::InlinedVector<Matcher<geometry_internal::Circle>, 4> circle_matchers;
  for (const auto& c : expected.PerimeterCircles()) {
    circle_matchers.push_back(geometry_internal::CircleEq(c));
  }
  return ExplainMatchResult(
      AllOf(Property("Center", &BrushTipShape::Center,
                     PointEq(expected.Center())),
            Property("PerimeterCircles", &BrushTipShape::PerimeterCircles,
                     ElementsAreArray(circle_matchers))),
      arg, result_listener);
}
MATCHER_P2(BrushTipShapeNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " BrushTipShape (expected: ", PrintToString(expected),
                        ", tolerance: ", tolerance, ")")) {
  absl::InlinedVector<Matcher<geometry_internal::Circle>, 4> circle_matchers;
  for (const auto& c : expected.PerimeterCircles()) {
    circle_matchers.push_back(geometry_internal::CircleNear(c, tolerance));
  }
  return ExplainMatchResult(
      AllOf(Property("Center", &BrushTipShape::Center,
                     PointNear(expected.Center(), tolerance)),
            Property("PerimeterCircles", &BrushTipShape::PerimeterCircles,
                     ElementsAreArray(circle_matchers))),
      arg, result_listener);
}

MATCHER_P(ModeledStrokeInputEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " ModeledStrokeInput (expected: ",
                       PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("position", &ModeledStrokeInput::position,
                  PointEq(expected.position)),
            Field("velocity", &ModeledStrokeInput::velocity,
                  VecEq(expected.velocity)),
            Field("traveled_distance", &ModeledStrokeInput::traveled_distance,
                  FloatEq(expected.traveled_distance)),
            Field("elapsed_time", &ModeledStrokeInput::elapsed_time,
                  Duration32Eq(expected.elapsed_time)),
            Field("pressure", &ModeledStrokeInput::pressure,
                  FloatEq(expected.pressure)),
            Field("tilt", &ModeledStrokeInput::tilt, AngleEq(expected.tilt)),
            Field("orientation", &ModeledStrokeInput::orientation,
                  AngleEq(expected.orientation))),
      arg, result_listener);
}
MATCHER_P2(ModeledStrokeInputNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " ModeledStrokeInput (expected: ",
                        PrintToString(expected), ", tolerance: ", tolerance,
                        ")")) {
  return ExplainMatchResult(
      AllOf(Field("position", &ModeledStrokeInput::position,
                  PointNear(expected.position, tolerance)),
            Field("velocity", &ModeledStrokeInput::velocity,
                  VecNear(expected.velocity, tolerance)),
            Field("traveled_distance", &ModeledStrokeInput::traveled_distance,
                  FloatNear(expected.traveled_distance, tolerance)),
            Field("elapsed_time", &ModeledStrokeInput::elapsed_time,
                  Duration32Near(expected.elapsed_time, tolerance)),
            Field("pressure", &ModeledStrokeInput::pressure,
                  FloatNear(expected.pressure, tolerance)),
            Field("tilt", &ModeledStrokeInput::tilt,
                  AngleNear(expected.tilt, tolerance)),
            Field("orientation", &ModeledStrokeInput::orientation,
                  NormalizedAngleNear(expected.orientation, tolerance))),
      arg, result_listener);
}

}  // namespace

Matcher<BrushTipState> BrushTipStateEq(const BrushTipState& expected) {
  return BrushTipStateEqMatcher(expected);
}
Matcher<BrushTipState> BrushTipStateNear(const BrushTipState& expected,
                                         float tolerance) {
  return BrushTipStateNearMatcher(expected, tolerance);
}
Matcher<BrushTipShape> BrushTipShapeEq(const BrushTipShape& expected) {
  return BrushTipShapeEqMatcher(expected);
}
Matcher<BrushTipShape> BrushTipShapeNear(const BrushTipShape& expected,
                                         float tolerance) {
  return BrushTipShapeNearMatcher(expected, tolerance);
}
Matcher<ModeledStrokeInput> ModeledStrokeInputEq(
    const ModeledStrokeInput& expected) {
  return ModeledStrokeInputEqMatcher(expected);
}
Matcher<ModeledStrokeInput> ModeledStrokeInputNear(
    const ModeledStrokeInput& expected, float tolerance) {
  return ModeledStrokeInputNearMatcher(expected, tolerance);
}

}  // namespace ink::strokes_internal
