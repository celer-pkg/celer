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

#include "ink/rendering/skia/native/internal/create_mesh_specification.h"

#include <cstddef>

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"
#include "ink/types/small_array.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"

namespace ink::skia_native_internal {
namespace {

using ::ink::skia_common_internal::MeshSpecificationData;

static_assert(SkMeshSpecification::kMaxAttributes >=
              MeshSpecificationData::kMaxAttributes);
static_assert(SkMeshSpecification::kMaxVaryings >=
              MeshSpecificationData::kMaxVaryings);

SkMeshSpecification::Attribute::Type ToSkiaAttributeType(
    MeshSpecificationData::AttributeType type) {
  switch (type) {
    case MeshSpecificationData::AttributeType::kFloat2:
      return SkMeshSpecification::Attribute::Type::kFloat2;
    case MeshSpecificationData::AttributeType::kFloat3:
      return SkMeshSpecification::Attribute::Type::kFloat3;
    case MeshSpecificationData::AttributeType::kUByte4:
      return SkMeshSpecification::Attribute::Type::kUByte4_unorm;
  }
  ABSL_LOG(FATAL) << "Non-enumerator value";
}

SmallArray<SkMeshSpecification::Attribute,
           MeshSpecificationData::kMaxAttributes>
ToSkiaAttributes(
    const SmallArray<MeshSpecificationData::Attribute,
                     MeshSpecificationData::kMaxAttributes>& attributes) {
  SmallArray<SkMeshSpecification::Attribute,
             SkMeshSpecification::kMaxAttributes>
      skia_attributes(attributes.Size());

  for (int i = 0; i < skia_attributes.Size(); ++i) {
    const auto& attribute = attributes[i];
    ABSL_CHECK(!attribute.name.empty());
    skia_attributes[i] = {
        .type = ToSkiaAttributeType(attribute.type),
        .offset = static_cast<size_t>(attribute.offset),
        .name = SkString(attribute.name),
    };
  }

  return skia_attributes;
}

SkMeshSpecification::Varying::Type ToSkiaVaryingType(
    MeshSpecificationData::VaryingType type) {
  switch (type) {
    case MeshSpecificationData::VaryingType::kFloat2:
      return SkMeshSpecification::Varying::Type::kFloat2;
    case MeshSpecificationData::VaryingType::kFloat4:
      return SkMeshSpecification::Varying::Type::kFloat4;
  }
  ABSL_LOG(FATAL) << "Non-enumerator value";
}

SmallArray<SkMeshSpecification::Varying, MeshSpecificationData::kMaxVaryings>
ToSkiaVaryings(
    const SmallArray<MeshSpecificationData::Varying,
                     MeshSpecificationData::kMaxVaryings>& varyings) {
  SmallArray<SkMeshSpecification::Varying, MeshSpecificationData::kMaxVaryings>
      skia_varyings(varyings.Size());

  for (int i = 0; i < skia_varyings.Size(); ++i) {
    const auto& varying = varyings[i];
    ABSL_CHECK(!varying.name.empty());
    skia_varyings[i] = {
        .type = ToSkiaVaryingType(varying.type),
        .name = SkString(varying.name),
    };
  }

  return skia_varyings;
}

SkMeshSpecification::Uniform::Type ToSkiaUniformType(
    MeshSpecificationData::UniformType type) {
  switch (type) {
    case MeshSpecificationData::UniformType::kFloat:
      return SkMeshSpecification::Uniform::Type::kFloat;
    case MeshSpecificationData::UniformType::kFloat4:
      return SkMeshSpecification::Uniform::Type::kFloat4;
    case MeshSpecificationData::UniformType::kInt:
      return SkMeshSpecification::Uniform::Type::kInt;
  }
  ABSL_LOG(FATAL) << "Non-enumerator value";
}

struct SkiaUniformTypeAndName {
  SkMeshSpecification::Uniform::Type type;
  absl::string_view name;
};

SmallArray<SkiaUniformTypeAndName, MeshSpecificationData::kMaxUniforms>
ToSkiaUniformTypesAndNames(
    const SmallArray<MeshSpecificationData::Uniform,
                     MeshSpecificationData::kMaxUniforms>& uniforms) {
  SmallArray<SkiaUniformTypeAndName, MeshSpecificationData::kMaxUniforms>
      skia_uniform_types(uniforms.Size());

  for (int i = 0; i < skia_uniform_types.Size(); ++i) {
    absl::string_view name =
        MeshSpecificationData::GetUniformName(uniforms[i].id);
    ABSL_CHECK(!name.empty()) << "Non-enumerator value";
    skia_uniform_types[i] = {.type = ToSkiaUniformType(uniforms[i].type),
                             .name = name};
  }

  return skia_uniform_types;
}

absl::Status ValidateUniforms(
    const SkMeshSpecification& spec,
    absl::Span<const SkiaUniformTypeAndName> expected_uniforms) {
  if (spec.uniforms().size() != expected_uniforms.size()) {
    return absl::InvalidArgumentError(
        absl::StrFormat("Created `SkMeshSpecification` uniform count (%d) does "
                        "not match expected uniform count (%d)",
                        spec.uniforms().size(), expected_uniforms.size()));
  }

  for (const auto& expected_uniform : expected_uniforms) {
    const auto* skia_uniform = spec.findUniform(expected_uniform.name);
    if (skia_uniform == nullptr) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "Created `SkMeshSpecification` does not have uniform named: '%s'",
          expected_uniform.name));
    }
    if (skia_uniform->type != expected_uniform.type) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "Unexpected type for uniform named '%s'", expected_uniform.name));
    }
  }

  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<sk_sp<SkMeshSpecification>> CreateMeshSpecification(
    const skia_common_internal::MeshSpecificationData& data) {
  auto skia_attributes = ToSkiaAttributes(data.attributes);
  auto skia_varyings = ToSkiaVaryings(data.varyings);
  auto skia_uniform_types_and_names = ToSkiaUniformTypesAndNames(data.uniforms);

  SkMeshSpecification::Result result = SkMeshSpecification::Make(
      skia_attributes.Values(), data.vertex_stride, skia_varyings.Values(),
      SkString(data.vertex_shader_source),
      SkString(data.fragment_shader_source),
      // The shaders work with linear, premultiplied, non-clamped sRGB colors.
      SkColorSpace::MakeSRGBLinear(), kPremul_SkAlphaType);

  if (result.specification == nullptr) {
    return absl::InvalidArgumentError(absl::StrCat(
        "`SkMeshSpecification::Make()` failed: ",
        absl::string_view(result.error.data(), result.error.size())));
  }

  auto status = ValidateUniforms(*result.specification,
                                 skia_uniform_types_and_names.Values());
  if (!status.ok()) return status;

  return result.specification;
}

}  // namespace ink::skia_native_internal
