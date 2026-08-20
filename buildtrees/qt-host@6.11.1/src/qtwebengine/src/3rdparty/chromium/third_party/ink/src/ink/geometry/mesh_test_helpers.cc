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

#include "ink/geometry/mesh_test_helpers.h"

#include <cmath>
#include <cstdint>
#include <utility>

#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink {

MeshFormat MakeSinglePackedPositionFormat() {
  auto format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ABSL_CHECK_OK(format);
  return *format;
}

MutableMesh MakeStraightLineMutableMesh(
    uint32_t n_triangles, const MeshFormat& format,
    const AffineTransform& vertex_transform) {
  uint32_t n_vertices = n_triangles + 2;
  MutableMesh mesh(format);
  for (uint32_t i = 0; i < n_vertices; ++i) {
    mesh.AppendVertex(vertex_transform.Apply(
        Point{static_cast<float>(i), (i % 2) ? -1.f : 0.f}));
  }
  for (uint32_t i = 0; i < n_triangles; ++i) {
    if (i % 2 == 0) {
      mesh.AppendTriangleIndices({i, i + 1, i + 2});
    } else {
      mesh.AppendTriangleIndices({i, i + 2, i + 1});
    }
  }
  return mesh;
}

PartitionedMesh MakeStraightLinePartitionedMesh(
    uint32_t num_triangles, const MeshFormat& format,
    const AffineTransform& vertex_transform) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(num_triangles, format, vertex_transform));
  ABSL_CHECK_OK(shape);
  return *std::move(shape);
}

MutableMesh MakeCoiledRingMutableMesh(uint32_t n_triangles,
                                      uint32_t n_subdivisions,
                                      const MeshFormat& format,
                                      const AffineTransform& vertex_transform) {
  auto kStep = Angle::Degrees(360.f / n_subdivisions);
  uint32_t n_vertices = n_triangles + 2;
  MutableMesh mesh(format);
  for (uint32_t i = 0; i < n_vertices; ++i) {
    mesh.AppendVertex(vertex_transform.Apply(
        Point{0, 0} + Vec::FromDirectionAndMagnitude(
                          std::floor(0.5f * i) * kStep, i % 2 ? 1.f : 0.75f)));
  }
  for (uint32_t i = 0; i < n_triangles; ++i) {
    if (i % 2 == 0) {
      mesh.AppendTriangleIndices({i, i + 1, i + 2});
    } else {
      mesh.AppendTriangleIndices({i, i + 2, i + 1});
    }
  }
  return mesh;
}

PartitionedMesh MakeCoiledRingPartitionedMesh(
    uint32_t n_triangles, uint32_t n_subdivisions, const MeshFormat& format,
    const AffineTransform& vertex_transform) {
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMesh(MakeCoiledRingMutableMesh(
          n_triangles, n_subdivisions, format, vertex_transform));
  ABSL_CHECK_OK(shape);
  return *std::move(shape);
}

MutableMesh MakeStarMutableMesh(uint32_t n_triangles, const MeshFormat& format,
                                const AffineTransform& vertex_transform) {
  uint32_t n_vertices = 2 * n_triangles + 1;
  auto step = Angle::Degrees(180.f / n_triangles);
  MutableMesh mesh(format);
  for (uint32_t i = 0; i < n_vertices; ++i) {
    mesh.AppendVertex(vertex_transform.Apply(
        Point{0, 0} +
        Vec::FromDirectionAndMagnitude(i * step, i % 2 ? 1.f : .25f)));
  }
  for (uint32_t i = 0; i < n_triangles; ++i) {
    mesh.AppendTriangleIndices({2 * i, 2 * i + 1, 2 * i + 2});
  }
  return mesh;
}

PartitionedMesh MakeStarPartitionedMesh(
    uint32_t n_triangles, const MeshFormat& format,
    const AffineTransform& vertex_transform) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStarMutableMesh(n_triangles, format, vertex_transform));
  ABSL_CHECK_OK(shape);
  return *std::move(shape);
}

}  // namespace ink
