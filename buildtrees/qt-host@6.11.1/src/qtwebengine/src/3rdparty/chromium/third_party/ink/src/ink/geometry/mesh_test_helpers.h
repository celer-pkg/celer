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

#ifndef INK_GEOMETRY_MESH_TEST_HELPERS_H_
#define INK_GEOMETRY_MESH_TEST_HELPERS_H_

#include <cstdint>

#include "ink/geometry/affine_transform.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/partitioned_mesh.h"

namespace ink {

// Creates a `MeshFormat` with a single `kFloat2PackedInOneFloat` attribute, and
// `k32BitUnpacked16BitPacked` index format.
MeshFormat MakeSinglePackedPositionFormat();

// Constructs a straight line triangle strip mesh with vertex attributes
// specified in `format`, and `n_triangles` triangles. The structure of the mesh
// is:
//     0-----2-----4-----6  ...
//      \ A / \ C / \ E / \
//       \ / B \ / D \ / F \
//        1-----3-----5-----7  ...
// The mesh will have `n_triangles` + 2 vertices.
// The vertex positions are (0, 0), (1, -1), (2, 0), (3, -1), ...
// The triangle indices are (0, 1, 2), (1, 3, 2), (2, 3, 4), (3, 5, 4), ...
// This does not set any attributes other than position.
//
// Optional argument `vertex_transform` will be applied to each vertex position
// when creating the mesh; this defaults to the identity transform.
MutableMesh MakeStraightLineMutableMesh(
    uint32_t n_triangles, const MeshFormat& format = MeshFormat(),
    const AffineTransform& vertex_transform = {});

// Same as `MakeStraightLineMutableMesh` above, except that instead of returning
// a `MutableMesh`, it returns a `PartitionedMesh` built from that mesh.
PartitionedMesh MakeStraightLinePartitionedMesh(
    uint32_t num_triangles, const MeshFormat& format = MeshFormat(),
    const AffineTransform& vertex_transform = {});

// Constructs a ring-like triangle strip mesh with vertex attributes specified
// in `format`, and `n_triangles` triangles.
//
// The mesh vertices start along the x-axis and proceed counter-clockwise after
// every second vertex, creating a ring that is comprising of `n_subdivisions`
// segments, with an inner radius of 0.75 and an outer radius of 1. If
// `n_triangles` > 2 * `n_subdivisions`, then the vertices and triangles will
// continue winding, overlapping the ones earlier in the mesh.
//
// The mesh will have `n_triangles` + 2 vertices.
// The vertex positions are (0.75, 0), (1, 0), (0.75 * cosφ, 0.75 * sinφ),
// (cosφ, sinφ), (0.75 * cos2φ, 0.75 * sin2φ), ...
// where φ = 2π / `n_subdivisions`.
// The triangle indices are (0, 1, 2), (1, 3, 2), (2, 3, 4), (3, 5, 4), ...
// This does not set any attributes other than position.
//
// Optional argument `vertex_transform` will be applied to each vertex position
// when creating the mesh; this defaults to the identity transform.
MutableMesh MakeCoiledRingMutableMesh(
    uint32_t n_triangles, uint32_t n_subdivisions,
    const MeshFormat& format = MeshFormat(),
    const AffineTransform& vertex_transform = {});

// Same as `MakeCoiledRingMutableMesh` above, except that instead of returning a
// `MutableMesh`, it returns a `PartitionedMesh` built from that mesh.
PartitionedMesh MakeCoiledRingPartitionedMesh(
    uint32_t n_triangles, uint32_t n_subdivisions,
    const MeshFormat& format = MeshFormat(),
    const AffineTransform& vertex_transform = {});

// Constructs a star-like mesh with vertex attributes specified in `format`, and
// `n_triangles` triangles.
//
// The mesh vertices start along the x-axis and proceed counter-clockwise,
// rotating π / `n_triangles` after each one, and alternating from inside to
// outside. The even vertices form a `n_triangles`-sided polygon with a
// circumradius of 0.25, and the odd vertices form a `n_triangles`-sided polygon
// with a circumradius of 1.
//
// The mesh will have 2 * `n_triangles` + 1 vertices.
// The vertex positions are (0.25, 0), (cosφ, sinφ),
// (0.25 * cos2φ, 0.25 * sin2φ), (cos3φ, sin3φ), ...
// where φ = π / `n_triangles`.
// The triangle indices are (0, 1, 2), (2, 3, 4), (4, 5, 6), (6, 7, 8), ...
// This does not set any attributes other than position.
//
// Optional argument `vertex_transform` will be applied to each vertex position
// when creating the mesh; this defaults to the identity transform.
MutableMesh MakeStarMutableMesh(uint32_t n_triangles,
                                const MeshFormat& format = MeshFormat(),
                                const AffineTransform& vertex_transform = {});

// Same as `MakeStarMutableMesh` above, except that instead of returning a
// `MutableMesh`, it returns a `PartitionedMesh` built from that mesh.
PartitionedMesh MakeStarPartitionedMesh(
    uint32_t n_triangles, const MeshFormat& format = MeshFormat(),
    const AffineTransform& vertex_transform = {});

}  // namespace ink

#endif  // INK_GEOMETRY_MESH_TEST_HELPERS_H_
