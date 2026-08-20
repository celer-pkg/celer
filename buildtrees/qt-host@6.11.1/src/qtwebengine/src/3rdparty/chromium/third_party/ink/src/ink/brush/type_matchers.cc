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

#include "ink/brush/type_matchers.h"

#include <tuple>
#include <variant>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/geometry/type_matchers.h"
#include "ink/types/type_matchers.h"

namespace ink {
namespace {

using ::testing::_;
using ::testing::ContainerEq;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatEq;
using ::testing::Matcher;
using ::testing::Pointwise;
using ::testing::Property;
using ::testing::VariantWith;

MATCHER_P(CubicBezierParametersEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " CubicBezier (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(
          Field("x1", &EasingFunction::CubicBezier::x1, FloatEq(expected.x1)),
          Field("y1", &EasingFunction::CubicBezier::y1, FloatEq(expected.y1)),
          Field("x2", &EasingFunction::CubicBezier::x2, FloatEq(expected.x2)),
          Field("y2", &EasingFunction::CubicBezier::y2, FloatEq(expected.y2))),
      arg, result_listener);
}

MATCHER_P(LinearParametersEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Linear (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      Field("points", &EasingFunction::Linear::points, Eq(expected.points)),
      arg, result_listener);
}

MATCHER_P(StepsParametersEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Steps (expected: ", ::testing::PrintToString(expected),
                       ")")) {
  return ExplainMatchResult(
      AllOf(Field("steps", &EasingFunction::Steps::step_count,
                  Eq(expected.step_count)),
            Field("step_position", &EasingFunction::Steps::step_position,
                  Eq(expected.step_position))),
      arg, result_listener);
}

[[maybe_unused]] Matcher<EasingFunction::Parameters> EasingFunctionParametersEq(
    EasingFunction::Predefined predefined) {
  return VariantWith<EasingFunction::Predefined>(Eq(predefined));
}

[[maybe_unused]] Matcher<EasingFunction::Parameters> EasingFunctionParametersEq(
    const EasingFunction::CubicBezier& bezier) {
  return VariantWith<EasingFunction::CubicBezier>(
      CubicBezierParametersEqMatcher(bezier));
}

[[maybe_unused]] Matcher<EasingFunction::Parameters> EasingFunctionParametersEq(
    const EasingFunction::Linear& linear) {
  return VariantWith<EasingFunction::Linear>(LinearParametersEqMatcher(linear));
}

[[maybe_unused]] Matcher<EasingFunction::Parameters> EasingFunctionParametersEq(
    const EasingFunction::Steps& steps) {
  return VariantWith<EasingFunction::Steps>(StepsParametersEqMatcher(steps));
}

MATCHER_P(EasingFunctionEqMatcher, expected, "") {
  return ExplainMatchResult(
      Field(
          "parameters", &EasingFunction::parameters,
          std::visit(
              [](auto&& params) { return EasingFunctionParametersEq(params); },
              expected.parameters)),
      arg, result_listener);
}

