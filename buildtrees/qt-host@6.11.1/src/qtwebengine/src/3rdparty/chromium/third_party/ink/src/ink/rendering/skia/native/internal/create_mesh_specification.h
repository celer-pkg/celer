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

#ifndef INK_RENDERING_SKIA_NATIVE_INTERNAL_CREATE_MESH_SPECIFICATION_H_
#define INK_RENDERING_SKIA_NATIVE_INTERNAL_CREATE_MESH_SPECIFICATION_H_

#include "absl/status/statusor.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {

// Creates an `SkMeshSpecification` based on the platform-independent `data`.
//
// * CHECK-validates that:
//     * Every enum member variable in `data` is equal to one of the named
//       enumerators for that variable's type.
//     * Every attribute and varying name in `data` is non-empty.
// * Returns an invalid-argument error if:
//     * `SkMeshSpecification::Make()` returns an error.
//     * The uniforms of the created `SkMeshSpecification` do not map 1:1 to
//       those in `data.uniforms`.
absl::StatusOr<sk_sp<SkMeshSpecification>> CreateMeshSpecification(
    const skia_common_internal::MeshSpecificationData& data);

}  // namespace ink::skia_native_internal

#endif  // INK_RENDERING_SKIA_NATIVE_INTERNAL_CREATE_MESH_SPECIFICATION_H_
