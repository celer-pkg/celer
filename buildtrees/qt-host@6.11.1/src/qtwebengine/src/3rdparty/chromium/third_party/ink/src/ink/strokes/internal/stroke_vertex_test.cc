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

#include "ink/strokes/internal/stroke_vertex.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"
#include "ink/types/small_array.h"

namespace ink::strokes_internal {
namespace {

using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::FloatEq;
using ::testing::Optional;
using ::testing::Pointwise;

TEST(StrokeVertexLabelTest, LabelConstantProperties) {
  EXPECT_EQ(StrokeVertex::kInteriorLabel.DecodeSideCategory(),
            StrokeVertex::SideCategory::kInterior);
  EXPECT_EQ(StrokeVertex::kInteriorLabel.DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kInterior);
  EXPECT_EQ(StrokeVertex::kInteriorLabel.DecodeMargin(), 0);
  EXPECT_EQ(StrokeVertex::kInteriorLabel.DerivativeOutsetSign(), 0);

  EXPECT_EQ(StrokeVertex::kExteriorLeftLabel.DecodeSideCategory(),
            StrokeVertex::SideCategory::kExteriorLeft);
  EXPECT_GT(StrokeVertex::kExteriorLeftLabel.DecodeMargin(), 0);
  EXPECT_EQ(StrokeVertex::kExteriorLeftLabel.DerivativeOutsetSign(), -1);

  EXPECT_EQ(StrokeVertex::kExteriorRightLabel.DecodeSideCategory(),
            StrokeVertex::SideCategory::kExteriorRight);
  EXPECT_GT(StrokeVertex::kExteriorRightLabel.DecodeMargin(), 0);
  EXPECT_EQ(StrokeVertex::kExteriorRightLabel.DerivativeOutsetSign(), 1);

  EXPECT_EQ(StrokeVertex::kExteriorFrontLabel.DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kExteriorFront);
  EXPECT_GT(StrokeVertex::kExteriorFrontLabel.DecodeMargin(), 0);
  EXPECT_EQ(StrokeVertex::kExteriorFrontLabel.DerivativeOutsetSign(), -1);

  EXPECT_EQ(StrokeVertex::kExteriorBackLabel.DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kExteriorBack);
  EXPECT_GT(StrokeVertex::kExteriorBackLabel.DecodeMargin(), 0);
  EXPECT_EQ(StrokeVertex::kExteriorBackLabel.DerivativeOutsetSign(), 1);
}

TEST(StrokeVertexLabelTest, DecodedMarginDoesNotExceedMaximumFiniteValue) {
  ASSERT_LT(StrokeVertex::kMaximumMargin,
            std::numeric_limits<float>::infinity());
  EXPECT_EQ(StrokeVertex::kExteriorLeftLabel
                .WithMargin(std::numeric_limits<float>::infinity())
                .DecodeMargin(),
            StrokeVertex::kMaximumMargin);
}

TEST(StrokeVertexLabelTest, InteriorWithAnyMarginIsStillInterior) {
  EXPECT_EQ(StrokeVertex::kInteriorLabel.WithMargin(0),
            StrokeVertex::kInteriorLabel);
  EXPECT_EQ(StrokeVertex::kInteriorLabel.WithMargin(1),
            StrokeVertex::kInteriorLabel);
  EXPECT_EQ(StrokeVertex::kInteriorLabel.WithMargin(
                std::numeric_limits<float>::infinity()),
            StrokeVertex::kInteriorLabel);
}

TEST(StrokeVertexLabelTest, NonInteriorWithZeroMarginMaintainsCategory) {
  EXPECT_EQ(StrokeVertex::kExteriorLeftLabel.WithMargin(0).DecodeSideCategory(),
            StrokeVertex::SideCategory::kExteriorLeft);
  EXPECT_EQ(
      StrokeVertex::kExteriorRightLabel.WithMargin(0).DecodeSideCategory(),
      StrokeVertex::SideCategory::kExteriorRight);

  EXPECT_EQ(
      StrokeVertex::kExteriorFrontLabel.WithMargin(0).DecodeForwardCategory(),
      StrokeVertex::ForwardCategory::kExteriorFront);
  EXPECT_EQ(
      StrokeVertex::kExteriorBackLabel.WithMargin(0).DecodeForwardCategory(),
      StrokeVertex::ForwardCategory::kExteriorBack);
}

TEST(StrokeVertexLabelTest, NonInteriorWithInfiniteMarginMaintainsCategory) {
  constexpr float kInf = std::numeric_limits<float>::infinity();
  EXPECT_EQ(
      StrokeVertex::kExteriorLeftLabel.WithMargin(kInf).DecodeSideCategory(),
      StrokeVertex::SideCategory::kExteriorLeft);
  EXPECT_EQ(
      StrokeVertex::kExteriorRightLabel.WithMargin(kInf).DecodeSideCategory(),
      StrokeVertex::SideCategory::kExteriorRight);

  EXPECT_EQ(StrokeVertex::kExteriorFrontLabel.WithMargin(kInf)
                .DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kExteriorFront);
  EXPECT_EQ(
      StrokeVertex::kExteriorBackLabel.WithMargin(kInf).DecodeForwardCategory(),
      StrokeVertex::ForwardCategory::kExteriorBack);
}

TEST(StrokeVertexLabelTest, NegativeMarginIsEquivalentToZero) {
  EXPECT_EQ(StrokeVertex::kExteriorLeftLabel.WithMargin(-1),
            StrokeVertex::kExteriorLeftLabel.WithMargin(0));
}

TEST(StrokeVertexTest,
     MakeCustomPackingArrayReturnsNulloptForPositionAndDerivatives) {
  auto mesh_format = MeshFormat::Create(
      {{MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
        MeshFormat::AttributeId::kPosition},
       {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
        MeshFormat::AttributeId::kSideDerivative},
       {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
        MeshFormat::AttributeId::kForwardDerivative}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(mesh_format.status(), absl::OkStatus());

  StrokeVertex::CustomPackingArray packing_array =
      StrokeVertex::MakeCustomPackingArray(*mesh_format);
  EXPECT_THAT(packing_array.Size(), Eq(mesh_format->Attributes().size()));
  EXPECT_THAT(packing_array.Values(), Each(Eq(std::nullopt)));
}

TEST(StrokeVertexTest,
     MakeCustomPackingArrayReturnsParamsForUBytePackedColorAndLabels) {
  auto mesh_format = MeshFormat::Create(
      {{MeshFormat::AttributeType::kFloat2Unpacked,
        MeshFormat::AttributeId::kPosition},
       {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
        MeshFormat::AttributeId::kOpacityShift},
       {MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10,
        MeshFormat::AttributeId::kColorShiftHsl},
       {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
        MeshFormat::AttributeId::kSideLabel},
       {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
        MeshFormat::AttributeId::kForwardLabel}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(mesh_format.status(), absl::OkStatus());

  StrokeVertex::CustomPackingArray packing_array =
      StrokeVertex::MakeCustomPackingArray(*mesh_format);
  EXPECT_THAT(
      packing_array.Values(),
      ElementsAre(Eq(std::nullopt),
                  Optional(MeshAttributeCodingParamsEq(
                      {.components = {{.offset = -1, .scale = 1 / 127.f}}})),
                  Optional(MeshAttributeCodingParamsEq(
                      {.components = {{.offset = -1, .scale = 1 / 511.f},
                                      {.offset = -1, .scale = 1 / 511.f},
                                      {.offset = -1, .scale = 1 / 511.f}}})),
                  Optional(MeshAttributeCodingParamsEq(
                      {.components = {{.offset = -128, .scale = 1}}})),
                  Optional(MeshAttributeCodingParamsEq(
                      {.components = {{.offset = -128, .scale = 1}}}))));
}

TEST(StrokeVertexTest, MakeCustomPackingArraySkipsMatchingAttributes) {
  auto mesh_format = MeshFormat::Create(
      {{MeshFormat::AttributeType::kFloat2Unpacked,
        MeshFormat::AttributeId::kPosition},
       {MeshFormat::AttributeType::kFloat1Unpacked,
        MeshFormat::AttributeId::kOpacityShift},
       {MeshFormat::AttributeType::kFloat3Unpacked,
        MeshFormat::AttributeId::kColorShiftHsl},
       {MeshFormat::AttributeType::kFloat2Unpacked,
        MeshFormat::AttributeId::kSideDerivative},
       {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
        MeshFormat::AttributeId::kForwardDerivative}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(mesh_format.status(), absl::OkStatus());

  std::vector<MeshFormat::AttributeId> skipped_attribute_ids = {
      MeshFormat::AttributeId::kOpacityShift,
      MeshFormat::AttributeId::kSideDerivative,
      MeshFormat::AttributeId::kCustom0};
  StrokeVertex::CustomPackingArray packing_array =
      StrokeVertex::MakeCustomPackingArray(*mesh_format, skipped_attribute_ids);

  // The number of elements in the returned array should be reduced by the
  // number of `skipped_attribute_ids` present in the `mesh_format`, which in
  // this case is all but one of them.
  EXPECT_THAT(packing_array.Size(), Eq(mesh_format->Attributes().size() -
                                       (skipped_attribute_ids.size() - 1)));
  EXPECT_THAT(packing_array.Values(), Each(Eq(std::nullopt)));
}

TEST(StrokeVertexTest, MemoryLayoutMatchesUnpackedFullMeshFormat) {
  MeshFormat format = StrokeVertex::FullMeshFormat();

  // At least for now while all attributes are float-based, they have the same
  // alignment requirement. We therefore expect no padding bytes in the struct,
  // and so its size should equal the unpacked stride.
  EXPECT_EQ(format.UnpackedVertexStride(), sizeof(StrokeVertex));

  // Position:
  EXPECT_EQ(StrokeVertex::kFullFormatAttributeIndices.position,
            format.PositionAttributeIndex());
  MeshFormat::Attribute attribute =
      format.Attributes()[StrokeVertex::kFullFormatAttributeIndices.position];
  EXPECT_EQ(attribute.unpacked_offset, offsetof(StrokeVertex, position));
  EXPECT_EQ(attribute.unpacked_width, sizeof(decltype(StrokeVertex::position)));

  // Opacity shift:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.opacity_shift,
            format.Attributes().size());
  attribute = format.Attributes()[StrokeVertex::kFullFormatAttributeIndices
                                      .opacity_shift];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.opacity_shift));
  EXPECT_EQ(
      attribute.unpacked_width,
      sizeof(decltype(StrokeVertex::NonPositionAttributes::opacity_shift)));

  // HSL shift:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.hsl_shift,
            format.Attributes().size());
  attribute =
      format.Attributes()[StrokeVertex::kFullFormatAttributeIndices.hsl_shift];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.hsl_shift));
  EXPECT_EQ(attribute.unpacked_width,
            sizeof(decltype(StrokeVertex::NonPositionAttributes::hsl_shift)));

  // Side derivative:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.side_derivative,
            format.Attributes().size());
  attribute = format.Attributes()[StrokeVertex::kFullFormatAttributeIndices
                                      .side_derivative];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.side_derivative));
  EXPECT_EQ(
      attribute.unpacked_width,
      sizeof(decltype(StrokeVertex::NonPositionAttributes::side_derivative)));

  // Side label:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.side_label,
            format.Attributes().size());
  attribute =
      format.Attributes()[StrokeVertex::kFullFormatAttributeIndices.side_label];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.side_label));
  EXPECT_EQ(attribute.unpacked_width,
            sizeof(decltype(StrokeVertex::NonPositionAttributes::side_label)));

  // Forward derivative:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.forward_derivative,
            format.Attributes().size());
  attribute = format.Attributes()[StrokeVertex::kFullFormatAttributeIndices
                                      .forward_derivative];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.forward_derivative));
  EXPECT_EQ(
      attribute.unpacked_width,
      sizeof(
          decltype(StrokeVertex::NonPositionAttributes::forward_derivative)));

  // Forward label:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.forward_label,
            format.Attributes().size());
  attribute = format.Attributes()[StrokeVertex::kFullFormatAttributeIndices
                                      .forward_label];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.forward_label));
  EXPECT_EQ(
      attribute.unpacked_width,
      sizeof(decltype(StrokeVertex::NonPositionAttributes::forward_label)));

  // Surface UV:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.surface_uv,
            format.Attributes().size());
  attribute =
      format.Attributes()[StrokeVertex::kFullFormatAttributeIndices.surface_uv];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.surface_uv));
  EXPECT_EQ(attribute.unpacked_width,
            sizeof(decltype(StrokeVertex::NonPositionAttributes::surface_uv)));

  // Texture animation offset:
  ASSERT_LT(StrokeVertex::kFullFormatAttributeIndices.animation_offset,
            format.Attributes().size());
  attribute = format.Attributes()[StrokeVertex::kFullFormatAttributeIndices
                                      .animation_offset];
  EXPECT_EQ(attribute.unpacked_offset,
            offsetof(StrokeVertex, non_position_attributes.animation_offset));
  EXPECT_EQ(
      attribute.unpacked_width,
      sizeof(decltype(StrokeVertex::NonPositionAttributes::animation_offset)));
}

