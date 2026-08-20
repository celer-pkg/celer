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

#include "ink/rendering/skia/common_internal/mesh_specification_data.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "ink/brush/brush.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/stroke_vertex.h"
#include "ink/strokes/stroke.h"
#include "ink/types/duration.h"

namespace ink::skia_common_internal {
namespace {

using ::absl_testing::IsOk;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Not;

bool IsValidAttributeType(MeshSpecificationData::AttributeType type) {
  switch (type) {
    case MeshSpecificationData::AttributeType::kFloat2:
    case MeshSpecificationData::AttributeType::kFloat3:
    case MeshSpecificationData::AttributeType::kUByte4:
      return true;
  }
  return false;
}

bool IsValidVaryingType(MeshSpecificationData::VaryingType type) {
  switch (type) {
    case MeshSpecificationData::VaryingType::kFloat2:
    case MeshSpecificationData::VaryingType::kFloat4:
      return true;
  }
  return false;
}

bool IsValidUniformType(MeshSpecificationData::UniformType type) {
  switch (type) {
    case MeshSpecificationData::UniformType::kFloat:
    case MeshSpecificationData::UniformType::kFloat4:
    case MeshSpecificationData::UniformType::kInt:
      return true;
  }
  return false;
}

bool IsValidUniformId(MeshSpecificationData::UniformId id) {
  switch (id) {
    case MeshSpecificationData::UniformId::kObjectToCanvasLinearComponent:
    case MeshSpecificationData::UniformId::kBrushColor:
    case MeshSpecificationData::UniformId::kPositionUnpackingTransform:
    case MeshSpecificationData::UniformId::kSideDerivativeUnpackingTransform:
    case MeshSpecificationData::UniformId::kForwardDerivativeUnpackingTransform:
    case MeshSpecificationData::UniformId::kTextureMapping:
    case MeshSpecificationData::UniformId::kTextureAnimationProgress:
    case MeshSpecificationData::UniformId::kNumTextureAnimationFrames:
    case MeshSpecificationData::UniformId::kNumTextureAnimationRows:
    case MeshSpecificationData::UniformId::kNumTextureAnimationColumns:
      return true;
  }
  return false;
}

bool IsUnpackingTransformUniformId(MeshSpecificationData::UniformId id) {
  switch (id) {
    case MeshSpecificationData::UniformId::kObjectToCanvasLinearComponent:
    case MeshSpecificationData::UniformId::kBrushColor:
    case MeshSpecificationData::UniformId::kTextureMapping:
    case MeshSpecificationData::UniformId::kTextureAnimationProgress:
    case MeshSpecificationData::UniformId::kNumTextureAnimationFrames:
    case MeshSpecificationData::UniformId::kNumTextureAnimationRows:
    case MeshSpecificationData::UniformId::kNumTextureAnimationColumns:
      break;
    case MeshSpecificationData::UniformId::kPositionUnpackingTransform:
    case MeshSpecificationData::UniformId::kSideDerivativeUnpackingTransform:
    case MeshSpecificationData::UniformId::kForwardDerivativeUnpackingTransform:
      return true;
  }
  return false;
}

MATCHER_P2(ShaderAttributeIs, type, name, "") {
  return arg.type == type && arg.name == name;
}

// Checks that the `MeshSpecificationData` has valid attributes, varyings, and
// uniforms.
MATCHER_P(SpecificationDataHasValidShaderVariableValues, mesh_format, "") {
  // Attributes
  for (int i = 0; i < arg.attributes.Size(); ++i) {
    if (arg.attributes[i].name.empty()) {
      *result_listener << absl::StrFormat("attribute i = %d has an empty name",
                                          i);
      return false;
    }

    if (!IsValidAttributeType(arg.attributes[i].type)) {
      *result_listener << absl::StrFormat(
          "attribute i = %d, name = '%s' has type with non-enumerator value %d",
          i, arg.attributes[i].name, static_cast<int>(arg.attributes[i].type));
      return false;
    }

    if (arg.attributes[i].offset < 0) {
      *result_listener << absl::StrFormat(
          "attribute i = %d has a negative offset %d", i,
          arg.attributes[i].offset);
      return false;
    }
  }

  // Varyings
  for (int i = 0; i < arg.varyings.Size(); ++i) {
    if (arg.varyings[i].name.empty()) {
      *result_listener << absl::StrFormat("varying i = %d has an empty name",
                                          i);
      return false;
    }

    if (!IsValidVaryingType(arg.varyings[i].type)) {
      *result_listener << absl::StrFormat(
          "varying i = %d, name = '%s' has type with non-enumerator value %d",
          i, arg.varyings[i].name, static_cast<int>(arg.varyings[i].type));
      return false;
    }
  }

  // Uniforms
  for (int i = 0; i < arg.uniforms.Size(); ++i) {
    if (!IsValidUniformId(arg.uniforms[i].id)) {
      *result_listener << absl::StrFormat(
          "uniform i = %d has id with non-enumerator value %d", i,
          static_cast<int>(arg.uniforms[i].id));
      return false;
    }

    if (!IsValidUniformType(arg.uniforms[i].type)) {
      *result_listener << absl::StrFormat(
          "uniform i = %d has type with non-enumerator value %d", i,
          static_cast<int>(arg.uniforms[i].type));
      return false;
    }

    bool is_unpacking_transform =
        IsUnpackingTransformUniformId(arg.uniforms[i].id);
    if (is_unpacking_transform &&
        !arg.uniforms[i].unpacking_attribute_index.has_value()) {
      *result_listener << absl::StrFormat(
          "uniform i = %d, with id value %d is an unpacking transform, but has "
          "no `unpacking_attribute_index`.",
          i, static_cast<int>(arg.uniforms[i].id));
      return false;
    }

    if (!is_unpacking_transform &&
        arg.uniforms[i].unpacking_attribute_index.has_value()) {
      *result_listener << absl::StrFormat(
          "uniform i = %d, with id value %d is *not* an unpacking transform, "
          "but `unpacking_attribute_index` has value %d",
          i, static_cast<int>(arg.uniforms[i].id),
          *arg.uniforms[i].unpacking_attribute_index);
      return false;
    }

    if (arg.uniforms[i].unpacking_attribute_index.has_value() &&
        (*arg.uniforms[i].unpacking_attribute_index < 0 ||
         static_cast<size_t>(*arg.uniforms[i].unpacking_attribute_index) >=
             mesh_format.Attributes().size())) {
      *result_listener << absl::StrFormat(
          "uniform i = %d has an out of bounds `unpacking_attribute_index` = "
          "%d, the `MeshFormat` has %d attributes",
          i, *arg.uniforms[i].unpacking_attribute_index,
          mesh_format.Attributes().size());
      return false;
    }
  }
  return true;
}

TEST(MeshSpecificationDataTest, GetUniformName) {
  EXPECT_THAT(
      MeshSpecificationData::GetUniformName(
          MeshSpecificationData::UniformId::kObjectToCanvasLinearComponent),
      Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::kBrushColor),
              Not(IsEmpty()));
  EXPECT_THAT(
      MeshSpecificationData::GetUniformName(
          MeshSpecificationData::UniformId::kPositionUnpackingTransform),
      Not(IsEmpty()));
  EXPECT_THAT(
      MeshSpecificationData::GetUniformName(
          MeshSpecificationData::UniformId::kSideDerivativeUnpackingTransform),
      Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::
                      kForwardDerivativeUnpackingTransform),
              Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::kTextureMapping),
              Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::kTextureAnimationProgress),
              Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::kNumTextureAnimationFrames),
              Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  MeshSpecificationData::UniformId::kNumTextureAnimationRows),
              Not(IsEmpty()));
  EXPECT_THAT(
      MeshSpecificationData::GetUniformName(
          MeshSpecificationData::UniformId::kNumTextureAnimationColumns),
      Not(IsEmpty()));
  EXPECT_THAT(MeshSpecificationData::GetUniformName(
                  static_cast<MeshSpecificationData::UniformId>(99)),
              IsEmpty());
}

