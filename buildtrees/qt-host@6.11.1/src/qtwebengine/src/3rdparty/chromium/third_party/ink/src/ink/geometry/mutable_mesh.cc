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

#include "ink/geometry/mutable_mesh.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <iterator>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/base/nullability.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/types/internal/float.h"
#include "ink/types/small_array.h"

namespace ink {

MutableMesh MutableMesh::Clone() const {
  MutableMesh cloned_mesh = MutableMesh(format_);
  cloned_mesh.vertex_data_ = vertex_data_;
  cloned_mesh.index_data_ = index_data_;
  cloned_mesh.vertex_count_ = vertex_count_;
  cloned_mesh.triangle_count_ = triangle_count_;
  return cloned_mesh;
}

MutableMesh MutableMesh::FromMesh(const Mesh& mesh) {
  MutableMesh mutable_mesh(mesh.Format());
  mutable_mesh.Resize(mesh.VertexCount(), mesh.TriangleCount());
  int n_attrs = mesh.Format().Attributes().size();
  for (uint32_t vertex_idx = 0; vertex_idx < mesh.VertexCount(); ++vertex_idx) {
    for (int attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
      mutable_mesh.SetFloatVertexAttribute(
          vertex_idx, attr_idx,
          mesh.FloatVertexAttribute(vertex_idx, attr_idx));
    }
  }
  if (mesh.Format().GetIndexFormat() ==
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked) {
    // The indices are stored in the same format in both meshes, we can just
    // copy them over directly, which is much faster.
    std::memcpy(mutable_mesh.index_data_.data(), mesh.RawIndexData().data(),
                3 * Mesh::kBytesPerIndex * mesh.TriangleCount());
  } else {
    for (uint32_t tri_idx = 0; tri_idx < mesh.TriangleCount(); ++tri_idx) {
      mutable_mesh.SetTriangleIndices(tri_idx, mesh.TriangleIndices(tri_idx));
    }
  }

  return mutable_mesh;
}

void MutableMesh::SetFloatVertexAttribute(uint32_t vertex_index,
                                          uint32_t attribute_index,
                                          SmallArray<float, 4> value) {
  ABSL_DCHECK_LT(vertex_index, VertexCount());
  ABSL_DCHECK_LT(attribute_index, format_.Attributes().size());
  const MeshFormat::Attribute attr = format_.Attributes()[attribute_index];
  ABSL_DCHECK_EQ(MeshFormat::ComponentCount(attr.type), value.Size());
  std::byte* dst =
      &vertex_data_[vertex_index * VertexStride() + attr.unpacked_offset];
  const float* src = value.Values().data();
  switch (attr.type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      std::memcpy(dst, src, sizeof(float));
      break;
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      std::memcpy(dst, src, 2 * sizeof(float));
      break;
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      std::memcpy(dst, src, 3 * sizeof(float));
      break;
    case MeshFormat::AttributeType::kFloat4Unpacked:
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      std::memcpy(dst, src, 4 * sizeof(float));
      break;
  }
}

absl::Status MutableMesh::ValidateTriangles() const {
  uint32_t n_triangles = TriangleCount();
  uint32_t n_vertices = VertexCount();
  for (uint32_t i = 0; i < n_triangles; ++i) {
    std::array<uint32_t, 3> indices = TriangleIndices(i);
    if (indices[0] >= n_vertices || indices[1] >= n_vertices ||
        indices[2] >= n_vertices)
      return absl::FailedPreconditionError(
          absl::Substitute("Triangle at index $0 refers to a non-existent "
                           "vertex (indices: $1, vertex count: $2)",
                           i, absl::StrJoin(indices, " "), n_vertices));
    if (indices[0] == indices[1] || indices[0] == indices[2] ||
        indices[1] == indices[2])
      return absl::FailedPreconditionError(absl::Substitute(
          "Triangle at index $0 does not refer to three distinct vertices "
          "(indices: $1)",
          i, absl::StrJoin(indices, " ")));
  }
  return absl::OkStatus();
}

namespace {

mesh_internal::AttributeBoundsArray ComputeAttributeBoundsForPartition(
    const MutableMesh& mesh, const mesh_internal::PartitionInfo& partition,
    const absl::flat_hash_set<MeshFormat::AttributeId>& omit_set) {
  constexpr float kInf = std::numeric_limits<float>::infinity();

  ABSL_CHECK_GE(partition.vertex_indices.size(), 0u);
  absl::Span<const MeshFormat::Attribute> old_attributes =
      mesh.Format().Attributes();
  size_t n_old_attrs = old_attributes.size();
  ABSL_CHECK_GT(n_old_attrs, omit_set.size());
  int n_new_attrs = n_old_attrs - omit_set.size();
  mesh_internal::AttributeBoundsArray bounds_array(n_new_attrs);
  for (size_t old_attr_idx = 0, new_attr_idx = 0; old_attr_idx < n_old_attrs;
       ++old_attr_idx) {
    MeshFormat::Attribute attribute = old_attributes[old_attr_idx];
    if (omit_set.contains(attribute.id)) continue;
    int n_components = MeshFormat::ComponentCount(attribute.type);
    MeshAttributeBounds& bounds = bounds_array[new_attr_idx];
    bounds.minimum = SmallArray<float, 4>(n_components, kInf);
    bounds.maximum = SmallArray<float, 4>(n_components, -kInf);
    for (uint32_t vertex_idx : partition.vertex_indices) {
      SmallArray<float, 4> value =
          mesh.FloatVertexAttribute(vertex_idx, old_attr_idx);
      for (int component_idx = 0; component_idx < n_components;
           ++component_idx) {
        bounds.minimum[component_idx] =
            std::min(value[component_idx], bounds.minimum[component_idx]);
        bounds.maximum[component_idx] =
            std::max(value[component_idx], bounds.maximum[component_idx]);
      }
    }
    ++new_attr_idx;
  }
  return bounds_array;
}

mesh_internal::AttributeBoundsArray ComputeTotalAttributeBounds(
    absl::Span<const mesh_internal::AttributeBoundsArray>
        partition_attribute_bounds) {
  auto join_attr_bounds = [](const MeshAttributeBounds& to_add,
                             MeshAttributeBounds& current) {
    for (int i = 0; i < current.minimum.Size(); ++i) {
      current.minimum[i] = std::min(current.minimum[i], to_add.minimum[i]);
      current.maximum[i] = std::max(current.maximum[i], to_add.maximum[i]);
    }
  };

  ABSL_CHECK(!partition_attribute_bounds.empty());
  mesh_internal::AttributeBoundsArray total_bounds =
      partition_attribute_bounds[0];
  int n_attrs = partition_attribute_bounds[0].Size();
  for (size_t partition_idx = 1;
       partition_idx < partition_attribute_bounds.size(); ++partition_idx) {
    for (int attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
      join_attr_bounds(partition_attribute_bounds[partition_idx][attr_idx],
                       total_bounds[attr_idx]);
    }
  }
  return total_bounds;
}

// These bitmasks represent the "nudges" that might be made to a triangle to try
// to un-flip, with each bit corresponding to either the x- or y-component of
// one of the triangle's vertices. The 1s-digit corresponds to the x-component
// of the first vertex, the 2s-digit to the y-component of the same, the
// 4s-digit to the x-component of the second vertex, and so on.
//
// These are just the values from 1 to 63 (0 is excluded because it corresponds
// to a no-op change), but are listed out in order of increasing number of
// vertices affected, then increasing number of components affected, then
// increasing bitmask value. This ordering means that, in cases where multiple
// solutions exist to correct the mesh, we select the one that changes the mesh
// the least.
constexpr uint16_t kNudgeDeltaBitMasks[] = {
    // One vertex, one component.
    0b000001, 0b000010, 0b000100, 0b001000, 0b010000, 0b100000,
    // One vertex, two components.
    0b000011, 0b001100, 0b110000,
    // Two vertices, two components.
    0b000101, 0b000110, 0b001001, 0b001010, 0b010001, 0b010010, 0b010100,
    0b011000, 0b100001, 0b100010, 0b100100, 0b101000,
    // Two vertices, three components.
    0b000111, 0b001011, 0b001101, 0b001110, 0b010011, 0b011100, 0b100011,
    0b101100, 0b110001, 0b110010, 0b110100, 0b111000,
    // Two vertices, four components.
    0b001111, 0b110011, 0b111100,
    // Three vertices, three components.
    0b010101, 0b010110, 0b011001, 0b011010, 0b100101, 0b100110, 0b101001,
    0b101010,
    // Three vertices, four components.
    0b010111, 0b011011, 0b011101, 0b011110, 0b100111, 0b101011, 0b101101,
    0b101110, 0b110101, 0b110110, 0b111001, 0b111010,
    // Three vertices, five components.
    0b011111, 0b101111, 0b110111, 0b111011, 0b111101, 0b111110,
    // Three vertices, six components.
    0b111111};

// Some useful constants for bitmask comparisons.
constexpr uint16_t kNudgeX0Bitmask = 0b100000;
constexpr uint16_t kNudgeY0Bitmask = 0b010000;
constexpr uint16_t kNudgeX1Bitmask = 0b001000;
constexpr uint16_t kNudgeY1Bitmask = 0b000100;
constexpr uint16_t kNudgeX2Bitmask = 0b000010;
constexpr uint16_t kNudgeY2Bitmask = 0b000001;
constexpr uint16_t kNudgeVertexBitmasks[] = {0b110000, 0b001100, 0b000011};
constexpr uint16_t kNudgeAllComponentsBitmask = 0b111111;

// The current state of a triangle as the algorithm progresses.
enum class TriFlipState : uint8_t {
  // The triangle was not flipped by quantization.
  kNotFlipped,
  // The triangle was flipped by quantization, and has not yet been corrected.
  kFlipped,
  // The triangle was flipped by quantization, but has been corrected.
  kFixed
};

// Contains intermediate data for the flipped triangle correction algorithm.
struct FlippedTriangleCorrectionData {
  // Pointers to the mesh we're packing, and the packing params used to do so.
  // The owner of this object is responsible for ensuring that the mesh and
  // packing params outlive this.
  const MutableMesh* absl_nonnull mesh;
  const MeshAttributeCodingParams* absl_nonnull packing_params;
  // The vertex positions of the mesh, rescaled to the range [0, 2^n_bits - 1]
  // and rounded to the nearest integer.
  std::vector<Point> quantized_vertex_positions;
  // The queue of indices of flipped triangles that remain to be corrected.
  std::deque<uint32_t> flipped_tri_queue;
  // The states of each triangle in the mesh as the algorithm progresses.
  std::vector<TriFlipState> tri_flip_states;
  // The set of adjacent triangles for each triangle in the mesh, where
  // "adjacent" means that they share one or more vertices.
  std::vector<absl::flat_hash_set<uint32_t>> adjacent_triangles;
  // The bounds of the representable quantized values.
  Rect quantization_bounds;
  // The corrected vertices, in the mesh's coordinate space.
  absl::flat_hash_map<uint32_t, Point> corrected_vertices;
};

Point QuantizePoint(Point p, const MeshAttributeCodingParams& packing_params) {
  auto quantize = [](MeshAttributeCodingParams::ComponentCodingParams params,
                     float value) {
    return std::round((value - params.offset) / params.scale) * params.scale +
           params.offset;
  };

  return {quantize(packing_params.components[0], p.x),
          quantize(packing_params.components[1], p.y)};
}

void PopulateQuantizedVertexPositions(FlippedTriangleCorrectionData& data) {
  uint32_t n_vertices = data.mesh->VertexCount();
  data.quantized_vertex_positions.resize(n_vertices);
  for (uint32_t i = 0; i < n_vertices; ++i) {
    data.quantized_vertex_positions[i] =
        QuantizePoint(data.mesh->VertexPosition(i), *data.packing_params);
  }
}

void PopulateFlippedTris(FlippedTriangleCorrectionData& data) {
  uint32_t n_triangles = data.mesh->TriangleCount();
  data.tri_flip_states.resize(n_triangles);
  for (uint32_t i = 0; i < n_triangles; ++i) {
    std::array<uint32_t, 3> indices = data.mesh->TriangleIndices(i);
    Triangle t{data.quantized_vertex_positions[indices[0]],
               data.quantized_vertex_positions[indices[1]],
               data.quantized_vertex_positions[indices[2]]};
    // Because `MutableMesh::AsMeshes` has already checked that the
    // pre-quantization triangles all has non-negative area, we only need to
    // check the post-quantization triangles here.
    if (t.SignedArea() < 0) {
      data.flipped_tri_queue.push_back(i);
      data.tri_flip_states[i] = TriFlipState::kFlipped;
    } else {
      data.tri_flip_states[i] = TriFlipState::kNotFlipped;
    }
  }
}

void PopulateAdjacentTriangleMap(FlippedTriangleCorrectionData& data) {
  uint32_t n_vertices = data.mesh->VertexCount();
  uint32_t n_triangles = data.mesh->TriangleCount();
  std::vector<absl::InlinedVector<uint32_t, 3>> vertex_to_tris;
  vertex_to_tris.resize(n_vertices);
  for (uint32_t tri_idx = 0; tri_idx < n_triangles; ++tri_idx) {
    for (uint32_t vtx_idx : data.mesh->TriangleIndices(tri_idx)) {
      vertex_to_tris[vtx_idx].push_back(tri_idx);
    }
  }
  data.adjacent_triangles.resize(n_triangles);
  for (uint32_t tri_idx = 0; tri_idx < n_triangles; ++tri_idx) {
    auto inserter = std::inserter(data.adjacent_triangles[tri_idx],
                                  data.adjacent_triangles[tri_idx].end());
    for (uint32_t vtx_idx : data.mesh->TriangleIndices(tri_idx)) {
      absl::c_copy(vertex_to_tris[vtx_idx], inserter);
    }

    // We don't consider a triangle to be adjacent to itself.
    data.adjacent_triangles[tri_idx].erase(tri_idx);
  }
}

Rect CalculateQuantizationBounds(const FlippedTriangleCorrectionData& data,
                                 SmallArray<uint8_t, 4> bits_per_component) {
  ABSL_DCHECK_EQ(bits_per_component.Size(), 2);
  float max_value_for_x_bits =
      mesh_internal::MaxValueForBits(bits_per_component[0]);
  float max_value_for_y_bits =
      mesh_internal::MaxValueForBits(bits_per_component[1]);
  auto x_component = data.packing_params->components[0];
  auto y_component = data.packing_params->components[1];
  return Rect::FromTwoPoints(
      {x_component.offset, y_component.offset},
      {x_component.offset + max_value_for_x_bits * x_component.scale,
       y_component.offset + max_value_for_y_bits * y_component.scale});
}

Triangle GetQuantizedTriangle(const FlippedTriangleCorrectionData& data,
                              const std::array<uint32_t, 3>& indices) {
  return Triangle{data.quantized_vertex_positions[indices[0]],
                  data.quantized_vertex_positions[indices[1]],
                  data.quantized_vertex_positions[indices[2]]};
}

// Returns a bitmask, in the same format as kNudgeDeltaBitmasks, which indicates
// which of vertices specified in `vertex_indices` have already been modified.
// Note that even if only one component of the vertex was actually modified, the
// bitmask will have 1s for both the x- and y-components.
uint16_t GetBitmaskOfAlreadyCorrectedVertices(
    const FlippedTriangleCorrectionData& data,
    const std::array<uint32_t, 3>& vertex_indices) {
  uint16_t bitmask = 0;
  for (int i = 0; i < 3; ++i) {
    if (data.corrected_vertices.contains(vertex_indices[i]))
      bitmask |= kNudgeVertexBitmasks[i];
  }
  return bitmask;
}

// Returns three vectors indicating the direction and amount that each component
// of each of the vertices specified in `vertex_indices` could be nudged. This
// will always be the opposite direction as the quantization, e.g. a vertex at
// (4.2, 5.8) rounds to (4, 6), so the returned vector would have a positive
// x-component and a negative y-component.
std::array<Vec, 3> GetNudgeVectors(
    const FlippedTriangleCorrectionData& data,
    const std::array<uint32_t, 3>& vertex_indices) {
  auto get_nudge_direction = [&data](uint32_t vertex_idx) {
    Point original = data.mesh->VertexPosition(vertex_idx);
    Point quantized = data.quantized_vertex_positions[vertex_idx];
    return Vec{std::copysign(1.f * data.packing_params->components[0].scale,
                             original.x - quantized.x),
               std::copysign(1.f * data.packing_params->components[1].scale,
                             original.y - quantized.y)};
  };
  return {get_nudge_direction(vertex_indices[0]),
          get_nudge_direction(vertex_indices[1]),
          get_nudge_direction(vertex_indices[2])};
}

// Returns a copy of `quantized_triangle` with the nudge applied.
// `nudge_bitmask` specifies which vertices and components to apply the nudge
// to, and `nudge_vectors` contains the direction of the nudge for each vertex
// and component.
Triangle GetNudgedTriangle(const Triangle& quantized_triangle,
                           const std::array<Vec, 3>& nudge_vectors,
                           uint16_t nudge_bitmask,
                           const MeshAttributeCodingParams& transform) {
  const float x0_nudge =
      (kNudgeX0Bitmask & nudge_bitmask) ? nudge_vectors[0].x : 0;
  const float y0_nudge =
      (kNudgeY0Bitmask & nudge_bitmask) ? nudge_vectors[0].y : 0;
  const float x1_nudge =
      (kNudgeX1Bitmask & nudge_bitmask) ? nudge_vectors[1].x : 0;
  const float y1_nudge =
      (kNudgeY1Bitmask & nudge_bitmask) ? nudge_vectors[1].y : 0;
  const float x2_nudge =
      (kNudgeX2Bitmask & nudge_bitmask) ? nudge_vectors[2].x : 0;
  const float y2_nudge =
      (kNudgeY2Bitmask & nudge_bitmask) ? nudge_vectors[2].y : 0;
  // Due to floating-point error, the nudged points may not lie exactly on the
  // quantization points, which can lead to an incorrect triangle area, so we
  // re-quantize after applying the nudge.
  return {
      QuantizePoint(quantized_triangle.p0 + Vec{x0_nudge, y0_nudge}, transform),
      QuantizePoint(quantized_triangle.p1 + Vec{x1_nudge, y1_nudge}, transform),
      QuantizePoint(quantized_triangle.p2 + Vec{x2_nudge, y2_nudge},
                    transform)};
}

// A candidate nudge for fixing a flipped triangle.
struct NudgeCandidate {
  // The nudge bitmask, see `kNudgeDeltaBitMasks` for details.
  uint16_t bitmask;
  // The adjacent triangles that are also fixed by this candidate.
  absl::flat_hash_set<uint32_t> newly_fixed_adjacent_tris;
  // The adjacent triangles that are newly broken by this candidate.
  absl::flat_hash_set<uint32_t> newly_flipped_adjacent_tris;