TEST(StrokeVertexTest, FindAttributeIndicesResultMatchesFullFormatIndices) {
  StrokeVertex::FormatAttributeIndices indices =
      StrokeVertex::FindAttributeIndices(StrokeVertex::FullMeshFormat());

  EXPECT_EQ(indices.position,
            StrokeVertex::kFullFormatAttributeIndices.position);
  EXPECT_EQ(indices.opacity_shift,
            StrokeVertex::kFullFormatAttributeIndices.opacity_shift);
  EXPECT_EQ(indices.hsl_shift,
            StrokeVertex::kFullFormatAttributeIndices.hsl_shift);
  EXPECT_EQ(indices.side_derivative,
            StrokeVertex::kFullFormatAttributeIndices.side_derivative);
  EXPECT_EQ(indices.side_label,
            StrokeVertex::kFullFormatAttributeIndices.side_label);
  EXPECT_EQ(indices.forward_derivative,
            StrokeVertex::kFullFormatAttributeIndices.forward_derivative);
  EXPECT_EQ(indices.forward_label,
            StrokeVertex::kFullFormatAttributeIndices.forward_label);
  EXPECT_EQ(indices.surface_uv,
            StrokeVertex::kFullFormatAttributeIndices.surface_uv);
  EXPECT_EQ(indices.animation_offset,
            StrokeVertex::kFullFormatAttributeIndices.animation_offset);
}