TEST(MeshSpecificationDataTest, CreateForInProgressStroke) {
  auto data = MeshSpecificationData::CreateForInProgressStroke();

  EXPECT_GE(data.attributes.Size(), 0);
  EXPECT_EQ(data.vertex_stride,
            StrokeVertex::FullMeshFormat().UnpackedVertexStride());
  EXPECT_GE(data.varyings.Size(), 0);
  EXPECT_GE(data.uniforms.Size(), 0);
  EXPECT_THAT(data.vertex_shader_source, Not(IsEmpty()));
  EXPECT_THAT(data.fragment_shader_source, Not(IsEmpty()));
  EXPECT_THAT(data, SpecificationDataHasValidShaderVariableValues(
                        StrokeVertex::FullMeshFormat()));
}

TEST(MeshSpecificationDataTest, CreateFromFullMeshFormatIsOk) {
  EXPECT_EQ(MeshSpecificationData::CreateForInProgressStroke(
                StrokeVertex::FullMeshFormat())
                .status(),
            absl::OkStatus());
}

TEST(MeshSpecificationDataTest, CreateFromUnpackedStrokeFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {
          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
           MeshFormat::AttributeId::kPosition},
          {MeshFormat::AttributeType::kFloat1Unpacked,
           MeshFormat::AttributeId::kOpacityShift},
          {MeshFormat::AttributeType::kFloat3Unpacked,
           MeshFormat::AttributeId::kColorShiftHsl},
          {MeshFormat::AttributeType::kFloat2Unpacked,
           MeshFormat::AttributeId::kSideDerivative},
          {MeshFormat::AttributeType::kFloat1Unpacked,
           MeshFormat::AttributeId::kSideLabel},
          {MeshFormat::AttributeType::kFloat2Unpacked,
           MeshFormat::AttributeId::kForwardDerivative},
          {MeshFormat::AttributeType::kFloat1Unpacked,
           MeshFormat::AttributeId::kForwardLabel},
          {MeshFormat::AttributeType::kFloat2Unpacked,
           MeshFormat::AttributeId::kSurfaceUv},
          {MeshFormat::AttributeType::kFloat1Unpacked,
           MeshFormat::AttributeId::kAnimationOffset},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_THAT(format, IsOk());

  absl::StatusOr<MeshSpecificationData> data =
      MeshSpecificationData::CreateForStroke(*format);
  ASSERT_THAT(data, IsOk());
  EXPECT_THAT(
      data->attributes.Values(),
      ElementsAre(
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kFloat2,
                            "positionAndOpacityShift"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kFloat3,
                            "sideDerivativeAndLabel"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kFloat3,
                            "forwardDerivativeAndLabel"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kFloat3,
                            "hslShift"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kFloat3,
                            "surfaceUvAndAnimationOffset")));
}

