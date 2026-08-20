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

#ifndef INK_GEOMETRY_MUTABLE_MESH_H_
#define INK_GEOMETRY_MUTABLE_MESH_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/types/small_array.h"

namespace ink {

// A readable and writable mesh made up of vertices and triangles. Vertices have
// attributes, as specified by the `MeshFormat`. A position attribute is
// required, and there may optionally be additional non-geometric attributes,
// e.g. texture coordinates. Unlike `Mesh`, the attributes are stored at full
// precision, never packed; see `MeshFormat` for details on attribute packing.
class MutableMesh {
 public:
  // Constructs an empty mesh with a default-constructed `MeshFormat`.
  MutableMesh() = default;

  // Constructs an empty mesh with the given format.
  explicit MutableMesh(const MeshFormat& format) : format_(format) {
    // Consistency check, it should be impossible for this to fail.
    ABSL_CHECK(format_.UnpackedIndexStride() == 2 ||
               format_.UnpackedIndexStride() == 4);
  }

  // Constructs a `MutableMesh` from a `Mesh`, copying (and unpacking)
  // the vertex and triangle data.
  static MutableMesh FromMesh(const Mesh& mesh);

  // `MutableMesh` does not define a copy constructor or copy assignment
  // operator, in order to prevent expensive accidental copies. You may make a
  // copy of a `MutableMesh` with the `Clone` method below.
  MutableMesh(const MutableMesh&) = delete;
  MutableMesh(MutableMesh&&) = default;
  MutableMesh& operator=(const MutableMesh&) = delete;
  MutableMesh& operator=(MutableMesh&&) = default;

  // Returns a copy of this `MutableMesh`.
  MutableMesh Clone() const;

  // Clears all of the vertices and triangles in the mesh. This does not
  // deallocate the memory used by the vertex and triangle buffers.
  void Clear() {
    vertex_data_.clear();
    index_data_.clear();
    vertex_count_ = 0;
    triangle_count_ = 0;
  }

  // Clears the mesh, as per `Clear`(), and resets the mesh format to
  // the given one.
  void Reset(MeshFormat format) {
    // Consistency check, it should be impossible for this to fail.
    ABSL_CHECK(format.UnpackedIndexStride() == 2 ||
               format.UnpackedIndexStride() == 4);
    Clear();
    format_ = format;
  }

  // Returns the number of vertices in the mesh.
  uint32_t VertexCount() const {
    ABSL_DCHECK_EQ(vertex_data_.size() % VertexStride(), 0u);
    ABSL_DCHECK_EQ(vertex_data_.size() / VertexStride(), vertex_count_);
    return vertex_count_;
  }

  // Appends a vertex to the mesh at the given position. All other vertex
  // attributes will be initialized to zero.
  void AppendVertex(Point position) {
    vertex_data_.resize(vertex_data_.size() + VertexStride());
    ++vertex_count_;
    SetVertexPosition(VertexCount() - 1, position);
  }

  // Returns the position of the vertex at the given index. This DCHECK-fails if
  // `index` >= `VertexCount()`.
  Point VertexPosition(uint32_t index) const {
    ABSL_DCHECK_LT(index, VertexCount());
    Point position;
    static_assert(sizeof(position) == 2 * sizeof(float));
    std::memcpy(
        &position,
        &vertex_data_[index * VertexStride() +
                      format_.Attributes()[VertexPositionAttributeIndex()]
                          .unpacked_offset],
        sizeof(position));
    return position;
  }

  // Sets the position of the vertex at the given index. This DCHECK-fails if
  // `index` >= `VertexCount()`.
  void SetVertexPosition(uint32_t index, Point position) {
    ABSL_DCHECK_LT(index, VertexCount());
    static_assert(sizeof(position) == 2 * sizeof(float));
    std::memcpy(
        &vertex_data_[index * VertexStride() +
                      format_.Attributes()[VertexPositionAttributeIndex()]
                          .unpacked_offset],
        &position, sizeof(position));
  }

  // Returns the index of the vertex attribute that contains the vertex's
  // position. This is equivalent to:
  //   mesh.Format().PositionAttributeIndex();
  uint32_t VertexPositionAttributeIndex() const {
    return format_.PositionAttributeIndex();
  }

  // Returns the value of the attribute at index `attribute_index` on the vertex
  // at `vertex_index`. This DCHECK-fails if `vertex_index` >= `VertexCount()`,
  // or if `attribute_index` >= `Format().Attributes().size()`.
  SmallArray<float, 4> FloatVertexAttribute(uint32_t vertex_index,
                                            uint32_t attribute_index) const {
    return mesh_internal::ReadUnpackedFloatAttributeFromByteArray(
        vertex_index, attribute_index, vertex_data_, format_);
  }