TEST(StrokeVertexTest, FindAttributeIndicesReturnsMinusOneForNotFound) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat3Unpacked,
                           MeshFormat::AttributeId::kCustom1}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  StrokeVertex::FormatAttributeIndices indices =
      StrokeVertex::FindAttributeIndices(*format);
  EXPECT_EQ(indices.position, 1);
  EXPECT_EQ(indices.opacity_shift, -1);
  EXPECT_EQ(indices.hsl_shift, -1);
  EXPECT_EQ(indices.side_derivative, -1);
  EXPECT_EQ(indices.side_label, -1);
  EXPECT_EQ(indices.forward_derivative, -1);
  EXPECT_EQ(indices.forward_label, -1);
  EXPECT_EQ(indices.surface_uv, -1);
  EXPECT_EQ(indices.animation_offset, -1);
}

TEST(StrokeVertexTest, EqualityOfNonPositionAttributes) {
  StrokeVertex::NonPositionAttributes a = {
      .opacity_shift = 0.3,
      .hsl_shift = {-0.25, 0.5, -0.7},
      .side_derivative = {1, 2},
      .side_label = StrokeVertex::kExteriorLeftLabel,
      .forward_derivative = {3, 4},
      .forward_label = StrokeVertex::kExteriorFrontLabel,
      .surface_uv = {0.75, 0.125},
      .animation_offset = 0.25,
  };
  StrokeVertex::NonPositionAttributes b = a;

  // The "NOLINT" comments help prevent suggestions to use EXPECT_EQ / NE
  // instead but we want to explicitly test the operators, and test them
  // returning both `true` and `false`.

  EXPECT_TRUE(a == b);  // NOLINT

  b.opacity_shift = 0.1;
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.hsl_shift = {-1, 0, 0.5};
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.side_derivative = {2, 3};
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.side_label.encoded_value = 2;
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.forward_derivative = {1, 7};
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.forward_label.encoded_value = 15;
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.surface_uv.y = 0.5;
  EXPECT_FALSE(a == b);  // NOLINT

  a = b;
  b.animation_offset = 0.5;
  EXPECT_FALSE(a == b);  // NOLINT
}