MATCHER_P(EnabledToolTypesEqMatcher, expected, "") {
  return ExplainMatchResult(
      AllOf(
          Field(&BrushBehavior::EnabledToolTypes::unknown,
                Eq(expected.unknown)),
          Field(&BrushBehavior::EnabledToolTypes::mouse, Eq(expected.mouse)),
          Field(&BrushBehavior::EnabledToolTypes::touch, Eq(expected.touch)),
          Field(&BrushBehavior::EnabledToolTypes::stylus, Eq(expected.stylus))),
      arg, result_listener);
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::SourceNode& expected) {
  return VariantWith<BrushBehavior::SourceNode>(AllOf(
      Field("source", &BrushBehavior::SourceNode::source, Eq(expected.source)),
      Field("source_out_of_range_behavior",
            &BrushBehavior::SourceNode::source_out_of_range_behavior,
            Eq(expected.source_out_of_range_behavior)),
      Field("source_value_range",
            &BrushBehavior::SourceNode::source_value_range,
            ContainerEq(expected.source_value_range))));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::ConstantNode& expected) {
  return VariantWith<BrushBehavior::ConstantNode>(Field(
      "value", &BrushBehavior::ConstantNode::value, FloatEq(expected.value)));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::NoiseNode& expected) {
  return VariantWith<BrushBehavior::NoiseNode>(
      AllOf(Field("seed", &BrushBehavior::NoiseNode::seed, Eq(expected.seed)),
            Field("vary_over", &BrushBehavior::NoiseNode::vary_over,
                  Eq(expected.vary_over)),
            Field("base_period", &BrushBehavior::NoiseNode::base_period,
                  FloatEq(expected.base_period))));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::FallbackFilterNode& expected) {
  return VariantWith<BrushBehavior::FallbackFilterNode>(Field(
      "is_fallback_for", &BrushBehavior::FallbackFilterNode::is_fallback_for,
      Eq(expected.is_fallback_for)));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::ToolTypeFilterNode& expected) {
  return VariantWith<BrushBehavior::ToolTypeFilterNode>(
      Field("enabled_tool_types",
            &BrushBehavior::ToolTypeFilterNode::enabled_tool_types,
            EnabledToolTypesEqMatcher(expected.enabled_tool_types)));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::DampingNode& expected) {
  return VariantWith<BrushBehavior::DampingNode>(
      AllOf(Field("damping_source", &BrushBehavior::DampingNode::damping_source,
                  Eq(expected.damping_source)),
            Field("damping_gap", &BrushBehavior::DampingNode::damping_gap,
                  FloatEq(expected.damping_gap))));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::ResponseNode& expected) {
  return VariantWith<BrushBehavior::ResponseNode>(
      Field("response_curve", &BrushBehavior::ResponseNode::response_curve,
            EasingFunctionEqMatcher(expected.response_curve)));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::BinaryOpNode& expected) {
  return VariantWith<BrushBehavior::BinaryOpNode>(
      Field("operation", &BrushBehavior::BinaryOpNode::operation,
            Eq(expected.operation)));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::InterpolationNode& expected) {
  return VariantWith<BrushBehavior::InterpolationNode>(AllOf(
      Field("interpolation", &BrushBehavior::InterpolationNode::interpolation,
            Eq(expected.interpolation))));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::TargetNode& expected) {
  return VariantWith<BrushBehavior::TargetNode>(AllOf(
      Field("target", &BrushBehavior::TargetNode::target, Eq(expected.target)),
      Field("target_modifier_range",
            &BrushBehavior::TargetNode::target_modifier_range,
            ContainerEq(expected.target_modifier_range))));
}

Matcher<BrushBehavior::Node> BrushBehaviorNodeEqMatcher(
    const BrushBehavior::PolarTargetNode& expected) {
  return VariantWith<BrushBehavior::PolarTargetNode>(AllOf(
      Field("target", &BrushBehavior::PolarTargetNode::target,
            Eq(expected.target)),
      Field("angle_range", &BrushBehavior::PolarTargetNode::angle_range,
            ContainerEq(expected.angle_range)),
      Field("magnitude_range", &BrushBehavior::PolarTargetNode::magnitude_range,
            ContainerEq(expected.magnitude_range))));
}

MATCHER(BrushBehaviorNodePointwiseEqMatcher, "") {
  return ExplainMatchResult(BrushBehaviorNodeEq(std::get<1>(arg)),
                            std::get<0>(arg), result_listener);
}

MATCHER_P(BrushBehaviorEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushBehavior (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      Field("nodes", &BrushBehavior::nodes,
            Pointwise(BrushBehaviorNodeEq(), expected.nodes)),
      arg, result_listener);
}

MATCHER(BrushBehaviorPointwiseEqMatcher, "") {
  return ExplainMatchResult(BrushBehaviorEq(std::get<1>(arg)), std::get<0>(arg),
                            result_listener);
}

