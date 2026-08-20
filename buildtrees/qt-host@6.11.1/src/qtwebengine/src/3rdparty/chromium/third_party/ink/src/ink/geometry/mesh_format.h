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

#ifndef INK_GEOMETRY_MESH_FORMAT_H_
#define INK_GEOMETRY_MESH_FORMAT_H_

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_constants.h"
#include "ink/types/small_array.h"

namespace ink {

// Contains the specification of the format of the vertex attributes and indices
// for a `MutableMesh` and its equivalent `Mesh`.
//
// `MutableMesh` stores each vertex attribute value as a collection of 32-bit
// floats. For example, the 2-D position of each vertex is a pair a floats.
//
// `Mesh` supports vertex attribute "packing", in which each attribute value may
// be stored in a lossy fixed-precision format that uses fewer total bits than
// the original "unpacked" values. Each attribute may use a different packing
// scheme; see `AttributeType`.
//
// Since `MutableMesh` does not use packing, we call it an "unpacked mesh". In
// contrast, a `Mesh` is called a "packed mesh", even if its `MeshFormat`
// specifies that some (or even all) of the attributes should be stored without
// packing.
//
// A `MutableMesh` has a fixed `MeshFormat` that describes both the unpacked
// format of each of its attributes and also the packing scheme, if any, that
// will be used when converting to an equivalent `Mesh`. The `MeshFormat` for
// the resulting `Mesh` is identical to the original one; that is, the same
// `MeshFormat` instance describes both the unpacked format (as used in the
// `MutableMesh`) and the packed format (as used in the `Mesh`). This design
// guarantees that the unpacked and packed formats are compatible, eliminating
// the need for validation at conversion time. However, it also requires that
// callers of `MeshFormat` functions are responsible for knowing whether they
// need information about the unpacked form of an attribute or the packed form.
// Most API clients should call methods of `MutableMesh` or `Mesh` instead of
// calling `MeshFormat` member functions directly.
class MeshFormat {
 public:
  // Indicates the type and size of a vertex attribute and how it is packed when
  // stored in `Mesh`. Unless otherwise specified, attributes are stored in a
  // lossy fixed-precision packed format. The maximum error for a lossily packed
  // attribute is:
  //   .5 * (max - min) / (2^bits - 1)
  // where `min` and `max` are the minimum and maximum values, and `bits` is the
  // number of bits used per component. Note that the extrema are calculated for
  // each component of an attribute, so the error may vary between components as
  // well.
  //
  // In `MutableMesh`, vertex attributes are always stored unpacked, while in
  // `Mesh`, they are only unpacked if the `AttributeType` is a so-called
  // "unpacked type", e.g. `kFloat2Unpacked`.
  //
  // If you're planning to serialize a Mesh into a CodedMesh proto, it's
  // recommended to use packed attribute types only, since otherwise the
  // serialization will be lossy.
  //
  // LINT.IfChange(attribute_types)
  enum class AttributeType : uint8_t {
    // One float, stored unchanged and losslessly, even in a packed mesh.
    kFloat1Unpacked,
    // One float, packed into 1 unsigned byte.
    // [0] : 0xFF
    kFloat1PackedInOneUnsignedByte,
    // Two floats, stored unchanged and losslessly, even in a packed mesh.
    kFloat2Unpacked,
    // Two floats, packed into the mantissa of a single float, using 12 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFFF000
    // [1] : 0x000FFF
    kFloat2PackedInOneFloat,
    // Two floats, packed into 3 unsigned bytes, using 12 bits each.
    // [0] : 0xFF, 0xF0, 0x00
    // [1] : 0x00, 0x0F, 0xFF
    kFloat2PackedInThreeUnsignedBytes_XY12,
    // Two floats, packed into 4 unsigned bytes, using 12 bits for the first and
    // 20 bits for the second.
    // [0] : 0xFF, 0xF0, 0x00, 0x00
    // [1] : 0x00, 0x0F, 0xFF, 0xFF
    kFloat2PackedInFourUnsignedBytes_X12_Y20,
    // Three floats, stored unchanged and losslessly, even in a packed mesh.
    kFloat3Unpacked,
    // Three floats, packed into the mantissa of a single float, using 8 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFF0000
    // [1] : 0x00FF00
    // [2] : 0x0000FF
    kFloat3PackedInOneFloat,
    // Three floats, packed into the mantissa of two floats, using 16 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFFFF00, 0x000000
    // [1] : 0x0000FF, 0xFF0000
    // [2] : 0x000000, 0x00FFFF
    kFloat3PackedInTwoFloats,
    // Three floats, packed into 4 unsigned bytes, using 10 bits each for the
    // first, second, and third floats.
    // [0] : 0xFF, 0xC0, 0x00, 0x00
    // [1] : 0x00, 0x3F, 0xF0, 0x00
    // [2] : 0x00, 0x00, 0x0F, 0xFC
    // There will be two bits leftover which can be found at the end of the 4th
    // byte. The leftover bits will be set to 0 during packing and ignored
    // during unpacking.
    kFloat3PackedInFourUnsignedBytes_XYZ10,
    // Four floats, stored unchanged and losslessly, even in a packed mesh.
    kFloat4Unpacked,
    // Four floats, packed into the mantissa of a single float, using 6 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFC0000
    // [1] : 0x03F000
    // [2] : 0x000FC0
    // [3] : 0x00003F
    kFloat4PackedInOneFloat,
    // Four floats, packed into the mantissa of two floats, using 12 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFFF000, 0x000000
    // [1] : 0x000FFF, 0x000000
    // [2] : 0x000000, 0xFFF000
    // [3] : 0x000000, 0x000FFF
    kFloat4PackedInTwoFloats,
    // Four floats, packed into the mantissa of three floats, using 18 bits
    // each. The values are stored in the following bits of the mantissa:
    // [0] : 0xFFFFC0, 0x000000, 0x000000
    // [1] : 0x00003F, 0xFFF000, 0x000000
    // [2] : 0x000000, 0x000FFF, 0xFC0000
    // [3] : 0x000000, 0x000000, 0x03FFFF
    kFloat4PackedInThreeFloats,
  };
  // LINT.ThenChange(
  //   fuzz_domains.cc:attribute_types,
  //   ../storage/proto/coded.proto:attribute_types,
  // )

