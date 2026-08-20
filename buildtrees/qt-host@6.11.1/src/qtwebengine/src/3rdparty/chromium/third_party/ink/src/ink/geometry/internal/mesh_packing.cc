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

#include "ink/geometry/internal/mesh_packing.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/base/nullability.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/types/internal/float.h"
#include "ink/types/small_array.h"

namespace ink::mesh_internal {
namespace {

using ComponentCodingParams = MeshAttributeCodingParams::ComponentCodingParams;

bool ValuesAreFinite(const SmallArray<float, 4>& values) {
  return absl::c_all_of(values.Values(), ink_internal::IsFinite);
}

bool PackedFloatValuesAreFinite(MeshFormat::AttributeType type,
                                absl::Span<const std::byte> packed_bytes) {
  if (!MeshFormat::IsPackedAsFloat(type)) return true;
  SmallArray<float, 4> packed_floats(MeshFormat::PackedAttributeSize(type) /
                                     sizeof(float));

  // Repack the attribute in a SmallArray of floats.
  std::memcpy(packed_floats.Values().data(), packed_bytes.data(),
              MeshFormat::PackedAttributeSize(type));

  return absl::c_all_of(packed_floats.Values(), ink_internal::IsFinite);
}

bool IsValidCodingParams(MeshFormat::AttributeType type,
                         const MeshAttributeCodingParams& coding_params) {
  // Offset and scale are ignored for unpacked types.
  if (MeshFormat::IsUnpackedType(type)) return true;
  if (coding_params.components.Size() != MeshFormat::ComponentCount(type))
    return false;
  return absl::c_all_of(
      coding_params.components.Values(), [](const ComponentCodingParams& c) {
        return std::isfinite(c.offset) && std::isfinite(c.scale) && c.scale > 0;
      });
}

bool UnpackedFloatValuesAreRepresentable(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value) {
  std::optional<SmallArray<uint8_t, 4>> n_bits =
      MeshFormat::PackedBitsPerComponent(type);
  // Any value is valid for an unpacked format.
  if (n_bits == std::nullopt) return true;

  SmallArray<uint32_t, 4> max_values(packing_params.components.Size());
  for (int i = 0; i < packing_params.components.Size(); ++i) {
    max_values[i] = MaxValueForBits(n_bits.value()[i]);
  }

  for (int i = 0; i < packing_params.components.Size(); ++i) {
    float packed =
        std::round((unpacked_value[i] - packing_params.components[i].offset) /
                   packing_params.components[i].scale);
    if (packed < 0 || packed > max_values[i]) return false;
  }
  return true;
}

void PackFloat1PackedInOneUnsignedByte(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte)
                  .value_or(SmallArray<uint8_t, 4>({0})) ==
              (SmallArray<uint8_t, 4>({8})));
  packed_bytes[0] = static_cast<std::byte>(
      PackSingleFloat(packing_params.components[0], unpacked_value[0]));
}

void PackFloat2PackedInOneFloat(const MeshAttributeCodingParams& packing_params,
                                const SmallArray<float, 4>& unpacked_value,
                                absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat2PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
              (SmallArray<uint8_t, 4>({12, 12})));
  float packed_float = {static_cast<float>(packed0 << 12 | packed1)};
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float));
  std::memcpy(packed_bytes.data(), &packed_float, sizeof(float));
}

void PackFloat2PackedInThreeUnsignedBytes_XY12(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12)
          .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
      (SmallArray<uint8_t, 4>({12, 12})));
  packed_bytes[0] = static_cast<std::byte>(packed0 >> 4);
  packed_bytes[1] =
      static_cast<std::byte>(((packed0 & 0xF) << 4) + (packed1 >> 8));
  packed_bytes[2] = static_cast<std::byte>(packed1 & 0xFF);
}

void PackFloat3PackedInFourUnsignedBytes_XYZ10(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10)
          .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
      (SmallArray<uint8_t, 4>({10, 10, 10})));
  packed_bytes[0] = static_cast<std::byte>(packed0 >> 2);
  packed_bytes[1] =
      static_cast<std::byte>(((packed0 & 0x03) << 6) + (packed1 >> 4));
  packed_bytes[2] =
      static_cast<std::byte>(((packed1 & 0x0F) << 4) + (packed2 >> 6));
  packed_bytes[3] = static_cast<std::byte>((packed2 & 0x3F) << 2);
}

void PackFloat2PackedInFourUnsignedBytes_X12_Y20(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20)
          .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
      (SmallArray<uint8_t, 4>({12, 20})));
  packed_bytes[0] = static_cast<std::byte>(packed0 >> 4);
  packed_bytes[1] =
      static_cast<std::byte>(((packed0 & 0xF) << 4) + (packed1 >> 16));
  packed_bytes[2] = static_cast<std::byte>((packed1 & 0xFF00) >> 8);
  packed_bytes[3] = static_cast<std::byte>(packed1 & 0xFF);
}