MATCHER_P(BrushTipEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushTip (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("scale", &BrushTip::scale, VecEq(expected.scale)),
            Field("corner_rounding", &BrushTip::corner_rounding,
                  FloatEq(expected.corner_rounding)),
            Field("slant", &BrushTip::slant, AngleEq(expected.slant)),
            Field("pinch", &BrushTip::pinch, FloatEq(expected.pinch)),
            Field("rotation", &BrushTip::rotation, AngleEq(expected.rotation)),
            Field("opacity_multiplier", &BrushTip::opacity_multiplier,
                  FloatEq(expected.opacity_multiplier)),
            Field("particle_gap_distance_scale",
                  &BrushTip::particle_gap_distance_scale,
                  FloatEq(expected.particle_gap_distance_scale)),
            Field("particle_gap_duration", &BrushTip::particle_gap_duration,
                  Duration32Eq(expected.particle_gap_duration)),
            Field("behaviors", &BrushTip::behaviors,
                  Pointwise(BrushBehaviorEq(), expected.behaviors))),
      arg, result_listener);
}

MATCHER(BrushTipPointwiseEqMatcher, "") {
  return ExplainMatchResult(BrushTipEq(std::get<1>(arg)), std::get<0>(arg),
                            result_listener);
}

MATCHER(BrushPaintTextureKeyframeEqMatcher, "") {
  return ExplainMatchResult(AllOf(Field(&BrushPaint::TextureKeyframe::progress,
                                        Eq(std::get<1>(arg).progress)),
                                  Field(&BrushPaint::TextureKeyframe::size,
                                        Eq(std::get<1>(arg).size)),
                                  Field(&BrushPaint::TextureKeyframe::offset,
                                        Eq(std::get<1>(arg).offset)),
                                  Field(&BrushPaint::TextureKeyframe::rotation,
                                        Eq(std::get<1>(arg).rotation)),
                                  Field(&BrushPaint::TextureKeyframe::opacity,
                                        Eq(std::get<1>(arg).opacity))),
                            std::get<0>(arg), result_listener);
}

MATCHER_P(BrushPaintTextureLayerEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushPaintTextureLayer (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field(&BrushPaint::TextureLayer::client_texture_id,
                  Eq(expected.client_texture_id)),
            Field(&BrushPaint::TextureLayer::mapping, Eq(expected.mapping)),
            Field(&BrushPaint::TextureLayer::origin, Eq(expected.origin)),
            Field(&BrushPaint::TextureLayer::size_unit, Eq(expected.size_unit)),
            Field(&BrushPaint::TextureLayer::wrap_x, Eq(expected.wrap_x)),
            Field(&BrushPaint::TextureLayer::wrap_y, Eq(expected.wrap_y)),
            Field(&BrushPaint::TextureLayer::size, Eq(expected.size)),
            Field(&BrushPaint::TextureLayer::offset, Eq(expected.offset)),
            Field(&BrushPaint::TextureLayer::rotation, Eq(expected.rotation)),
            Field(&BrushPaint::TextureLayer::size_jitter,
                  Eq(expected.size_jitter)),
            Field(&BrushPaint::TextureLayer::offset_jitter,
                  Eq(expected.offset_jitter)),
            Field(&BrushPaint::TextureLayer::rotation_jitter,
                  Eq(expected.rotation_jitter)),
            Field(&BrushPaint::TextureLayer::opacity, Eq(expected.opacity)),
            Field("keyframes", &BrushPaint::TextureLayer::keyframes,
                  Pointwise(BrushPaintTextureKeyframeEqMatcher(),
                            expected.keyframes)),
            Field(&BrushPaint::TextureLayer::blend_mode,
                  Eq(expected.blend_mode))),
      arg, result_listener);
}

MATCHER(BrushPaintTextureLayerPointwiseEqMatcher, "") {
  return ExplainMatchResult(BrushPaintTextureLayerEq(std::get<1>(arg)),
                            std::get<0>(arg), result_listener);
}

MATCHER_P(BrushPaintEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushPaint (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field(
          "texture_layers", &BrushPaint::texture_layers,
          Pointwise(BrushPaintTextureLayerEq(), expected.texture_layers))),
      arg, result_listener);
}

MATCHER_P(BrushCoatEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushCoat (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("tip", &BrushCoat::tip, BrushTipEq(expected.tip)),
            Field("paint", &BrushCoat::paint, BrushPaintEq(expected.paint))),
      arg, result_listener);
}