TEST(StrokeVertexTest, LerpNonPositionAttributes) {
  StrokeVertex::Label shared_side_label = StrokeVertex::kExteriorLeftLabel;
  StrokeVertex::NonPositionAttributes a = {
      .opacity_shift = 0.1,
      .hsl_shift = {0.2, 0.3, 0.5},
      .side_derivative = {1, 2},
      .side_label = shared_side_label,
      .forward_derivative = {3, 4},
      .forward_label = StrokeVertex::kExteriorFrontLabel,
      .surface_uv = {0.6, 0.7},
      .animation_offset = 0.25,
  };
  StrokeVertex::NonPositionAttributes b = {
      .opacity_shift = -0.3,
      .hsl_shift = {0.4, 0.5, 0.7},
      .side_derivative = {-5, 4},
      .side_label = shared_side_label,
      .forward_derivative = {8, 1},
      .forward_label = StrokeVertex::kExteriorBackLabel,
      .surface_uv = {0.7, 0.8},
      .animation_offset = 0.25,
  };

  ASSERT_NE(a.side_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(a.forward_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(b.side_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(b.forward_label, StrokeVertex::kInteriorLabel);

  StrokeVertex::NonPositionAttributes result = Lerp(a, b, -0.1);
  EXPECT_FLOAT_EQ(result.opacity_shift, 0.14);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.18, 0.28, 0.48}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, a.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, a.forward_label);
  EXPECT_THAT(result.surface_uv, PointEq({0.59, 0.69}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.25);

  result = Lerp(a, b, 0);
  EXPECT_FLOAT_EQ(result.opacity_shift, 0.1);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.2, 0.3, 0.5}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, a.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, a.forward_label);
  EXPECT_THAT(result.surface_uv, PointEq({0.6, 0.7}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.25);

  result = Lerp(a, b, 0.25);
  EXPECT_FLOAT_EQ(result.opacity_shift, 0);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.25, 0.35, 0.55}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, a.side_label);
  EXPECT_EQ(result.side_label, b.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.surface_uv, PointEq({0.625, 0.725}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.25);

  result = Lerp(a, b, 1);
  EXPECT_FLOAT_EQ(result.opacity_shift, -0.3);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.4, 0.5, 0.7}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, b.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, b.forward_label);
  EXPECT_THAT(result.surface_uv, PointEq({0.7, 0.8}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.25);

  result = Lerp(a, b, 1.2);
  EXPECT_FLOAT_EQ(result.opacity_shift, -0.38);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.44, 0.54, 0.74}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, b.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, b.forward_label);
  EXPECT_THAT(result.surface_uv, PointEq({0.72, 0.82}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.25);
}

