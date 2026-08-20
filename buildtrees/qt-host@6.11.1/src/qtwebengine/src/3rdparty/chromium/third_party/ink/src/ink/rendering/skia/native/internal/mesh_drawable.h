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

#ifndef INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_DRAWABLE_H_
#define INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_DRAWABLE_H_

#include <cstdint>
#include <optional>

#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/rendering/skia/native/internal/mesh_uniform_data.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"

namespace ink::skia_native_internal {

// A drawable object based on `SkMesh`.
//
// One drawable consists of one or more "partitions" of vertices and triangle
// indices. Every partition in the drawable uses the same `SkMeshSpecification`
// and uniform values.
class MeshDrawable {
 public:
  // A single partition of the mesh.
  //
  // The members correspond to a subset of parameters of `SkMesh::MakeIndexed()`
  // with implicit `Mode::kTriangles` and vertex and index offsets of 0.
  struct Partition {
    sk_sp<SkMesh::VertexBuffer> vertex_buffer;
    sk_sp<SkMesh::IndexBuffer> index_buffer;
    int32_t vertex_count;
    int32_t index_count;
    SkRect bounds;
  };

  // Creates and returns a new `MeshDrawable` with the given `specification`,
  // `partitions`, and optional `starting_uniforms`.
  //
  // This function validates that `SkMesh::MakeIndexed()` succeeds for every
  // provided `Partition`, and returns an invalid-argument error otherwise.
  //
  // CHECK-fails if `specification` or any buffer in `partitions` is null.
  static absl::StatusOr<MeshDrawable> Create(
      sk_sp<SkMeshSpecification> specification, sk_sp<SkBlender> blender,
      sk_sp<SkShader> shader, absl::InlinedVector<Partition, 1> partitions,
      std::optional<MeshUniformData> starting_uniforms = std::nullopt);

  MeshDrawable() = default;
  MeshDrawable(const MeshDrawable&) = default;
  MeshDrawable(MeshDrawable&&) = default;
  MeshDrawable& operator=(const MeshDrawable&) = default;
  MeshDrawable& operator=(MeshDrawable&&) = default;
  ~MeshDrawable() = default;

  // Returns true if the drawable has the brush-color uniform.
  bool HasBrushColor() const;

  // Sets the value of the brush-color uniform.
  //
  // CHECK-fails if the drawable was created with an `SkMeshSpecification` that
  // does not have this uniform.
  void SetBrushColor(const Color& color);

  // Returns true if the drawable has the texture-mapping uniform.
  //
  // TODO: b/375203215 - Get rid of this uniform once we are able to mix tiling
  // and winding textures in a single `BrushPaint`.
  bool HasTextureMapping() const;

  // Sets the value of the texture-mapping uniform.
  //
  // CHECK-fails if the drawable was created with an `SkMeshSpecification` that
  // does not have this uniform.
  //
  // TODO: b/375203215 - Get rid of this uniform once we are able to mix tiling
  // and winding textures in a single `BrushPaint`.
  void SetTextureMapping(BrushPaint::TextureMapping mapping);

  // Returns true if the drawable has an object-to-canvas uniform.
  bool HasObjectToCanvas() const;

  // Sets the value of the object-to-canvas uniform.
  //
  // CHECK-fails if the drawable was created with an `SkMeshSpecification` that
  // does not have this uniform.
  void SetObjectToCanvas(const AffineTransform& transform);

  void SetImageFilter(sk_sp<SkImageFilter> image_filter);

  // Draws the mesh-drawable into the provided `canvas`.
  void Draw(SkCanvas& canvas) const;

 private:
  MeshDrawable(sk_sp<SkMeshSpecification> specification,
               sk_sp<SkBlender> blender, sk_sp<SkShader> shader,
               absl::InlinedVector<Partition, 1> partitions,
               MeshUniformData uniform_data);

  sk_sp<SkMeshSpecification> specification_;
  sk_sp<SkBlender> blender_;
  sk_sp<SkShader> shader_;
  absl::InlinedVector<Partition, 1> partitions_;
  MeshUniformData uniform_data_;
  sk_sp<SkImageFilter> image_filter_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline bool MeshDrawable::HasBrushColor() const {
  return uniform_data_.HasBrushColor();
}

inline void MeshDrawable::SetBrushColor(const Color& color) {
  uniform_data_.SetBrushColor(color);
}

inline bool MeshDrawable::HasTextureMapping() const {
  return uniform_data_.HasTextureMapping();
}

inline void MeshDrawable::SetTextureMapping(
    BrushPaint::TextureMapping mapping) {
  uniform_data_.SetTextureMapping(mapping);
}

inline bool MeshDrawable::HasObjectToCanvas() const {
  return uniform_data_.HasObjectToCanvasLinearComponent();
}

inline void MeshDrawable::SetObjectToCanvas(const AffineTransform& transform) {
  uniform_data_.SetObjectToCanvasLinearComponent(transform);
}

inline void MeshDrawable::SetImageFilter(sk_sp<SkImageFilter> image_filter) {
  image_filter_ = image_filter;
}

}  // namespace ink::skia_native_internal

#endif  // INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_DRAWABLE_H_