void PackFloat3PackedInOneFloat(const MeshAttributeCodingParams& packing_params,
                                const SmallArray<float, 4>& unpacked_value,
                                absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat3PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({8, 8, 8})));
  float packed_float = {
      static_cast<float>(packed0 << 16 | packed1 << 8 | packed2)};
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float));
  std::memcpy(packed_bytes.data(), &packed_float, sizeof(float));
}

void PackFloat3PackedInTwoFloats(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat3PackedInTwoFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({16, 16, 16})));
  std::array<float, 2> floats = {
      static_cast<float>(packed0 << 8 | packed1 >> 8),
      static_cast<float>((packed1 & 0x000000FF) << 16 | packed2)};
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float) * 2);
  std::memcpy(packed_bytes.data(), floats.data(), sizeof(float) * 2);
}

void PackFloat4PackedInOneFloat(const MeshAttributeCodingParams& packing_params,
                                const SmallArray<float, 4>& unpacked_value,
                                absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  uint32_t packed3 =
      PackSingleFloat(packing_params.components[3], unpacked_value[3]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({6, 6, 6, 6})));
  float packed_float = {static_cast<float>(packed0 << 18 | packed1 << 12 |
                                           packed2 << 6 | packed3)};
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float));
  std::memcpy(packed_bytes.data(), &packed_float, sizeof(float));
}

void PackFloat4PackedInTwoFloats(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  uint32_t packed3 =
      PackSingleFloat(packing_params.components[3], unpacked_value[3]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInTwoFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({12, 12, 12, 12})));
  std::array<float, 2> floats = {static_cast<float>(packed0 << 12 | packed1),
                                 static_cast<float>(packed2 << 12 | packed3)};
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float) * 2);
  std::memcpy(packed_bytes.data(), floats.data(), sizeof(float) * 2);
}

void PackFloat4PackedInThreeFloats(
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value,
    absl::Span<std::byte> packed_bytes) {
  uint32_t packed0 =
      PackSingleFloat(packing_params.components[0], unpacked_value[0]);
  uint32_t packed1 =
      PackSingleFloat(packing_params.components[1], unpacked_value[1]);
  uint32_t packed2 =
      PackSingleFloat(packing_params.components[2], unpacked_value[2]);
  uint32_t packed3 =
      PackSingleFloat(packing_params.components[3], unpacked_value[3]);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInThreeFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({18, 18, 18, 18})));
  std::array<float, 3> floats = {
      static_cast<float>(packed0 << 6 | packed1 >> 12),
      static_cast<float>((packed1 & 0x00000FFF) << 12 | packed2 >> 6),
      static_cast<float>((packed2 & 0x0000003F) << 18 | packed3),
  };
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float) * 3);
  std::memcpy(packed_bytes.data(), floats.data(), sizeof(float) * 3);
}

}  // namespace

uint32_t PackSingleFloat(const ComponentCodingParams& packing_params,
                         float value) {
  return std::round((value - packing_params.offset) / packing_params.scale);
}

