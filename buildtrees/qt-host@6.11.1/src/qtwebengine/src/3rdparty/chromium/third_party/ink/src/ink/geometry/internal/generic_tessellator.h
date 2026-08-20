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

#ifndef INK_GEOMETRY_INTERNAL_GENERIC_TESSELLATOR_H_
#define INK_GEOMETRY_INTERNAL_GENERIC_TESSELLATOR_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "libtess2/tesselator.h"

namespace ink::geometry_internal {

// This struct contains two fields:
// 1) `vertices`: A vector of vertices in the result, which may contain vertices
// not in the original contours, resulting from edge intersections.
// 2) `indices`: A flat vector of vertex indices making up the triangles in the
// tessellation e.g. A `TessellationResult` with four vertices {v0, v1, v2, v3},
// and two triangles {v0, v1, v2} and {v1, v2, v3} will have indices = {0, 1, 2,
// 1, 2, 3}.
template <typename VertexType>
struct ABSL_MUST_USE_RESULT TessellationResult {
  std::vector<VertexType> vertices;
  std::vector<uint32_t> indices;
};

inline constexpr int kTriangleSize = 3;
inline constexpr int kVertexSize = 2;
inline constexpr int kMaxNeighbors = 4;
inline constexpr int kConnectedPolygonsResultSize = kTriangleSize * 2;

// Tries to tessellate `edges` with libtess2. In case of error, the result
// will be empty.
//
// libtess2 only handles geometry, not any interpolation of non-position
// attributes, so we have to handle that ourselves. That's done by:
//
//   * Run tessellation in `TESS_CONNECTED_POLYGONS` mode.
//   * Look to see if an output vertex is represented in our input.
//     * If yes, copy the Vertex from the input.
//     * If not, grab the relevant neighboring vertices (listed CCW) from the
//       output polygon and its neighbors (a neighboring polygon shares an
//       edge CCW of the vertex with the corresponding index). Interpolating
//       non-position Vertex attributes by doing a weighted average between
//       neighbors, where each gets a weight proportional to the inverse of
//       distance to the new vertex. (Note that in some cases the
//       intersection vertices have already been created by preprocessing,
//       as is the case in `StrokeOutlineConverter::CreateProcessedElement`.
//       In those cases, there won't be any new intersection vertices
//       created during tessellation.)
//
// Template parameter `VertexTessellationHelper` provides the information about
// the vertex type required for tessellation. It should provide the following
// aliases and static member functions (using `MyVertex` as an example vertex
// type):

// struct MyVertexTessellationHelper {
//   // The type of vertex to use for tessellation. It must store its x- and
//   // y-coordinates as floats, which must be at offsets 0 bytes and 4 bytes,
//   // respectively.
//   using VertexType = MyVertex;

//   // Returns the x- and y-coordinates of the
//   static float GetX(const VertexType& vertex);
//   static float GetY(const VertexType& vertex);

//   // Returns the byte offset of the floats containing the x- and
//   // y-coordinates within the vertex type.
//   static constexpr size_t GetXOffSetBytes();
//   static constexpr size_t GetYOffSetBytes();

//   // Creates a new instance of the vertex type with the given x- and
//   // y-coordinates.
//   static VertexType CreateVertex(float x, float y);

//   // Populates any non-position attributes on vertex `dst`. `vertices`
//   // contains the result vertices from tessellation (newly-created vertices
//   // may not have non-position attributes yet), and `neighbor_indices`
//   // contains the indices in `vertices` of the vertices connected to `dst`
//   // by an edge.
//   // Note: The position attributes `position.x` and `position.y` must be
//   // populated on `dst` before calling this method.
//   static void PopulateNonPositionAttributes(
//       absl::Span<const VertexType> vertices,
//       absl::Span<const uint32_t> neighbor_indices,
//       VertexType& dst);
// };
template <typename VertexTessellationHelper>
TessellationResult<typename VertexTessellationHelper::VertexType> Tessellate(
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessWindingRule winding_rule);

template <typename VertexTessellationHelper>
TessellationResult<typename VertexTessellationHelper::VertexType> Tessellate(
    absl::Span<const typename VertexTessellationHelper::VertexType> vertices);

// Indicates which direction to look for the adjacent vertex in `AdjacentTo`.
// These enum values are chosen s.t. `kCounterClockwise` is a positive step mod
// 3 and `kClockwise` is a negative step mod 3.
enum class VertexOffset {
  kCounterClockwise = 1,
  kClockwise = 2,
};

// Return the index of the vertex adjacent to the vertex at `tri_idx` within
// `triangle`, in the direction of `offset`. `triangle` points to the three
// vertex indices that make up the triangle. `tri_idx` indicates which vertex of
// `triangle` to find the adjacent vertex to, and must be 0, 1, or 2. `offset`
// indicates whether to find the adjacent vertex in the clockwise or
// counter-clockwise direction.
TESSindex AdjacentTo(const TESSindex* triangle, int tri_idx,
                     VertexOffset offset);

template <typename VertexTessellationHelper>
bool TessellateConnectedPolygons(
    TESStesselator* tess,
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessWindingRule winding_rule);

// Finds neighbor of an intersection vertex in an adjoining polygon. Since
// it's a different polygon, we have to find the intersection vertex before
// taking the offset.
TESSindex FindNeighbor(const TESSindex* elems, TESSindex output_vertex_index,
                       TESSindex neighbor_triangle_index, VertexOffset offset);

std::array<uint32_t, kMaxNeighbors> FindNeighbors(TESStesselator* tess,
                                                  const TESSindex* triangle,
                                                  int v);

template <typename VertexTessellationHelper>
typename VertexTessellationHelper::VertexType InterpolateIntersectionVertex(
    TESStesselator* tess, const TESSindex* triangle, int v,
    const std::vector<TESSindex>& output_vertex_input_indices,
    absl::Span<const typename VertexTessellationHelper::VertexType> vertices);

template <typename VertexTessellationHelper>
void PopulateResult(
    TESStesselator* tess,
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessellationResult<typename VertexTessellationHelper::VertexType>& result);

// ---------------------------------------------------------------------------
//                     Implementation details below

inline TESSindex AdjacentTo(const TESSindex* triangle, int tri_idx,
                            VertexOffset offset) {
  return triangle[(tri_idx + static_cast<int>(offset)) % kTriangleSize];
}

template <typename VertexTessellationHelper>
bool TessellateConnectedPolygons(
    TESStesselator* tess,
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessWindingRule winding_rule) {
  // We can call `tessAddContour` on Vertex[] as long as we can tell it the
  // right start/stride/count for finding coords. A coord is vertex size
  // adjacent TESSreals, i.e. two adjacent floats.
  static_assert(VertexTessellationHelper::GetXOffSetBytes() == 0);
  static_assert(VertexTessellationHelper::GetYOffSetBytes() ==
                sizeof(TESSreal));
  static_assert(
      std::is_same_v<decltype(VertexTessellationHelper::GetX(edges[0][0])),
                     TESSreal>);
  static_assert(
      std::is_same_v<decltype(VertexTessellationHelper::GetY(edges[0][0])),
                     TESSreal>);

  float this_x, this_y, min_x, max_x, min_y, max_y;
  min_x = max_x = min_y = max_y = 0;
  for (const auto& edge : edges) {
    for (const auto& vertex : edge) {
      this_x = VertexTessellationHelper::GetX(vertex);
      this_y = VertexTessellationHelper::GetY(vertex);
      min_x = std::min(min_x, this_x);
      max_x = std::max(max_x, this_x);
      min_y = std::min(min_y, this_y);
      max_y = std::max(max_y, this_y);
    }
    tessAddContour(tess, kVertexSize, edge.data(),
                   sizeof(typename VertexTessellationHelper::VertexType),
                   edge.size());
  }

  // nullptr for the normal means it's automatically computed.
  return tessTesselate(tess, winding_rule, TESS_CONNECTED_POLYGONS,
                       kTriangleSize, kVertexSize, /*normal=*/nullptr);
}

inline TESSindex FindNeighbor(const TESSindex* elems,
                              TESSindex output_vertex_index,
                              TESSindex neighbor_triangle_index,
                              VertexOffset offset) {
  if (neighbor_triangle_index == TESS_UNDEF) return TESS_UNDEF;

  const TESSindex* neighbor_triangle =
      &elems[neighbor_triangle_index * kConnectedPolygonsResultSize];
  for (int other_v = 0; other_v < kTriangleSize; other_v++) {
    if (neighbor_triangle[other_v] == output_vertex_index) {
      return AdjacentTo(neighbor_triangle, other_v, offset);
    }
  }
  return TESS_UNDEF;
}

inline std::array<uint32_t, kMaxNeighbors> FindNeighbors(
    TESStesselator* tess, const TESSindex* triangle, int v) {
  const TESSindex* elems = tessGetElements(tess);
  const TESSindex output_vertex_index = triangle[v];
  const TESSindex* neighboring_triangles = &triangle[kTriangleSize];

  std::array<uint32_t, kMaxNeighbors> neighbor_vertex_output_indices;

  // There are at most four neighboring vertices, we're only looking at vertices
  // on the triangle and neighbors which share an edge. The vertices are listed
  // CCW. The neighboring triangles share the edge that originates from the
  // corresponding vertex (the neighbor is CCW of the corresponding vertex).
  //
  // Starting with the same-polygon neighbor CCW of the intersection vertex,
  // circle that CCW. Start with the two other vertices on the triangle.
  neighbor_vertex_output_indices[0] =
      AdjacentTo(triangle, v, VertexOffset::kCounterClockwise);
  neighbor_vertex_output_indices[1] =
      AdjacentTo(triangle, v, VertexOffset::kClockwise);
  // Then the vertex CW of the intersection vertex, from the neighbor
  // triangle sharing an edge CW of that vertex.
  neighbor_vertex_output_indices[2] = FindNeighbor(
      elems, output_vertex_index,
      AdjacentTo(neighboring_triangles, v, VertexOffset::kClockwise),
      VertexOffset::kClockwise);
  // Finally, the vertex CCW of the intersection vertex, from the neighbor
  // triangle sharing an edge CCW of that vertex.
  neighbor_vertex_output_indices[3] =
      FindNeighbor(elems, output_vertex_index, neighboring_triangles[v],
                   VertexOffset::kCounterClockwise);

  std::array<uint32_t, kMaxNeighbors> neighbor_vertex_input_indices;
  const TESSindex* output_vertex_input_indices = tessGetVertexIndices(tess);
  for (int i = 0; i < kMaxNeighbors; i++) {
    const TESSindex neighbor_output_index = neighbor_vertex_output_indices[i];
    neighbor_vertex_input_indices[i] =
        neighbor_output_index == TESS_UNDEF
            ? TESS_UNDEF
            : output_vertex_input_indices[neighbor_output_index];
  }
  return neighbor_vertex_input_indices;
}

template <typename VertexTessellationHelper>
typename VertexTessellationHelper::VertexType InterpolateIntersectionVertex(
    TESStesselator* tess, const TESSindex* triangle, int v,
    const std::vector<TESSindex>& output_vertex_input_indices,
    absl::Span<const typename VertexTessellationHelper::VertexType> vertices) {
  const TESSreal* output_vertices = tessGetVertices(tess);
  const TESSindex output_vertex_index = triangle[v];
  const TESSreal* output_vertex =
      &output_vertices[output_vertex_index * kVertexSize];
  const float output_x = output_vertex[0];
  const float output_y = output_vertex[1];

  std::array<uint32_t, kMaxNeighbors> neighbor_vertex_input_indices =
      FindNeighbors(tess, triangle, v);

  // Interpolate non-position data from the neighbors.
  typename VertexTessellationHelper::VertexType intersection =
      VertexTessellationHelper::CreateVertex(output_x, output_y);
  VertexTessellationHelper::PopulateNonPositionAttributes(
      vertices, neighbor_vertex_input_indices, intersection);
  return intersection;
}

template <typename VertexTessellationHelper>
void PopulateResult(
    TESStesselator* tess,
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessellationResult<typename VertexTessellationHelper::VertexType>& result) {
  const int n_elements = tessGetElementCount(tess);
  const TESSindex* elems = tessGetElements(tess);

  // Copying this so we can update it with the indices of newly-generated
  // intersection vertices in the result once those are interpolated. We want
  // some sort of memoization since we'll run into the same vertex on multiple
  // triangles.
  std::vector<TESSindex> output_vertex_input_indices(
      tessGetVertexIndices(tess),
      tessGetVertexIndices(tess) + tessGetVertexCount(tess));

  // Copy the input vertices to the result.
  result.vertices.reserve(tessGetVertexCount(tess));
  for (const auto& edge : edges) {
    result.vertices.insert(result.vertices.end(), edge.begin(), edge.end());
  }

  result.indices.reserve(tessGetElementCount(tess) * kTriangleSize);
  for (int e = 0; e < n_elements; e++) {
    const TESSindex* triangle = &elems[e * kConnectedPolygonsResultSize];
    for (int v = 0; v < kTriangleSize; v++) {
      const TESSindex output_vertex_index = triangle[v];

      // If we're done with this polygon, skip the remaining indices.
      if (output_vertex_index == TESS_UNDEF) break;

      const TESSindex input_vertex_index =
          output_vertex_input_indices[output_vertex_index];
      if (input_vertex_index != TESS_UNDEF) {
        // If we have the vertex, point to it.
        result.indices.push_back(input_vertex_index);
      } else {
        // Otherwise, create a new vertex and point to that, recording the
        // index for when we encounter it on other triangles.
        int new_index = result.vertices.size();
        output_vertex_input_indices[output_vertex_index] = new_index;
        result.indices.push_back(new_index);
        result.vertices.push_back(
            InterpolateIntersectionVertex<VertexTessellationHelper>(
                tess, triangle, v, output_vertex_input_indices,
                absl::MakeSpan(result.vertices)));
      }
    }
  }
}

template <typename VertexTessellationHelper>
TessellationResult<typename VertexTessellationHelper::VertexType> Tessellate(
    absl::Span<
        const absl::Span<const typename VertexTessellationHelper::VertexType>>
        edges,
    TessWindingRule winding_rule) {
  TessellationResult<typename VertexTessellationHelper::VertexType> result;
  if (edges.empty() || edges[0].empty()) return result;

  // This is using default allocation, but could customize that by passing
  // TESSAlloc*.
  TESStesselator* tess = tessNewTess(/*alloc=*/nullptr);
  ABSL_CHECK(tess != nullptr);
  if (TessellateConnectedPolygons<VertexTessellationHelper>(tess, edges,
                                                            winding_rule)) {
    PopulateResult<VertexTessellationHelper>(tess, edges, result);
  }
  tessDeleteTess(tess);
  return result;
}

template <typename VertexTessellationHelper>
TessellationResult<typename VertexTessellationHelper::VertexType> Tessellate(
    absl::Span<const typename VertexTessellationHelper::VertexType> vertices) {
  if (vertices.empty()) {
    return TessellationResult<typename VertexTessellationHelper::VertexType>();
  }
  return Tessellate<VertexTessellationHelper>({vertices}, TESS_WINDING_NONZERO);
}

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_GENERIC_TESSELLATOR_H_
