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

#include "ink/rendering/skia/native/internal/mesh_uniform_data.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/color/type_matchers.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/type_matchers.h"
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

namespace ink::skia_native_internal {
namespace {

TEST(MeshUniformDataTest, DefaultConstructed) {
  MeshUniformData data;

  EXPECT_FALSE(data.HasObjectToCanvasLinearComponent());
  EXPECT_FALSE(data.HasBrushColor());
  EXPECT_EQ(data.Get(), nullptr);
}

TEST(MeshUniformDataTest, WithoutUniforms) {
  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      {{.type = SkMeshSpecification::Attribute::Type::kFloat2,
        .offset = 0,
        .name = SkString("position")}},
      /* vertexStride = */ 8,
      /* varyings = */ {}, SkString(R"(
        Varyings main(const Attributes attributes) {
          Varyings varyings;
          varyings.position = attributes.position;
          return varyings;
        }
      )"),
      SkString(R"(
        float2 main(const Varyings varyings) {
          return varyings.position;
        }
      )"));
  ASSERT_NE(result.specification, nullptr);

  MeshUniformData data(*result.specification);

  EXPECT_FALSE(data.HasObjectToCanvasLinearComponent());
  EXPECT_FALSE(data.HasBrushColor());
  EXPECT_EQ(data.Get(), nullptr);
}

// Returns the stored linear component of an affine transform with the
// translation set to zeroes.
AffineTransform GetStoredAffineTransformLinearComponent(const uint8_t* data) {
  std::array<float, 4> stored;
  std::memcpy(stored.data(), data, 4 * sizeof(float));
  // The components are stored in column-major order inside `data`, and are
  // passed to `AffineTransform` in row-major order.
  return AffineTransform(stored[0], stored[2], 0, stored[1], stored[3], 0);
}

TEST(MeshUniformDataTest, WithObjectToCanvasLinearComponent) {
  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      {{.type = SkMeshSpecification::Attribute::Type::kFloat2,
        .offset = 0,
        .name = SkString("position")}},
      /* vertexStride = */ 8,
      /* varyings = */ {}, SkString(R"(
        uniform float4 uObjectToCanvasLinearComponent;

        Varyings main(const Attributes attributes) {
          Varyings varyings;
          varyings.position = attributes.position;
          // Use the transform in some way in case at some point Skia will
          // optimize away inactive uniforms.
          varyings.position += uObjectToCanvasLinearComponent.xy;
          return varyings;
        }
      )"),
      SkString(R"(
        float2 main(const Varyings varyings) {
          return varyings.position;
        }
      )"));

  ASSERT_NE(result.specification, nullptr);
  ASSERT_EQ(result.specification->uniforms().size(), 1);

  MeshUniformData data(*result.specification);

  ASSERT_TRUE(data.HasObjectToCanvasLinearComponent());
  EXPECT_FALSE(data.HasBrushColor());

  // Check that setting the brush color stores the expected bytes:
  data.SetObjectToCanvasLinearComponent(AffineTransform::Rotate(kQuarterTurn));

  sk_sp<const SkData> first_get_data = data.Get();
  ASSERT_NE(first_get_data, nullptr);
  EXPECT_EQ(first_get_data->size(), result.specification->uniformSize());
  EXPECT_THAT(
      GetStoredAffineTransformLinearComponent(
          first_get_data->bytes() + result.specification->uniforms()[0].offset),
      AffineTransformEq(AffineTransform::Rotate(kQuarterTurn)));

  // Setting a new value for the color should not affect the values pointed to
  // by the result of the first call:
  data.SetObjectToCanvasLinearComponent(AffineTransform::Scale(3, 2));

  EXPECT_THAT(
      GetStoredAffineTransformLinearComponent(
          first_get_data->bytes() + result.specification->uniforms()[0].offset),
      AffineTransformEq(AffineTransform::Rotate(kQuarterTurn)));

  sk_sp<const SkData> second_get_data = data.Get();
  EXPECT_NE(first_get_data, second_get_data);

  ASSERT_NE(second_get_data, nullptr);
  EXPECT_EQ(second_get_data->size(), result.specification->uniformSize());
  EXPECT_THAT(GetStoredAffineTransformLinearComponent(
                  second_get_data->bytes() +
                  result.specification->uniforms()[0].offset),
              AffineTransformEq(AffineTransform::Scale(3, 2)));
}

Color GetStoredColor(const uint8_t* data) {
  Color::RgbaFloat stored;
  std::memcpy(&stored, data, sizeof(stored));
  return Color::FromFloat(stored.r, stored.g, stored.b, stored.a,
                          Color::Format::kLinear, ColorSpace::kSrgb);
}

TEST(MeshUniformDataTest, WithBrushColor) {
  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      {{.type = SkMeshSpecification::Attribute::Type::kFloat2,
        .offset = 0,
        .name = SkString("position")}},
      /* vertexStride = */ 8,
      /* varyings = */ {}, SkString(R"(
        Varyings main(const Attributes attributes) {
          Varyings varyings;
          varyings.position = attributes.position;
          return varyings;
        }
      )"),
      SkString(R"(
        layout(color) uniform float4 uBrushColor;

        float2 main(const Varyings varyings, out float4 color) {
          color = uBrushColor;
          return varyings.position;
        }
      )"));

  ASSERT_NE(result.specification, nullptr);
  ASSERT_EQ(result.specification->uniforms().size(), 1);

  MeshUniformData data(*result.specification);

  EXPECT_FALSE(data.HasObjectToCanvasLinearComponent());
  ASSERT_TRUE(data.HasBrushColor());

  // Check that setting the brush color stores the expected bytes:
  data.SetBrushColor(Color::Cyan());

  sk_sp<const SkData> first_get_data = data.Get();
  ASSERT_NE(first_get_data, nullptr);
  EXPECT_EQ(first_get_data->size(), result.specification->uniformSize());
  EXPECT_THAT(GetStoredColor(first_get_data->bytes() +
                             result.specification->uniforms()[0].offset),
              ColorNearlyEquals(Color::Cyan()));

  // Setting a new value for the color should not affect the values pointed to
  // by the result of the first call:
  data.SetBrushColor(Color::Blue());

  EXPECT_THAT(GetStoredColor(first_get_data->bytes() +
                             result.specification->uniforms()[0].offset),
              ColorNearlyEquals(Color::Cyan()));

  sk_sp<const SkData> second_get_data = data.Get();
  EXPECT_NE(first_get_data, second_get_data);

  ASSERT_NE(second_get_data, nullptr);
  EXPECT_EQ(second_get_data->size(), result.specification->uniformSize());
  EXPECT_THAT(GetStoredColor(second_get_data->bytes() +
                             result.specification->uniforms()[0].offset),
              ColorNearlyEquals(Color::Blue()));
}

MeshAttributeCodingParams GetStoredCodingParams(const uint8_t* data) {
  std::array<float, 4> values;
  std::memcpy(values.data(), data, 4 * sizeof(float));
  return {.components = {{.offset = values[0], .scale = values[1]},
                         {.offset = values[2], .scale = values[3]}}};
}

struct MeshFormatAndSpecification {
  MeshFormat format;
  sk_sp<SkMeshSpecification> specification;
};

MeshFormatAndSpecification MakePackedPositionMeshFormatAndSpecification() {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ABSL_CHECK_OK(format);

  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      {{.type = SkMeshSpecification::Attribute::Type::kFloat2,
        .offset = 0,
        .name = SkString("position")}},
      /* vertexStride = */ 8,
      /* varyings = */ {}, SkString(R"(
        uniform float4 uPositionUnpackingTransform;

        Varyings main(const Attributes attributes) {
          Varyings varyings;
          varyings.position =
              uPositionUnpackingTransform.yw * attributes.position +
              uPositionUnpackingTransform.xz;
          return varyings;
        }
      )"),
      SkString(R"(
        float2 main(const Varyings varyings) {
          return varyings.position;
        }
      )"));

  ABSL_CHECK_NE(result.specification, nullptr);
  ABSL_CHECK_EQ(result.specification->uniforms().size(), 1);

  return {
      .format = *std::move(format),
      .specification = std::move(result.specification),
  };
}

TEST(MeshUniformDataTest, WithPositionUnpackingTransform) {
  MeshFormatAndSpecification format_and_spec =
      MakePackedPositionMeshFormatAndSpecification();

  // Check that providing the position unpacking transform stores the expected
  // bytes:
  MeshAttributeCodingParams position_params = {
      .components = {
          {.offset = 0, .scale = 0.5},
          {.offset = 0.7, .scale = 0.25},
      }};

  MeshUniformData data(
      *format_and_spec.specification, format_and_spec.format.Attributes(),
      [&position_params](int /* attribute_index */)
          -> const MeshAttributeCodingParams& { return position_params; });

  EXPECT_FALSE(data.HasObjectToCanvasLinearComponent());
  EXPECT_FALSE(data.HasBrushColor());

  sk_sp<const SkData> sk_data = data.Get();
  ASSERT_NE(sk_data, nullptr);
  EXPECT_EQ(sk_data->size(), format_and_spec.specification->uniformSize());

  EXPECT_THAT(GetStoredCodingParams(
                  sk_data->bytes() +
                  format_and_spec.specification->uniforms()[0].offset),
              MeshAttributeCodingParamsEq(position_params));
}

TEST(MeshUniformDataTest, WithAllMutableUniforms) {
  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      {{.type = SkMeshSpecification::Attribute::Type::kFloat2,
        .offset = 0,
        .name = SkString("position")}},
      /* vertexStride = */ 8,
      /* varyings = */ {}, SkString(R"(
        uniform float4 uObjectToCanvasLinearComponent;

        Varyings main(const Attributes attributes) {
          Varyings varyings;
          varyings.position = attributes.position;
          // Use the transform in some way in case at some point Skia will
          // optimize away inactive uniforms.
          varyings.position += uObjectToCanvasLinearComponent.xy;
          return varyings;
        }
      )"),
      SkString(R"(
        layout(color) uniform float4 uBrushColor;

        float2 main(const Varyings varyings, out float4 color) {
          color = uBrushColor;
          return varyings.position;
        }
      )"));

  ASSERT_NE(result.specification, nullptr);
  ASSERT_EQ(result.specification->uniforms().size(), 2);

  const SkMeshSpecification::Uniform*
      object_to_canvas_linear_component_uniform =
          result.specification->findUniform("uObjectToCanvasLinearComponent");
  ASSERT_NE(object_to_canvas_linear_component_uniform, nullptr);

  const SkMeshSpecification::Uniform* color_uniform =
      result.specification->findUniform("uBrushColor");
  ASSERT_NE(color_uniform, nullptr);

  MeshUniformData data(*result.specification);

  ASSERT_TRUE(data.HasObjectToCanvasLinearComponent());
  ASSERT_TRUE(data.HasBrushColor());

  data.SetBrushColor(Color::Blue());
  sk_sp<const SkData> first_get_data = data.Get();
  ASSERT_NE(first_get_data, nullptr);
  EXPECT_THAT(GetStoredColor(first_get_data->bytes() + color_uniform->offset),
              ColorNearlyEquals(Color::Blue()));

  // Setting the object-to-canvas transform should update the stored value and
  // also not modify the value of the stored color:
  data.SetObjectToCanvasLinearComponent(AffineTransform::ShearX(5.0));
  sk_sp<const SkData> second_get_data = data.Get();
  ASSERT_NE(second_get_data, nullptr);
  EXPECT_THAT(GetStoredAffineTransformLinearComponent(
                  second_get_data->bytes() +
                  object_to_canvas_linear_component_uniform->offset),
              AffineTransformEq(AffineTransform::ShearX(5.0)));
  EXPECT_THAT(GetStoredColor(second_get_data->bytes() + color_uniform->offset),
              ColorNearlyEquals(Color::Blue()));

  // Setting a new value of the color should also not modify the value of the
  // stored object-to-canvas transform:
  data.SetBrushColor(Color::Cyan());
  sk_sp<const SkData> third_get_data = data.Get();
  ASSERT_NE(third_get_data, nullptr);
  EXPECT_THAT(GetStoredAffineTransformLinearComponent(
                  third_get_data->bytes() +
                  object_to_canvas_linear_component_uniform->offset),
              AffineTransformEq(AffineTransform::ShearX(5.0)));
  EXPECT_THAT(GetStoredColor(third_get_data->bytes() + color_uniform->offset),
              ColorNearlyEquals(Color::Cyan()));
}

TEST(MeshUniformDataDeathTest,
     SetObjectToCanvasLinearComponentWithoutUniformPresent) {
  MeshUniformData data;
  ASSERT_FALSE(data.HasObjectToCanvasLinearComponent());
  EXPECT_DEATH_IF_SUPPORTED(
      data.SetObjectToCanvasLinearComponent(AffineTransform::Identity()), "");
}

TEST(MeshUniformDataDeathTest, SetBrushColorWithoutUniformPresent) {
  MeshUniformData data;
  ASSERT_FALSE(data.HasBrushColor());
  EXPECT_DEATH_IF_SUPPORTED(data.SetBrushColor(Color::Blue()), "");
}

TEST(MeshUniformDataDeathTest,
     GetUnpackingTransformReturnsMismatchedComponentCount) {
  MeshFormatAndSpecification format_and_spec =
      MakePackedPositionMeshFormatAndSpecification();

  MeshAttributeCodingParams params;
  params.components.Resize(3);

  EXPECT_DEATH_IF_SUPPORTED(
      MeshUniformData data(
          *format_and_spec.specification, format_and_spec.format.Attributes(),
          [&params](int /* attribute_index */)
              -> const MeshAttributeCodingParams& { return params; }),
      "");
}

}  // namespace
}  // namespace ink::skia_native_internal
