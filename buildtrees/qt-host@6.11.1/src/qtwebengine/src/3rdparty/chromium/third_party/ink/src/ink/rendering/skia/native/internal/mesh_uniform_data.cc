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

#include <cstdint>
#include <cstring>
#include <optional>

#include "absl/functional/function_ref.h"
#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/types/span.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {
namespace {

using ::ink::skia_common_internal::MeshSpecificationData;

sk_sp<SkData> MakeUninitializedIfNonZeroSize(const SkMeshSpecification& spec) {
  if (spec.uniformSize() == 0) return nullptr;
  return SkData::MakeUninitialized(spec.uniformSize());
}

SkMeshSpecification::Uniform::Type ExpectedSkiaUniformType(
    MeshSpecificationData::UniformId uniform_id) {
  switch (uniform_id) {
    case MeshSpecificationData::UniformId::kObjectToCanvasLinearComponent:
    case MeshSpecificationData::UniformId::kBrushColor:
    case MeshSpecificationData::UniformId::kPositionUnpackingTransform:
    case MeshSpecificationData::UniformId::kSideDerivativeUnpackingTransform:
    case MeshSpecificationData::UniformId::kForwardDerivativeUnpackingTransform:
      return SkMeshSpecification::Uniform::Type::kFloat4;
    case MeshSpecificationData::UniformId::kTextureMapping:
      return SkMeshSpecification::Uniform::Type::kInt;
    case MeshSpecificationData::UniformId::kTextureAnimationProgress:
      return SkMeshSpecification::Uniform::Type::kFloat;
    case MeshSpecificationData::UniformId::kNumTextureAnimationFrames:
      return SkMeshSpecification::Uniform::Type::kInt;
    case MeshSpecificationData::UniformId::kNumTextureAnimationRows:
      return SkMeshSpecification::Uniform::Type::kInt;
    case MeshSpecificationData::UniformId::kNumTextureAnimationColumns:
      return SkMeshSpecification::Uniform::Type::kInt;
  }
  ABSL_LOG(FATAL) << "Got `uniform_id` with non-enumerator value: "
                  << static_cast<int>(uniform_id);
}

int16_t FindUniformOffset(const SkMeshSpecification& spec,
                          MeshSpecificationData::UniformId uniform_id) {
  const SkMeshSpecification::Uniform* uniform =
      spec.findUniform(MeshSpecificationData::GetUniformName(uniform_id));
  if (uniform == nullptr ||
      uniform->type != ExpectedSkiaUniformType(uniform_id)) {
    return -1;
  }
  return uniform->offset;
}

std::optional<MeshSpecificationData::UniformId> FindUnpackingTransformUniformId(
    MeshFormat::AttributeId attribute_id) {
  switch (attribute_id) {
    case MeshFormat::AttributeId::kPosition:
      return MeshSpecificationData::UniformId::kPositionUnpackingTransform;
    case MeshFormat::AttributeId::kSideDerivative:
      return MeshSpecificationData::UniformId::
          kSideDerivativeUnpackingTransform;
    case MeshFormat::AttributeId::kForwardDerivative:
      return MeshSpecificationData::UniformId::
          kForwardDerivativeUnpackingTransform;
    default:
      return std::nullopt;
  }
}

}  // namespace

MeshUniformData::MeshUniformData(const SkMeshSpecification& spec)
    : data_(MakeUninitializedIfNonZeroSize(spec)),
      object_to_canvas_linear_component_offset_(FindUniformOffset(
          spec,
          MeshSpecificationData::UniformId::kObjectToCanvasLinearComponent)),
      brush_color_offset_(FindUniformOffset(
          spec, MeshSpecificationData::UniformId::kBrushColor)),
      texture_mapping_offset_(FindUniformOffset(
          spec, MeshSpecificationData::UniformId::kTextureMapping)) {}

namespace {

void Copy2DUnpackingParams(const MeshAttributeCodingParams& unpacking_transform,
                           void* sk_data_target_address) {
  ABSL_CHECK_EQ(unpacking_transform.components.Size(), 2);
  float values[] = {unpacking_transform.components[0].offset,
                    unpacking_transform.components[0].scale,
                    unpacking_transform.components[1].offset,
                    unpacking_transform.components[1].scale};
  std::memcpy(sk_data_target_address, values, 4 * sizeof(float));
}

void SetUnpackingTransform(MeshSpecificationData::UniformId uniform_id,
                           const MeshAttributeCodingParams& unpacking_transform,
                           void* sk_data_target_address) {
  switch (uniform_id) {
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
      Copy2DUnpackingParams(unpacking_transform, sk_data_target_address);
      break;
  }
}

void InitializeAttributeUnpackingTransforms(
    const SkMeshSpecification& spec,
    absl::Span<const MeshFormat::Attribute> ink_attributes,
    absl::FunctionRef<const MeshAttributeCodingParams&(int)>
        get_attribute_unpacking_transform,
    void* sk_writable_data) {
  for (int i = 0; i < static_cast<int>(ink_attributes.size()); ++i) {
    std::optional<MeshSpecificationData::UniformId> uniform_id =
        FindUnpackingTransformUniformId(ink_attributes[i].id);
    if (!uniform_id.has_value()) continue;

    int16_t uniform_byte_offset = FindUniformOffset(spec, *uniform_id);
    if (uniform_byte_offset == -1) continue;

    SetUnpackingTransform(
        *uniform_id, get_attribute_unpacking_transform(i),
        static_cast<char*>(sk_writable_data) + uniform_byte_offset);
  }
}

}  // namespace

MeshUniformData::MeshUniformData(
    const SkMeshSpecification& spec,
    absl::Span<const MeshFormat::Attribute> ink_attributes,
    absl::FunctionRef<const MeshAttributeCodingParams&(int)>
        get_attribute_unpacking_transform)
    : MeshUniformData(spec) {
  InitializeAttributeUnpackingTransforms(spec, ink_attributes,
                                         get_attribute_unpacking_transform,
                                         data_->writable_data());
}

void MeshUniformData::SetBrushColor(const Color& color) {
  ABSL_CHECK(HasBrushColor());
  if (!data_->unique()) {
    data_ = SkData::MakeWithCopy(data_->bytes(), data_->size());
  }
  Color::RgbaFloat rgba =
      color.InColorSpace(ColorSpace::kSrgb).AsFloat(Color::Format::kLinear);
  static_assert(sizeof(rgba) == 4 * sizeof(float));
  std::memcpy(static_cast<char*>(data_->writable_data()) + brush_color_offset_,
              &rgba, sizeof(rgba));
}

void MeshUniformData::SetTextureMapping(BrushPaint::TextureMapping mapping) {
  ABSL_CHECK(HasTextureMapping());
  if (!data_->unique()) {
    data_ = SkData::MakeWithCopy(data_->bytes(), data_->size());
  }
  int mapping_int = static_cast<int>(mapping);
  std::memcpy(
      static_cast<char*>(data_->writable_data()) + texture_mapping_offset_,
      &mapping_int, sizeof(int));
}

void MeshUniformData::SetObjectToCanvasLinearComponent(
    const AffineTransform& transform) {
  ABSL_CHECK(HasObjectToCanvasLinearComponent());
  if (!data_->unique()) {
    data_ = SkData::MakeWithCopy(data_->bytes(), data_->size());
  }
  float values[] = {transform.A(), transform.D(), transform.B(), transform.E()};
  std::memcpy(static_cast<char*>(data_->writable_data()) +
                  object_to_canvas_linear_component_offset_,
              values, 4 * sizeof(float));
}

}  // namespace ink::skia_native_internal
