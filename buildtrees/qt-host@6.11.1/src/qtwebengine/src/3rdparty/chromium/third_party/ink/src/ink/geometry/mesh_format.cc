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

#include "ink/geometry/mesh_format.h"

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_constants.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

bool IsValidAttributeType(MeshFormat::AttributeType type) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
    case MeshFormat::AttributeType::kFloat4Unpacked:
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return true;
  }
  return false;
}

bool IsValidAttributeId(MeshFormat::AttributeId id) {
  switch (id) {
    case MeshFormat::AttributeId::kPosition:
    case MeshFormat::AttributeId::kColorShiftHsl:
    case MeshFormat::AttributeId::kOpacityShift:
    case MeshFormat::AttributeId::kTexture:
    case MeshFormat::AttributeId::kSideDerivative:
    case MeshFormat::AttributeId::kSideLabel:
    case MeshFormat::AttributeId::kForwardDerivative:
    case MeshFormat::AttributeId::kForwardLabel:
    case MeshFormat::AttributeId::kSurfaceUv:
    case MeshFormat::AttributeId::kAnimationOffset:
    case MeshFormat::AttributeId::kCustom0:
    case MeshFormat::AttributeId::kCustom1:
    case MeshFormat::AttributeId::kCustom2:
    case MeshFormat::AttributeId::kCustom3:
    case MeshFormat::AttributeId::kCustom4:
    case MeshFormat::AttributeId::kCustom5:
    case MeshFormat::AttributeId::kCustom6:
    case MeshFormat::AttributeId::kCustom7:
    case MeshFormat::AttributeId::kCustom8:
    case MeshFormat::AttributeId::kCustom9:
      return true;
  }
  return false;
}

}  // namespace

MeshFormat::MeshFormat()
    : attributes_({Attribute{.type = AttributeType::kFloat2Unpacked,
                             .id = AttributeId::kPosition}}),
      position_attribute_index_(0),
      index_format_(IndexFormat::k32BitUnpacked16BitPacked) {
  PopulateOffsetWidthAndStride();
}

absl::StatusOr<MeshFormat> MeshFormat::Create(
    absl::Span<const std::pair<AttributeType, AttributeId>> attributes,
    IndexFormat index_format) {
  if (attributes.empty()) {
    return absl::InvalidArgumentError("Empty attributes");
  }
  if (attributes.size() > mesh_internal::kMaxVertexAttributes) {
    return absl::InvalidArgumentError("Maximum number of attributes exceeded");
  }

  int position_attribute_index = -1;
  std::bitset<256> attribute_ids;
  for (size_t i = 0; i < attributes.size(); ++i) {
    AttributeType type = attributes[i].first;
    if (!IsValidAttributeType(type)) {
      return absl::InvalidArgumentError(absl::Substitute(
          "Attribute at index $0 has unrecognized AttributeType $1", i,
          static_cast<uint8_t>(type)));
    }
    AttributeId id = attributes[i].second;
    if (!IsValidAttributeId(id)) {
      return absl::InvalidArgumentError(absl::Substitute(
          "Attribute at index $0 has unrecognized AttributeId $1", i,
          static_cast<uint8_t>(id)));
    }
    if (attribute_ids.test(static_cast<uint8_t>(id))) {
      return absl::InvalidArgumentError(
          absl::Substitute("Found more than one $0 attribute", id));
    }
    attribute_ids.set(static_cast<uint8_t>(id));
    if (id == AttributeId::kPosition) {
      position_attribute_index = i;
    }
  }
  if (position_attribute_index < 0) {
    return absl::InvalidArgumentError("Missing a kPosition attribute");
  }

  if (ComponentCount(attributes[position_attribute_index].first) != 2) {
    return absl::InvalidArgumentError(
        "Position attribute does not have 2 components");
  }

  MeshFormat format;
  format.attributes_.Resize(attributes.size());
  for (size_t i = 0; i < attributes.size(); ++i) {
    format.attributes_[i].type = attributes[i].first;
    format.attributes_[i].id = attributes[i].second;
  }
  format.position_attribute_index_ = position_attribute_index;
  format.index_format_ = index_format;

  format.PopulateOffsetWidthAndStride();

  return format;
}

