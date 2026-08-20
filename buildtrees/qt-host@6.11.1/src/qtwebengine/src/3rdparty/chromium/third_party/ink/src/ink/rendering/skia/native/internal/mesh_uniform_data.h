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

#ifndef INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_UNIFORM_DATA_H_
#define INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_UNIFORM_DATA_H_

#include <cstdint>

#include "absl/functional/function_ref.h"
#include "absl/types/span.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {

// A wrapper over `sk_sp<SkData>` for holding Ink-specific uniform values for
// creating an `SkMesh`.
//
// Usage:
//   1. Construct with a particular `SkMeshSpecification`.
//   2. Use the appropriate setters to give the stored uniforms their values.
//   3. Pass the `sk_sp<SkData>` returned by `MeshUniformData::Get()` as an
//      argument to one of the `SkMesh::Make*` factories along with the
//      `SkMeshSpecification` used during uniform data construction.
//
// See comments on `skia_common_internal::MeshSpecificationData::UniformId` for
// information on each uniform.
class MeshUniformData {
 public:
  // Constructs the data to hold the uniforms in `spec`.
  //
  // This allocates the data necessary to hold uniform values, but does not set
  // them.
  explicit MeshUniformData(const SkMeshSpecification& spec);

  // Constructs the data to hold the uniforms in `spec` and initializes the
  // values of any unpacking transforms.
  //
  //   * `ink_attributes` should come from the `MeshFormat` used to create the
  //     `spec`.
  //   * The `get_attribute_unpacking_transform` callback should take an
  //     attribute index in the range [0, ink_attributes.size()) and return the
  //     associated coding parameters.
  //
  // CHECK-fails if the `MeshAttributeCodingParams` returned by the callback for
  // a given index does not have the same number of components as the
  // `MeshFormat::Attribute` at that index.
  explicit MeshUniformData(
      const SkMeshSpecification& spec,
      absl::Span<const MeshFormat::Attribute> ink_attributes,
      absl::FunctionRef<const MeshAttributeCodingParams&(int)>
          get_attribute_unpacking_transform);

  MeshUniformData() = default;
  MeshUniformData(const MeshUniformData&) = default;
  MeshUniformData(MeshUniformData&&) = default;
  MeshUniformData& operator=(const MeshUniformData&) = default;
  MeshUniformData& operator=(MeshUniformData&&) = default;
  ~MeshUniformData() = default;

  // The following getters return whether the data has specific uniforms:
  bool HasObjectToCanvasLinearComponent() const;
  bool HasBrushColor() const;
  bool HasTextureMapping() const;

  // The following setters update the values for each uniform.
  //
  // A call to set a value CHECK-validates that the uniform is present, which
  // can be verified by calling the appropriate "Has" function above.

  void SetObjectToCanvasLinearComponent(const AffineTransform& transform);
  void SetBrushColor(const Color& color);
  void SetTextureMapping(BrushPaint::TextureMapping mapping);

  // Returns the data for `SkMesh` creation. This function returns `nullptr` if
  // this uniform data was either default-constructed, or constructed from a
  // mesh specification that does not have any uniforms.
  //
  // The values stored in the returned memory will be unaffected by future calls
  // to set new uniform values.
  sk_sp<const SkData> Get() const { return data_; }

 private:
  // TODO: b/284117747 - Make `data_` "double or triple buffered" to increase
  // the likelihood of finding a unique one and not reallocating every frame.
  sk_sp<SkData> data_;

  // Offsets in bytes into `data_` for where to copy uniform values.
  int16_t object_to_canvas_linear_component_offset_ = -1;
  int16_t brush_color_offset_ = -1;
  int16_t texture_mapping_offset_ = -1;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline bool MeshUniformData::HasObjectToCanvasLinearComponent() const {
  return object_to_canvas_linear_component_offset_ != -1;
}

inline bool MeshUniformData::HasBrushColor() const {
  return brush_color_offset_ != -1;
}

inline bool MeshUniformData::HasTextureMapping() const {
  return texture_mapping_offset_ != -1;
}

}  // namespace ink::skia_native_internal

#endif  // INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_UNIFORM_DATA_H_
