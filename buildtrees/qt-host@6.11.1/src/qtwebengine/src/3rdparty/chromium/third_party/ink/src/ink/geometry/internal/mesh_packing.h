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

#ifndef INK_GEOMETRY_INTERNAL_MESH_PACKING_H_
#define INK_GEOMETRY_INTERNAL_MESH_PACKING_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_constants.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/types/small_array.h"

// For a `kFloatNPackedInKFloats` attribute, there are four forms that the
// packed vertex data can exist in:
//
//   1. An array of raw bytes
//   2. The K packed floats stored in those raw bytes
//   3. The N integers that are packed into those floats
//   4. The N unpacked float values
//
// The attribute type determines the mapping between (2) and (3), and the coding
// params determine the mapping between (3) and (4).
//
// For a `kFloatNUnpacked` attribute, the N unpacked float values are stored
// directly as raw bytes.  To put it a different way, (2) is identical to (4),
// (3) doesn't exist, and the (unneeded) coding params would just describe the
// identity transform.

namespace ink::mesh_internal {

// Convenience alias for an array of `MeshAttributeCodingParams`s, with a
// maximum number of elements equal to the maximum number of vertex attributes.
using CodingParamsArray =
    SmallArray<MeshAttributeCodingParams, kMaxVertexAttributes>;

// Convenience alias for an array of `MeshAttributeBounds`s, with a maximum
// number of elements equal to the maximum number of vertex attributes.
using AttributeBoundsArray =
    SmallArray<MeshAttributeBounds, kMaxVertexAttributes>;

// Returns the largest integer that can be represented with `n_bits` bits.
// `n_bits` must be <= 32.
constexpr uint32_t MaxValueForBits(uint8_t n_bits) {
  return n_bits == 32 ? std::numeric_limits<uint32_t>::max()
                      : (1 << n_bits) - 1;
}

// Packs a mesh attribute value (`unpacked_value`) into a packed integer
// representation stored in one or more floats. `PackAttribute` and
// `UnpackAttribute` are not exactly inverse functions, because packing is
// lossy. See `MeshFormat::AttributeType` for more details on how each attribute
// is packed.
//
// For packed attribute types, the arguments have the following restrictions:
// - All must be the correct size for `type`
// - `packing_params` and `unpacked_value`  must be finite and non-NaN
// - Every `scale` in `packing_params` must be > 0
// - `unpacked_value` must be representable by `type`, i.e.:
//   `(unpacked_value[i] - offset[i]) / scale[i]` must lie in the interval
//   [0, 2^`MeshFormat::PackedBitsPerComponent(type)` - 1]
//
// For unpacked attribute types, the arguments have the following restrictions:
// - `packing_params` is ignored
// - `unpacked_value` and `packed_bytes` (or `packed_value` for
//   `UnpackAttributes`) must be the correct size for `type`
// - `unpacked_value` (or `packed_value` for `UnpackAttributes`) must be finite
//   and non-NaN
//
// For performance reasons, these conditions are only enforced via DCHECK; the
// logic in `Mesh` is expected to guarantee that they are met.
//
// See `MeshFormat::AttributeType` for more details on how each attribute is
// packed, and the comment at the top of this file for an overview of packing.
void PackAttribute(MeshFormat::AttributeType type,
                   const MeshAttributeCodingParams& packing_params,
                   const SmallArray<float, 4>& unpacked_value,
                   absl::Span<std::byte> packed_bytes);

// Unpacks a mesh attribute value from a packed integer representation stored in
// one or more floats (`packed_value`). See `PackAttribute` for more details;
// all the same warnings and restrictions apply here. In addition:
// - Every element of `packed_value` must lie in the interval [0, 2^24 - 1]
//
// See `MeshFormat::AttributeType` for more details on how each attribute is
// packed, and the comment at the top of this file for an overview of unpacking.
SmallArray<float, 4> UnpackAttribute(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& unpacking_params,
    absl::Span<const std::byte> packed_value);

// Returns the integer value that should be packed into a float.  The packing
// transform must be valid, and the unpacked value must be in range for that
// transform.
uint32_t PackSingleFloat(
    const MeshAttributeCodingParams::ComponentCodingParams& packing_params,
    float unpacked_value);

// Extracts the integer values from the packed float values for an attribute.
// The arguments have the following restrictions:
// - `type` must be a packed attribute type
// - `packed_value` must lie in the interval [0, 2^24 - 1]
SmallArray<uint32_t, 4> UnpackIntegersFromPackedAttribute(
    MeshFormat::AttributeType type, absl::Span<const std::byte> packed_value);

// Extracts the float values from the byte values for an attribute.
// The arguments have the following restrictions:
// - `type` must be an unpacked attribute type
SmallArray<float, 4> ReadFloatsFromUnpackedAttribute(
    MeshFormat::AttributeType type, absl::Span<const std::byte> packed_value);

// Reads/writes triangle indices from/to a vector of bytes. The following
// conditions are expected to be enforced by the logic in `Mesh` and
// `MutableMesh`, and are enforced via DCHECK:
// - `index_stride` must be 2 or 4
// - `index_data.size()` must be a multiple of 3 * `index_stride`
// - `vertex_indices` must contain 3 elements
// This also DCHECK-fails if `triangle_index` is out-of-bounds for `index_data`,
// i.e. if 3 * `triangle_index` * `index_stride` >= `index_data.size()`.
std::array<uint32_t, 3> ReadTriangleIndicesFromByteArray(
    uint32_t triangle_index, uint8_t index_stride,
    absl::Span<const std::byte> index_data);
void WriteTriangleIndicesToByteArray(uint32_t triangle_index,
                                     uint8_t index_stride,
                                     absl::Span<const uint32_t> vertex_indices,
                                     std::vector<std::byte>& index_data);

// Returns the value of the attribute at index `attribute_index` on the vertex
// at `vertex_index`, stored in the `vertex_data` with given mesh `format`.
// This DCHECK-fails if:
// - `vertex_data.size()` is not divisible by `format.UnpackedVertexStride()`
// - `vertex_index` >=  `vertex_data.size()` / `format.UnpackedVertexStride()`
// - `attribute_index` >= `format.Attributes().size()`
SmallArray<float, 4> ReadUnpackedFloatAttributeFromByteArray(
    uint32_t vertex_index, uint32_t attribute_index,
    absl::Span<const std::byte> vertex_data, const MeshFormat& format);

// Splits the triangle indices contained in `index_data` into partitions, each
// of which refers to no more than `max_vertices_per_partition` vertices. This
// DCHECK-fails if `index_data.size()` is not divisible by
// 3 * `MeshFormat::UnpackedIndexSize(index_format)`; the logic in `MutableMesh`
// is expected to guarantee this.
struct PartitionInfo {
  // Indices of the vertices in the original `MutableMesh`.
  std::vector<uint32_t> vertex_indices;
  // The indices of the vertices in `vertex_indices` (not the original
  // `MutableMesh`) that make up each triangle.
  std::vector<std::array<uint32_t, 3>> triangles;
};
absl::InlinedVector<PartitionInfo, 1> PartitionTriangles(
    absl::Span<const std::byte> index_data,
    MeshFormat::IndexFormat index_format, uint64_t max_vertices_per_partition);

// Returns the `MeshAttributeCodingParams` for packing or unpacking the given
// attribute type. `bounds` must contain the minimum and maximum values of the
// attribute, respectively.
//
// This CHECK-fails if `bounds` has the wrong number of components, or if the
// minimum is greater than the maximum. For unpacked types, the bounds are not
// required and will be ignored.
//
// This returns an error if the difference between the minimum and maximum
// values exceeds the maximum float value.
absl::StatusOr<MeshAttributeCodingParams> ComputeCodingParams(
    MeshFormat::AttributeType type, const MeshAttributeBounds& bounds);
absl::StatusOr<MeshAttributeCodingParams> ComputeCodingParamsForBitSizes(
    SmallArray<uint8_t, 4> bits_per_component,
    const MeshAttributeBounds& bounds);

// Returns a `CodingParamsArray` constructed by calling `ComputeCodingParams`
// for each attribute in `format` and element in `bounds`.
// `custom_coding_params_array` may be used to specify coding params to use
// instead of the default; params must not be given for unpacked attributes.
//
// Returns an error if:
// - `bounds.Size()` != `format.Attributes.size()`
// - `custom_coding_params_array` is not empty and
//   `custom_coding_params_array.size()` != `format.Attributes.size()`
// - Any non-nullopt element of `custom_coding_params_array` corresponds to an
//   unpacked attribute
// - Any non-nullopt element of `custom_coding_params_array` is not valid for
//   the corresponding attribute, per `IsValidPackingParams`
// - Any non-nullopt element of `custom_coding_params_array` is unable to
//   represent the corresponding values in `bounds`, per
//   `UnpackedValuesAreRepresentable`
// - `ComputeCodingParams` fails for any attribute without a custom set of
//   coding params
absl::StatusOr<CodingParamsArray> ComputeCodingParamsArray(
    const MeshFormat& format, const AttributeBoundsArray& bounds,
    absl::Span<const std::optional<MeshAttributeCodingParams>>
        custom_coding_params_array = {});

// Returns a vector of bytes containing a packed copy of a subset of the
// vertices and attributes stored in `unpacked_vertex_data`.
// `unpacked_vertex_data` is expected to contain vertices in unpacked form, in
// the format specified by `format`.  `partition_vertex_indices` specifies the
// desired subset, and its order, by indices of the vertices in
// `unpacked_vertex_data`. `omit_set` indicates attributes that should be
// omitted from the packed data. `packing_params_array` is used to perform the
// packing of the non-omitted attributes, and is expected to be the result of
// calling `ComputePackingParams`.
//
// `override_vertex_positions` should contain a map from vertex indices to a
// position to be used instead of the one contained in `unpacked_vertex_data`,
// e.g. for corrections to prevent triangles being flipped by quantization.
//
// This CHECK-fails if:
// - `unpacked_vertex_data` or `partition_vertex_indices` is empty
// - `unpacked_vertex_data.size()` is not divisible by
//   `format.UnpackedVertexStride()`
// - `omit_set.size() >= format.Attributes.size()`
// - `packing_params_array.Size() != format.Attributes.size() - omit_set.size()`
//
// This also DCHECK-fails (for performance reasons) if
// `partition_vertex_indices` contains any element >=
// `unpacked_vertex_data.size()` / `format.UnpackedVertexStride()`.
//
// These conditions are all expected to be guaranteed by the logic in
// `MutableMesh`.
std::vector<std::byte> CopyAndPackPartitionVertices(
    absl::Span<const std::byte> unpacked_vertex_data,
    absl::Span<const uint32_t> partition_vertex_indices,
    const MeshFormat& original_format,
    const absl::flat_hash_set<MeshFormat::AttributeId>& omit_set,
    const CodingParamsArray& packing_params_array,
    const absl::flat_hash_map<uint32_t, Point>& override_vertex_positions);

}  // namespace ink::mesh_internal

#endif  // INK_GEOMETRY_INTERNAL_MESH_PACKING_H_