void PackAttribute(MeshFormat::AttributeType type,
                   const MeshAttributeCodingParams& packing_params,
                   const SmallArray<float, 4>& unpacked_value,
                   absl::Span<std::byte> packed_bytes) {
  ABSL_DCHECK(IsValidCodingParams(type, packing_params))
      << "Invalid packing params";
  ABSL_DCHECK_EQ(unpacked_value.Size(), MeshFormat::ComponentCount(type));
  ABSL_DCHECK(ValuesAreFinite(unpacked_value));
  ABSL_DCHECK(UnpackedFloatValuesAreRepresentable(type, packing_params,
                                                  unpacked_value));
  switch (type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat4Unpacked:
      ABSL_DCHECK_EQ(packed_bytes.size(),
                     sizeof(float) * unpacked_value.Size());
      std::memcpy(packed_bytes.data(), unpacked_value.Values().data(),
                  sizeof(float) * unpacked_value.Size());
      break;
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      PackFloat1PackedInOneUnsignedByte(packing_params, unpacked_value,
                                        packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
      PackFloat2PackedInOneFloat(packing_params, unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      PackFloat2PackedInThreeUnsignedBytes_XY12(packing_params, unpacked_value,
                                                packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      PackFloat2PackedInFourUnsignedBytes_X12_Y20(packing_params,
                                                  unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
      PackFloat3PackedInOneFloat(packing_params, unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
      PackFloat3PackedInTwoFloats(packing_params, unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      PackFloat3PackedInFourUnsignedBytes_XYZ10(packing_params, unpacked_value,
                                                packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
      PackFloat4PackedInOneFloat(packing_params, unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
      PackFloat4PackedInTwoFloats(packing_params, unpacked_value, packed_bytes);
      break;
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      PackFloat4PackedInThreeFloats(packing_params, unpacked_value,
                                    packed_bytes);
      break;
  }
}

namespace {

bool PackedFloatValuesAreRepresentable(
    MeshFormat::AttributeType type, absl::Span<const std::byte> packed_bytes) {
  // Nothing to check if the type doesn't pack into floats.
  if (!MeshFormat::IsPackedAsFloat(type)) return true;

  SmallArray<float, 4> packed_floats(MeshFormat::PackedAttributeSize(type) /
                                     sizeof(float));

  // Repack the attribute in a SmallArray of floats.
  std::memcpy(packed_floats.Values().data(), packed_bytes.data(),
              MeshFormat::PackedAttributeSize(type));

  return absl::c_all_of(packed_floats.Values(), [](float f) {
    return 0 <= f && f <= MaxValueForBits(24);
  });
}

float UnpackSingleFloat(const ComponentCodingParams& unpacking_params,
                        uint32_t packed_value) {
  return packed_value * unpacking_params.scale + unpacking_params.offset;
}

SmallArray<float, 4> UnpackFloatsFromFloat1Unpacked(
    absl::Span<const std::byte> packed_bytes) {
  ABSL_DCHECK_EQ(packed_bytes.size(), sizeof(float));
  float unpacked_float;
  std::memcpy(&unpacked_float, packed_bytes.data(), sizeof(float));

  return {unpacked_float};
}

SmallArray<float, 4> UnpackFloatsFromFloat2Unpacked(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 2);
  SmallArray<float, 4> unpacked_floats(2);
  std::memcpy(unpacked_floats.Values().data(), packed_value.data(),
              sizeof(float) * 2);

  return unpacked_floats;
}

SmallArray<float, 4> UnpackFloatsFromFloat3Unpacked(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 3);
  SmallArray<float, 4> unpacked_floats(3);
  std::memcpy(unpacked_floats.Values().data(), packed_value.data(),
              sizeof(float) * 3);

  return unpacked_floats;
}

SmallArray<float, 4> UnpackFloatsFromFloat4Unpacked(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 4);
  SmallArray<float, 4> unpacked_floats(4);
  std::memcpy(unpacked_floats.Values().data(), packed_value.data(),
              sizeof(float) * 4);

  return unpacked_floats;
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat1PackedInOneUnsignedByte(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(
      packed_value.size(),
      MeshFormat::PackedAttributeSize(
          MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte));

  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte)
                  .value_or(SmallArray<uint8_t, 4>({0})) ==
              (SmallArray<uint8_t, 4>({8})));
  return {static_cast<uint32_t>(packed_value[0])};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat2PackedInOneFloat(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float));
  float packed_float;
  std::memcpy(&packed_float, packed_value.data(), sizeof(float));

  uint32_t packed0 = (static_cast<uint32_t>(packed_float) & 0xFFF000) >> 12;
  uint32_t packed1 = (static_cast<uint32_t>(packed_float) & 0x000FFF);

  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat2PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
              (SmallArray<uint8_t, 4>({12, 12})));
  return {packed0, packed1};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat2PackedInThreeUnsignedBytes_XY12(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(
      packed_value.size(),
      MeshFormat::PackedAttributeSize(
          MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12));

  uint32_t packed0 = (static_cast<uint32_t>(packed_value[0]) << 4) +
                     (static_cast<uint32_t>(packed_value[1]) >> 4);

  uint32_t packed1 = ((static_cast<uint32_t>(packed_value[1]) & 0x0F) << 8) +
                     (static_cast<uint32_t>(packed_value[2]));

  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12)
          .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
      (SmallArray<uint8_t, 4>({12, 12})));
  return {packed0, packed1};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat3PackedInFourUnsignedBytes_XYZ10(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(
      packed_value.size(),
      MeshFormat::PackedAttributeSize(
          MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10));

  uint32_t packed0 = (static_cast<uint32_t>(packed_value[0]) << 2) +
                     (static_cast<uint32_t>(packed_value[1]) >> 6);

  uint32_t packed1 = ((static_cast<uint32_t>(packed_value[1]) & 0x3F) << 4) +
                     (static_cast<uint32_t>(packed_value[2]) >> 4);

  uint32_t packed2 = ((static_cast<uint32_t>(packed_value[2]) & 0x0F) << 6) +
                     ((static_cast<uint32_t>(packed_value[3]) & 0xFC) >> 2);

  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10)
          .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
      (SmallArray<uint8_t, 4>({10, 10, 10})));
  return {packed0, packed1, packed2};
}

SmallArray<uint32_t, 4>
UnpackIntegersFromFloat2PackedInFourUnsignedBytes_X12_Y20(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(
      packed_value.size(),
      MeshFormat::PackedAttributeSize(
          MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20));

  uint32_t packed0 = (static_cast<uint32_t>(packed_value[0]) << 4) +
                     (static_cast<uint32_t>(packed_value[1]) >> 4);

  uint32_t packed1 = ((static_cast<uint32_t>(packed_value[1]) & 0xF) << 16) +
                     (static_cast<uint32_t>(packed_value[2]) << 8) +
                     (static_cast<uint32_t>(packed_value[3]));

  ABSL_DCHECK(
      MeshFormat::PackedBitsPerComponent(
          MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20)
          .value_or(SmallArray<uint8_t, 4>({0, 0})) ==
      (SmallArray<uint8_t, 4>({12, 20})));
  return {packed0, packed1};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat3PackedInOneFloat(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float));
  float packed_float;
  std::memcpy(&packed_float, packed_value.data(), sizeof(float));

  uint32_t packed0 = (static_cast<uint32_t>(packed_float) & 0xFF0000) >> 16;
  uint32_t packed1 = (static_cast<uint32_t>(packed_float) & 0x00FF00) >> 8;
  uint32_t packed2 = (static_cast<uint32_t>(packed_float) & 0x0000FF);

  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat3PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({8, 8, 8})));
  return {packed0, packed1, packed2};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat3PackedInTwoFloats(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 2);
  std::array<float, 2> packed_floats;
  std::memcpy(&packed_floats, packed_value.data(), sizeof(float) * 2);

  uint32_t packed0 = (static_cast<uint32_t>(packed_floats[0]) & 0xFFFF00) >> 8;
  uint32_t packed1 = (static_cast<uint32_t>(packed_floats[0]) & 0x0000FF) << 8 |
                     (static_cast<uint32_t>(packed_floats[1]) & 0xFF0000) >> 16;
  uint32_t packed2 = (static_cast<uint32_t>(packed_floats[1]) & 0x00FFFF);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat3PackedInTwoFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({16, 16, 16})));
  return {packed0, packed1, packed2};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat4PackedInOneFloat(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float));
  float packed_float;
  std::memcpy(&packed_float, packed_value.data(), sizeof(float));

  uint32_t packed0 = (static_cast<uint32_t>(packed_float) & 0xFC0000) >> 18;
  uint32_t packed1 = (static_cast<uint32_t>(packed_float) & 0x03F000) >> 12;
  uint32_t packed2 = (static_cast<uint32_t>(packed_float) & 0x000FC0) >> 6;
  uint32_t packed3 = (static_cast<uint32_t>(packed_float) & 0x00003F);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInOneFloat)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({6, 6, 6, 6})));
  return {packed0, packed1, packed2, packed3};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat4PackedInTwoFloats(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 2);
  std::array<float, 2> packed_floats;
  std::memcpy(&packed_floats, packed_value.data(), sizeof(float) * 2);

  uint32_t packed0 = (static_cast<uint32_t>(packed_floats[0]) & 0xFFF000) >> 12;
  uint32_t packed1 = (static_cast<uint32_t>(packed_floats[0]) & 0x000FFF);
  uint32_t packed2 = (static_cast<uint32_t>(packed_floats[1]) & 0xFFF000) >> 12;
  uint32_t packed3 = (static_cast<uint32_t>(packed_floats[1]) & 0x000FFF);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInTwoFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({12, 12, 12, 12})));
  return {packed0, packed1, packed2, packed3};
}

