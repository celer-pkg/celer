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

#include "ink/geometry/tessellator.h"

#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/generic_tessellator.h"
#include "ink/geometry/internal/point_tessellation_helper.h"  // IWYU pragma: keep
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/point.h"

namespace ink {

absl::StatusOr<Mesh> CreateMeshFromPolyline(absl::Span<const Point> points) {
  if (points.size() < 3) {
    return absl::InvalidArgumentError(
        absl::StrCat("Can not tessellate polyline with size: ", points.size(),
                     ". The polyline must have at least three points."));
  }
  geometry_internal::TessellationResult<Point> result =
      geometry_internal::Tessellate<geometry_internal::PointTessellationHelper>(
          points);
  if (result.indices.empty()) {
    return absl::InternalError("Could not tessellate polyline.");
  }

  const auto& vertices = result.vertices;
  std::vector<float> vertex_position_x;
  std::vector<float> vertex_position_y;
  vertex_position_x.reserve(vertices.size());
  vertex_position_y.reserve(vertices.size());

  for (const auto& vertex : vertices) {
    vertex_position_x.push_back(vertex.x);
    vertex_position_y.push_back(vertex.y);
  }

  return Mesh::Create(MeshFormat(), {vertex_position_x, vertex_position_y},
                      result.indices);
}

}  // namespace ink
