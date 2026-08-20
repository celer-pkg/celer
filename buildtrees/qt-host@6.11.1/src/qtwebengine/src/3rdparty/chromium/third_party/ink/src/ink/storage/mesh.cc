// Copyright 2024-2025 Google LLC
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

#include "ink/storage/mesh.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/storage/mesh_format.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/coded_numeric_run.pb.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/types/iterator_range.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

using ::ink::proto::CodedMesh;
using ::ink::proto::CodedNumericRun;

// How many bits to pack vertex data into when encoding an unpacked mesh vertex
// attribute. We use the full fidelity of a float mantissa.
constexpr uint8_t kBitsPerUnpackedComponent =
    std::numeric_limits<float>::digits;

CodedNumericRun* InitCodedAttributeComponent(
    uint8_t component_index, const MeshAttributeCodingParams& coding_params,
    uint32_t vertex_count, CodedNumericRun* coded_component) {
  coded_component->mutable_deltas()->Reserve(vertex_count);
  coded_component->set_offset(coding_params.components[component_index].offset);
  coded_component->set_scale(coding_params.components[component_index].scale);
  return coded_component;
}

SmallArray<CodedNumericRun*, 4> InitCodedAttributeComponents(
    MeshFormat::AttributeId attribute_id,
    const MeshAttributeCodingParams& coding_params, uint32_t vertex_count,
    CodedMesh& coded_mesh) {
  uint8_t component_count = coding_params.components.Size();
  SmallArray<CodedNumericRun*, 4> coded_components(component_count);
  if (attribute_id == MeshFormat::AttributeId::kPosition) {
    ABSL_DCHECK_EQ(component_count, 2);
    coded_components[0] = InitCodedAttributeComponent(
        0, coding_params, vertex_count, coded_mesh.mutable_x_stroke_space());
    coded_components[1] = InitCodedAttributeComponent(
        1, coding_params, vertex_count, coded_mesh.mutable_y_stroke_space());
  } else {
    for (uint8_t c = 0; c < component_count; ++c) {
      coded_components[c] = InitCodedAttributeComponent(
          c, coding_params, vertex_count,
          coded_mesh.add_other_attribute_components());
    }
  }
  return coded_components;
}

void EncodePackedMeshAttribute(const Mesh& mesh, uint32_t attribute_index,
                               CodedMesh& coded_mesh) {
  uint32_t vertex_count = mesh.VertexCount();
  const MeshFormat::Attribute& attribute =
      mesh.Format().Attributes()[attribute_index];
  uint8_t component_count = MeshFormat::ComponentCount(attribute.type);
  SmallArray<CodedNumericRun*, 4> coded_components =
      InitCodedAttributeComponents(
          attribute.id, mesh.VertexAttributeUnpackingParams(attribute_index),
          vertex_count, coded_mesh);
  SmallArray<int, 4> previous_integers(component_count, 0);
  for (uint32_t v = 0; v < vertex_count; ++v) {
    SmallArray<uint32_t, 4> next_integers =
        mesh.PackedIntegersForFloatVertexAttribute(v, attribute_index);
    for (uint8_t c = 0; c < component_count; ++c) {
      coded_components[c]->add_deltas(next_integers[c] - previous_integers[c]);
      previous_integers[c] = next_integers[c];
    }
  }
}

void EncodeUnpackedMeshAttribute(const Mesh& mesh, uint32_t attribute_index,
                                 CodedMesh& coded_mesh) {
  // TODO: b/294865374 - Handle flipped-triangle correction.  Possibly this
  // function could work by (1) creating an equivalent MeshFormat using only
  // packed attributes, (2) creating a new MutableMesh with the same data as the
  // Mesh, but with the new format, (3) calling MutableMesh::AsMeshes to perform
  // the packing and flipped-triangle correction, and (4) calling
  // EncodePackedMeshPositions().
  uint32_t vertex_count = mesh.VertexCount();
  ABSL_DCHECK_GT(vertex_count, 0);
  std::optional<MeshAttributeBounds> attribute_bounds =
      mesh.AttributeBounds(attribute_index);
  ABSL_CHECK(attribute_bounds.has_value());  // we know mesh is non-empty

  const MeshFormat::Attribute& attribute =
      mesh.Format().Attributes()[attribute_index];
  uint8_t component_count = MeshFormat::ComponentCount(attribute.type);

  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      mesh_internal::ComputeCodingParamsForBitSizes(
          SmallArray<uint8_t, 4>(component_count, kBitsPerUnpackedComponent),
          *attribute_bounds);
  ABSL_CHECK_OK(coding_params);  // Mesh type guarantees valid bounds

  SmallArray<CodedNumericRun*, 4> coded_components =
      InitCodedAttributeComponents(attribute.id, *coding_params, vertex_count,
                                   coded_mesh);
  SmallArray<int, 4> previous_integers(component_count, 0);
  for (uint32_t v = 0; v < vertex_count; ++v) {
    SmallArray<float, 4> next_floats =
        mesh.FloatVertexAttribute(v, attribute_index);
    for (uint8_t c = 0; c < component_count; ++c) {
      int next_integer = mesh_internal::PackSingleFloat(
          coding_params->components[c], next_floats[c]);
      coded_components[c]->add_deltas(next_integer - previous_integers[c]);
      previous_integers[c] = next_integer;
    }
  }
}