SmallArray<uint32_t, 4> UnpackIntegersFromFloat4PackedInThreeFloats(
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK_EQ(packed_value.size(), sizeof(float) * 3);
  std::array<float, 3> packed_floats;
  std::memcpy(&packed_floats, packed_value.data(), sizeof(float) * 3);

  uint32_t packed0 = (static_cast<uint32_t>(packed_floats[0]) & 0xFFFFC0) >> 6;
  uint32_t packed1 = (static_cast<uint32_t>(packed_floats[0]) & 0x00003F)
                         << 12 |
                     (static_cast<uint32_t>(packed_floats[1]) & 0xFFF000) >> 12;
  uint32_t packed2 = (static_cast<uint32_t>(packed_floats[1]) & 0x000FFF) << 6 |
                     (static_cast<uint32_t>(packed_floats[2]) & 0xFC0000) >> 18;
  uint32_t packed3 = (static_cast<uint32_t>(packed_floats[2]) & 0x03FFFF);
  ABSL_DCHECK(MeshFormat::PackedBitsPerComponent(
                  MeshFormat::AttributeType::kFloat4PackedInThreeFloats)
                  .value_or(SmallArray<uint8_t, 4>({0, 0, 0, 0})) ==
              (SmallArray<uint8_t, 4>({18, 18, 18, 18})));
  return {packed0, packed1, packed2, packed3};
}

}  // namespace

