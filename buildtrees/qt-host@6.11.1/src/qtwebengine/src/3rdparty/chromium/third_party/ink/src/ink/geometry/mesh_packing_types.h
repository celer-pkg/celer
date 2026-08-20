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

#ifndef INK_GEOMETRY_MESH_PACKING_TYPES_H_
#define INK_GEOMETRY_MESH_PACKING_TYPES_H_

#include "ink/types/small_array.h"

namespace ink {

// The parameters for the transformation between the packed integer
// representation of a vertex attribute and its actual ("unpacked") value.
// Objects of this type are used both for packing and for unpacking; see
// internal/mesh_packing.h. We use the word "coding" in this type as a neutral
// name that applies equally to packing and unpacking.
struct MeshAttributeCodingParams {
  struct ComponentCodingParams {
    float offset;
    float scale;
  };
  SmallArray<ComponentCodingParams, 4> components;
};

// Contains the bounds of a single vertex attribute in a mesh.
struct MeshAttributeBounds {
  SmallArray<float, 4> minimum;
  SmallArray<float, 4> maximum;
};

}  // namespace ink

#endif  // INK_GEOMETRY_MESH_PACKING_TYPES_H_