absl::StatusOr<MeshFormat> MeshFormat::WithoutAttributes(
    absl::Span<const AttributeId> attributes_to_remove) const {
  absl::flat_hash_set<AttributeId> not_yet_removed(attributes_to_remove.begin(),
                                                   attributes_to_remove.end());
  if (not_yet_removed.contains(AttributeId::kPosition)) {
    return absl::InvalidArgumentError(
        "cannot remove the kPosition attribute, because every MeshFormat must "
        "include it");
  }

  std::vector<std::pair<AttributeType, AttributeId>> new_attributes;
  if (Attributes().size() > attributes_to_remove.size()) {
    new_attributes.reserve(Attributes().size() - attributes_to_remove.size());
  }
  for (Attribute attribute : Attributes()) {
    if (not_yet_removed.erase(attribute.id)) {
      continue;
    }
    new_attributes.emplace_back(attribute.type, attribute.id);
  }

  if (!not_yet_removed.empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("cannot remove the ", *not_yet_removed.begin(),
                     " attribute, because this MeshFormat doesn't include it"));
  }

  return MeshFormat::Create(new_attributes, GetIndexFormat());
}

uint8_t MeshFormat::TotalComponentCount() const {
  uint8_t total = 0;
  for (Attribute attribute : Attributes()) {
    total += ComponentCount(attribute.type);
  }
  return total;
}

uint8_t MeshFormat::ComponentCount(AttributeType type) {
  switch (type) {
    case AttributeType::kFloat1Unpacked:
    case AttributeType::kFloat1PackedInOneUnsignedByte:
      return 1;
    case AttributeType::kFloat2Unpacked:
    case AttributeType::kFloat2PackedInOneFloat:
    case AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return 2;
    case AttributeType::kFloat3Unpacked:
    case AttributeType::kFloat3PackedInOneFloat:
    case AttributeType::kFloat3PackedInTwoFloats:
    case AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return 3;
    case AttributeType::kFloat4Unpacked:
    case AttributeType::kFloat4PackedInOneFloat:
    case AttributeType::kFloat4PackedInTwoFloats:
    case AttributeType::kFloat4PackedInThreeFloats:
      return 4;
  }
  ABSL_LOG(FATAL) << "Unrecognized AttributeType " << static_cast<int>(type);
}

std::optional<SmallArray<uint8_t, 4>> MeshFormat::PackedBitsPerComponent(
    AttributeType type) {
  switch (type) {
    case AttributeType::kFloat1Unpacked:
    case AttributeType::kFloat2Unpacked:
    case AttributeType::kFloat3Unpacked:
    case AttributeType::kFloat4Unpacked:
      return std::nullopt;
    case AttributeType::kFloat1PackedInOneUnsignedByte:
      return SmallArray<uint8_t, 4>({8});
    case AttributeType::kFloat2PackedInOneFloat:
    case AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      return SmallArray<uint8_t, 4>({12, 12});
    case AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return SmallArray<uint8_t, 4>({12, 20});
    case AttributeType::kFloat4PackedInTwoFloats:
      return SmallArray<uint8_t, 4>({12, 12, 12, 12});
    case AttributeType::kFloat3PackedInOneFloat:
      return SmallArray<uint8_t, 4>({8, 8, 8});
    case AttributeType::kFloat3PackedInTwoFloats:
      return SmallArray<uint8_t, 4>({16, 16, 16});
    case AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return SmallArray<uint8_t, 4>({10, 10, 10});
    case AttributeType::kFloat4PackedInOneFloat:
      return SmallArray<uint8_t, 4>({6, 6, 6, 6});
    case AttributeType::kFloat4PackedInThreeFloats:
      return SmallArray<uint8_t, 4>({18, 18, 18, 18});
  }
  ABSL_LOG(FATAL) << "Unrecognized AttributeType " << static_cast<int>(type);
}

uint8_t MeshFormat::UnpackedAttributeSize(AttributeType type) {
  return 4 * ComponentCount(type);
}

bool MeshFormat::IsPackedAsFloat(AttributeType type) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return true;
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat4Unpacked:
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return false;
  }
  ABSL_LOG(FATAL) << "Unrecognized AttributeType " << static_cast<int>(type);
}

