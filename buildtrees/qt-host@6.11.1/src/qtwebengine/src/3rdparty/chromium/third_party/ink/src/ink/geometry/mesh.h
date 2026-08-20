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

#ifndef INK_GEOMETRY_MESH_H_
#define INK_GEOMETRY_MESH_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "absl/base/nullability.h"
#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/types/small_array.h"

namespace ink {

// A read-only mesh made up of vertices and triangles. Vertices have attributes,
// as specified by the `MeshFormat`. A position attribute is required, and there
// may optionally be additional non-geometric attributes, e.g. texture
// coordinates. The vertex attributes may be packed for more efficient storage;
// see `MeshFormat` for details on attribute packing.
//
// `Mesh` stores its data in a `std::shared_ptr`; making a copy only involves
// copying the `std::shared_ptr`, making it very cheap.
class Mesh {
 public:
  // Constructs a mesh with the given format and unpacked attribute values.
  // `vertex_attributes` should contain one span for each component of the
  // attributes on the vertex, in order, and `triangle_indices` should contain
  // the vertex indices that make up each triangle. E.g., if the vertex has two
  // attributes of types `kFloat2Unpacked` and `kFloat4PackedInTwoFloats`, the
  // first two spans are used to populate the components of the first attribute,
  // and the third through sixth spans are used to populate the components of
  // the second attribute.
  //
  // Optional argument `packing_params` specifies the transform to use to pack
  // each vertex attribute. Each packing transform corresponds to the attribute
  // at the same index in `format`. `std::nullopt` may be used to indicate that
  // that attribute should use the default packing transform calculation (using
  // maximum precision for the attribute's bounds). Packing transforms must not
  // be specified for unpacked attribute types; `std::nullopt` should be used
  // instead. If `packing_params` is empty, all attributes will use the default
  // packing transform calculation. If no vertex attributes are given,
  // `packing_params` will be ignored.
  //
  // Returns an error if:
  // - `vertex_attributes.size()` != the total number of vertex
  //   attribute components specified in `format`
  // - Any element of `vertex_attributes` is a different size than the
  //   others
  // - Any attribute value is non-finite
  // - The range of values for any attribute (i.e. max - min) is larger than
  //   std::numeric_limits<float>::max()
  // - More than 2^16 (65536) vertices are given
  // - `triangle_indices.size()` is not divisible by 3
  // - `triangle_indices` contains any element >=
  //   `vertex_attributes[0].size`
  // - `triangle_indices` contains any element that cannot by
  //   represented by the `MeshFormat::IndexFormat` specified by `format`
  // - `packing_params` is not empty and `packing_params.size()` !=
  //   `format.Attributes().size()`
  // - Any non-null element of `packing_params` corresponds to an unpacked
  //   attribute
  // - Any non-null element of `packing_params` is not a valid transform for
  //   the corresponding attribute (i.e. wrong number of components, non-finite
  //   values)
  // - Any non-null element of `packing_params` is unable to represent the
  //   minimum and maximum values of the corresponding attribute
  static absl::StatusOr<Mesh> Create(
      const MeshFormat& format,
      absl::Span<const absl::Span<const float>> vertex_attributes,
      absl::Span<const uint32_t> triangle_indices,
      absl::Span<const std::optional<MeshAttributeCodingParams>>
          packing_params = {});