  // Returns true if `this` is a better nudge that `other`.
  bool IsBetterThan(const NudgeCandidate& other) const {
    // Prefer changes that don't cause adjacent triangles to flip.
    if (newly_flipped_adjacent_tris.size() <
        other.newly_flipped_adjacent_tris.size())
      return true;
    if (newly_flipped_adjacent_tris.size() >
        other.newly_flipped_adjacent_tris.size())
      return false;

    // Prefer changes that also fix other adjacent triangles.
    if (newly_fixed_adjacent_tris.size() >
        other.newly_fixed_adjacent_tris.size())
      return true;
    if (newly_fixed_adjacent_tris.size() <
        other.newly_fixed_adjacent_tris.size())
      return false;

    // We also prefer changes that affect fewer vertices, or affect fewer
    // components if tied. However, that's baked into the ordering of
    // `kNudgeDeltaBitmasks`, so we don't need to check it here.
    return false;
  }
};

// Attempts to construct the nudge candidate, returning std::nullopt if the
// nudge is disqualified, which occurs when:
// - It doesn't fixed the triangle
// - It mutates an already-corrected vertex
// - It moves a vertex outside of the representable bounds
// - It re-flips a triangle that has already been corrected.
std::optional<NudgeCandidate> MaybeMakeNudgeCandidate(
    uint32_t tri_idx, const std::array<uint32_t, 3>& indices,
    const Triangle& quantized_triangle, uint16_t nudge_bitmask,
    uint16_t already_corrected_bitmask, const std::array<Vec, 3>& nudge_vectors,
    const FlippedTriangleCorrectionData& data) {
  if (nudge_bitmask & already_corrected_bitmask) {
    // This nudge moves an already-altered vertex. We'll skip it, so that we
    // don't get caught in an infinite loop of re-flipping the same set of tris.
    return std::nullopt;
  }
  Triangle nudged_triangle = GetNudgedTriangle(
      quantized_triangle, nudge_vectors, nudge_bitmask, *data.packing_params);
  if (nudged_triangle.SignedArea() < 0) {
    // This nudge didn't fix it, the triangle is still flipped.
    return std::nullopt;
  }
  if (!data.quantization_bounds.Contains(*Envelope(nudged_triangle).AsRect())) {
    // This nudge take the triangle outside bounds of representable values.
    return std::nullopt;
  }

  // Returns the position of the vertex with the nudge applied, if it's
  // affected, or std::nullopt otherwise.
  auto maybe_get_nudged_vertex = [&indices, nudge_bitmask, &nudged_triangle](
                                     uint32_t vtx_idx) -> std::optional<Point> {
    if (vtx_idx == indices[0] && (nudge_bitmask & kNudgeVertexBitmasks[0]))
      return nudged_triangle.p0;
    if (vtx_idx == indices[1] && (nudge_bitmask & kNudgeVertexBitmasks[1]))
      return nudged_triangle.p1;
    if (vtx_idx == indices[2] && (nudge_bitmask & kNudgeVertexBitmasks[2]))
      return nudged_triangle.p2;
    return std::nullopt;
  };

  NudgeCandidate candidate{.bitmask = nudge_bitmask};
  for (uint32_t adj_tri_idx : data.adjacent_triangles[tri_idx]) {
    std::array<uint32_t, 3> adj_indices =
        data.mesh->TriangleIndices(adj_tri_idx);
    std::optional<Point> corrected_p0 = maybe_get_nudged_vertex(adj_indices[0]);
    std::optional<Point> corrected_p1 = maybe_get_nudged_vertex(adj_indices[1]);
    std::optional<Point> corrected_p2 = maybe_get_nudged_vertex(adj_indices[2]);
    if (!corrected_p0.has_value() && !corrected_p1.has_value() &&
        !corrected_p2.has_value()) {
      // This triangle is unaffected by the change.
      continue;
    }
    Triangle adj_tri{.p0 = corrected_p0.value_or(
                         data.quantized_vertex_positions[adj_indices[0]]),
                     .p1 = corrected_p1.value_or(
                         data.quantized_vertex_positions[adj_indices[1]]),
                     .p2 = corrected_p2.value_or(
                         data.quantized_vertex_positions[adj_indices[2]])};
    TriFlipState state = data.tri_flip_states[adj_tri_idx];
    // Detect if this triangle changes state. In order to ensure that we don't
    // loop forever, we only allow triangles change from kNotFlipped to kFlipped
    // tp kFixed, and reject candidates that would re-flip an already fixed
    // triangle.
    if (adj_tri.SignedArea() < 0) {
      if (state == TriFlipState::kNotFlipped) {
        candidate.newly_flipped_adjacent_tris.insert(adj_tri_idx);
      } else if (state == TriFlipState::kFixed) {
        // Don't consider candidates that re-flip an already fixed triangle.
        return std::nullopt;
      }
    } else if (state == TriFlipState::kFlipped) {
      candidate.newly_fixed_adjacent_tris.insert(adj_tri_idx);
    }
  }
  return candidate;
}

void RecordCorrectionAndAddNewFlippedTrisToQueue(
    uint32_t tri_idx, const std::array<uint32_t, 3>& indices,
    const Triangle& quantized_triangle, const std::array<Vec, 3>& nudge_vectors,
    const NudgeCandidate& candidate, FlippedTriangleCorrectionData& data) {
  auto record_vertex = [&data](uint32_t idx, Point p) {
    data.quantized_vertex_positions[idx] = p;
    data.corrected_vertices.insert({idx, p});
  };
  Triangle nudged_triangle =
      GetNudgedTriangle(quantized_triangle, nudge_vectors, candidate.bitmask,
                        *data.packing_params);
  if (candidate.bitmask & kNudgeVertexBitmasks[0]) {
    record_vertex(indices[0], nudged_triangle.p0);
  }
  if (candidate.bitmask & kNudgeVertexBitmasks[1]) {
    record_vertex(indices[1], nudged_triangle.p1);
  }
  if (candidate.bitmask & kNudgeVertexBitmasks[2]) {
    record_vertex(indices[2], nudged_triangle.p2);
  }

  data.tri_flip_states[tri_idx] = TriFlipState::kFixed;
  for (uint32_t adj_tri_idx : candidate.newly_fixed_adjacent_tris) {
    data.tri_flip_states[adj_tri_idx] = TriFlipState::kFixed;
  }
  for (uint32_t adj_tri_idx : candidate.newly_flipped_adjacent_tris) {
    data.tri_flip_states[adj_tri_idx] = TriFlipState::kFlipped;
  }

  if (!candidate.newly_flipped_adjacent_tris.empty()) {
    // Sort the newly added triangles before adding them to the queue to be
    // fixed -- this keeps the algorithm deterministic.
    std::vector<uint32_t> newly_flipped_tris;
    newly_flipped_tris.reserve(candidate.newly_fixed_adjacent_tris.size());
    absl::c_copy(candidate.newly_flipped_adjacent_tris,
                 std::back_inserter(newly_flipped_tris));
    absl::c_sort(newly_flipped_tris);
    absl::c_copy(newly_flipped_tris,
                 std::back_inserter(data.flipped_tri_queue));
  }
}

// Returns a map from vertex indices to positions for those vertices that need
// be changed to preserve triangle winding post-quantization. In the event that
// no correction can be found, this will return an empty map, allowing
// `MutableMesh::AsMeshes` to continue.
absl::flat_hash_map<uint32_t, Point> GetCorrectedPackedVertexPositions(
    const MutableMesh& mesh, const MeshAttributeCodingParams packing_params) {
  std::optional<SmallArray<uint8_t, 4>> bits_per_component =
      MeshFormat::PackedBitsPerComponent(
          mesh.Format().Attributes()[mesh.VertexPositionAttributeIndex()].type);
  // Unpacked types are not quantized, and so packing this mesh will not flip
  // any triangles.
  if (!bits_per_component.has_value()) return {};

  // If the mesh already has triangles with negative area, we don't attempt to
  // correct quantization errors.
  for (uint32_t tri_idx = 0; tri_idx < mesh.TriangleCount(); ++tri_idx) {
    if (mesh.GetTriangle(tri_idx).SignedArea() < 0) {
      return {};
    }
  }

  FlippedTriangleCorrectionData data = {
      .mesh = &mesh,
      .packing_params = &packing_params,
  };
  PopulateQuantizedVertexPositions(data);
  PopulateFlippedTris(data);

  // If no triangles were flipped, then we don't need to correct any vertices.
  if (data.flipped_tri_queue.empty()) return {};

  PopulateAdjacentTriangleMap(data);
  data.quantization_bounds =
      CalculateQuantizationBounds(data, *bits_per_component);

  // Iterator through the flipped triangles, nudging the vertices to try to
  // un-flip them.
  while (!data.flipped_tri_queue.empty()) {
    uint32_t tri_idx = data.flipped_tri_queue.front();
    data.flipped_tri_queue.pop_front();
    // The triangle could have been fixed by nudging a vertex contained in an
    // adjacent triangle; if so, we don't need to do anything else for this one.
    if (data.tri_flip_states[tri_idx] == TriFlipState::kFixed) continue;

    std::array<uint32_t, 3> indices = mesh.TriangleIndices(tri_idx);
    uint16_t already_corrected_bitmask =
        GetBitmaskOfAlreadyCorrectedVertices(data, indices);
    if (already_corrected_bitmask == kNudgeAllComponentsBitmask) {
      // All three vertices have already been corrected, so this triangle
      // can't be corrected.
      return {};
    }

    size_t n_adjacent_flipped_tris = absl::c_count_if(
        data.adjacent_triangles[tri_idx], [tri_idx, &data](uint32_t t) {
          return t != tri_idx &&
                 data.tri_flip_states[t] == TriFlipState::kFlipped;
        });

    // Try to find the best change to correct this triangle.
    Triangle quantized_triangle = GetQuantizedTriangle(data, indices);
    std::array<Vec, 3> nudge_vectors = GetNudgeVectors(data, indices);
    std::optional<NudgeCandidate> best_nudge;
    for (uint16_t nudge_bitmask : kNudgeDeltaBitMasks) {
      std::optional<NudgeCandidate> candidate = MaybeMakeNudgeCandidate(
          tri_idx, indices, quantized_triangle, nudge_bitmask,
          already_corrected_bitmask, nudge_vectors, data);
      if (!candidate.has_value()) {
        // Failed to make the candidate, likely because it flipped an
        // already-fixed triangle.
        continue;
      }
      if (candidate->newly_fixed_adjacent_tris.size() ==
              n_adjacent_flipped_tris &&
          candidate->newly_flipped_adjacent_tris.empty()) {
        // This fixes all flipped adjacent triangles (if any), and doesn't
        // cause any new flips -- we won't find a better candidate than this.
        best_nudge = std::move(candidate);
        break;
      }

      if (!best_nudge.has_value() || candidate->IsBetterThan(*best_nudge)) {
        best_nudge = std::move(candidate);
      }
    }

    if (!best_nudge.has_value()) return {};

    RecordCorrectionAndAddNewFlippedTrisToQueue(
        tri_idx, indices, quantized_triangle, nudge_vectors, *best_nudge, data);
  }

  return std::move(data.corrected_vertices);
}

}  // namespace

absl::StatusOr<absl::InlinedVector<Mesh, 1>> MutableMesh::AsMeshes(
    absl::Span<const std::optional<MeshAttributeCodingParams>> packing_params,
    absl::Span<const MeshFormat::AttributeId> omit_attributes) const {
  uint32_t n_triangles = TriangleCount();
  if (n_triangles == 0) {
    // There's nothing to partition, just return an empty list.
    return absl::InlinedVector<Mesh, 1>();
  }
  if (absl::Status status = ValidateTriangles(); !status.ok()) {
    return status;
  }

  absl::StatusOr<MeshFormat> new_format =
      format_.WithoutAttributes(omit_attributes);
  if (!new_format.ok()) return new_format.status();

  absl::flat_hash_set<MeshFormat::AttributeId> omit_set(omit_attributes.begin(),
                                                        omit_attributes.end());

  uint32_t n_vertices = VertexCount();
  uint32_t n_attrs = format_.Attributes().size();
  for (uint32_t vertex_idx = 0; vertex_idx < n_vertices; ++vertex_idx) {
    for (uint32_t attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
      if (omit_set.contains(format_.Attributes()[attr_idx].id)) continue;
      SmallArray<float, 4> value = FloatVertexAttribute(vertex_idx, attr_idx);
      if (!absl::c_all_of(value.Values(), ink_internal::IsFinite)) {
        return absl::FailedPreconditionError(absl::Substitute(
            "Failed to pack mesh: vertex $0 has non-finite value for attribute "
            "$1: {$2}",
            vertex_idx, attr_idx, absl::StrJoin(value.Values(), ", ")));
      }
    }
  }

  // Consistency check; the fact that there are valid triangles guarantees that
  // we have vertices.
  ABSL_DCHECK_GT(n_vertices, 0);

  constexpr uint32_t kMaxVerticesPerPartition = 1 << 8 * Mesh::kBytesPerIndex;
  absl::InlinedVector<mesh_internal::PartitionInfo, 1> partitions =
      mesh_internal::PartitionTriangles(index_data_, format_.GetIndexFormat(),
                                        kMaxVerticesPerPartition);
  absl::InlinedVector<mesh_internal::AttributeBoundsArray, 1>
      partition_attribute_bounds(partitions.size());
  for (size_t i = 0; i < partitions.size(); ++i) {
    partition_attribute_bounds[i] =
        ComputeAttributeBoundsForPartition(*this, partitions[i], omit_set);
  }

  // We use the total bounds to compute the packing params for all partitions so
  // that vertices that are in multiple partitions line up.
  mesh_internal::AttributeBoundsArray total_bounds =
      ComputeTotalAttributeBounds(partition_attribute_bounds);
  absl::StatusOr<mesh_internal::CodingParamsArray> packing_params_array =
      mesh_internal::ComputeCodingParamsArray(*new_format, total_bounds,
                                              packing_params);
  if (!packing_params_array.ok()) {
    return packing_params_array.status();
  }

  // TODO: b/283825926 - Try mitigating cases in which we cannot find a solution
  // for flipped triangles by retrying with a different scaling factor.
  absl::flat_hash_map<uint32_t, Point> corrected_vertex_positions =
      GetCorrectedPackedVertexPositions(
          *this, (*packing_params_array)[new_format->PositionAttributeIndex()]);

  absl::InlinedVector<Mesh, 1> meshes;
  for (size_t partition_idx = 0; partition_idx < partitions.size();
       ++partition_idx) {
    const mesh_internal::PartitionInfo& partition = partitions[partition_idx];
    std::vector<std::byte> partition_vertex_data =
        mesh_internal::CopyAndPackPartitionVertices(
            vertex_data_, partition.vertex_indices, format_, omit_set,
            *packing_params_array, corrected_vertex_positions);

    std::vector<std::byte> partition_index_data(3 * partition.triangles.size() *
                                                Mesh::kBytesPerIndex);
    for (uint32_t tri_idx = 0; tri_idx < partition.triangles.size();
         ++tri_idx) {
      mesh_internal::WriteTriangleIndicesToByteArray(
          tri_idx, Mesh::kBytesPerIndex, partition.triangles[tri_idx],
          partition_index_data);
    }

    meshes.push_back(Mesh(*new_format, *packing_params_array,
                          std::move(partition_attribute_bounds[partition_idx]),
                          std::move(partition_vertex_data),
                          std::move(partition_index_data)));
  }

  return meshes;
}

}  // namespace ink
