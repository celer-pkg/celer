// Copyright 2024-2025 Google LLC
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

#include <jni.h>

#include <array>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "ink/geometry/internal/jni/mesh_format_jni_helper.h"
#include "ink/geometry/mesh_format.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"

namespace {

using ::ink::MeshFormat;
using ::ink::jni::CastToMeshFormat;
using ::ink::jni::ThrowExceptionFromStatus;
using ::ink::skia_common_internal::MeshSpecificationData;

absl::StatusOr<MeshSpecificationData> GetMeshSpecificationData(
    jlong raw_ptr_to_mesh_format, jboolean packed) {
  const MeshFormat& mesh_format = CastToMeshFormat(raw_ptr_to_mesh_format);
  return packed ? MeshSpecificationData::CreateForStroke(mesh_format)
                : MeshSpecificationData::CreateForInProgressStroke(mesh_format);
}

}  // namespace

extern "C" {

JNI_METHOD(rendering_android_canvas_internal, CanvasMeshRenderer, void,
           fillSkiaMeshSpecData)
(JNIEnv* env, jobject object, jlong raw_ptr_to_mesh_format, jboolean packed,
 jintArray attribute_types_out, jintArray attribute_offsets_out,
 jobjectArray attribute_names_out, jintArray vertex_stride_out,
 jintArray varying_types_out, jobjectArray varying_names_out,
 jintArray uniform_ids_out, jintArray uniform_unpacking_indices_out,
 jobjectArray uniform_names_out, jobjectArray vertex_shader_out,
 jobjectArray fragment_shader_out) {
  const auto spec_data =
      GetMeshSpecificationData(raw_ptr_to_mesh_format, packed);
  if (!spec_data.ok()) {
    ThrowExceptionFromStatus(env, spec_data.status());
    return;
  }

  // Attributes
  const auto& attributes = spec_data->attributes;
  std::array<int, MeshSpecificationData::kMaxAttributes> attribute_types;
  std::array<int, MeshSpecificationData::kMaxAttributes> attribute_offsets;
  attribute_types.fill(-1);
  for (int i = 0; i < attributes.Size(); i++) {
    attribute_types[i] = static_cast<int>(attributes[i].type);
    attribute_offsets[i] = attributes[i].offset;
    env->SetObjectArrayElement(
        attribute_names_out, i,
        env->NewStringUTF(std::string(attributes[i].name).c_str()));
  }
  env->SetIntArrayRegion(attribute_types_out, 0,
                         MeshSpecificationData::kMaxAttributes,
                         attribute_types.data());
  env->SetIntArrayRegion(attribute_offsets_out, 0,
                         MeshSpecificationData::kMaxAttributes,
                         attribute_offsets.data());

  // Vertex stride
  env->SetIntArrayRegion(vertex_stride_out, 0, 1, &spec_data->vertex_stride);

  // Varyings
  const auto& varyings = spec_data->varyings;
  std::array<int, MeshSpecificationData::kMaxVaryings> varying_types;
  varying_types.fill(-1);
  for (int i = 0; i < varyings.Size(); ++i) {
    varying_types[i] = static_cast<int>(varyings[i].type);
    env->SetObjectArrayElement(
        varying_names_out, i,
        env->NewStringUTF(std::string(varyings[i].name).c_str()));
  }
  env->SetIntArrayRegion(varying_types_out, 0,
                         MeshSpecificationData::kMaxVaryings,
                         varying_types.data());

  // Uniforms
  const auto& uniforms = spec_data->uniforms;
  std::array<int, MeshSpecificationData::kMaxUniforms> uniform_ids;
  std::array<int, MeshSpecificationData::kMaxUniforms>
      uniform_unpacking_indices;
  uniform_ids.fill(-1);
  uniform_unpacking_indices.fill(-1);
  for (int i = 0; i < uniforms.Size(); i++) {
    uniform_ids[i] = static_cast<int>(uniforms[i].id);
    if (uniforms[i].unpacking_attribute_index.has_value()) {
      uniform_unpacking_indices[i] = *uniforms[i].unpacking_attribute_index;
    }
    absl::string_view uniform_name =
        MeshSpecificationData::GetUniformName(uniforms[i].id);
    env->SetObjectArrayElement(
        uniform_names_out, i,
        env->NewStringUTF(std::string(uniform_name).c_str()));
  }
  env->SetIntArrayRegion(uniform_ids_out, 0,
                         MeshSpecificationData::kMaxUniforms,
                         uniform_ids.data());
  env->SetIntArrayRegion(uniform_unpacking_indices_out, 0,
                         MeshSpecificationData::kMaxUniforms,
                         uniform_unpacking_indices.data());

  // Vertex shader
  env->SetObjectArrayElement(
      vertex_shader_out, 0,
      env->NewStringUTF(spec_data->vertex_shader_source.c_str()));

  // Fragment shader
  env->SetObjectArrayElement(
      fragment_shader_out, 0,
      env->NewStringUTF(spec_data->fragment_shader_source.c_str()));
}

// TODO: b/306376686 - add the `BrushPaint` argument to this method to check if
// it can be used with `raw_ptr_to_mesh_format`.
JNI_METHOD(rendering_android_canvas_internal, CanvasMeshRenderer, jboolean,
           nativeIsMeshFormatRenderable)
(JNIEnv* env, jobject object, jlong raw_ptr_to_mesh_format, jboolean packed) {
  return GetMeshSpecificationData(raw_ptr_to_mesh_format, packed).ok();
}

}  // extern "C"