  // Constructs an empty mesh, with a default-constructed `MeshFormat`. Note
  // that, since `Mesh` is read-only, you can't do much with an empty mesh. See
  // `Mesh::Create` and `MutableMesh::ConvertToMeshes` for creating non-empty
  // meshes.
  Mesh()
      : data_(std::make_shared<const Data>(Data{
            .unpacking_params = {
                {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}})) {}

  // `Mesh` is cheap to copy, as the copies just share the same (immutable)
  // underlying data.
  Mesh(const Mesh&) = default;
  Mesh& operator=(const Mesh&) = default;

  // Returns the number of vertices in the mesh.
  uint32_t VertexCount() const {
    ABSL_DCHECK_EQ(data_->vertex_data.size() % VertexStride(), 0u);
    ABSL_DCHECK_EQ(data_->vertex_data.size() / VertexStride(),
                   data_->vertex_count);
    return data_->vertex_count;
  }

  // Returns the position of the vertex at the given index. This DCHECK-fails if
  // `index` >= `VertexCount()`.
  Point VertexPosition(uint32_t index) const {
    SmallArray<float, 4> value =
        FloatVertexAttribute(index, VertexPositionAttributeIndex());
    return {value[0], value[1]};
  }

  // Returns the index of the vertex attribute that contains the vertex's
  // position.
  uint32_t VertexPositionAttributeIndex() const {
    return Format().PositionAttributeIndex();
  }

  // Returns the (unpacked) value of the attribute at index `attribute_index` on
  // the vertex at `vertex_index`. DCHECK-fails if:
  // - `vertex_index` >= `VertexCount()`
  // - `attribute_index` >= `Format().Attributes().size()`.
  SmallArray<float, 4> FloatVertexAttribute(uint32_t vertex_index,
                                            uint32_t attribute_index) const;

  // Returns the packed integer values for the attribute at index
  // `attribute_index` on the vertex at `vertex_index`.
  //
  // This DCHECK-fails if `vertex_index` >= `VertexCount()`, or if
  // `attribute_index` >= `Format().Attributes().size()`, or if the attribute in
  // question is not packed.
  SmallArray<uint32_t, 4> PackedIntegersForFloatVertexAttribute(
      uint32_t vertex_index, uint32_t attribute_index) const;

  // Returns the number of triangles in the mesh.
  uint32_t TriangleCount() const {
    ABSL_DCHECK_EQ(data_->index_data.size() % (3 * kBytesPerIndex), 0u);
    ABSL_DCHECK_EQ(data_->index_data.size() / (3 * kBytesPerIndex),
                   data_->triangle_count);
    return data_->triangle_count;
  }

  // Returns the indices of the vertices that make up the triangle at the given
  // index. This DCHECK-fails if `index` >= `TriangleCount()`.
  std::array<uint32_t, 3> TriangleIndices(uint32_t index) const {
    return mesh_internal::ReadTriangleIndicesFromByteArray(
        index, kBytesPerIndex, data_->index_data);
  }

  // Returns the (position-only) triangle at the given index. This DCHECK-fails
  // if `index` >= `TriangleCount()`.
  Triangle GetTriangle(uint32_t index) const;

  // Returns the format of the mesh.
  const MeshFormat& Format() const { return data_->format; }

  // Returns the bounding box the vertices in the mesh.
  Envelope Bounds() const {
    Envelope env;
    if (std::optional<MeshAttributeBounds> bounds =
            AttributeBounds(VertexPositionAttributeIndex())) {
      env.Add(Point{bounds->minimum[0], bounds->minimum[1]});
      env.Add(Point{bounds->maximum[0], bounds->maximum[1]});
    }
    return env;
  }

  // Returns a `MeshAttributeBounds` for the vertex attribute at
  // `attribute_index`, or std::nullopt if the mesh contains no vertices.
  // CHECK-fails if `attribute_index` >= `Format().Attributes().size()`.
  std::optional<MeshAttributeBounds> AttributeBounds(
      uint32_t attribute_index) const {
    ABSL_CHECK_LT(attribute_index, Format().Attributes().size());
    if (!data_->attribute_bounds.has_value()) return std::nullopt;
    return (*data_->attribute_bounds)[attribute_index];
  }

  // Returns the `MeshAttributeCodingParams` for unpacking the vertex attribute
  // at `attribute_index`.
  // CHECK-fails if `attribute_index` >= `Format().Attributes().size()`.
  const MeshAttributeCodingParams& VertexAttributeUnpackingParams(
      uint32_t attribute_index) const {
    ABSL_CHECK_LT(attribute_index, Format().Attributes().size());
    return data_->unpacking_params[attribute_index];
  }

  // Returns the raw data of the mesh's vertices.
  absl::Span<const std::byte> RawVertexData() const {
    return data_->vertex_data;
  }

  // Returns the number of bytes used to represent a vertex in this mesh. This
  // is equivalent to:
  //   mesh.Format().PackedVertexStride();
  uint32_t VertexStride() const { return Format().PackedVertexStride(); }

  // Returns the raw data of the mesh's triangle indices. These are stored
  // unsigned using 2 bytes per index (i.e. as uint16_t).
  absl::Span<const std::byte> RawIndexData() const { return data_->index_data; }

  // Returns the number of bytes used to represent a triangle index in this
  // mesh, which is always two bytes (i.e. sizeof(uint16_t)).
  uint32_t IndexStride() const { return kBytesPerIndex; }

 private:
  struct Data {
    MeshFormat format;
    mesh_internal::CodingParamsArray unpacking_params;
    std::optional<mesh_internal::AttributeBoundsArray> attribute_bounds;
    std::vector<std::byte> vertex_data;
    std::vector<std::byte> index_data;
    uint32_t vertex_count = 0;
    uint32_t triangle_count = 0;
  };

  // `MutableMesh::AsMeshes` requires access to the private ctor, to avoid
  // recomputing the packing transform and avoid making an extra copy of each
  // partition.
  friend class MutableMesh;

  // `Mesh` always uses 16-bit indices.
  static constexpr uint8_t kBytesPerIndex = sizeof(uint16_t);

  Mesh(const MeshFormat& format,
       mesh_internal::CodingParamsArray unpacking_transforms,
       std::optional<mesh_internal::AttributeBoundsArray> attribute_bounds,
       std::vector<std::byte> vertex_data, std::vector<std::byte> index_data)
      : data_(CreateMeshData(format, std::move(unpacking_transforms),
                             std::move(attribute_bounds),
                             std::move(vertex_data), std::move(index_data))) {}

  // Helper function for Create(). Packs the vertex attributes into a vector of
  // bytes.
  static std::vector<std::byte> PackVertexByteData(
      const MeshFormat& format,
      absl::Span<const absl::Span<const float>> vertex_attributes,
      const mesh_internal::CodingParamsArray& packing_params_array);

  // Helper function for the private Mesh constructor. Creates a new Data struct
  // while avoiding field initialization order issues.
  static std::shared_ptr<const Data> CreateMeshData(
      const MeshFormat& format,
      mesh_internal::CodingParamsArray unpacking_transforms,
      std::optional<mesh_internal::AttributeBoundsArray> attribute_bounds,
      std::vector<std::byte> vertex_data, std::vector<std::byte> index_data) {
    uint32_t vertex_count = vertex_data.size() / format.PackedVertexStride();
    uint32_t triangle_count = index_data.size() / (3 * kBytesPerIndex);
    return std::make_shared<const Data>(Data{
        .format = format,
        .unpacking_params = std::move(unpacking_transforms),
        .attribute_bounds = std::move(attribute_bounds),
        .vertex_data = std::move(vertex_data),
        .index_data = std::move(index_data),
        .vertex_count = vertex_count,
        .triangle_count = triangle_count,
    });
  }

  // Returns a span into the vector that contains the bytes of the packed floats
  // that encode the attribute value at index `attribute_index` on the vertex at
  // `vertex_index`. If the attribute is not packed, this will be the same as
  // the unpacked values returned by FloatVertexAttribute().
  //
  // This DCHECK-fails if `vertex_index` >= `VertexCount()`, or if
  // `attribute_index` >= `Format().Attributes().size()`.
  absl::Span<const std::byte> PackedVertexAttribute(
      uint32_t vertex_index, uint32_t attribute_index) const;

  absl_nonnull std::shared_ptr<const Data> data_;
};

}  // namespace ink

#endif  // INK_GEOMETRY_MESH_H_
