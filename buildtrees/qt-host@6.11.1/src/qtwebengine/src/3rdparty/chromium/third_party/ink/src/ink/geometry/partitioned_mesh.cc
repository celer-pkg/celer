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

#include "ink/geometry/partitioned_mesh.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "absl/base/nullability.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "absl/functional/function_ref.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/intersects_internal.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/internal/static_rtree.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {

// Convenience alias for the R-Tree.
using RTree =
    geometry_internal::StaticRTree<PartitionedMesh::TriangleIndexPair>;

PartitionedMesh PartitionedMesh::WithEmptyGroups(uint32_t num_groups) {
  return *PartitionedMesh::FromMeshGroups(std::vector<MeshGroup>(num_groups));
}

absl::StatusOr<PartitionedMesh> PartitionedMesh::FromMutableMesh(
    const MutableMesh& mesh,
    absl::Span<const absl::Span<const uint32_t>> outlines,
    absl::Span<const MeshFormat::AttributeId> omit_attributes,
    absl::Span<const std::optional<MeshAttributeCodingParams>> packing_params) {
  MutableMeshGroup group = {
      .mesh = &mesh,
      .outlines = outlines,
      .omit_attributes = omit_attributes,
      .packing_params = packing_params,
  };
  return PartitionedMesh::FromMutableMeshGroups(absl::MakeConstSpan(&group, 1));
}

absl::StatusOr<PartitionedMesh> PartitionedMesh::FromMutableMeshGroups(
    absl::Span<const MutableMeshGroup> groups) {
  std::vector<std::vector<std::vector<VertexIndexPair>>>
      all_partitioned_outlines;
  all_partitioned_outlines.reserve(groups.size());

  std::vector<std::vector<absl::Span<const VertexIndexPair>>>
      all_partitioned_outline_spans;
  all_partitioned_outline_spans.reserve(groups.size());

  std::vector<absl::InlinedVector<Mesh, 1>> all_meshes;
  all_meshes.reserve(groups.size());

  std::vector<MeshGroup> mesh_groups;
  mesh_groups.reserve(groups.size());

  for (const MutableMeshGroup& group : groups) {
    const MutableMesh& mesh = *group.mesh;
    absl::Span<const absl::Span<const uint32_t>> outlines = group.outlines;

    all_partitioned_outlines.emplace_back();
    std::vector<std::vector<VertexIndexPair>>& group_partitioned_outlines =
        all_partitioned_outlines.back();

    uint32_t n_vertices = mesh.VertexCount();
    for (uint32_t o_idx = 0; o_idx < outlines.size(); ++o_idx) {
      for (uint32_t v_idx = 0; v_idx < outlines[o_idx].size(); ++v_idx) {
        if (outlines[o_idx][v_idx] >= n_vertices) {
          return absl::InvalidArgumentError(absl::Substitute(
              "Vertex $0 in outline $1 refers to non-existent "
              "vertex $2 (vertices: $3)",
              v_idx, o_idx, outlines[o_idx][v_idx], n_vertices));
        }
      }
    }

    absl::StatusOr<absl::InlinedVector<Mesh, 1>> group_meshes =
        mesh.AsMeshes(group.packing_params, group.omit_attributes);
    if (!group_meshes.ok()) {
      return group_meshes.status();
    }
    all_meshes.push_back(*std::move(group_meshes));

    // There's a bit of performance that we're leaving on the table here:
    // - we're computing the partitions twice (once here, once in
    //   `MutableMesh::AsMeshes`)
    // - we're copying the `VertexIndexPair`s into `FromMeshes`, when we could
    //   be moving them
    // - we're constructing the partition map even when everything fits in one
    //   partition
    // However, this code is already destined for the trash bin: once
    // `MutableMesh` is changed to always use 16-bit indices (b/295166196),
    // there will be no need to do partitioning, and this code can be deleted.
    if (!outlines.empty()) {
      constexpr uint32_t kMaxVerticesPerPartition = 1 << (8 * sizeof(uint16_t));
      absl::InlinedVector<mesh_internal::PartitionInfo, 1> partitions =
          mesh_internal::PartitionTriangles(mesh.RawIndexData(),
                                            mesh.Format().GetIndexFormat(),
                                            kMaxVerticesPerPartition);
      absl::flat_hash_map<uint32_t, VertexIndexPair> partition_map;
      partition_map.reserve(mesh.VertexCount());
      for (size_t p_idx = 0; p_idx < partitions.size(); ++p_idx) {
        const std::vector<uint32_t>& vertex_indices =
            partitions[p_idx].vertex_indices;
        for (size_t v_idx = 0; v_idx < vertex_indices.size(); ++v_idx) {
          // This insertion may fail, since some vertices will exist in multiple
          // partitions, but that's fine -- this just causes the outline to use
          // the first vertex it finds.
          partition_map.insert(
              {vertex_indices[v_idx],
               {.mesh_index = static_cast<uint16_t>(p_idx),
                .vertex_index = static_cast<uint16_t>(v_idx)}});
        }
      }

      for (size_t o_idx = 0; o_idx < outlines.size(); ++o_idx) {
        if (outlines[o_idx].empty()) continue;
        group_partitioned_outlines.emplace_back();
        std::vector<VertexIndexPair>& outline_index_pairs =
            group_partitioned_outlines.back();
        outline_index_pairs.reserve(outlines[o_idx].size());
        for (uint32_t index : outlines[o_idx]) {
          auto it = partition_map.find(index);
          if (it != partition_map.end()) {
            outline_index_pairs.push_back(it->second);
          }
        }
      }
    }

    std::vector<absl::Span<const VertexIndexPair>>
        group_partitioned_outline_spans(group_partitioned_outlines.size());
    for (size_t o_idx = 0; o_idx < group_partitioned_outlines.size(); ++o_idx) {
      group_partitioned_outline_spans[o_idx] =
          absl::MakeConstSpan(group_partitioned_outlines[o_idx]);
    }
    all_partitioned_outline_spans.push_back(
        std::move(group_partitioned_outline_spans));

    mesh_groups.push_back(MeshGroup{
        .meshes = absl::MakeConstSpan(all_meshes.back()),
        .outlines = absl::MakeConstSpan(all_partitioned_outline_spans.back()),
    });
  }
  return FromMeshGroups(absl::MakeConstSpan(mesh_groups));
}

