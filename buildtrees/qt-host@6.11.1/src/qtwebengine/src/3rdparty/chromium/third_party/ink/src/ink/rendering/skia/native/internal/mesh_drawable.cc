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

#include "ink/rendering/skia/native/internal/mesh_drawable.h"

#include <optional>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ink/rendering/skia/native/internal/mesh_uniform_data.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkImageFilters.h"

namespace ink::skia_native_internal {
namespace {

// Calls `SkMesh::MakeIndexed()` with a default mode and offsets.
//
// This wrapper helps to make it clear that certain parameters are the same for
// both initial validation and drawing.
SkMesh::Result MakeSkiaMesh(sk_sp<SkMeshSpecification> specification,
                            const MeshDrawable::Partition& partition,
                            sk_sp<const SkData> uniforms) {
  return SkMesh::MakeIndexed(std::move(specification), SkMesh::Mode::kTriangles,
                             partition.vertex_buffer, partition.vertex_count,
                             /* vertexOffset = */ 0, partition.index_buffer,
                             partition.index_count,
                             /* indexOffset = */ 0, std::move(uniforms),
                             /* children= */ {}, partition.bounds);
}

// Validates that `SkMesh::MakeIndexed()` will succeed for every partition.
//
// Creating an `SkMesh` is a relatively inexpensive operation that is
// analogous to gathering every setting for the graphics pipeline, so we are
// ok creating the `SkMesh` and "throwing away" the results here.
absl::Status ValidatePartitions(
    sk_sp<SkMeshSpecification> specification,
    absl::Span<const MeshDrawable::Partition> partitions,
    sk_sp<const SkData> uniform_data) {
  for (const MeshDrawable::Partition& partition : partitions) {
    ABSL_CHECK_NE(partition.vertex_buffer, nullptr);
    ABSL_CHECK_NE(partition.index_buffer, nullptr);

    SkMesh::Result result =
        MakeSkiaMesh(specification, partition, uniform_data);
    if (!result.mesh.isValid()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "`SkMesh::MakeIndex()` returned error: ",
          absl::string_view(result.error.data(), result.error.size())));
    }
  }

  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<MeshDrawable> MeshDrawable::Create(
    sk_sp<SkMeshSpecification> specification, sk_sp<SkBlender> blender,
    sk_sp<SkShader> shader, absl::InlinedVector<Partition, 1> partitions,
    std::optional<MeshUniformData> starting_uniforms) {
  ABSL_CHECK_NE(specification, nullptr);

  MeshUniformData uniform_data = starting_uniforms.has_value()
                                     ? *std::move(starting_uniforms)
                                     : MeshUniformData(*specification);
  absl::Status status =
      ValidatePartitions(specification, partitions, uniform_data.Get());
  if (!status.ok()) return status;

  return MeshDrawable(std::move(specification), std::move(blender),
                      std::move(shader), std::move(partitions),
                      std::move(uniform_data));
}

void MeshDrawable::Draw(SkCanvas& canvas) const {
  // We do not cache an `SkMesh` for each partition inside of the drawable
  // object. Instead, we create them on the stack inside this function, because:
  //   * Creating an `SkMesh` is a light-weight operation.
  //   * The `SkMesh` must be recreated if any uniform values must change.
  //   * In order to support shader-based antialiasing, we will need to update
  //     a uniform value anytime the object-to-canvas transformation changes.

  // TODO: b/267164444 - Use shader uniforms instead of `SkPaint`, once that's
  // exposed on Android. (We could do it here in the native renderer right now,
  // but we'd prefer to keep the native and Android renderers consistent.)
  //
  // We would prefer not to actively use the `SkPaint` or `SkBlender` when
  // drawing, because:
  //   * Color uniforms need to be set on the mesh instead of on a paint,
  //     because the mesh SkSL is the only place where we can apply per-vertex
  //     color shift.
  //   * We can make use of `SkMesh::ChildPtr` for sampling textures in the
  //     `SkMesh` instead of the `SkPaint` so that an object can be drawn with
  //     two textures and use a different set of texture coordiantes to sample
  //     from each.
  SkPaint paint;
  paint.setShader(shader_);
  paint.setColor(SK_ColorWHITE);
  paint.setImageFilter(image_filter_);
  sk_sp<const SkData> uniform_data = uniform_data_.Get();
  for (const Partition& partition : partitions_) {
    SkMesh mesh = MakeSkiaMesh(specification_, partition, uniform_data).mesh;
    canvas.drawMesh(mesh, blender_, paint);
  }
}

MeshDrawable::MeshDrawable(sk_sp<SkMeshSpecification> specification,
                           sk_sp<SkBlender> blender, sk_sp<SkShader> shader,
                           absl::InlinedVector<Partition, 1> partitions,
                           MeshUniformData uniform_data)
    : specification_(std::move(specification)),
      blender_(std::move(blender)),
      shader_(std::move(shader)),
      partitions_(std::move(partitions)),
      uniform_data_(std::move(uniform_data)) {}

}  // namespace ink::skia_native_internal