TEST(MeshSpecificationDataTest, CreateFromStrokeFormatWithNoAnimationOffset) {
  // Create a format that uses X12_Y20 for surface UV and has no mesh attribute
  // for animation offset (i.e. a format for a winding-textured, extruded
  // (non-particle) stroke).
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {
          {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
           MeshFormat::AttributeId::kPosition},
          {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
           MeshFormat::AttributeId::kOpacityShift},
          {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
           MeshFormat::AttributeId::kSideDerivative},
          {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
           MeshFormat::AttributeId::kSideLabel},
          {MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
           MeshFormat::AttributeId::kForwardDerivative},
          {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
           MeshFormat::AttributeId::kForwardLabel},
          {MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
           MeshFormat::AttributeId::kSurfaceUv},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_THAT(format, IsOk());

  // The above mesh format should be valid for creating a
  // `MeshSpecificationData`.
  absl::StatusOr<MeshSpecificationData> data =
      MeshSpecificationData::CreateForStroke(*format);
  ASSERT_THAT(data, IsOk());
  EXPECT_THAT(
      data->attributes.Values(),
      ElementsAre(
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kUByte4,
                            "positionAndOpacityShift"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kUByte4,
                            "sideDerivativeAndLabel"),
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kUByte4,
                            "forwardDerivativeAndLabel"),
          // Note that the shader attribute will still be called
          // "surfaceUvAndAnimationOffset", even though all four bytes are used
          // for the surface UV, and zero bytes are used for the animation
          // offset.
          ShaderAttributeIs(MeshSpecificationData::AttributeType::kUByte4,
                            "surfaceUvAndAnimationOffset")));
}

TEST(MeshSpecificationDataTest,
     CreateFromNonInProgressStrokeFormatReturnsError) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {
          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
           MeshFormat::AttributeId::kPosition},
          {MeshFormat::AttributeType::kFloat3Unpacked,
           MeshFormat::AttributeId::kCustom0},
          {MeshFormat::AttributeType::kFloat3Unpacked,
           MeshFormat::AttributeId::kColorShiftHsl},
          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
           MeshFormat::AttributeId::kSideDerivative},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  ASSERT_THAT(*format, Not(MeshFormatEq(StrokeVertex::FullMeshFormat())));

  absl::Status not_in_progress =
      MeshSpecificationData::CreateForInProgressStroke(*format).status();
  EXPECT_EQ(not_in_progress.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(not_in_progress.message(),
              HasSubstr("not from an `InProgressStroke`"));
}