absl::StatusOr<PartitionedMesh> PartitionedMesh::FromMeshes(
    absl::Span<const Mesh> meshes,
    absl::Span<const absl::Span<const VertexIndexPair>> outlines) {
  MeshGroup group = {.meshes = meshes, .outlines = outlines};
  return PartitionedMesh::FromMeshGroups(absl::MakeConstSpan(&group, 1));
}

absl::StatusOr<PartitionedMesh> PartitionedMesh::FromMeshGroups(
    absl::Span<const MeshGroup> groups) {
  absl::StatusOr<std::unique_ptr<Data>> data = Data::FromMeshGroups(groups);
  if (!data.ok()) return data.status();
  return PartitionedMesh(*std::move(data));
}

absl::StatusOr<absl_nonnull std::unique_ptr<PartitionedMesh::Data>>
PartitionedMesh::Data::FromMeshGroups(absl::Span<const MeshGroup> groups) {
  size_t total_meshes = 0;
  size_t total_outlines = 0;
  for (const MeshGroup& group : groups) {
    total_meshes += group.meshes.size();
    total_outlines += group.outlines.size();
  }
  if (total_meshes > std::numeric_limits<uint16_t>::max()) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Too many meshes; PartitionedMesh supports a maximum of "
        "2^16 (65536) meshes ($0 meshes given)",
        total_meshes));
  }

  absl::InlinedVector<MeshFormat, 1> group_formats;
  group_formats.reserve(groups.size());
  for (const MeshGroup& group : groups) {
    if (group.meshes.empty()) {
      group_formats.push_back(MeshFormat());
    } else {
      const MeshFormat& format = group.meshes.front().Format();
      for (size_t i = 1; i < group.meshes.size(); ++i) {
        if (group.meshes[i].Format() != format) {
          return absl::InvalidArgumentError(
              absl::Substitute("Every mesh must have the same format. "
                               "meshes[0]->Format() is $0, and "
                               "meshes[$1]->Format() is $2",
                               format, i, group.meshes[i].Format()));
        }
      }
      group_formats.push_back(format);
    }

    for (size_t i = 0; i < group.meshes.size(); ++i) {
      if (group.meshes[i].TriangleCount() == 0) {
        return absl::InvalidArgumentError(
            absl::Substitute("Mesh at index $0 contains no triangles", i));
      }
    }
    for (uint32_t o_idx = 0; o_idx < group.outlines.size(); ++o_idx) {
      if (group.outlines[o_idx].empty()) {
        return absl::InvalidArgumentError(
            absl::Substitute("Outline at index $0 contains no points", o_idx));
      }
      for (uint32_t v_idx = 0; v_idx < group.outlines[o_idx].size(); ++v_idx) {
        if (group.outlines[o_idx][v_idx].mesh_index >= group.meshes.size()) {
          return absl::InvalidArgumentError(absl::Substitute(
              "Vertex $0 in outline $1 refers to non-existent mesh $2 (meshes: "
              "$3)",
              v_idx, o_idx, group.outlines[o_idx][v_idx].mesh_index,
              group.meshes.size()));
        }
        if (group.outlines[o_idx][v_idx].vertex_index >=
            group.meshes[group.outlines[o_idx][v_idx].mesh_index]
                .VertexCount()) {
          return absl::InvalidArgumentError(absl::Substitute(
              "Vertex $0 in outline $1 refers to non-existent vertex $2 in "
              "mesh "
              "$3 (vertices: $4)",
              v_idx, o_idx, group.outlines[o_idx][v_idx].vertex_index,
              group.outlines[o_idx][v_idx].mesh_index,
              group.meshes[group.outlines[o_idx][v_idx].mesh_index]
                  .VertexCount()));
        }
      }
    }
  }

  auto data = std::make_unique<PartitionedMesh::Data>();
  data->meshes_.reserve(total_meshes);
  data->outlines_.reserve(total_outlines);
  data->group_first_mesh_indices_.reserve(groups.size());
  data->group_first_outline_indices_.reserve(groups.size());
  data->group_formats_ = std::move(group_formats);

  for (const MeshGroup& group : groups) {
    uint16_t group_first_mesh_index = data->meshes_.size();
    data->group_first_mesh_indices_.push_back(group_first_mesh_index);
    data->meshes_.insert(data->meshes_.end(), group.meshes.begin(),
                         group.meshes.end());
    uint32_t group_first_outline_index = data->outlines_.size();
    data->group_first_outline_indices_.push_back(group_first_outline_index);
    for (absl::Span<const VertexIndexPair> outline : group.outlines) {
      data->outlines_.push_back(
          std::vector<VertexIndexPair>(outline.begin(), outline.end()));
    }
  }

  return std::move(data);
}