SmallArray<float, 4> UnpackAttribute(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& unpacking_params,
    absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK(IsValidCodingParams(type, unpacking_params))
      << "Invalid unpacking params";
  ABSL_DCHECK(PackedFloatValuesAreFinite(type, packed_value));
  ABSL_DCHECK(PackedFloatValuesAreRepresentable(type, packed_value))
      << "Cannot unpack: Unrepresentable value found";
  ABSL_DCHECK_EQ(packed_value.size(), MeshFormat::PackedAttributeSize(type));
  uint8_t num_components = MeshFormat::ComponentCount(type);

  if (MeshFormat::IsUnpackedType(type)) {
    return ReadFloatsFromUnpackedAttribute(type, packed_value);
  }

  SmallArray<uint32_t, 4> packed_integers =
      UnpackIntegersFromPackedAttribute(type, packed_value);
  ABSL_DCHECK_EQ(packed_integers.Size(), num_components);
  SmallArray<float, 4> unpacked(num_components);
  ABSL_DCHECK_EQ(unpacking_params.components.Size(), num_components);
  for (uint8_t i = 0; i < num_components; ++i) {
    unpacked[i] =
        UnpackSingleFloat(unpacking_params.components[i], packed_integers[i]);
  }
  return unpacked;
}

SmallArray<uint32_t, 4> UnpackIntegersFromPackedAttribute(
    MeshFormat::AttributeType type, absl::Span<const std::byte> packed_value) {
  ABSL_DCHECK(PackedFloatValuesAreFinite(type, packed_value));
  ABSL_DCHECK(PackedFloatValuesAreRepresentable(type, packed_value))
      << "Cannot unpack: Unrepresentable value found";
  switch (type) {
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      return UnpackIntegersFromFloat1PackedInOneUnsignedByte(packed_value);
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
      return UnpackIntegersFromFloat2PackedInOneFloat(packed_value);
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
      return UnpackIntegersFromFloat2PackedInThreeUnsignedBytes_XY12(
          packed_value);
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return UnpackIntegersFromFloat2PackedInFourUnsignedBytes_X12_Y20(
          packed_value);
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
      return UnpackIntegersFromFloat3PackedInOneFloat(packed_value);
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
      return UnpackIntegersFromFloat3PackedInTwoFloats(packed_value);
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return UnpackIntegersFromFloat3PackedInFourUnsignedBytes_XYZ10(
          packed_value);
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
      return UnpackIntegersFromFloat4PackedInOneFloat(packed_value);
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
      return UnpackIntegersFromFloat4PackedInTwoFloats(packed_value);
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return UnpackIntegersFromFloat4PackedInThreeFloats(packed_value);
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat4Unpacked:
      break;
  }
  ABSL_LOG(FATAL) << "Non-packed AttributeType: " << static_cast<uint8_t>(type);
}

SmallArray<float, 4> ReadFloatsFromUnpackedAttribute(
    MeshFormat::AttributeType type, absl::Span<const std::byte> packed_value) {
  switch (type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
      return UnpackFloatsFromFloat1Unpacked(packed_value);
    case MeshFormat::AttributeType::kFloat2Unpacked:
      return UnpackFloatsFromFloat2Unpacked(packed_value);
    case MeshFormat::AttributeType::kFloat3Unpacked:
      return UnpackFloatsFromFloat3Unpacked(packed_value);
    case MeshFormat::AttributeType::kFloat4Unpacked:
      return UnpackFloatsFromFloat4Unpacked(packed_value);
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      break;
  }
  ABSL_LOG(FATAL) << "Packed AttributeType: " << static_cast<uint8_t>(type);
}

std::array<uint32_t, 3> ReadTriangleIndicesFromByteArray(
    uint32_t triangle_index, uint8_t index_stride,
    absl::Span<const std::byte> index_data) {
  ABSL_DCHECK_EQ(index_data.size() % (3 * index_stride), 0);
  size_t offset = triangle_index * size_t{3} * index_stride;
  ABSL_DCHECK_LT(offset, index_data.size()) << "Triangle index out-of-bounds";
  std::array<uint32_t, 3> indices;
  if (index_stride == 2) {
    std::array<uint16_t, 3> indices16;
    std::memcpy(indices16.data(), &index_data[offset], 3 * sizeof(uint16_t));
    absl::c_copy(indices16, indices.begin());
  } else {
    ABSL_DCHECK_EQ(index_stride, 4);
    std::memcpy(indices.data(), &index_data[offset], 3 * sizeof(uint32_t));
  }
  return indices;
}