  // Indicates what a vertex attribute is used for or represents.  `MeshFormat`,
  // `Mesh`, and `MutableMesh` do not actually interact with attribute ID values
  // other than `kPosition`, but they are carried as metadata.
  //
  // The `kCustomN` values may be used for custom, client-specific IDs.
  //
  // LINT.IfChange(attribute_id)
  enum class AttributeId : uint8_t {
    kPosition,
    kColorShiftHsl,
    kOpacityShift,
    kTexture,
    kSideDerivative,
    kSideLabel,
    kForwardDerivative,
    kForwardLabel,
    kSurfaceUv,
    kAnimationOffset,
    kCustom0,
    kCustom1,
    kCustom2,
    kCustom3,
    kCustom4,
    kCustom5,
    kCustom6,
    kCustom7,
    kCustom8,
    kCustom9,
  };
  // LINT.ThenChange(
  //   fuzz_domains.cc:attribute_id,
  //   ../storage/proto/coded.proto:attribute_id,
  // )

  // An attribute stored on the vertex.
  struct Attribute {
    // The type of the vertex attribute, and how it is packed.
    AttributeType type;
    // The ID for the vertex attribute.
    AttributeId id;
    // The offset in bytes from the start of the vertex to the start of the
    // attribute, for unpacked and packed meshes, respectively.
    uint16_t unpacked_offset;
    uint16_t packed_offset;
    // The number of bytes used to store the attribute, for unpacked and packed
    // meshes, respectively.
    uint8_t unpacked_width;
    uint8_t packed_width;
  };

  // Indicates how the triangle index is stored, in `MutableMesh` and `Mesh`,
  // e.g. `k32BitUnpacked16BitPacked` means that `MutableMesh` uses 32-bit
  // indices and `Mesh` uses 16-bit indices.
  // TODO: b/295166196 - Delete this once `MutableMesh` uses 16-bit indices.
  // LINT.IfChange(index_formats)
  enum class IndexFormat : uint8_t {
    k16BitUnpacked16BitPacked,
    k32BitUnpacked16BitPacked,
  };
  // LINT.ThenChange(fuzz_domains.cc:index_formats)

  // Constructs a `MeshFormat` with a single attribute with type
  // `kFloat2Unpacked` and ID `kPosition`, and index format
  // `k32BitUnpacked16BitPacked`.
  MeshFormat();

  // Constructs a MeshFormat from the attribute type/name pairs, position index,
  // and index format. Returns an error if:
  // - `attributes` is empty
  // - `attributes.size` > `MaxAttributes()`
  // - Any `AttributeType` is not a named value
  // - Any `AttributeId` is not either a named value or a value >= 128
  // - Any `AttributeId` value appears more than once
  // - There isn't any attribute with  `AttributeId::kPosition`
  // - The position attribute doesn't have a component count of 2
  static absl::StatusOr<MeshFormat> Create(
      absl::Span<const std::pair<AttributeType, AttributeId>> attributes,
      IndexFormat index_format);

  // Constructs a new MeshFormat that is the same as this one, but with the
  // specified attributes removed. Returns an error if:
  // - Any of the attributes to remove are not in the original format
  // - `kPosition` is in the list of attributes IDs to remove
  absl::StatusOr<MeshFormat> WithoutAttributes(
      absl::Span<const AttributeId> attributes_to_remove) const;

  // Returns the list of attributes on a vertex.
  absl::Span<const Attribute> Attributes() const {
    return attributes_.Values();
  }