Envelope PartitionedMesh::Bounds() const {
  if (data_ == nullptr) return {};
  Envelope bounds;
  for (size_t i = 0; i < data_->Meshes().size(); ++i) {
    bounds.Add(data_->Meshes()[i].Bounds());
  }
  return bounds;
}

namespace {

// This is a helper function for `VisitIntersectedTriangles` that handles the
// type-independent logic.
template <typename QueryType>
void VisitIntersectedTrianglesHelper(
    const QueryType& query,
    absl::FunctionRef<
        PartitionedMesh::FlowControl(PartitionedMesh::TriangleIndexPair)>
        visitor,
    const AffineTransform& query_to_this, absl::Span<const Mesh> meshes,
    const RTree& rtree) {
  // This is an `auto` instead of `QueryType` because the `Rect` overload of
  // `AffineTransform::Apply` returns a `Quad`, not a `Rect`.
  auto transformed_query = query_to_this.Apply(query);
  auto visitor_wrapper = [transformed_query, visitor,
                          &meshes](PartitionedMesh::TriangleIndexPair index) {
    if (!geometry_internal::IntersectsInternal(
            transformed_query,
            meshes[index.mesh_index].GetTriangle(index.triangle_index))) {
      return true;
    }
    return visitor(index) == PartitionedMesh::FlowControl::kContinue;
  };
  rtree.VisitIntersectedElements(*Envelope(transformed_query).AsRect(),
                                 visitor_wrapper);
}

}  // namespace

void PartitionedMesh::VisitIntersectedTriangles(
    Point query, absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  VisitIntersectedTrianglesHelper(query, visitor, query_to_this,
                                  data_->Meshes(), data_->SpatialIndex());
}

void PartitionedMesh::VisitIntersectedTriangles(
    const Segment& query,
    absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  VisitIntersectedTrianglesHelper(query, visitor, query_to_this,
                                  data_->Meshes(), data_->SpatialIndex());
}

void PartitionedMesh::VisitIntersectedTriangles(
    const Triangle& query,
    absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  VisitIntersectedTrianglesHelper(query, visitor, query_to_this,
                                  data_->Meshes(), data_->SpatialIndex());
}

void PartitionedMesh::VisitIntersectedTriangles(
    const Rect& query,
    absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  VisitIntersectedTrianglesHelper(query, visitor, query_to_this,
                                  data_->Meshes(), data_->SpatialIndex());
}

void PartitionedMesh::VisitIntersectedTriangles(
    const Quad& query,
    absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  VisitIntersectedTrianglesHelper(query, visitor, query_to_this,
                                  data_->Meshes(), data_->SpatialIndex());
}

