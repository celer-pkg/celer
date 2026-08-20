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

#include "ink/storage/mesh_format.h"

#include <utility>

#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/internal/mesh_constants.h"
#include "ink/geometry/mesh_format.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

proto::MeshFormat::AttributeType EncodeMeshAttributeType(
    MeshFormat::AttributeType type) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
      return proto::MeshFormat::ATTR_TYPE_FLOAT1_UNPACKED;
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      return proto::MeshFormat::ATTR_TYPE_FLOAT1_PACKED_IN_ONE_BYTE;
    case MeshFormat::AttributeType::kFloat2Unpacked:
      return proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED;
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
      return proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT;
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      return proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_THREE_BYTES_XY12;
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_FOUR_BYTES_X12_Y20;
    case MeshFormat::AttributeType::kFloat3Unpacked:
      return proto::MeshFormat::ATTR_TYPE_FLOAT3_UNPACKED;
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
      return proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_ONE_FLOAT;
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
      return proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_TWO_FLOATS;
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_FOUR_BYTES_XYZ10;
    case MeshFormat::AttributeType::kFloat4Unpacked:
      return proto::MeshFormat::ATTR_TYPE_FLOAT4_UNPACKED;
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
      return proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_ONE_FLOAT;
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
      return proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_TWO_FLOATS;
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_THREE_FLOATS;
  }
  ABSL_LOG(FATAL) << "Invalid AttributeType value: " << static_cast<int>(type);
}

absl::StatusOr<MeshFormat::AttributeType> DecodeMeshAttributeType(
    proto::MeshFormat::AttributeType type_proto) {
  switch (type_proto) {
    case proto::MeshFormat::ATTR_TYPE_FLOAT1_UNPACKED:
      return MeshFormat::AttributeType::kFloat1Unpacked;
    case proto::MeshFormat::ATTR_TYPE_FLOAT1_PACKED_IN_ONE_BYTE:
      return MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte;
    case proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED:
      return MeshFormat::AttributeType::kFloat2Unpacked;
    case proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT:
      return MeshFormat::AttributeType::kFloat2PackedInOneFloat;
    case proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_THREE_BYTES_XY12:
      return MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12;
    case proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_FOUR_BYTES_X12_Y20:
      return MeshFormat::AttributeType::
          kFloat2PackedInFourUnsignedBytes_X12_Y20;
    case proto::MeshFormat::ATTR_TYPE_FLOAT3_UNPACKED:
      return MeshFormat::AttributeType::kFloat3Unpacked;
    case proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_ONE_FLOAT:
      return MeshFormat::AttributeType::kFloat3PackedInOneFloat;
    case proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_TWO_FLOATS:
      return MeshFormat::AttributeType::kFloat3PackedInTwoFloats;
    case proto::MeshFormat::ATTR_TYPE_FLOAT3_PACKED_IN_FOUR_BYTES_XYZ10:
      return MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10;
    case proto::MeshFormat::ATTR_TYPE_FLOAT4_UNPACKED:
      return MeshFormat::AttributeType::kFloat4Unpacked;
    case proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_ONE_FLOAT:
      return MeshFormat::AttributeType::kFloat4PackedInOneFloat;
    case proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_TWO_FLOATS:
      return MeshFormat::AttributeType::kFloat4PackedInTwoFloats;
    case proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_THREE_FLOATS:
      return MeshFormat::AttributeType::kFloat4PackedInThreeFloats;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.MeshFormat.AttributeType value: ", type_proto));
  }
}