TEST(MeshSpecificationDataTest, CreateForStrokeWithGeneratedShape) {
  absl::StatusOr<StrokeInputBatch> input = StrokeInputBatch::Create(
      {{.position = {0, 0}, .elapsed_time = Duration32::Zero()}});
  ASSERT_EQ(input.status(), absl::OkStatus());

  Stroke stroke(Brush(), *input);
  ASSERT_THAT(stroke.GetShape().Meshes(), Not(IsEmpty()));

  ASSERT_EQ(stroke.GetShape().RenderGroupCount(), 1u);
  const MeshFormat& format = stroke.GetShape().RenderGroupFormat(0);

  absl::StatusOr<MeshSpecificationData> data =
      MeshSpecificationData::CreateForStroke(format);
  ASSERT_EQ(data.status(), absl::OkStatus());

  EXPECT_GE(data->attributes.Size(), 0);
  EXPECT_EQ(data->vertex_stride, format.PackedVertexStride());
  EXPECT_GE(data->varyings.Size(), 0);
  EXPECT_GE(data->uniforms.Size(), 0);
  EXPECT_THAT(data->vertex_shader_source, Not(IsEmpty()));
  EXPECT_THAT(data->fragment_shader_source, Not(IsEmpty()));
  EXPECT_THAT(*data, SpecificationDataHasValidShaderVariableValues(
                         StrokeVertex::FullMeshFormat()));
}

// Returns a format identical to `starting_format` except that an attribute with
// `attribute_id_to_skip` will be removed.
MeshFormat MakeFormatWithSkippedAttribute(
    const MeshFormat& starting_format,
    MeshFormat::AttributeId attribute_id_to_skip) {
  std::vector<std::pair<MeshFormat::AttributeType, MeshFormat::AttributeId>>
      new_types_and_ids;
  for (const MeshFormat::Attribute& attribute : starting_format.Attributes()) {
    if (attribute.id == attribute_id_to_skip) continue;
    new_types_and_ids.push_back({attribute.type, attribute.id});
  }

  auto new_format =
      MeshFormat::Create(new_types_and_ids, starting_format.GetIndexFormat());
  ABSL_CHECK_OK(new_format);
  return *new_format;
}

TEST(MeshSpecificationDataTest, CreateForStrokeWithoutHslColorShiftIsOk) {
  MeshFormat format_without_hsl = MakeFormatWithSkippedAttribute(
      StrokeVertex::FullMeshFormat(), MeshFormat::AttributeId::kColorShiftHsl);
  EXPECT_EQ(MeshSpecificationData::CreateForStroke(format_without_hsl).status(),
            absl::OkStatus());
}

TEST(MeshSpecificationDataTest, CreateForStrokeWithoutSurfaceUvIsOk) {
  MeshFormat format_without_uv = MakeFormatWithSkippedAttribute(
      StrokeVertex::FullMeshFormat(), MeshFormat::AttributeId::kSurfaceUv);
  EXPECT_EQ(MeshSpecificationData::CreateForStroke(format_without_uv).status(),
            absl::OkStatus());
}

TEST(MeshSpecificationDataTest,
     CreateForStrokeWithoutRequiredAttributesReturnsError) {
  MeshFormat format_with_missing_required_attribute =
      MakeFormatWithSkippedAttribute(StrokeVertex::FullMeshFormat(),
                                     MeshFormat::AttributeId::kOpacityShift);
  {
    absl::Status status = MeshSpecificationData::CreateForStroke(
                              format_with_missing_required_attribute)
                              .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("are required"));
  }

  {
    MeshFormat format_with_missing_required_attribute =
        MakeFormatWithSkippedAttribute(
            StrokeVertex::FullMeshFormat(),
            MeshFormat::AttributeId::kSideDerivative);
    absl::Status status = MeshSpecificationData::CreateForStroke(
                              format_with_missing_required_attribute)
                              .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("are required"));
  }

  {
    MeshFormat format_with_missing_required_attribute =
        MakeFormatWithSkippedAttribute(StrokeVertex::FullMeshFormat(),
                                       MeshFormat::AttributeId::kSideLabel);
    absl::Status status = MeshSpecificationData::CreateForStroke(
                              format_with_missing_required_attribute)
                              .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("are required"));
  }

  {
    MeshFormat format_with_missing_required_attribute =
        MakeFormatWithSkippedAttribute(
            StrokeVertex::FullMeshFormat(),
            MeshFormat::AttributeId::kForwardDerivative);
    absl::Status status = MeshSpecificationData::CreateForStroke(
                              format_with_missing_required_attribute)
                              .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("are required"));
  }

  {
    MeshFormat format_with_missing_required_attribute =
        MakeFormatWithSkippedAttribute(StrokeVertex::FullMeshFormat(),
                                       MeshFormat::AttributeId::kForwardLabel);
    absl::Status status = MeshSpecificationData::CreateForStroke(
                              format_with_missing_required_attribute)
                              .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("are required"));
  }
}