namespace {

// This is a helper function for the `PartitionedMesh` overload of
// `VisitIntersectedTriangles`, that handles the case in which the given
// transform is invertible.
void VisitIntersectedTrianglesWithPartitionedMeshWithInvertibleTransform(
    absl::Span<const Mesh> meshes, const RTree& rtree,
    const PartitionedMesh& query, const AffineTransform& query_to_target,
    const AffineTransform& target_to_query,
    absl::FunctionRef<
        PartitionedMesh::FlowControl(PartitionedMesh::TriangleIndexPair)>
        visitor) {
  // To find the intersected triangles, we'll first find all triangles that
  // intersect the bounds of `query`, and then test those triangles to see if
  // they actually intersect `query` itself and not just its bounds.
  Rect query_bounds =
      *Envelope(query_to_target.Apply(*query.Bounds().AsRect())).AsRect();

  auto visitor_wrapper = [&meshes, &query, &target_to_query,
                          visitor](PartitionedMesh::TriangleIndexPair index) {
    // This triangle hits the bounding box of `query`, now we need to check
    // that it actually hits `query` itself. Note that we can't call
    // `IntersectsInternal` here, because it would result in a circular
    // dependency, so we use the `Triangle` overload of
    // `VisitIntersectedTriangles` instead.
    bool found_intersection = false;
    query.VisitIntersectedTriangles(
        meshes[index.mesh_index].GetTriangle(index.triangle_index),

        [&found_intersection](PartitionedMesh::TriangleIndexPair query_index) {
          found_intersection = true;
          return PartitionedMesh::FlowControl::kBreak;
        },
        target_to_query);

    if (found_intersection) {
      return visitor(index) == PartitionedMesh::FlowControl::kContinue;
    }
    return true;
  };

  rtree.VisitIntersectedElements(query_bounds, visitor_wrapper);
}

}  // namespace

void PartitionedMesh::VisitIntersectedTriangles(
    const PartitionedMesh& query,
    absl::FunctionRef<FlowControl(TriangleIndexPair)> visitor,
    const AffineTransform& query_to_this) const {
  if (!data_) return;

  // If `query` is empty, it can't intersect this shape.
  if (query.Meshes().empty()) return;

  // `PartitionedMesh` intersection is a little bit more complicated, so we
  // can't just use `VisitIntersectedTrianglesHelper` here.

  // First, we need to try to get the inverse of `this_to_query`, since the
  // approach is different depending on whether the transform is invertible.
  std::optional<AffineTransform> this_to_query = query_to_this.Inverse();
  if (this_to_query.has_value()) {
    VisitIntersectedTrianglesWithPartitionedMeshWithInvertibleTransform(
        data_->Meshes(), data_->SpatialIndex(), query, query_to_this,
        *this_to_query, visitor);
  } else {
    // Since `query_to_this` is not invertible, it must collapse `query` to
    // either a segment or a point.
    Segment collapsed_query = geometry_internal::CalculateCollapsedSegment(
        query.Meshes(), *query.Bounds().AsRect(), query_to_this);

    VisitIntersectedTriangles(collapsed_query, visitor);
  }
}

namespace {

// This is a helper function for `Coverage` that contains the type-independent
// logic for computing the proportion of the area covered by the query.
template <typename QueryType>
float ComputeCoverage(const QueryType& query,
                      const AffineTransform& query_to_target,
                      const PartitionedMesh& target,
                      float total_absolute_area) {
  float covered_area = 0;
  target.VisitIntersectedTriangles(
      query,
      [&target, &covered_area](const PartitionedMesh::TriangleIndexPair index) {
        covered_area += std::abs(target.Meshes()[index.mesh_index]
                                     .GetTriangle(index.triangle_index)
                                     .SignedArea());
        return PartitionedMesh::FlowControl::kContinue;
      },
      query_to_target);
  return covered_area / total_absolute_area;
}

}  // namespace

float PartitionedMesh::Coverage(const Triangle& query,
                                const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return 0;
  return ComputeCoverage(query, query_to_this, *this,
                         data_->TotalAbsoluteArea());
}

float PartitionedMesh::Coverage(const Rect& query,
                                const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return 0;
  return ComputeCoverage(query, query_to_this, *this,
                         data_->TotalAbsoluteArea());
}

float PartitionedMesh::Coverage(const Quad& query,
                                const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return 0;
  return ComputeCoverage(query, query_to_this, *this,
                         data_->TotalAbsoluteArea());
}

float PartitionedMesh::Coverage(const PartitionedMesh& query,
                                const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return 0;
  return ComputeCoverage(query, query_to_this, *this,
                         data_->TotalAbsoluteArea());
}