TEST(StrokeVertexTest, BarycentricLerpNonPositionAttributes) {
  StrokeVertex::Label ac_side_label = StrokeVertex::kExteriorLeftLabel;
  StrokeVertex::Label b_side_label = StrokeVertex::kExteriorRightLabel;

  StrokeVertex::Label a_forward_label = StrokeVertex::kExteriorFrontLabel;
  StrokeVertex::Label bc_forward_label = StrokeVertex::kExteriorBackLabel;

  StrokeVertex::NonPositionAttributes a = {
      .opacity_shift = -1,
      .hsl_shift = {-0.5, 0, 0.5},
      .side_derivative = {1, 2},
      .side_label = ac_side_label,
      .forward_derivative = {3, 4},
      .forward_label = a_forward_label,
      .surface_uv = {0, 1},
      .animation_offset = 0.5,
  };
  StrokeVertex::NonPositionAttributes b = {
      .opacity_shift = 0,
      .hsl_shift = {0, 0.4, 0.2},
      .side_derivative = {-5, 4},
      .side_label = b_side_label,
      .forward_derivative = {8, 1},
      .forward_label = bc_forward_label,
      .surface_uv = {0.5, 0.5},
      .animation_offset = 0.5,
  };
  StrokeVertex::NonPositionAttributes c = {
      .opacity_shift = 1,
      .hsl_shift = {1, -0.5, -0.2},
      .side_derivative = {3, 5},
      .side_label = ac_side_label,
      .forward_derivative = {8, -2},
      .forward_label = bc_forward_label,
      .surface_uv = {1, 0},
      .animation_offset = 0.5,
  };

  ASSERT_NE(a.side_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(a.forward_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(b.side_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(b.forward_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(c.side_label, StrokeVertex::kInteriorLabel);
  ASSERT_NE(c.forward_label, StrokeVertex::kInteriorLabel);

  StrokeVertex::NonPositionAttributes result =
      BarycentricLerp(a, b, c, {0.25, 0.75, 0});
  EXPECT_FLOAT_EQ(result.opacity_shift, -0.25);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {-0.125, 0.3, 0.275}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.surface_uv, PointEq({0.375, 0.625}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.5);

  result = BarycentricLerp(a, b, c, {0.25, 0, 0.75});
  EXPECT_FLOAT_EQ(result.opacity_shift, 0.5);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.625, -0.375, -0.025}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, a.side_label);
  EXPECT_EQ(result.side_label, c.side_label);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.surface_uv, PointEq({0.75, 0.25}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.5);

  result = BarycentricLerp(a, b, c, {0, 0.25, 0.75});
  EXPECT_FLOAT_EQ(result.opacity_shift, 0.75);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.75, -0.275, -0.1}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, b.forward_label);
  EXPECT_EQ(result.forward_label, c.forward_label);
  EXPECT_THAT(result.surface_uv, PointEq({0.875, 0.125}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.5);

  result = BarycentricLerp(a, b, c, {0.25, 0.25, 0.5});
  EXPECT_FLOAT_EQ(result.opacity_shift, 0.25);
  EXPECT_THAT(result.hsl_shift, Pointwise(FloatEq(), {0.375, -0.15, 0.075}));
  EXPECT_THAT(result.side_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.side_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.forward_derivative, VecEq({0, 0}));
  EXPECT_EQ(result.forward_label, StrokeVertex::kInteriorLabel);
  EXPECT_THAT(result.surface_uv, PointEq({0.625, 0.375}));
  EXPECT_FLOAT_EQ(result.animation_offset, 0.5);
}

TEST(StrokeVertexTest, GetFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  StrokeVertex expected = {
      .position = {1, 2},
      .non_position_attributes =
          {
              .opacity_shift = 0.25,
              .hsl_shift = {0.25, 0.5, 0.75},
              .side_derivative = {3, 4},
              .side_label = StrokeVertex::kExteriorLeftLabel,
              .forward_derivative = {6, 7},
              .forward_label = StrokeVertex::kExteriorBackLabel,
              .surface_uv = {0.8, 0.9},
              .animation_offset = 0.25,
          },
  };

  uint32_t index = 2;
  mesh.SetVertexPosition(index, expected.position);
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.opacity_shift,
      {expected.non_position_attributes.opacity_shift});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.hsl_shift,
      {expected.non_position_attributes.hsl_shift[0],
       expected.non_position_attributes.hsl_shift[1],
       expected.non_position_attributes.hsl_shift[2]});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_derivative,
      {expected.non_position_attributes.side_derivative.x,
       expected.non_position_attributes.side_derivative.y});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_label,
      {expected.non_position_attributes.side_label.encoded_value});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_derivative,
      {expected.non_position_attributes.forward_derivative.x,
       expected.non_position_attributes.forward_derivative.y});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_label,
      {expected.non_position_attributes.forward_label.encoded_value});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.surface_uv,
      {expected.non_position_attributes.surface_uv.x,
       expected.non_position_attributes.surface_uv.y});
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.animation_offset,
      {expected.non_position_attributes.animation_offset});

  StrokeVertex actual = StrokeVertex::GetFromMesh(mesh, index);
  EXPECT_THAT(actual.position, PointEq(expected.position));
  EXPECT_EQ(actual.non_position_attributes, expected.non_position_attributes);
}

