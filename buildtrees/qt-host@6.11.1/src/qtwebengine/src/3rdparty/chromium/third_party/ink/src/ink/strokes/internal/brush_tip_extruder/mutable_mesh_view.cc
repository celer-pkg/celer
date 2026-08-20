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

#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <variant>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {

using ::ink::strokes_internal::StrokeVertex;

MutableMeshView::MutableMeshView(MutableMesh& mesh) : data_(&mesh) {
  ABSL_CHECK(MeshFormat::IsUnpackedEquivalent(mesh.Format(),
                                              StrokeVertex::FullMeshFormat()));
  ResetMutationTracking();
}

MutableMeshView::MutableMeshView(
    std::vector<strokes_internal::LegacyVertex>& vertices,
    std::vector<uint32_t>& indices)
    : data_(LegacyVectors{.vertices = &vertices, .indices = &indices}) {
  ResetMutationTracking();
}

uint32_t MutableMeshView::VertexCount() const {
  ABSL_CHECK(HasMeshData());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    ABSL_DCHECK_LE(legacy->vertices->size(),
                   std::numeric_limits<uint32_t>::max());
    return legacy->vertices->size();
  }
  return std::get<MutableMesh*>(data_)->VertexCount();
}

uint32_t MutableMeshView::TriangleCount() const {
  ABSL_CHECK(HasMeshData());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    ABSL_DCHECK_EQ(legacy->indices->size() % 3, 0);
    ABSL_DCHECK_LE(legacy->indices->size() / 3,
                   std::numeric_limits<uint32_t>::max());
    return legacy->indices->size() / 3;
  }
  return std::get<MutableMesh*>(data_)->TriangleCount();
}

Point MutableMeshView::GetPosition(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    return (*legacy->vertices)[index].position;
  }
  return std::get<MutableMesh*>(data_)->VertexPosition(index);
}

ExtrudedVertex MutableMeshView::GetVertex(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    return ExtrudedVertex::FromLegacy((*legacy->vertices)[index]);
  }

  StrokeVertex vertex =
      StrokeVertex::GetFromMesh(*std::get<MutableMesh*>(data_), index);
  return {.position = vertex.position,
          .new_non_position_attributes = vertex.non_position_attributes};
}

Vec MutableMeshView::GetSideDerivative(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());

  if (const MutableMesh* const* mesh = std::get_if<MutableMesh*>(&data_)) {
    return StrokeVertex::GetSideDerivativeFromMesh(**mesh, index);
  }
  return {0, 0};
}

Vec MutableMeshView::GetForwardDerivative(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());

  if (const MutableMesh* const* mesh = std::get_if<MutableMesh*>(&data_)) {
    return StrokeVertex::GetForwardDerivativeFromMesh(**mesh, index);
  }
  return {0, 0};
}

StrokeVertex::Label MutableMeshView::GetSideLabel(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());

  if (const MutableMesh* const* mesh = std::get_if<MutableMesh*>(&data_)) {
    return StrokeVertex::GetSideLabelFromMesh(**mesh, index);
  }
  return StrokeVertex::kInteriorLabel;
}

StrokeVertex::Label MutableMeshView::GetForwardLabel(uint32_t index) const {
  ABSL_CHECK_LT(index, VertexCount());

  if (const MutableMesh* const* mesh = std::get_if<MutableMesh*>(&data_)) {
    return StrokeVertex::GetForwardLabelFromMesh(**mesh, index);
  }
  return StrokeVertex::kInteriorLabel;
}

Triangle MutableMeshView::GetTriangle(uint32_t triangle) const {
  ABSL_CHECK_LT(triangle, TriangleCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    std::array<uint32_t, 3> indices = GetTriangleIndices(triangle);
    return {.p0 = GetPosition(indices[0]),
            .p1 = GetPosition(indices[1]),
            .p2 = GetPosition(indices[2])};
  }

  return std::get<MutableMesh*>(data_)->GetTriangle(triangle);
}

std::array<uint32_t, 3> MutableMeshView::GetTriangleIndices(
    uint32_t triangle) const {
  ABSL_CHECK_LT(triangle, TriangleCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    uint32_t* index_data = legacy->indices->data() + 3 * triangle;
    return {index_data[0], index_data[1], index_data[2]};
  }

  return std::get<MutableMesh*>(data_)->TriangleIndices(triangle);
}

uint32_t MutableMeshView::GetVertexIndex(uint32_t triangle,
                                         uint32_t triangle_vertex) const {
  ABSL_CHECK_LT(triangle, TriangleCount());
  ABSL_CHECK_LT(triangle_vertex, 3u);
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    return (*legacy->indices)[3 * triangle + triangle_vertex];
  }

  return std::get<MutableMesh*>(data_)->TriangleIndices(
      triangle)[triangle_vertex];
}

