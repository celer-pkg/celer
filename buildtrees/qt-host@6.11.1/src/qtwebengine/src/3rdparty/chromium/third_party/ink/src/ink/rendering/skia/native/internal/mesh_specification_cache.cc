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

#include "ink/rendering/skia/native/internal/mesh_specification_cache.h"

#include <cstdint>
#include <utility>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"
#include "ink/rendering/skia/native/internal/create_mesh_specification.h"
#include "ink/strokes/in_progress_stroke.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {

using ::ink::skia_common_internal::MeshSpecificationData;

absl::StatusOr<sk_sp<SkMeshSpecification>> MeshSpecificationCache::GetFor(
    const InProgressStroke& stroke) {
  if (stroke.GetBrush() == nullptr) {
    return absl::InvalidArgumentError("`stroke.Start()` has not been called.");
  }

  if (in_progress_stroke_specification_ == nullptr) {
    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        CreateMeshSpecification(
            MeshSpecificationData::CreateForInProgressStroke());
    // TODO: b/284117747 - At least for now, if creating the
    // `MeshSpecificationData` succeeded, then creating the
    // `SkMeshSpecification` should always succeed. This may change, depending
    // on where `BrushFamily` is used during specification creation.
    ABSL_CHECK_OK(specification);
    in_progress_stroke_specification_ = *std::move(specification);
  }
  return in_progress_stroke_specification_;
}

absl::StatusOr<sk_sp<SkMeshSpecification>> MeshSpecificationCache::GetForStroke(
    const PartitionedMesh& stroke_shape, uint32_t coat_index) {
  if (stroke_shape.RenderGroupCount() <= coat_index) {
    return absl::InvalidArgumentError(absl::StrCat(
        "`stroke_shape` has only ", stroke_shape.RenderGroupCount(),
        " render groups, but `coat_index` was ", coat_index));
  }
  if (stroke_shape.RenderGroupMeshes(coat_index).empty()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "`stroke_shape` has no meshes for render group ", coat_index));
  }

  const MeshFormat& format = stroke_shape.RenderGroupFormat(coat_index);
  sk_sp<SkMeshSpecification>& cached_specification =
      stroke_specifications_[format];
  if (cached_specification == nullptr) {
    absl::StatusOr<MeshSpecificationData> specification_data =
        MeshSpecificationData::CreateForStroke(format);
    if (!specification_data.ok()) return specification_data.status();

    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        CreateMeshSpecification(*specification_data);
    // TODO: b/284117747 - At least for now, if creating the
    // `MeshSpecificationData` succeeded, then creating the
    // `SkMeshSpecification` should always succeed. This may change, depending
    // on where `BrushFamily` is used during specification creation.
    ABSL_CHECK_OK(specification);
    cached_specification = *std::move(specification);
  }
  return cached_specification;
}

}  // namespace ink::skia_native_internal