uint8_t MeshFormat::PackedAttributeSize(AttributeType type) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      return 1;
    case AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      return 3;
    case AttributeType::kFloat1Unpacked:
    case AttributeType::kFloat2PackedInOneFloat:
    case AttributeType::kFloat3PackedInOneFloat:
    case AttributeType::kFloat4PackedInOneFloat:
    case AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
    case AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return 4;
    case AttributeType::kFloat2Unpacked:
    case AttributeType::kFloat3PackedInTwoFloats:
    case AttributeType::kFloat4PackedInTwoFloats:
      return 8;
    case AttributeType::kFloat3Unpacked:
    case AttributeType::kFloat4PackedInThreeFloats:
      return 12;
    case AttributeType::kFloat4Unpacked:
      return 16;
  }
  ABSL_LOG(FATAL) << "Unrecognized AttributeType " << static_cast<int>(type);
}

uint8_t MeshFormat::UnpackedIndexSize(IndexFormat index_format) {
  switch (index_format) {
    case IndexFormat::k16BitUnpacked16BitPacked:
      return 2;
    case IndexFormat::k32BitUnpacked16BitPacked:
      return 4;
  }
  ABSL_LOG(FATAL) << "Unrecognized IndexFormat "
                  << static_cast<int>(index_format);
}

void MeshFormat::PopulateOffsetWidthAndStride() {
  uint16_t current_unpacked_offset = 0;
  uint16_t current_packed_offset = 0;
  for (Attribute& attr : attributes_.Values()) {
    attr.unpacked_offset = current_unpacked_offset;
    attr.packed_offset = current_packed_offset;
    attr.unpacked_width = UnpackedAttributeSize(attr.type);
    attr.packed_width = PackedAttributeSize(attr.type);
    current_unpacked_offset += attr.unpacked_width;
    current_packed_offset += attr.packed_width;
  }
  unpacked_vertex_stride_ = current_unpacked_offset;
  packed_vertex_stride_ = current_packed_offset;
  unpacked_index_stride_ = UnpackedIndexSize(index_format_);
}

bool MeshFormat::IsPackedEquivalent(const MeshFormat& first,
                                    const MeshFormat& second) {
  if (first.PositionAttributeIndex() != second.PositionAttributeIndex()) {
    return false;
  }
  const absl::Span<const MeshFormat::Attribute> first_attributes =
      first.Attributes();
  const absl::Span<const MeshFormat::Attribute> second_attributes =
      second.Attributes();
  if (first_attributes.size() != second_attributes.size()) {
    return false;
  }
  for (size_t i = 0; i < first_attributes.size(); ++i) {
    if (first_attributes[i].id != second_attributes[i].id ||
        first_attributes[i].type != second_attributes[i].type) {
      return false;
    }
  }
  return true;
}

bool MeshFormat::IsUnpackedEquivalent(const MeshFormat& first,
                                      const MeshFormat& second) {
  if (first.PositionAttributeIndex() != second.PositionAttributeIndex()) {
    return false;
  }
  const absl::Span<const MeshFormat::Attribute> first_attributes =
      first.Attributes();
  const absl::Span<const MeshFormat::Attribute> second_attributes =
      second.Attributes();
  if (first_attributes.size() != second_attributes.size()) {
    return false;
  }
  for (size_t i = 0; i < first_attributes.size(); ++i) {
    // The "set" of attributes are equivalent if attributes are in the same
    // order, and when unpacked contain the same amount of data (component size)
    // occupying the same amount of space (unpacked width) that pertain to the
    // same information (id).
    if (first_attributes[i].id != second_attributes[i].id ||
        ComponentCount(first_attributes[i].type) !=
            ComponentCount(second_attributes[i].type) ||
        first_attributes[i].unpacked_width !=
            second_attributes[i].unpacked_width) {
      return false;
    }
  }
  return true;
}

std::string MeshFormat::ToFormattedString() const {
  std::vector<std::string> attributes;
  attributes.reserve(attributes_.Size());
  for (MeshFormat::Attribute attribute : attributes_.Values()) {
    attributes.push_back(
        absl::StrCat("{", attribute.type, ", ", attribute.id, "}"));
  }
  return absl::StrCat("MeshFormat({", absl::StrJoin(attributes, ", "),
                      "}, position_attribute_index=", position_attribute_index_,
                      ", index_format=", index_format_, ")");
}