TEST(StrokeVertexTest, GetSideDerivativeFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  Vec expected_side_derivative = {1, 2};
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_derivative,
      {expected_side_derivative.x, expected_side_derivative.y});

  EXPECT_THAT(StrokeVertex::GetSideDerivativeFromMesh(mesh, index),
              VecEq(expected_side_derivative));
}

TEST(StrokeVertexTest, GetForwardDerivativeFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 4;
  Vec expected_forward_derivative = {3, 4};
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_derivative,
      {expected_forward_derivative.x, expected_forward_derivative.y});

  EXPECT_THAT(StrokeVertex::GetForwardDerivativeFromMesh(mesh, index),
              VecEq(expected_forward_derivative));
}

TEST(StrokeVertexTest, GetSideLabelFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  StrokeVertex::Label expected_side_label = StrokeVertex::kExteriorRightLabel;
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_label,
      {expected_side_label.encoded_value});

  EXPECT_EQ(StrokeVertex::GetSideLabelFromMesh(mesh, index),
            expected_side_label);
}

TEST(StrokeVertexTest, GetForwardLabelFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  StrokeVertex::Label expected_forward_label = StrokeVertex::kExteriorBackLabel;
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_label,
      {expected_forward_label.encoded_value});

  EXPECT_EQ(StrokeVertex::GetForwardLabelFromMesh(mesh, index),
            expected_forward_label);
}

