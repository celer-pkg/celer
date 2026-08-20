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

#include "ink/geometry/partitioned_mesh.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/storage/mesh.h"
#include "ink/storage/mesh_format.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/partitioned_mesh.h"
#include "ink/storage/proto/coded_numeric_run.pb.h"
#include "ink/storage/proto/mesh.pb.h"

namespace ink {
namespace {

void EncodeOutline(absl::Span<const PartitionedMesh::VertexIndexPair> outline,
                   ink::proto::CodedNumericRun* outline_proto) {
  std::vector<uint32_t> outline_vector;
  outline_vector.reserve(outline.size());
  for (PartitionedMesh::VertexIndexPair pair : outline) {
    outline_vector.push_back((static_cast<uint32_t>(pair.mesh_index) << 16) |
                             static_cast<uint32_t>(pair.vertex_index));
  }
  EncodeIntNumericRun(outline_vector.begin(), outline_vector.end(),
                      outline_proto);
}

absl::StatusOr<std::vector<PartitionedMesh::VertexIndexPair>> DecodeOutline(
    const ink::proto::CodedNumericRun& outline_proto) {
  auto range = DecodeIntNumericRun(outline_proto);
  if (!range.ok()) return range.status();

  std::vector<PartitionedMesh::VertexIndexPair> outline;
  outline.reserve(outline_proto.deltas_size());
  for (uint32_t packed : *range) {
    outline.push_back(PartitionedMesh::VertexIndexPair{
        .mesh_index = static_cast<uint16_t>(packed >> 16),
        .vertex_index = static_cast<uint16_t>(packed & 0xffff),
    });
  }
  return std::move(outline);
}

// Decodes a CodedModeledShape proto using the deprecated schema (that is,
// specifying the `format` field for a single-group `PartitionedMesh`, rather
// than using the `group_*` fields).
absl::StatusOr<PartitionedMesh> DecodePartitionedMeshGroupless(
    const ink::proto::CodedModeledShape& shape_proto) {
  absl::StatusOr<MeshFormat> format = MeshFormat();

  std::vector<Mesh> meshes;
  meshes.reserve(shape_proto.meshes_size());
  for (const ink::proto::CodedMesh& coded_mesh : shape_proto.meshes()) {
    absl::StatusOr<Mesh> mesh = DecodeMeshUsingFormat(*format, coded_mesh);
    if (!mesh.ok()) return mesh.status();
    meshes.push_back(*std::move(mesh));
  }

  std::vector<std::vector<PartitionedMesh::VertexIndexPair>> outline_vectors;
  outline_vectors.reserve(shape_proto.outlines_size());
  for (const ink::proto::CodedNumericRun& outline_proto :
       shape_proto.outlines()) {
    absl::StatusOr<std::vector<PartitionedMesh::VertexIndexPair>> outline =
        DecodeOutline(outline_proto);
    if (!outline.ok()) return outline.status();
    outline_vectors.push_back(*std::move(outline));
  }

  std::vector<absl::Span<const PartitionedMesh::VertexIndexPair>> outline_spans;
  outline_spans.reserve(outline_vectors.size());
  for (const auto& outline : outline_vectors) {
    outline_spans.push_back(outline);
  }

  return PartitionedMesh::FromMeshes(absl::MakeSpan(meshes), outline_spans);
}

}  // namespace

void EncodePartitionedMesh(const PartitionedMesh& shape,
                           ink::proto::CodedModeledShape& shape_proto) {
  uint32_t num_groups = shape.RenderGroupCount();

  uint32_t total_meshes = 0;
  uint32_t total_outlines = 0;
  for (uint32_t group_index = 0; group_index < num_groups; ++group_index) {
    total_meshes += shape.RenderGroupMeshes(group_index).size();
    total_outlines += shape.OutlineCount(group_index);
  }

  shape_proto.Clear();
  shape_proto.mutable_meshes()->Reserve(total_meshes);
  shape_proto.mutable_outlines()->Reserve(total_outlines);
  shape_proto.mutable_group_formats()->Reserve(num_groups);
  shape_proto.mutable_group_first_mesh_indices()->Reserve(num_groups);
  shape_proto.mutable_group_first_outline_indices()->Reserve(num_groups);
  for (uint32_t group_index = 0; group_index < num_groups; ++group_index) {
    shape_proto.add_group_first_mesh_indices(shape_proto.meshes_size());
    shape_proto.add_group_first_outline_indices(shape_proto.outlines_size());
    EncodeMeshFormat(shape.RenderGroupFormat(group_index),
                     *shape_proto.add_group_formats());
    for (const Mesh& mesh : shape.RenderGroupMeshes(group_index)) {
      EncodeMeshOmittingFormat(mesh, *shape_proto.add_meshes());
    }
    const uint32_t num_outlines = shape.OutlineCount(group_index);
    for (uint32_t outline_index = 0; outline_index < num_outlines;
         ++outline_index) {
      EncodeOutline(shape.Outline(group_index, outline_index),
                    shape_proto.add_outlines());
    }
  }
}

absl::StatusOr<PartitionedMesh> DecodePartitionedMesh(
    const ink::proto::CodedModeledShape& shape_proto) {
  const int num_groups = shape_proto.group_formats_size();
  const int num_meshes = shape_proto.meshes_size();
  const int num_outlines = shape_proto.outlines_size();

  if (shape_proto.group_first_mesh_indices_size() != num_groups ||
      shape_proto.group_first_outline_indices_size() != num_groups) {
    return absl::InvalidArgumentError(
        "`CodedModeledShape.group_*` fields must all be the same size");
  }

  if (num_groups == 0) {
    if (num_meshes > 0) {
      // There are meshes, but no render groups, so use the deprecated `format`
      // field and put all meshes into a single render group with that format.
      return DecodePartitionedMeshGroupless(shape_proto);
    }
  } else {
    if (shape_proto.group_first_mesh_indices(0) != 0) {
      return absl::InvalidArgumentError(
          "`CodedModeledShape.group_first_mesh_indices` must start with zero");
    }
    if (shape_proto.group_first_outline_indices(0) != 0) {
      return absl::InvalidArgumentError(
          "`CodedModeledShape.group_first_outline_indices` must start with "
          "zero");
    }
    for (int i = 1; i < num_groups; ++i) {
      if (shape_proto.group_first_mesh_indices(i) >
          static_cast<uint32_t>(num_meshes)) {
        return absl::InvalidArgumentError(
            absl::StrCat("`CodedModeledShape.group_first_mesh_indices(", i,
                         ")` is ", shape_proto.group_first_mesh_indices(i),
                         ", but `meshes_size` is only ", num_meshes));
      }
      if (shape_proto.group_first_outline_indices(i) >
          static_cast<uint32_t>(num_outlines)) {
        return absl::InvalidArgumentError(
            absl::StrCat("`CodedModeledShape.group_first_outline_indices(", i,
                         ")` is ", shape_proto.group_first_outline_indices(i),
                         ", but `outlines_size` is only ", num_outlines));
      }
      if (shape_proto.group_first_mesh_indices(i) <
          shape_proto.group_first_mesh_indices(i - 1)) {
        return absl::InvalidArgumentError(
            "`CodedModeledShape.group_first_mesh_indices` must be "
            "monotonically nondecreasing");
      }
      if (shape_proto.group_first_outline_indices(i) <
          shape_proto.group_first_outline_indices(i - 1)) {
        return absl::InvalidArgumentError(
            "`CodedModeledShape.group_first_outline_indices` must be "
            "monotonically nondecreasing");
      }
    }
  }

  std::vector<PartitionedMesh::MeshGroup> groups;
  groups.reserve(num_groups);
  std::vector<Mesh> meshes;
  meshes.reserve(num_meshes);
  std::vector<std::vector<PartitionedMesh::VertexIndexPair>> outlines;
  outlines.reserve(num_outlines);
  std::vector<absl::Span<const PartitionedMesh::VertexIndexPair>> outline_spans;
  outline_spans.reserve(num_outlines);

  for (int group_index = 0; group_index < num_groups; ++group_index) {
    absl::StatusOr<MeshFormat> format =
        DecodeMeshFormat(shape_proto.group_formats(group_index),
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
    if (!format.ok()) return format.status();

    const int next_group_index = group_index + 1;
    const bool is_last_group = next_group_index == num_groups;

    const uint32_t mesh_index_start = meshes.size();
    const uint32_t mesh_index_end =
        is_last_group ? num_meshes
                      : shape_proto.group_first_mesh_indices(next_group_index);
    for (uint32_t mesh_index = mesh_index_start; mesh_index < mesh_index_end;
         ++mesh_index) {
      absl::StatusOr<Mesh> mesh =
          DecodeMeshUsingFormat(*format, shape_proto.meshes(mesh_index));
      if (!mesh.ok()) return mesh.status();
      meshes.push_back(*std::move(mesh));
    }

    const uint32_t outline_index_start = outlines.size();
    const uint32_t outline_index_end =
        is_last_group
            ? num_outlines
            : shape_proto.group_first_outline_indices(next_group_index);
    for (uint32_t outline_index = outline_index_start;
         outline_index < outline_index_end; ++outline_index) {
      absl::StatusOr<std::vector<PartitionedMesh::VertexIndexPair>> outline =
          DecodeOutline(shape_proto.outlines(outline_index));
      if (!outline.ok()) return outline.status();
      outlines.push_back(*std::move(outline));
      outline_spans.push_back(absl::MakeConstSpan(outlines.back()));
    }

    groups.push_back(PartitionedMesh::MeshGroup{
        .meshes = absl::MakeConstSpan(meshes).subspan(mesh_index_start),
        .outlines =
            absl::MakeConstSpan(outline_spans).subspan(outline_index_start),
    });
  }

  return PartitionedMesh::FromMeshGroups(absl::MakeConstSpan(groups));
}

}  // namespace ink