  // Returns the total number of attribute components in this mesh format
  // (i.e. the sum of `MeshFormat::ComponentCount(attr.type)` across all of this
  // format's attributes). This will always be at least 2, since every
  // `MeshFormat` is guaranteed to contain a `kPosition` attribute with exactly
  // two components.
  uint8_t TotalComponentCount() const;

  // Returns the index of the attribute that's used as a vertex's position.
  uint8_t PositionAttributeIndex() const { return position_attribute_index_; }

  // Returns the number of bytes used to represent each vertex, for unpacked and
  // packed meshes, respectively.
  uint16_t UnpackedVertexStride() const { return unpacked_vertex_stride_; }
  uint16_t PackedVertexStride() const { return packed_vertex_stride_; }

  // Returns the format used to store triangle indices.
  // TODO: b/295166196 - Delete this once `MutableMesh` uses 16-bit indices.
  IndexFormat GetIndexFormat() const { return index_format_; }

  // Returns the number of bytes used to represent a single triangle index for
  // an unpacked mesh.
  // TODO: b/295166196 - Delete this once `MutableMesh` uses 16-bit indices.
  uint8_t UnpackedIndexStride() const { return unpacked_index_stride_; }

  // Returns whether two mesh formats have the same packed representation
  // and same packing scheme such that they can be passed to the same shader
  // that accepts packed attribute values.
  static bool IsPackedEquivalent(const MeshFormat& first,
                                 const MeshFormat& second);

  // Returns whether two mesh formats have the same unpacked representation.
  static bool IsUnpackedEquivalent(const MeshFormat& first,
                                   const MeshFormat& second);

  // Returns the number of values that the attribute encodes. E.g.
  // `ComponentCount(AttributeType::kFloat2PackedInOneFloat)` == 2.
  static uint8_t ComponentCount(AttributeType type);

  // Returns the number of bits used to represent each component in the packed
  // attribute, or std::nullopt if the attribute is not packed.
  static std::optional<SmallArray<uint8_t, 4>> PackedBitsPerComponent(
      AttributeType type);

  // Returns true if the attribute type is packed into a float. Returns false
  // for all unpacked types and types that are packed directly into bytes.
  static bool IsPackedAsFloat(AttributeType type);

  // Returns true if the attribute type is an "unpacked type"; i.e., if the
  // attribute value is always stored unpacked, even in a packed mesh.
  static bool IsUnpackedType(AttributeType type) {
    return !PackedBitsPerComponent(type).has_value();
  }

  // Returns the size in bytes of the attribute when unpacked.
  static uint8_t UnpackedAttributeSize(AttributeType type);

  // Returns the size in bytes of the attribute when packed.
  static uint8_t PackedAttributeSize(AttributeType type);

  // Returns the size in bytes of a single vertex index in an unpacked mesh.
  // TODO: b/295166196 - Delete this once `MutableMesh` uses 16-bit indices.
  static uint8_t UnpackedIndexSize(IndexFormat index_format);

  // Returns the maximum supported number of vertex attributes.
  static uint8_t MaxAttributes() { return mesh_internal::kMaxVertexAttributes; }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const MeshFormat& format) {
    sink.Append(format.ToFormattedString());
  }

  template <typename H>
  friend H AbslHashValue(H h, const MeshFormat& format);

 private:
  void PopulateOffsetWidthAndStride();

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  SmallArray<Attribute, mesh_internal::kMaxVertexAttributes> attributes_;
  uint8_t position_attribute_index_;
  IndexFormat index_format_;
  uint16_t unpacked_vertex_stride_;
  uint16_t packed_vertex_stride_;
  // TODO: b/295166196 - Delete this once `MutableMesh` uses 16-bit indices.
  uint8_t unpacked_index_stride_;
};

bool operator==(const MeshFormat& a, const MeshFormat& b);
inline bool operator!=(const MeshFormat& a, const MeshFormat& b) {
  return !(a == b);
}

namespace mesh_internal {
std::string ToFormattedString(MeshFormat::AttributeType type);
std::string ToFormattedString(MeshFormat::AttributeId id);
std::string ToFormattedString(MeshFormat::IndexFormat index_format);
}  // namespace mesh_internal

template <typename Sink>
void AbslStringify(Sink& sink, MeshFormat::AttributeType type) {
  sink.Append(mesh_internal::ToFormattedString(type));
}

template <typename Sink>
void AbslStringify(Sink& sink, MeshFormat::AttributeId id) {
  sink.Append(mesh_internal::ToFormattedString(id));
}

template <typename Sink>
void AbslStringify(Sink& sink, MeshFormat::IndexFormat index_format) {
  sink.Append(mesh_internal::ToFormattedString(index_format));
}

template <typename H>
H AbslHashValue(H h, const MeshFormat& format) {
  h = H::combine(std::move(h), format.index_format_);
  for (const MeshFormat::Attribute& attribute : format.Attributes()) {
    h = H::combine(std::move(h), attribute.type, attribute.id);
  }
  return h;
}

}  // namespace ink

#endif  // INK_GEOMETRY_MESH_FORMAT_H_