TEST(StrokeVertexTest, GetSurfaceUvFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  Point expected_surface_uv = {0.75, 0.125};
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.surface_uv,
      {expected_surface_uv.x, expected_surface_uv.y});

  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh, index),
              PointEq(expected_surface_uv));
}

TEST(StrokeVertexTest, GetAnimationOffsetFromMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  float expected_animation_offset = 0.25;
  mesh.SetFloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.animation_offset,
      {expected_animation_offset});

  EXPECT_FLOAT_EQ(StrokeVertex::GetAnimationOffsetFromMesh(mesh, index),
                  expected_animation_offset);
}

TEST(StrokeVertexTest, AppendToMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  StrokeVertex expected = {
      .position = {1, 2},
      .non_position_attributes =
          {
              .opacity_shift = 0.1,
              .hsl_shift = {0.2, 0.3, 0.4},
              .side_derivative = {3, 4},
              .side_label = {.encoded_value = 5},
              .forward_derivative = {6, 7},
              .forward_label = {.encoded_value = 8},
              .surface_uv = {0.9, 0.1},
              .animation_offset = 0.25,
          },
  };

  StrokeVertex::AppendToMesh(mesh, expected);

  EXPECT_EQ(mesh.VertexCount(), 6);
  EXPECT_EQ(mesh.TriangleCount(), 3);

  uint32_t index = 5;

  EXPECT_THAT(mesh.VertexPosition(index), PointEq(expected.position));

  SmallArray<float, 4> attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.opacity_shift);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_FLOAT_EQ(attribute[0], expected.non_position_attributes.opacity_shift);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.hsl_shift);
  ASSERT_EQ(attribute.Size(), 3);
  EXPECT_THAT(attribute.Values(),
              Pointwise(FloatEq(), expected.non_position_attributes.hsl_shift));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_derivative);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Vec{attribute[0], attribute[1]}),
              VecEq(expected.non_position_attributes.side_derivative));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_label);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_EQ(attribute[0],
            expected.non_position_attributes.side_label.encoded_value);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_derivative);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Vec{attribute[0], attribute[1]}),
              VecEq(expected.non_position_attributes.forward_derivative));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_label);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_EQ(attribute[0],
            expected.non_position_attributes.forward_label.encoded_value);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.surface_uv);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Point{attribute[0], attribute[1]}),
              PointEq(expected.non_position_attributes.surface_uv));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.animation_offset);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_FLOAT_EQ(attribute[0],
                  expected.non_position_attributes.animation_offset);
}

TEST(StrokeVertexTest, SetInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  StrokeVertex expected = {
      .position = {1, 2},
      .non_position_attributes =
          {
              .opacity_shift = -0.5,
              .hsl_shift = {-1, 0.7, 0.2},
              .side_derivative = {3, 4},
              .side_label = StrokeVertex::kExteriorLeftLabel,
              .forward_derivative = {6, 7},
              .forward_label = StrokeVertex::kExteriorBackLabel,
              .surface_uv = {0.8, 0.9},
              .animation_offset = 0.25,
          },
  };

  uint32_t index = 3;
  StrokeVertex::SetInMesh(mesh, index, expected);
  EXPECT_EQ(mesh.VertexCount(), 5);
  EXPECT_EQ(mesh.TriangleCount(), 3);

  EXPECT_THAT(mesh.VertexPosition(index), PointEq(expected.position));

  SmallArray<float, 4> attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.opacity_shift);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_FLOAT_EQ(attribute[0], expected.non_position_attributes.opacity_shift);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.hsl_shift);
  ASSERT_EQ(attribute.Size(), 3);
  EXPECT_THAT(attribute.Values(),
              Pointwise(FloatEq(), expected.non_position_attributes.hsl_shift));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_derivative);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Vec{attribute[0], attribute[1]}),
              VecEq(expected.non_position_attributes.side_derivative));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.side_label);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_EQ(attribute[0],
            expected.non_position_attributes.side_label.encoded_value);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_derivative);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Vec{attribute[0], attribute[1]}),
              VecEq(expected.non_position_attributes.forward_derivative));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.forward_label);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_EQ(attribute[0],
            expected.non_position_attributes.forward_label.encoded_value);

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.surface_uv);
  ASSERT_EQ(attribute.Size(), 2);
  EXPECT_THAT((Point{attribute[0], attribute[1]}),
              PointEq(expected.non_position_attributes.surface_uv));

  attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.animation_offset);
  ASSERT_EQ(attribute.Size(), 1);
  EXPECT_FLOAT_EQ(attribute[0],
                  expected.non_position_attributes.animation_offset);
}