std::vector<std::pair<MeshFormat::AttributeType, MeshFormat::AttributeId>>
GetFormatTypesAndIds(const MeshFormat& format) {
  std::vector<std::pair<MeshFormat::AttributeType, MeshFormat::AttributeId>>
      types_and_ids;
  for (const MeshFormat::Attribute& attribute : format.Attributes()) {
    types_and_ids.push_back({attribute.type, attribute.id});
  }
  return types_and_ids;
}

TEST(MeshSpecificationDataTest,
     CreateForStrokeWithUnsupportedAttributeOrderReturnsError) {
  // Modify the `InProgressStroke` format by performing a single swap of
  // attributes. Because HSL shift is the only format attribute that is not
  // required to be paired, any single swap will cause an unsupported order.

  auto types_and_ids = GetFormatTypesAndIds(StrokeVertex::FullMeshFormat());
  for (int i = 0; i + 1 < types_and_ids.size(); ++i) {
    for (int j = i + 1; j < types_and_ids.size(); ++j) {
      SCOPED_TRACE(absl::StrCat("Swapping attributes i = ", i, " and j = ", j));

      auto types_and_ids_with_swap = types_and_ids;
      std::swap(types_and_ids_with_swap[i], types_and_ids_with_swap[j]);
      absl::StatusOr<MeshFormat> reordered_format = MeshFormat::Create(
          types_and_ids_with_swap,
          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
      ASSERT_EQ(reordered_format.status(), absl::OkStatus());

      absl::Status unsupported_order =
          MeshSpecificationData::CreateForStroke(*reordered_format).status();
      EXPECT_EQ(unsupported_order.code(), absl::StatusCode::kInvalidArgument);
      EXPECT_THAT(unsupported_order.message(),
                  HasSubstr("must be immediately after"));
    }
  }
}

MeshFormat MakeFormatWithModifiedType(
    const std::vector<std::pair<MeshFormat::AttributeType,
                                MeshFormat::AttributeId>>& types_and_ids,
    int attribute_index, MeshFormat::AttributeType replacement_type) {
  auto modified_types_and_ids = types_and_ids;
  modified_types_and_ids[attribute_index].first = replacement_type;
  auto format =
      MeshFormat::Create(modified_types_and_ids,
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ABSL_CHECK_OK(format);
  return *format;
}

TEST(MeshSpecificationDataTest,
     CreateForStrokeWithUnsupportedAttributeTypesReturnsError) {
  auto types_and_ids = GetFormatTypesAndIds(StrokeVertex::FullMeshFormat());

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.position,
                MeshFormat::AttributeType::kFloat2Unpacked))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.opacity_shift,
                MeshFormat::AttributeType::kFloat2Unpacked))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.hsl_shift,
                MeshFormat::AttributeType::kFloat3PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.side_derivative,
                MeshFormat::AttributeType::kFloat3PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.side_label,
                MeshFormat::AttributeType::kFloat2PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.forward_derivative,
                MeshFormat::AttributeType::kFloat3PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.forward_label,
                MeshFormat::AttributeType::kFloat2PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }

  {
    absl::Status status =
        MeshSpecificationData::CreateForStroke(
            MakeFormatWithModifiedType(
                types_and_ids,
                StrokeVertex::kFullFormatAttributeIndices.surface_uv,
                MeshFormat::AttributeType::kFloat3PackedInOneFloat))
            .status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("Unsupported type"));
  }
}

}  // namespace
}  // namespace ink::skia_common_internal