void MutableMeshView::AppendVertex(const ExtrudedVertex& vertex) {
  ABSL_CHECK(HasMeshData());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    legacy->vertices->push_back(vertex.ToLegacy());
  } else {
    StrokeVertex::AppendToMesh(
        *std::get<MutableMesh*>(data_),
        {.position = vertex.position,
         .non_position_attributes = vertex.new_non_position_attributes});
  }
}

void MutableMeshView::AppendTriangleIndices(
    const std::array<uint32_t, 3>& indices) {
  ABSL_CHECK(HasMeshData());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    legacy->indices->insert(legacy->indices->end(),
                            {indices[0], indices[1], indices[2]});
  } else {
    std::get<MutableMesh*>(data_)->AppendTriangleIndices(indices);
  }
}

void MutableMeshView::SetVertex(uint32_t index, const ExtrudedVertex& vertex) {
  ABSL_CHECK_LT(index, VertexCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    (*legacy->vertices)[index] = vertex.ToLegacy();
  } else {
    StrokeVertex::SetInMesh(
        *std::get<MutableMesh*>(data_), index,
        {.position = vertex.position,
         .non_position_attributes = vertex.new_non_position_attributes});
  }
  first_mutated_vertex_ = std::min(first_mutated_vertex_, index);
}

void MutableMeshView::SetSideDerivative(uint32_t index, Vec derivative) {
  ABSL_CHECK_LT(index, VertexCount());
  if (std::holds_alternative<LegacyVectors>(data_)) return;

  StrokeVertex::SetSideDerivativeInMesh(*std::get<MutableMesh*>(data_), index,
                                        derivative);
  first_mutated_vertex_ = std::min(first_mutated_vertex_, index);
}

void MutableMeshView::SetForwardDerivative(uint32_t index, Vec derivative) {
  ABSL_CHECK_LT(index, VertexCount());
  if (std::holds_alternative<LegacyVectors>(data_)) return;

  StrokeVertex::SetForwardDerivativeInMesh(*std::get<MutableMesh*>(data_),
                                           index, derivative);
  first_mutated_vertex_ = std::min(first_mutated_vertex_, index);
}

void MutableMeshView::SetSideLabel(
    uint32_t index, strokes_internal::StrokeVertex::Label label) {
  ABSL_CHECK_LT(index, VertexCount());
  if (std::holds_alternative<LegacyVectors>(data_)) return;

  StrokeVertex::SetSideLabelInMesh(*std::get<MutableMesh*>(data_), index,
                                   label);
  first_mutated_vertex_ = std::min(first_mutated_vertex_, index);
}

void MutableMeshView::SetForwardLabel(
    uint32_t index, strokes_internal::StrokeVertex::Label label) {
  ABSL_CHECK_LT(index, VertexCount());
  if (std::holds_alternative<LegacyVectors>(data_)) return;

  StrokeVertex::SetForwardLabelInMesh(*std::get<MutableMesh*>(data_), index,
                                      label);
  first_mutated_vertex_ = std::min(first_mutated_vertex_, index);
}

void MutableMeshView::SetTriangleIndices(
    uint32_t triangle, const std::array<uint32_t, 3>& indices) {
  ABSL_CHECK_LT(triangle, TriangleCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    uint32_t* index_data = legacy->indices->data() + 3 * triangle;
    index_data[0] = indices[0];
    index_data[1] = indices[1];
    index_data[2] = indices[2];
  } else {
    std::get<MutableMesh*>(data_)->SetTriangleIndices(triangle, indices);
  }
  first_mutated_triangle_ = std::min(first_mutated_triangle_, triangle);
}

void MutableMeshView::InsertTriangleIndices(
    uint32_t triangle, const std::array<uint32_t, 3>& indices) {
  ABSL_CHECK_LE(triangle, TriangleCount());
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    auto iter = legacy->indices->begin() + 3 * triangle;
    legacy->indices->insert(iter, {indices[0], indices[1], indices[2]});
  } else {
    std::get<MutableMesh*>(data_)->InsertTriangleIndices(
        triangle, {indices[0], indices[1], indices[2]});
  }
  first_mutated_triangle_ = std::min(first_mutated_triangle_, triangle);
}

void MutableMeshView::Resize(uint32_t new_vertex_count,
                             uint32_t new_triangle_count) {
  if (const auto* legacy = std::get_if<LegacyVectors>(&data_)) {
    legacy->vertices->resize(new_vertex_count);
    legacy->indices->resize(3 * new_triangle_count);
  } else {
    std::get<MutableMesh*>(data_)->Resize(new_vertex_count, new_triangle_count);
  }

  first_mutated_vertex_ = std::min(first_mutated_vertex_, new_vertex_count);
  first_mutated_triangle_ =
      std::min(first_mutated_triangle_, new_triangle_count);
}

void MutableMeshView::ResetMutationTracking() {
  first_mutated_vertex_ = VertexCount();
  first_mutated_triangle_ = TriangleCount();
}

}  // namespace ink::brush_tip_extruder_internal
