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

#include "ink/geometry/mesh.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/triangle.h"
#include "ink/types/internal/float.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

std::optional<mesh_internal::AttributeBoundsArray> ComputeAttributeBounds(
    const MeshFormat& format,
    absl::Span<const absl::Span<const float>> vertex_attributes) {
  // Consistency check -- we've already validated that `vertex_attributes` has
  // the correct number of spans for `format`, and you can't create an empty
  // `MeshFormat`, so it should be impossible for this to fail.
  ABSL_CHECK(!vertex_attributes.empty());

  if (vertex_attributes[0].empty()) return std::nullopt;

  int n_attrs = format.Attributes().size();
  mesh_internal::AttributeBoundsArray bounds(n_attrs);
  int span_idx = 0;
  for (int attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
    const MeshFormat::Attribute attr = format.Attributes()[attr_idx];
    int n_components = MeshFormat::ComponentCount(attr.type);
    MeshAttributeBounds& b = bounds[attr_idx];
    b.minimum.Resize(n_components);
    b.maximum.Resize(n_components);
    for (int component_idx = 0; component_idx < n_components; ++component_idx) {
      auto minmax =
          absl::c_minmax_element(vertex_attributes[span_idx + component_idx]);
      b.minimum[component_idx] = *minmax.first;
      b.maximum[component_idx] = *minmax.second;
    }
    span_idx += n_components;
  }
  return bounds;
}

mesh_internal::CodingParamsArray MakeCodingParamsArrayForEmptyMesh(
    const MeshFormat& format) {
  int n_attrs = format.Attributes().size();
  mesh_internal::CodingParamsArray coding_params_array(n_attrs);
  for (int attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
    const MeshFormat::Attribute attr = format.Attributes()[attr_idx];
    coding_params_array[attr_idx].components =
        SmallArray<MeshAttributeCodingParams::ComponentCodingParams, 4>(
            MeshFormat::ComponentCount(attr.type), {.offset = 0, .scale = 1});
  }

  return coding_params_array;
}

}  // namespace

absl::StatusOr<Mesh> Mesh::Create(
    const MeshFormat& format,
    absl::Span<const absl::Span<const float>> vertex_attributes,
    absl::Span<const uint32_t> triangle_indices,
    absl::Span<const std::optional<MeshAttributeCodingParams>> packing_params) {
  size_t total_attr_components = format.TotalComponentCount();
  if (total_attr_components != vertex_attributes.size()) {
    return absl::InvalidArgumentError(
        absl::Substitute("Wrong number of vertex attributes; expected $0 total "
                         "components, found $1",
                         total_attr_components, vertex_attributes.size()));
  }
  // The check above should ensure that `vertex_attributes` is not empty.
  ABSL_DCHECK_GT(vertex_attributes.size(), 0);

  constexpr int kMaxVertices = 1 << (8 * kBytesPerIndex);
  size_t n_vertices = vertex_attributes[0].size();
  if (n_vertices > kMaxVertices) {
    return absl::InvalidArgumentError(
        absl::Substitute("Given more vertices than can be represented by the "
                         "index; vertices = $0, max = $1",
                         n_vertices, kMaxVertices));
  }
  for (size_t i = 1; i < vertex_attributes.size(); ++i) {
    if (vertex_attributes[i].size() != n_vertices) {
      return absl::InvalidArgumentError(absl::Substitute(
          "Vertex attributes have unequal lengths; span at index $0 has $1 "
          "elements, expected $2",
          i, vertex_attributes[i].size(), n_vertices));
    }
  }
  for (size_t i = 0; i < vertex_attributes.size(); ++i) {
    if (!absl::c_all_of(vertex_attributes[i], ink_internal::IsFinite)) {
      return absl::InvalidArgumentError(absl::Substitute(
          "Non-finite value found in vertex attribute span at index $0", i));
    }
  }
  if (triangle_indices.size() % 3 != 0) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Given a number of triangle indices that is not divisible by 3 ($0)",
        triangle_indices.size()));
  }
  if (!absl::c_all_of(triangle_indices,
                      [n_vertices](uint32_t i) { return i < n_vertices; })) {
    return absl::InvalidArgumentError(
        absl::Substitute("Found a triangle index that references a "
                         "non-existent vertex; vertices = $0",
                         n_vertices));
  }

  std::optional<mesh_internal::AttributeBoundsArray> attribute_bounds =
      ComputeAttributeBounds(format, vertex_attributes);
  mesh_internal::CodingParamsArray coding_params_array;
  if (attribute_bounds.has_value()) {
    auto result = mesh_internal::ComputeCodingParamsArray(
        format, *attribute_bounds, packing_params);
    if (!result.ok()) {
      return result.status();
    }
    coding_params_array = *result;
  } else {
    coding_params_array = MakeCodingParamsArrayForEmptyMesh(format);
  }
  std::vector<std::byte> vertex_data =
      PackVertexByteData(format, vertex_attributes, coding_params_array);

  std::vector<std::byte> index_data;
  index_data.resize(kBytesPerIndex * triangle_indices.size());
  size_t n_triangles = triangle_indices.size() / 3;
  for (size_t i = 0; i < n_triangles; ++i) {
    mesh_internal::WriteTriangleIndicesToByteArray(
        i, kBytesPerIndex, triangle_indices.subspan(3 * i, 3), index_data);
  }

  return Mesh(format, std::move(coding_params_array),
              std::move(attribute_bounds), std::move(vertex_data),
              std::move(index_data));
}