  // Sets the value of the attribute at index `attribute_index` on the vertex at
  // `vertex_index`. This DCHECK-fails if:
  // - `vertex_index` >= `VertexCount()`
  // - `attribute_index` >= `Format().Attributes().size()`
  // - `value` does not have the correct number of float elements for
  //   `MeshFormat::AttributeType` at index `attribute_index`
  void SetFloatVertexAttribute(uint32_t vertex_index, uint32_t attribute_index,
                               SmallArray<float, 4> value);

  // Returns the number of triangles in the mesh.
  uint32_t TriangleCount() const {
    ABSL_DCHECK_EQ(index_data_.size() % (3 * IndexStride()), 0u);
    ABSL_DCHECK_EQ(index_data_.size() / (3 * IndexStride()), triangle_count_);
    return triangle_count_;
  }

  // Appends a triangle to the mesh, consisting of the vertices at the given
  // vertex indices. This DCHECK-fails if any element of `vertex_indices` > the
  // maximum value representable by the index format.
  // Warning: This does not validate that the triangle's indices are valid (i.e.
  // that the mesh has vertices with those indices). The validity of triangle
  // vertices may be checked with `ValidateTriangles`, below.
  void AppendTriangleIndices(std::array<uint32_t, 3> vertex_indices) {
    index_data_.resize(index_data_.size() + 3 * IndexStride());
    ++triangle_count_;
    SetTriangleIndices(TriangleCount() - 1, vertex_indices);
  }

  // Returns the indices of the vertices that make up the triangle at the given
  // index. This DCHECK-fails if `index` >= `TriangleCount()`.
  std::array<uint32_t, 3> TriangleIndices(uint32_t index) const {
    return mesh_internal::ReadTriangleIndicesFromByteArray(index, IndexStride(),
                                                           index_data_);
  }

  // Replaces the triangle at the given index with a triangle consisting of the
  // vertices at the given vertex indices. This DCHECK-fails if `index` >=
  // `TriangleCount()`, or if any element of `vertex_indices` > the maximum
  // value representable by the index format.
  // Warning: This does not validate that the triangle's indices are valid (i.e.
  // that the mesh has vertices with those indices). The validity of triangle
  // vertices may be checked with `ValidateTriangles`, below.
  void SetTriangleIndices(uint32_t index,
                          std::array<uint32_t, 3> vertex_indices) {
    ABSL_DCHECK(absl::c_all_of(vertex_indices, [this](uint32_t i) {
      return i < (static_cast<uint64_t>(1) << (8 * IndexStride()));
    }));
    mesh_internal::WriteTriangleIndicesToByteArray(index, IndexStride(),
                                                   vertex_indices, index_data_);
  }

  // Inserts a triangle consisting of the vertices at the given vertex indices
  // at the given index, pushing all triangles after `index` backwards in the
  // triangle list. This DCHECK-fails if `index` > `TriangleCount()`.
  // Warning: This does not validate that the triangle's indices are valid (i.e.
  // that the mesh has vertices with those indices). The validity of triangle
  // vertices may be checked with `ValidateTriangles`, below.
  void InsertTriangleIndices(uint32_t index,
                             std::array<uint32_t, 3> vertex_indices) {
    ABSL_DCHECK_LE(index, TriangleCount());
    index_data_.insert(index_data_.begin() + 3 * IndexStride() * index,
                       3 * IndexStride(), std::byte(0));
    ++triangle_count_;
    ABSL_DCHECK_EQ(index_data_.size() % (3 * IndexStride()), 0u);
    SetTriangleIndices(index, vertex_indices);
  }

  // Resizes the `MutableMesh` such that it has `new_vertex_count` vertices and
  // `new_triangle_count` triangles.
  //
  // If `new_vertex_count` or `new_triangle_count` is less than the current
  // number of vertices or triangles, then the mesh will be reduced to the first
  // `new_vertex_count` vertices or `new_triangle_count` triangles,
  // respectively. If `new_vertex_count` or `new_triangle_count` is greater than
  // the current number of vertices or triangles, then additional vertices or
  // triangles will be inserted at the end, respectively. These vertices and/or
  // triangle will be zeroed out.
  //
  // Warning: This does not validate that the triangle's indices are valid (i.e.
  // that the mesh has vertices with those indices). The validity of triangle
  // vertices may be checked with `ValidateTriangles`, below.
  void Resize(uint32_t new_vertex_count, uint32_t new_triangle_count) {
    vertex_data_.resize(new_vertex_count * VertexStride());
    index_data_.resize(new_triangle_count * size_t{3} * IndexStride());
    vertex_count_ = new_vertex_count;
    triangle_count_ = new_triangle_count;
  }

