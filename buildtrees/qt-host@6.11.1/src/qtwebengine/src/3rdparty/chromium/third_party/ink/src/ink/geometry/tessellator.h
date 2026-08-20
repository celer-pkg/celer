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

#ifndef INK_GEOMETRY_TESSELLATOR_H_
#define INK_GEOMETRY_TESSELLATOR_H_

#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/point.h"

namespace ink {

// Constructs a `Mesh` that triangulates the interior of `polyline`. The
// interior is the union of all regions with a non-zero winding number (See
// https://en.wikipedia.org/wiki/Winding_number). The mesh has a default
// `MeshFormat` (only has position attributes). On success, the output mesh is
// guaranteed to be non-empty. In addition, on success, the number of vertices
// in the output mesh might be different from the number of vertices in
// `points`. For example, the method might add extra vertices in the mesh for
// the intersecting points in self-intersecting polyline.
//
// This method throws an error when `points` has less than three elements, or
// when all elements of `points` are collinear.
absl::StatusOr<Mesh> CreateMeshFromPolyline(absl::Span<const Point> points);

}  // namespace ink

#endif  // INK_GEOMETRY_TESSELLATOR_H_