void WriteTriangleIndicesToByteArray(uint32_t triangle_index,
                                     uint8_t index_stride,
                                     absl::Span<const uint32_t> vertex_indices,
                                     std::vector<std::byte>& index_data) {
  ABSL_DCHECK_EQ(vertex_indices.size(), 3);
  ABSL_DCHECK_EQ(index_data.size() % (3 * index_stride), 0);
  size_t offset = triangle_index * size_t{3} * index_stride;
  ABSL_DCHECK_LT(offset, index_data.size()) << "Triangle index out-of-bounds";
  if (index_stride == 2) {
    std::array<uint16_t, 3> indices16;
    absl::c_copy(vertex_indices, indices16.begin());
    std::memcpy(&index_data[offset], indices16.data(), 3 * sizeof(uint16_t));
  } else {
    ABSL_DCHECK_EQ(index_stride, 4);
    std::memcpy(&index_data[offset], vertex_indices.data(),
                3 * sizeof(uint32_t));
  }
}

namespace {

float UnalignedLoadFloat(const std::byte* absl_nonnull bytes) {
  float value;
  std::memcpy(&value, bytes, sizeof(float));
  return value;
}

}  // namespace

SmallArray<float, 4> ReadUnpackedFloatAttributeFromByteArray(
    uint32_t vertex_index, uint32_t attribute_index,
    absl::Span<const std::byte> vertex_data, const MeshFormat& format) {
  ABSL_DCHECK_EQ(vertex_data.size() % format.UnpackedVertexStride(), 0);
  ABSL_DCHECK_LT(vertex_index,
                 vertex_data.size() / format.UnpackedVertexStride());
  ABSL_DCHECK_LT(attribute_index, format.Attributes().size());
  const MeshFormat::Attribute attr = format.Attributes()[attribute_index];
  const std::byte* src =
      &vertex_data[vertex_index * format.UnpackedVertexStride() +
                   attr.unpacked_offset];
  switch (attr.type) {
    case MeshFormat::AttributeType::kFloat1Unpacked:
    case MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte:
      return {UnalignedLoadFloat(src)};
    case MeshFormat::AttributeType::kFloat2Unpacked:
    case MeshFormat::AttributeType::kFloat2PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12:
    case MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20:
      return {UnalignedLoadFloat(src), UnalignedLoadFloat(src + sizeof(float))};
    case MeshFormat::AttributeType::kFloat3Unpacked:
    case MeshFormat::AttributeType::kFloat3PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat3PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10:
      return {
          UnalignedLoadFloat(src),
          UnalignedLoadFloat(src + sizeof(float)),
          UnalignedLoadFloat(src + 2 * sizeof(float)),
      };
    case MeshFormat::AttributeType::kFloat4Unpacked:
    case MeshFormat::AttributeType::kFloat4PackedInOneFloat:
    case MeshFormat::AttributeType::kFloat4PackedInTwoFloats:
    case MeshFormat::AttributeType::kFloat4PackedInThreeFloats:
      return {
          UnalignedLoadFloat(src),
          UnalignedLoadFloat(src + sizeof(float)),
          UnalignedLoadFloat(src + 2 * sizeof(float)),
          UnalignedLoadFloat(src + 3 * sizeof(float)),
      };
  }
  ABSL_LOG(FATAL) << "Unrecognized AttributeType: " << attr.type;
}

absl::InlinedVector<PartitionInfo, 1> PartitionTriangles(
    absl::Span<const std::byte> index_data,
    MeshFormat::IndexFormat index_format, uint64_t max_vertices_per_partition) {
  uint8_t index_stride = MeshFormat::UnpackedIndexSize(index_format);
  ABSL_DCHECK(index_stride == 2 || index_stride == 4);
  ABSL_DCHECK_EQ(index_data.size() % (3 * index_stride), 0);

  // The number of vertices/triangles to reserve in each partition to avoid
  // repeatedly re-allocating. This was chosen semi-arbitrarily, based on a
  // vague recollection that most strokes have less than 300-400 vertices and
  // triangles.
  constexpr int kReserveSize = 500;

  absl::InlinedVector<PartitionInfo, 1> partitions(1);
  partitions.back().vertex_indices.reserve(kReserveSize);
  partitions.back().triangles.reserve(kReserveSize);
  absl::flat_hash_map<uint32_t, uint32_t> mesh_idx_to_partition_idx;
  mesh_idx_to_partition_idx.reserve(kReserveSize);
  uint32_t n_tris = index_data.size() / index_stride / 3;
  for (uint32_t tri_idx = 0; tri_idx < n_tris; ++tri_idx) {
    std::array<uint32_t, 3> mesh_tri =
        ReadTriangleIndicesFromByteArray(tri_idx, index_stride, index_data);

    // Check if this triangle would put us over the maximum number of vertices
    // for this partition.
    if (partitions.back().vertex_indices.size() >
        max_vertices_per_partition - 3) {
      int n_new_vertices =
          absl::c_count_if(mesh_tri, [&mesh_idx_to_partition_idx](uint32_t i) {
            return !mesh_idx_to_partition_idx.contains(i);
          });
      if (partitions.back().vertex_indices.size() + n_new_vertices >
          max_vertices_per_partition) {
        partitions.emplace_back();
        partitions.back().vertex_indices.reserve(kReserveSize);
        partitions.back().triangles.reserve(kReserveSize);
        mesh_idx_to_partition_idx.clear();
      }
    }

    std::array<uint32_t, 3> partition_tri;
    for (int i = 0; i < 3; ++i) {
      auto insertion_result = mesh_idx_to_partition_idx.insert(
          {mesh_tri[i], partitions.back().vertex_indices.size()});
      if (insertion_result.second) {
        partitions.back().vertex_indices.push_back(mesh_tri[i]);
      }
      partition_tri[i] = insertion_result.first->second;
    }
    partitions.back().triangles.push_back(partition_tri);
  }

  return partitions;
}