TEST(StrokeVertexTest, OutOfBoundsColorShiftValuesAreClamped) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(3, 1);

  StrokeVertex::SetInMesh(
      mesh, 0,
      {.non_position_attributes = {.opacity_shift = -2,
                                   .hsl_shift = {-2, -2, -2}}});
  StrokeVertex::NonPositionAttributes attributes =
      StrokeVertex::GetFromMesh(mesh, 0).non_position_attributes;
  EXPECT_EQ(attributes.opacity_shift, -1);
  EXPECT_THAT(attributes.hsl_shift, Each(-1));

  StrokeVertex::SetInMesh(mesh, 1,
                          {.non_position_attributes = {
                               .opacity_shift = 2, .hsl_shift = {2, 2, 2}}});
  attributes = StrokeVertex::GetFromMesh(mesh, 1).non_position_attributes;
  EXPECT_EQ(attributes.opacity_shift, 1);
  EXPECT_THAT(attributes.hsl_shift, Each(Eq(1)));

  StrokeVertex::AppendToMesh(
      mesh, {.non_position_attributes = {.opacity_shift = -2,
                                         .hsl_shift = {-2, -2, -2}}});
  attributes = StrokeVertex::GetFromMesh(mesh, 3).non_position_attributes;
  EXPECT_EQ(attributes.opacity_shift, -1);
  EXPECT_THAT(attributes.hsl_shift, Each(-1));

  StrokeVertex::AppendToMesh(
      mesh, {.non_position_attributes = {.opacity_shift = 2,
                                         .hsl_shift = {2, 2, 2}}});
  attributes = StrokeVertex::GetFromMesh(mesh, 4).non_position_attributes;
  EXPECT_EQ(attributes.opacity_shift, 1);
  EXPECT_THAT(attributes.hsl_shift, Each(Eq(1)));
}

TEST(StrokeVertexTest, SetSideDerivativeInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 2;
  ASSERT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.side_derivative,
              VecEq({0, 0}));

  Vec expected_side_derivative = {2, 3};
  StrokeVertex::SetSideDerivativeInMesh(mesh, index, expected_side_derivative);
  EXPECT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.side_derivative,
              VecEq(expected_side_derivative));
}

TEST(StrokeVertexTest, SetForwardDerivativeInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 2;
  ASSERT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.forward_derivative,
              VecEq({0, 0}));

  Vec expected_forward_derivative = {2, 3};
  StrokeVertex::SetForwardDerivativeInMesh(mesh, index,
                                           expected_forward_derivative);
  EXPECT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.forward_derivative,
              VecEq(expected_forward_derivative));
}

TEST(StrokeVertexTest, SetSideLabelInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 3;
  ASSERT_THAT(
      StrokeVertex::GetFromMesh(mesh, index).non_position_attributes.side_label,
      Eq(StrokeVertex::kInteriorLabel));

  StrokeVertex::SetSideLabelInMesh(mesh, index,
                                   StrokeVertex::kExteriorLeftLabel);
  EXPECT_THAT(
      StrokeVertex::GetFromMesh(mesh, index).non_position_attributes.side_label,
      Eq(StrokeVertex::kExteriorLeftLabel));
}

TEST(StrokeVertexTest, SetForwardLabelInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  mesh.Resize(5, 3);

  uint32_t index = 1;
  ASSERT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.forward_label,
              Eq(StrokeVertex::kInteriorLabel));

  StrokeVertex::SetForwardLabelInMesh(mesh, index,
                                      StrokeVertex::kExteriorFrontLabel);
  EXPECT_THAT(StrokeVertex::GetFromMesh(mesh, index)
                  .non_position_attributes.forward_label,
              Eq(StrokeVertex::kExteriorFrontLabel));
}

TEST(StrokeVertexDeathTest, MakeCustomPackingArrayWithTooManyAttributes) {
  auto format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom0},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom1},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom2},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom3},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom4},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom5},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom6},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom7},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kCustom8}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  EXPECT_DEATH_IF_SUPPORTED(StrokeVertex::MakeCustomPackingArray(*format), "");
}

}  // namespace
}  // namespace ink::strokes_internal