bool operator==(const MeshFormat& a, const MeshFormat& b) {
  return absl::c_equal(a.Attributes(), b.Attributes(),
                       [](const MeshFormat::Attribute& a,
                          const MeshFormat::Attribute& b) {
                         return a.type == b.type && a.id == b.id;
                       }) &&
         a.GetIndexFormat() == b.GetIndexFormat();
}

namespace mesh_internal {

std::string ToFormattedString(MeshFormat::AttributeType type) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
      return "kFloat1Unpacked";
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      return "kFloat1PackedInOneUnsignedByte";
    case MeshFormat::AttributeType::kFloat2Unpacked:
      return "kFloat2Unpacked";
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
      return "kFloat2PackedInOneFloat";
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      return "kFloat2PackedInThreeUnsignedBytes_XY12";
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return "kFloat2PackedInFourUnsignedBytes_X12_Y20";
    case MeshFormat::AttributeType::kFloat3Unpacked:
      return "kFloat3Unpacked";
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
      return "kFloat3PackedInOneFloat";
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
      return "kFloat3PackedInTwoFloats";
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return "kFloat3PackedInFourUnsignedBytes_XYZ10";
    case MeshFormat::AttributeType::kFloat4Unpacked:
      return "kFloat4Unpacked";
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
      return "kFloat4PackedInOneFloat";
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
      return "kFloat4PackedInTwoFloats";
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return "kFloat4PackedInThreeFloats";
  }
  return absl::StrCat("Invalid(", static_cast<int>(type), ")");
}

std::string ToFormattedString(MeshFormat::AttributeId id) {
  switch (id) {
    case MeshFormat::AttributeId::kPosition:
      return "kPosition";
    case MeshFormat::AttributeId::kColorShiftHsl:
      return "kColorShiftHsl";
    case MeshFormat::AttributeId::kOpacityShift:
      return "kOpacityShift";
    case MeshFormat::AttributeId::kTexture:
      return "kTexture";
    case MeshFormat::AttributeId::kSideDerivative:
      return "kSideDerivative";
    case MeshFormat::AttributeId::kSideLabel:
      return "kSideLabel";
    case MeshFormat::AttributeId::kForwardDerivative:
      return "kForwardDerivative";
    case MeshFormat::AttributeId::kForwardLabel:
      return "kForwardLabel";
    case MeshFormat::AttributeId::kSurfaceUv:
      return "kSurfaceUv";
    case MeshFormat::AttributeId::kAnimationOffset:
      return "kAnimationOffset";
    case MeshFormat::AttributeId::kCustom0:
      return "kCustom0";
    case MeshFormat::AttributeId::kCustom1:
      return "kCustom1";
    case MeshFormat::AttributeId::kCustom2:
      return "kCustom2";
    case MeshFormat::AttributeId::kCustom3:
      return "kCustom3";
    case MeshFormat::AttributeId::kCustom4:
      return "kCustom4";
    case MeshFormat::AttributeId::kCustom5:
      return "kCustom5";
    case MeshFormat::AttributeId::kCustom6:
      return "kCustom6";
    case MeshFormat::AttributeId::kCustom7:
      return "kCustom7";
    case MeshFormat::AttributeId::kCustom8:
      return "kCustom8";
    case MeshFormat::AttributeId::kCustom9:
      return "kCustom9";
  }
  return absl::StrCat("Invalid(", static_cast<int>(id), ")");
}

std::string ToFormattedString(MeshFormat::IndexFormat index_format) {
  switch (index_format) {
    case MeshFormat::IndexFormat::k16BitUnpacked16BitPacked:
      return "k16BitUnpacked16BitPacked";
    case MeshFormat::IndexFormat::k32BitUnpacked16BitPacked:
      return "k32BitUnpacked16BitPacked";
  }
  return absl::StrCat("Invalid(", static_cast<int>(index_format), ")");
}

}  // namespace mesh_internal
}  // namespace ink