SmallArray<float, 4> Mesh::FloatVertexAttribute(
    uint32_t vertex_index, uint32_t attribute_index) const {
  ABSL_DCHECK_LT(attribute_index, Format().Attributes().size());
  ABSL_DCHECK_LT(attribute_index, data_->unpacking_params.Size());
  absl::Span<const std::byte> packed_value =
      PackedVertexAttribute(vertex_index, attribute_index);
  return mesh_internal::UnpackAttribute(
      Format().Attributes()[attribute_index].type,
      data_->unpacking_params[attribute_index], absl::MakeSpan(packed_value));
}

SmallArray<uint32_t, 4> Mesh::PackedIntegersForFloatVertexAttribute(
    uint32_t vertex_index, uint32_t attribute_index) const {
  ABSL_DCHECK_LT(attribute_index, Format().Attributes().size());
  absl::Span<const std::byte> packed_value =
      PackedVertexAttribute(vertex_index, attribute_index);
  return mesh_internal::UnpackIntegersFromPackedAttribute(
      Format().Attributes()[attribute_index].type, packed_value);
}

absl::Span<const std::byte> Mesh::PackedVertexAttribute(
    uint32_t vertex_index, uint32_t attribute_index) const {
  ABSL_DCHECK_LT(vertex_index, VertexCount());
  ABSL_DCHECK_LT(attribute_index, Format().Attributes().size());
  ABSL_DCHECK_LT(attribute_index, data_->unpacking_params.Size());
  const MeshFormat::Attribute attr = Format().Attributes()[attribute_index];
  return absl::MakeSpan(
      &data_->vertex_data[vertex_index * VertexStride() + attr.packed_offset],
      attr.packed_width);
}

Triangle Mesh::GetTriangle(uint32_t index) const {
  std::array<uint32_t, 3> vertex_indices = TriangleIndices(index);
  return {.p0 = VertexPosition(vertex_indices[0]),
          .p1 = VertexPosition(vertex_indices[1]),
          .p2 = VertexPosition(vertex_indices[2])};
}

std::vector<std::byte> Mesh::PackVertexByteData(
    const MeshFormat& format,
    absl::Span<const absl::Span<const float>> vertex_attributes,
    const mesh_internal::CodingParamsArray& packing_params_array) {
  size_t n_vertices = vertex_attributes[0].size();
  std::vector<std::byte> vertex_data(n_vertices * format.PackedVertexStride());

  int n_attrs = format.Attributes().size();
  for (size_t vertex_idx = 0; vertex_idx < n_vertices; ++vertex_idx) {
    size_t vertex_offset = vertex_idx * format.PackedVertexStride();
    int span_idx = 0;
    for (int attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
      const MeshFormat::Attribute attr = format.Attributes()[attr_idx];
      int n_components = MeshFormat::ComponentCount(attr.type);
      SmallArray<float, 4> unpacked(n_components);
      for (int component_idx = 0; component_idx < n_components;
           ++component_idx, ++span_idx) {
        unpacked[component_idx] = vertex_attributes[span_idx][vertex_idx];
      }
      absl::Span<std::byte> packed_value = absl::MakeSpan(
          &vertex_data[vertex_offset + attr.packed_offset], attr.packed_width);

      mesh_internal::PackAttribute(attr.type, packing_params_array[attr_idx],
                                   unpacked, packed_value);
    }
  }

  return vertex_data;
}

}  // namespace ink