absl::StatusOr<MeshAttributeCodingParams> ComputeCodingParams(
    MeshFormat::AttributeType type, const MeshAttributeBounds& bounds) {
  int n_components = MeshFormat::ComponentCount(type);
  std::optional<SmallArray<uint8_t, 4>> bits_per_component =
      MeshFormat::PackedBitsPerComponent(type);
  if (!bits_per_component.has_value()) {
    // This is not a packed type, so we store values as is, with no offset and
    // a scale of 1.
    return MeshAttributeCodingParams{
        .components = SmallArray<ComponentCodingParams, 4>(
            n_components, {.offset = 0, .scale = 1})};
  }

  return ComputeCodingParamsForBitSizes(*bits_per_component, bounds);
}

absl::StatusOr<MeshAttributeCodingParams> ComputeCodingParamsForBitSizes(
    SmallArray<uint8_t, 4> bits_per_component,
    const MeshAttributeBounds& bounds) {
  uint8_t n_components = bits_per_component.Size();

  // Consistency check -- should be guaranteed by the logic in `Mesh` and
  // `MutableMesh`.
  ABSL_CHECK_EQ(bounds.minimum.Size(), n_components);
  ABSL_CHECK_EQ(bounds.maximum.Size(), n_components);

  MeshAttributeCodingParams coding_params{
      .components = SmallArray<ComponentCodingParams, 4>(n_components)};

  SmallArray<uint32_t, 4> max_values(n_components);
  for (int i = 0; i < n_components; ++i) {
    max_values[i] = MaxValueForBits(bits_per_component[i]);
  }

  for (int i = 0; i < n_components; ++i) {
    ABSL_CHECK_LE(bounds.minimum[i], bounds.maximum[i]);
    coding_params.components[i].offset = bounds.minimum[i];
    float range = bounds.maximum[i] - bounds.minimum[i];
    if (!std::isfinite(range))
      return absl::InvalidArgumentError(
          "Failed to compute coding params: range of values exceeds float "
          "precision");

    // If the min and max are the same, we would get a scale of zero, which
    // wouldn't make sense, since multiplying by zero is not invertible --
    // so in that case we set the scale to 1 instead.
    coding_params.components[i].scale = range > 0 ? range / max_values[i] : 1;
  }

  return coding_params;
}

absl::StatusOr<CodingParamsArray> ComputeCodingParamsArray(
    const MeshFormat& format, const AttributeBoundsArray& bounds,
    absl::Span<const std::optional<MeshAttributeCodingParams>>
        custom_coding_params_array) {
  size_t n_attrs = format.Attributes().size();
  if (bounds.Size() != n_attrs) {
    return absl::InvalidArgumentError(
        absl::Substitute("Size mismatch: `format` has $0 attributes, but "
                         "`bounds` has $1 elements",
                         n_attrs, bounds.Size()));
  }
  if (!custom_coding_params_array.empty()) {
    if (custom_coding_params_array.size() != n_attrs) {
      return absl::InvalidArgumentError(
          absl::Substitute("Wrong number of coding params for format; "
                           "attributes = $0, coding params = $1",
                           n_attrs, custom_coding_params_array.size()));
    }
    for (size_t i = 0; i < n_attrs; ++i) {
      if (!custom_coding_params_array[i].has_value()) continue;

      MeshFormat::AttributeType type = format.Attributes()[i].type;
      if (MeshFormat::IsUnpackedType(type)) {
        return absl::InvalidArgumentError(
            absl::Substitute("Coding params specified for attribute at index "
                             "$0, but the attribute is unpacked",
                             i));
      }

      const MeshAttributeCodingParams& params = *custom_coding_params_array[i];
      if (!IsValidCodingParams(type, params)) {
        return absl::InvalidArgumentError(absl::Substitute(
            "Coding params at index $0 is not valid for format", i));
      }

      if (!UnpackedFloatValuesAreRepresentable(type, params,
                                               bounds[i].minimum) ||
          !UnpackedFloatValuesAreRepresentable(type, params,
                                               bounds[i].maximum)) {
        return absl::InvalidArgumentError(
            absl::Substitute("Coding params at index $0 cannot represent all "
                             "values of its attribute",
                             i));
      }
    }
  }

  CodingParamsArray coding_params_array(n_attrs);
  for (size_t attr_idx = 0; attr_idx < n_attrs; ++attr_idx) {
    MeshFormat::AttributeType type = format.Attributes()[attr_idx].type;
    if (!custom_coding_params_array.empty() &&
        custom_coding_params_array[attr_idx].has_value() &&
        !MeshFormat::IsUnpackedType(type)) {
      coding_params_array[attr_idx] = *custom_coding_params_array[attr_idx];
    } else {
      auto result = mesh_internal::ComputeCodingParams(type, bounds[attr_idx]);
      if (!result.ok()) {
        return result.status();
      }
      coding_params_array[attr_idx] = *result;
    }
  }

  return coding_params_array;
}