proto::MeshFormat::AttributeId EncodeMeshAttributeId(
    MeshFormat::AttributeId id) {
  switch (id) {
    case MeshFormat::AttributeId::kPosition:
      return proto::MeshFormat::ATTR_ID_POSITION;
    case MeshFormat::AttributeId::kColorShiftHsl:
      return proto::MeshFormat::ATTR_ID_COLOR_SHIFT_HSL;
    case MeshFormat::AttributeId::kOpacityShift:
      return proto::MeshFormat::ATTR_ID_OPACITY_SHIFT;
    case MeshFormat::AttributeId::kTexture:
      return proto::MeshFormat::ATTR_ID_TEXTURE;
    case MeshFormat::AttributeId::kSideDerivative:
      return proto::MeshFormat::ATTR_ID_SIDE_DERIVATIVE;
    case MeshFormat::AttributeId::kSideLabel:
      return proto::MeshFormat::ATTR_ID_SIDE_LABEL;
    case MeshFormat::AttributeId::kForwardDerivative:
      return proto::MeshFormat::ATTR_ID_FORWARD_DERIVATIVE;
    case MeshFormat::AttributeId::kForwardLabel:
      return proto::MeshFormat::ATTR_ID_FORWARD_LABEL;
    case MeshFormat::AttributeId::kSurfaceUv:
      return proto::MeshFormat::ATTR_ID_SURFACE_UV;
    case MeshFormat::AttributeId::kAnimationOffset:
      return proto::MeshFormat::ATTR_ID_ANIMATION_OFFSET;
    case MeshFormat::AttributeId::kCustom0:
      return proto::MeshFormat::ATTR_ID_CUSTOM0;
    case MeshFormat::AttributeId::kCustom1:
      return proto::MeshFormat::ATTR_ID_CUSTOM1;
    case MeshFormat::AttributeId::kCustom2:
      return proto::MeshFormat::ATTR_ID_CUSTOM2;
    case MeshFormat::AttributeId::kCustom3:
      return proto::MeshFormat::ATTR_ID_CUSTOM3;
    case MeshFormat::AttributeId::kCustom4:
      return proto::MeshFormat::ATTR_ID_CUSTOM4;
    case MeshFormat::AttributeId::kCustom5:
      return proto::MeshFormat::ATTR_ID_CUSTOM5;
    case MeshFormat::AttributeId::kCustom6:
      return proto::MeshFormat::ATTR_ID_CUSTOM6;
    case MeshFormat::AttributeId::kCustom7:
      return proto::MeshFormat::ATTR_ID_CUSTOM7;
    case MeshFormat::AttributeId::kCustom8:
      return proto::MeshFormat::ATTR_ID_CUSTOM8;
    case MeshFormat::AttributeId::kCustom9:
      return proto::MeshFormat::ATTR_ID_CUSTOM9;
  }
  ABSL_LOG(FATAL) << "Invalid AttributeId value: " << static_cast<int>(id);
}