void EncodeMeshTriangleIndex(const Mesh& mesh,
                             CodedNumericRun& triangle_indices) {
  const uint32_t triangle_count = mesh.TriangleCount();
  triangle_indices.mutable_deltas()->Clear();
  triangle_indices.mutable_deltas()->Reserve(triangle_count * 3);
  int prev_triangle_index = 0;
  for (size_t i = 0; i < triangle_count; ++i) {
    for (int j = 0; j < 3; ++j) {
      uint32_t triangle_index = mesh.TriangleIndices(i)[j];
      triangle_indices.add_deltas(triangle_index - prev_triangle_index);
      prev_triangle_index = triangle_index;
    }
  }
}

}  // namespace

void EncodeMeshOmittingFormat(const Mesh& mesh,
                              ink::proto::CodedMesh& coded_mesh) {
  coded_mesh.Clear();

  const uint32_t vertex_count = mesh.VertexCount();
  if (vertex_count == 0) {
    return;
  }

  const MeshFormat& format = mesh.Format();
  int total_component_count = format.TotalComponentCount();
  int non_position_component_count = total_component_count - 2;
  coded_mesh.mutable_other_attribute_components()->Reserve(
      non_position_component_count);
  absl::Span<const MeshFormat::Attribute> attributes = format.Attributes();
  for (size_t i = 0; i < attributes.size(); ++i) {
    if (MeshFormat::IsUnpackedType(attributes[i].type)) {
      EncodeUnpackedMeshAttribute(mesh, i, coded_mesh);
    } else {
      EncodePackedMeshAttribute(mesh, i, coded_mesh);
    }
  }

  EncodeMeshTriangleIndex(mesh, *coded_mesh.mutable_triangle_index());
}

void EncodeMesh(const Mesh& mesh, CodedMesh& coded_mesh) {
  EncodeMeshOmittingFormat(mesh, coded_mesh);
  EncodeMeshFormat(mesh.Format(), *coded_mesh.mutable_format());
}

absl::StatusOr<Mesh> DecodeMesh(const ink::proto::CodedMesh& coded_mesh) {
  absl::StatusOr<MeshFormat> format = MeshFormat();
  if (coded_mesh.has_format()) {
    // TODO: b/295166196 - `IndexFormat`s will be removed soon; until then, just
    // assume a default `IndexFormat` here.
    format =
        DecodeMeshFormat(coded_mesh.format(),
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  }
  if (!format.ok()) return format.status();

  return DecodeMeshUsingFormat(*format, coded_mesh);
}

absl::StatusOr<Mesh> DecodeMeshUsingFormat(
    const MeshFormat& format, const ink::proto::CodedMesh& coded_mesh) {
  int total_component_count = format.TotalComponentCount();
  int non_position_component_count = total_component_count - 2;
  if (coded_mesh.other_attribute_components_size() !=
      non_position_component_count) {
    return absl::InvalidArgumentError(
        absl::StrCat("MeshFormat has ", non_position_component_count,
                     " non-position attribute components, but CodedMesh has ",
                     coded_mesh.other_attribute_components_size(),
                     " other_attribute_components"));
  }

  std::vector<std::vector<float>> component_vectors;
  component_vectors.reserve(total_component_count);
  int non_position_component_index = 0;
  for (const MeshFormat::Attribute& attribute : format.Attributes()) {
    if (attribute.id == MeshFormat::AttributeId::kPosition) {
      ABSL_DCHECK_EQ(MeshFormat::ComponentCount(attribute.type), 2);

      absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
          x_stroke_space = DecodeFloatNumericRun(coded_mesh.x_stroke_space());
      if (!x_stroke_space.ok()) {
        return x_stroke_space.status();
      }
      component_vectors.push_back(
          std::vector<float>(x_stroke_space->begin(), x_stroke_space->end()));

      absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
          y_stroke_space = DecodeFloatNumericRun(coded_mesh.y_stroke_space());
      if (!y_stroke_space.ok()) {
        return y_stroke_space.status();
      }
      component_vectors.push_back(
          std::vector<float>(y_stroke_space->begin(), y_stroke_space->end()));
    } else {
      int component_count = MeshFormat::ComponentCount(attribute.type);
      for (int i = 0; i < component_count; ++i) {
        absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
            component =
                DecodeFloatNumericRun(coded_mesh.other_attribute_components(
                    non_position_component_index));
        if (!component.ok()) {
          return component.status();
        }
        component_vectors.push_back(
            std::vector<float>(component->begin(), component->end()));
        non_position_component_index += 1;
      }
    }
  }

  std::vector<absl::Span<const float>> component_spans;
  component_spans.reserve(component_vectors.size());
  for (const std::vector<float>& component_vector : component_vectors) {
    component_spans.push_back(component_vector);
  }

  absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>>
      triangle_range = DecodeIntNumericRun(coded_mesh.triangle_index());
  if (!triangle_range.ok()) {
    return triangle_range.status();
  }
  std::vector<uint32_t> triangle_indices(triangle_range->begin(),
                                         triangle_range->end());

  return ink::Mesh::Create(format, component_spans, triangle_indices);
}

}  // namespace ink