  // Returns OK if:
  // - all triangles refer to vertices that exist in the `MutableMesh`, i.e.
  //   each of the triangle's indices are < `VertexCount()`
  // - all triangles refer to three distinct vertices, i.e. the triangle's
  //   indices contain no repeated values
  // This does not check that all vertices belong to a
  // triangle.
  absl::Status ValidateTriangles() const;

  // Returns the (position-only) triangle at the given index. This DCHECK-fails
  // if `index` >= `TriangleCount()`, or the triangle refers to a non-existent
  // vertex (see also `ValidateTriangles`).
  Triangle GetTriangle(uint32_t index) const {
    return TriangleFromIndices(TriangleIndices(index));
  }

  // Returns an immutable copy of the mesh.
  //
  // Optional argument `packing_params` specifies the transform to use to pack
  // each vertex attribute. Each packing transform corresponds to the attribute
  // at the same index in `Format()`. This argument is interpreted as in
  // `Mesh::Create`, and this method returns errors under the same conditions.
  //
  // Optional argument `omit_attributes` specifies attributes present in the
  // `MutableMesh` that should be omitted from any resulting `Mesh`.
  //
  // Depending on the format, this may be a lossy copy. The returned mesh may be
  // partitioned into multiple sub-meshes if this mesh is larger than can be
  // represented by the index format. Vertices that are not referenced by any
  // triangle will be stripped from the returned meshes.
  //
  // If the format is lossy, and if all triangles have non-negative area before
  // quantization, then this will detect triangles whose area becomes negative
  // due to quantization error, and attempt to correct them. This is done by
  // altering vertex positions, which means that a vertex may not be rounded to
  // the closest quantized position. The maximum amount of error in this case is
  // approximately double the normal error maximum (see
  // `MeshFormat::AttributeType`). Note that this does not always succeed, so
  // the result may still contain triangles with negative area.
  //
  // Returns an error if:
  // - `ValidateTriangleIndices` fails
  // - Any attribute value is non-finite
  // - The range of any attribute value (i.e. max - min) is greater than
  //   std::numeric_limits<float>::max()
  // - `packing_params` meets any of the error conditions for `Mesh::Create`
  // - `omit_attributes` contains `kPosition`
  // - `omit_attributes` contains attributes not in `Format()`
  //
  // TODO: b/295166196 - Once `MutableMesh` uses 16-bit indices, this can be
  // renamed to "AsMesh" and return a single `Mesh`, without needing to do any
  // partitioning.
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> AsMeshes(
      absl::Span<const std::optional<MeshAttributeCodingParams>>
          packing_params = {},
      absl::Span<const MeshFormat::AttributeId> omit_attributes = {}) const;

  // Returns the format of the mesh.
  const MeshFormat& Format() const { return format_; }

  // Returns the raw data of the mesh's vertices.
  absl::Span<const std::byte> RawVertexData() const { return vertex_data_; }

  // Returns the number of bytes used to represent a vertex in this mesh. This
  // is equivalent to:
  //   mesh.Format().UnpackedVertexStride();
  size_t VertexStride() const { return format_.UnpackedVertexStride(); }

  // Returns the raw data of the mesh's triangle indices.
  absl::Span<const std::byte> RawIndexData() const { return index_data_; }

  // Returns the number of bytes used to represent a triangle index in this
  // mesh. This is equivalent to:
  //   mesh.Format().UnpackedIndexStride();
  size_t IndexStride() const { return format_.UnpackedIndexStride(); }

 private:
  Triangle TriangleFromIndices(
      const std::array<uint32_t, 3>& vertex_indices) const {
    return {VertexPosition(vertex_indices[0]),
            VertexPosition(vertex_indices[1]),
            VertexPosition(vertex_indices[2])};
  }

  MeshFormat format_;
  std::vector<std::byte> vertex_data_;
  std::vector<std::byte> index_data_;
  uint32_t vertex_count_ = 0;
  uint32_t triangle_count_ = 0;
};

}  // namespace ink

#endif  // INK_GEOMETRY_MUTABLE_MESH_H_