namespace {

// This is a helper function for `CoverageIsGreaterThan` that contains the
// type-independent logic for computing the area covered by the query.
template <typename QueryType>
float CoverageIsGreaterThanHelper(const QueryType& query,
                                  const AffineTransform& query_to_target,
                                  const PartitionedMesh& target,
                                  float coverage_threshold,
                                  float total_absolute_area) {
  float area_threshold = coverage_threshold * total_absolute_area;
  float covered_area = 0;
  target.VisitIntersectedTriangles(
      query,
      [&target, &covered_area,
       area_threshold](const PartitionedMesh::TriangleIndexPair index) {
        covered_area += std::abs(target.Meshes()[index.mesh_index]
                                     .GetTriangle(index.triangle_index)
                                     .SignedArea());
        return covered_area > area_threshold
                   ? PartitionedMesh::FlowControl::kBreak
                   : PartitionedMesh::FlowControl::kContinue;
      },
      query_to_target);
  return covered_area > area_threshold;
}

}  // namespace

bool PartitionedMesh::CoverageIsGreaterThan(
    const Triangle& query, float coverage_threshold,
    const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return false;
  return CoverageIsGreaterThanHelper(query, query_to_this, *this,
                                     coverage_threshold,
                                     data_->TotalAbsoluteArea());
}

bool PartitionedMesh::CoverageIsGreaterThan(
    const Rect& query, float coverage_threshold,
    const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return false;
  return CoverageIsGreaterThanHelper(query, query_to_this, *this,
                                     coverage_threshold,
                                     data_->TotalAbsoluteArea());
}

bool PartitionedMesh::CoverageIsGreaterThan(
    const Quad& query, float coverage_threshold,
    const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return false;
  return CoverageIsGreaterThanHelper(query, query_to_this, *this,
                                     coverage_threshold,
                                     data_->TotalAbsoluteArea());
}

bool PartitionedMesh::CoverageIsGreaterThan(
    const PartitionedMesh& query, float coverage_threshold,
    const AffineTransform& query_to_this) const {
  if (data_ == nullptr) return false;
  return CoverageIsGreaterThanHelper(query, query_to_this, *this,
                                     coverage_threshold,
                                     data_->TotalAbsoluteArea());
}

const RTree& PartitionedMesh::Data::SpatialIndex() const {
  ABSL_CHECK(!meshes_.empty());

  absl::MutexLock lock(&cache_mutex_);

  // The index is already initialized, there's nothing to do.
  // NOMUTANTS -- Removing this would not have an observable effect on behavior
  // (just performance), since recomputating the index would yield the same
  // result.
  if (rtree_ != nullptr) return *rtree_;

  uint32_t n_tris = 0;
  for (const Mesh& mesh : meshes_) n_tris += mesh.TriangleCount();

  // This generates each valid `TriangleIndexPair` for this `PartitionedMesh`,
  // in order of mesh index, then triangle index.
  //
  // We want to start at mesh_index = 0, triangle_index = 0, but since they're
  // `uint16_t`s, we have to make a copy of the current index, then increment
  // the current value, then return the copy (analogous to the post-increment
  // operator).
  auto triangle_index_pair_generator =
      [this, current_index = TriangleIndexPair{.mesh_index = 0,
                                               .triangle_index = 0}]() mutable {
        TriangleIndexPair value_before_increment = current_index;
        ++current_index.triangle_index;
        if (current_index.triangle_index >=
            meshes_[current_index.mesh_index].TriangleCount()) {
          ++current_index.mesh_index;
          current_index.triangle_index = 0;
        }
        return value_before_increment;
      };
  // This gets the bounds for a `TriangleIndexPair` by looking up the triangle
  // in this `PartitionedMesh`'s meshes.
  auto bounds_func = [&meshes = meshes_](TriangleIndexPair idx) {
    return *Envelope(meshes[idx.mesh_index].GetTriangle(idx.triangle_index))
                .AsRect();
  };
  rtree_ = std::make_unique<RTree>(n_tris, triangle_index_pair_generator,
                                   bounds_func);

  return *rtree_;
}

float PartitionedMesh::Data::TotalAbsoluteArea() const {
  ABSL_CHECK(!meshes_.empty());

  absl::MutexLock lock(&cache_mutex_);
  if (!cached_total_absolute_area_.has_value()) {
    float total_abs_area = 0;
    for (const Mesh& mesh : meshes_) {
      for (uint32_t i = 0; i < mesh.TriangleCount(); ++i) {
        total_abs_area += std::abs(mesh.GetTriangle(i).SignedArea());
      }
    }
    cached_total_absolute_area_ = total_abs_area;
  }
  return *cached_total_absolute_area_;
}

}  // namespace ink
