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

#ifndef INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_SPECIFICATION_CACHE_H_
#define INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_SPECIFICATION_CACHE_H_

#include <cstdint>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/strokes/in_progress_stroke.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {

// A cache for `SkMeshSpecification`.
//
// The specification includes a large portion of the SkSL for rendering meshes,
// so it is an important optimization to reuse them and prevent redundant shader
// compilation.
class MeshSpecificationCache {
 public:
  // TODO: b/284117747 - The cache should be constructible with `SkColorSpace`
  // and `SkAlphaType` information.

  MeshSpecificationCache() = default;
  MeshSpecificationCache(const MeshSpecificationCache&) = delete;
  MeshSpecificationCache(MeshSpecificationCache&&) = default;
  MeshSpecificationCache& operator=(const MeshSpecificationCache&) = delete;
  MeshSpecificationCache& operator=(MeshSpecificationCache&&) = default;
  ~MeshSpecificationCache() = default;

  // Returns the specification for an `InProgressStroke`.
  //
  // An invalid-argument error is returned if `stroke.Start()` has not been
  // called.
  absl::StatusOr<sk_sp<SkMeshSpecification>> GetFor(
      const InProgressStroke& stroke);

  // Returns the specification for a `PartitionedMesh` created for a `Stroke`.
  //
  // An invalid-argument error is returned if `stroke_shape` either has no
  // meshes, or has an unsupported `MeshFormat`.
  // TODO: b/284117747 - Update to also take a `const BrushFamily&`.
  absl::StatusOr<sk_sp<SkMeshSpecification>> GetForStroke(
      const PartitionedMesh& stroke_shape, uint32_t coat_index);

 private:
  // TODO: b/284117747 - Update the in-progress stroke cache to a hash map if we
  // move to using Skia shader-uniforms in C++, which means the `BrushPaint`
  // would be included as an input to the specification. Similarly, the key to
  // the stroke hash map would need to be made of both the `MeshFormat` and the
  // `BrushPaint`.
  sk_sp<SkMeshSpecification> in_progress_stroke_specification_;
  absl::flat_hash_map<MeshFormat, sk_sp<SkMeshSpecification>>
      stroke_specifications_;
};

}  // namespace ink::skia_native_internal

#endif  // INK_RENDERING_SKIA_NATIVE_INTERNAL_MESH_SPECIFICATION_CACHE_H_