MATCHER(BrushCoatPointwiseEqMatcher, "") {
  return ExplainMatchResult(BrushCoatEq(std::get<1>(arg)), std::get<0>(arg),
                            result_listener);
}

Matcher<BrushFamily::InputModel> BrushFamilyInputModelEqMatcher(
    const BrushFamily::SpringModel& input_model) {
  return VariantWith<BrushFamily::SpringModel>(_);  // no fields to match
}

Matcher<BrushFamily::InputModel> BrushFamilyInputModelEqMatcher(
    const BrushFamily::ExperimentalRawPositionModel& input_model) {
  return VariantWith<BrushFamily::ExperimentalRawPositionModel>(
      _);  // no fields to match
}

[[maybe_unused]] Matcher<BrushFamily::InputModel> BrushFamilyInputModelEq(
    const BrushFamily::InputModel& expected) {
  return std::visit(
      [](const auto& expected) {
        return BrushFamilyInputModelEqMatcher(expected);
      },
      expected);
}

MATCHER_P(BrushFamilyEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " BrushFamily (expected: ",
                       ::testing::PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Property(&BrushFamily::GetCoats,
                     Pointwise(BrushCoatEq(), expected.GetCoats())),
            Property(&BrushFamily::GetClientBrushFamilyId,
                     Eq(expected.GetClientBrushFamilyId())),
            Property(&BrushFamily::GetInputModel,
                     BrushFamilyInputModelEq(expected.GetInputModel()))),
      arg, result_listener);
}

MATCHER_P(BrushEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Brush (expected: ", ::testing::PrintToString(expected),
                       ")")) {
  return ExplainMatchResult(
      AllOf(Property(&Brush::GetFamily, BrushFamilyEq(expected.GetFamily())),
            Property(&Brush::GetColor, Eq(expected.GetColor())),
            Property(&Brush::GetSize, Eq(expected.GetSize())),
            Property(&Brush::GetEpsilon, Eq(expected.GetEpsilon()))),
      arg, result_listener);
}

}  // namespace

Matcher<BrushBehavior::Node> BrushBehaviorNodeEq(
    const BrushBehavior::Node& expected) {
  return std::visit(
      [](const auto& expected) { return BrushBehaviorNodeEqMatcher(expected); },
      expected);
}

Matcher<std::tuple<BrushBehavior::Node, BrushBehavior::Node>>
BrushBehaviorNodeEq() {
  return BrushBehaviorNodePointwiseEqMatcher();
}

Matcher<BrushBehavior> BrushBehaviorEq(const BrushBehavior& expected) {
  return BrushBehaviorEqMatcher(expected);
}

Matcher<std::tuple<BrushBehavior, BrushBehavior>> BrushBehaviorEq() {
  return BrushBehaviorPointwiseEqMatcher();
}

Matcher<BrushTip> BrushTipEq(const BrushTip& expected) {
  return BrushTipEqMatcher(expected);
}

Matcher<std::tuple<BrushTip, BrushTip>> BrushTipEq() {
  return BrushTipPointwiseEqMatcher();
}

Matcher<BrushPaint> BrushPaintEq(const BrushPaint& expected) {
  return BrushPaintEqMatcher(expected);
}

Matcher<BrushPaint::TextureLayer> BrushPaintTextureLayerEq(
    const BrushPaint::TextureLayer& expected) {
  return BrushPaintTextureLayerEqMatcher(expected);
}

Matcher<std::tuple<BrushPaint::TextureLayer, BrushPaint::TextureLayer>>
BrushPaintTextureLayerEq() {
  return BrushPaintTextureLayerPointwiseEqMatcher();
}

Matcher<BrushCoat> BrushCoatEq(const BrushCoat& expected) {
  return BrushCoatEqMatcher(expected);
}

Matcher<std::tuple<BrushCoat, BrushCoat>> BrushCoatEq() {
  return BrushCoatPointwiseEqMatcher();
}

Matcher<BrushFamily> BrushFamilyEq(const BrushFamily& expected) {
  return BrushFamilyEqMatcher(expected);
}

Matcher<Brush> BrushEq(const Brush& expected) {
  return BrushEqMatcher(expected);
}

}  // namespace ink