std::vector<std::byte> CopyAndPackPartitionVertices(
    absl::Span<const std::byte> unpacked_vertex_data,
    absl::Span<const uint32_t> partition_vertex_indices,
    const MeshFormat& original_format,
    const absl::flat_hash_set<MeshFormat::AttributeId>& omit_set,
    const CodingParamsArray& packing_params_array,
    const absl::flat_hash_map<uint32_t, Point>& override_vertex_positions) {
  // These should all be guaranteed by logic in `MutableMesh`.
  ABSL_CHECK(!unpacked_vertex_data.empty()) << "Vertex data is empty";
  ABSL_CHECK(!partition_vertex_indices.empty()) << "Partition is empty";
  ABSL_CHECK_EQ(
      unpacked_vertex_data.size() % original_format.UnpackedVertexStride(), 0u)
      << "Vertex data is not divisible by vertex stride";

  uint32_t n_original_vertices =
      unpacked_vertex_data.size() / original_format.UnpackedVertexStride();
  absl::Span<const MeshFormat::Attribute> original_attrs =
      original_format.Attributes();

  // These should also be guaranteed by logic in `MutableMesh`.
  size_t n_original_attrs = original_attrs.size();
  ABSL_CHECK_GT(n_original_attrs, omit_set.size());
  size_t n_packed_attrs = n_original_attrs - omit_set.size();
  ABSL_CHECK_EQ(packing_params_array.Size(), n_packed_attrs)
      << "Wrong number of packing params";
  // This one we only DCHECK, for performance reasons.
  ABSL_DCHECK_LT(*absl::c_max_element(partition_vertex_indices),
                 n_original_vertices)
      << "Partition refers to non-existent vertex";

  size_t packed_vertex_stride = 0;
  std::vector<size_t> packed_attribute_offsets;
  packed_attribute_offsets.reserve(n_packed_attrs);
  for (MeshFormat::Attribute original_attr : original_attrs) {
    if (omit_set.contains(original_attr.id)) continue;
    packed_attribute_offsets.push_back(packed_vertex_stride);
    packed_vertex_stride += original_attr.packed_width;
  }

  // Returns the unpacked attribute value read from `unpacked_vertex_data` or
  // the override position, if applicable.
  auto get_unpacked_attribute_value =
      [&](uint32_t original_vertex_index,
          int original_attribute_index) -> SmallArray<float, 4> {
    if (original_attribute_index == original_format.PositionAttributeIndex()) {
      auto override_it = override_vertex_positions.find(original_vertex_index);
      if (override_it != override_vertex_positions.end()) {
        return {override_it->second.x, override_it->second.y};
      }
    }
    return ReadUnpackedFloatAttributeFromByteArray(
        original_vertex_index, original_attribute_index, unpacked_vertex_data,
        original_format);
  };

  std::vector<std::byte> partition_vertex_data(partition_vertex_indices.size() *
                                               packed_vertex_stride);
  for (uint32_t partition_vertex_idx = 0;
       partition_vertex_idx < partition_vertex_indices.size();
       ++partition_vertex_idx) {
    uint32_t original_vertex_idx =
        partition_vertex_indices[partition_vertex_idx];
    for (size_t original_attr_idx = 0, packed_attr_idx = 0;
         original_attr_idx < n_original_attrs; ++original_attr_idx) {
      MeshFormat::Attribute original_attr = original_attrs[original_attr_idx];
      if (omit_set.contains(original_attr.id)) continue;
      absl::Span<std::byte> packed_value = absl::MakeSpan(
          &partition_vertex_data[partition_vertex_idx * packed_vertex_stride +
                                 packed_attribute_offsets[packed_attr_idx]],
          original_attr.packed_width);
      mesh_internal::PackAttribute(
          original_attr.type, packing_params_array[packed_attr_idx],
          get_unpacked_attribute_value(original_vertex_idx, original_attr_idx),
          packed_value);
      ++packed_attr_idx;
    }
  }

  return partition_vertex_data;
}

}  // namespace ink::mesh_internal