absl::StatusOr<MeshFormat::AttributeId> DecodeMeshAttributeId(
    proto::MeshFormat::AttributeId id_proto) {
  switch (id_proto) {
    case proto::MeshFormat::ATTR_ID_POSITION:
      return MeshFormat::AttributeId::kPosition;
    case proto::MeshFormat::ATTR_ID_COLOR_SHIFT_HSL:
      return MeshFormat::AttributeId::kColorShiftHsl;
    case proto::MeshFormat::ATTR_ID_OPACITY_SHIFT:
      return MeshFormat::AttributeId::kOpacityShift;
    case proto::MeshFormat::ATTR_ID_TEXTURE:
      return MeshFormat::AttributeId::kTexture;
    case proto::MeshFormat::ATTR_ID_SIDE_DERIVATIVE:
      return MeshFormat::AttributeId::kSideDerivative;
    case proto::MeshFormat::ATTR_ID_SIDE_LABEL:
      return MeshFormat::AttributeId::kSideLabel;
    case proto::MeshFormat::ATTR_ID_FORWARD_DERIVATIVE:
      return MeshFormat::AttributeId::kForwardDerivative;
    case proto::MeshFormat::ATTR_ID_FORWARD_LABEL:
      return MeshFormat::AttributeId::kForwardLabel;
    case proto::MeshFormat::ATTR_ID_SURFACE_UV:
      return MeshFormat::AttributeId::kSurfaceUv;
    case proto::MeshFormat::ATTR_ID_ANIMATION_OFFSET:
      return MeshFormat::AttributeId::kAnimationOffset;
    case proto::MeshFormat::ATTR_ID_CUSTOM0:
      return MeshFormat::AttributeId::kCustom0;
    case proto::MeshFormat::ATTR_ID_CUSTOM1:
      return MeshFormat::AttributeId::kCustom1;
    case proto::MeshFormat::ATTR_ID_CUSTOM2:
      return MeshFormat::AttributeId::kCustom2;
    case proto::MeshFormat::ATTR_ID_CUSTOM3:
      return MeshFormat::AttributeId::kCustom3;
    case proto::MeshFormat::ATTR_ID_CUSTOM4:
      return MeshFormat::AttributeId::kCustom4;
    case proto::MeshFormat::ATTR_ID_CUSTOM5:
      return MeshFormat::AttributeId::kCustom5;
    case proto::MeshFormat::ATTR_ID_CUSTOM6:
      return MeshFormat::AttributeId::kCustom6;
    case proto::MeshFormat::ATTR_ID_CUSTOM7:
      return MeshFormat::AttributeId::kCustom7;
    case proto::MeshFormat::ATTR_ID_CUSTOM8:
      return MeshFormat::AttributeId::kCustom8;
    case proto::MeshFormat::ATTR_ID_CUSTOM9:
      return MeshFormat::AttributeId::kCustom9;
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "invalid ink.proto.MeshFormat.AttributeId value: ", id_proto));
  }
}

}  // namespace

void EncodeMeshFormat(const MeshFormat& format,
                      proto::MeshFormat& format_proto) {
  format_proto.mutable_attribute_types()->Clear();
  format_proto.mutable_attribute_types()->Reserve(format.Attributes().size());
  format_proto.mutable_attribute_ids()->Clear();
  format_proto.mutable_attribute_ids()->Reserve(format.Attributes().size());
  for (MeshFormat::Attribute attribute : format.Attributes()) {
    format_proto.add_attribute_types(EncodeMeshAttributeType(attribute.type));
    format_proto.add_attribute_ids(EncodeMeshAttributeId(attribute.id));
  }
}

absl::StatusOr<MeshFormat> DecodeMeshFormat(
    const proto::MeshFormat& format_proto,
    MeshFormat::IndexFormat index_format) {
  // Validate the number of attributes so that we can safely construct a
  // SmallArray of that size.
  int num_attributes = format_proto.attribute_types_size();
  if (num_attributes > MeshFormat::MaxAttributes()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "too many attributes in MeshFormat proto (has ", num_attributes,
        ", but max is ", MeshFormat::MaxAttributes(), ")"));
  }

  if (format_proto.attribute_ids_size() != num_attributes) {
    return absl::InvalidArgumentError(
        absl::StrCat("attribute count mismatch in MeshFormat proto (has",
                     num_attributes, " attributes types, but ",
                     format_proto.attribute_ids_size(), " attribute IDs)"));
  }

  SmallArray<std::pair<MeshFormat::AttributeType, MeshFormat::AttributeId>,
             mesh_internal::kMaxVertexAttributes>
      attributes(num_attributes);
  for (int i = 0; i < num_attributes; ++i) {
    absl::StatusOr<MeshFormat::AttributeType> attr_type =
        DecodeMeshAttributeType(format_proto.attribute_types(i));
    if (!attr_type.ok()) return attr_type.status();

    absl::StatusOr<MeshFormat::AttributeId> attr_id =
        DecodeMeshAttributeId(format_proto.attribute_ids(i));
    if (!attr_id.ok()) return attr_id.status();

    attributes[i] = {*attr_type, *attr_id};
  }
  return MeshFormat::Create(attributes.Values(), index_format);
}

}  // namespace ink
