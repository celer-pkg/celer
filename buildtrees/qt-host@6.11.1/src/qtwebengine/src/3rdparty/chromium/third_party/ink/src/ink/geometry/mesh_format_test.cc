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

#include <optional>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/fuzz_domains.h"

namespace ink {
namespace {

using ::absl_testing::IsOk;
using ::testing::ElementsAre;
using ::testing::FieldsAre;
using ::testing::HasSubstr;

using AttrType = ::ink::MeshFormat::AttributeType;
using AttrId = ::ink::MeshFormat::AttributeId;

TEST(MeshFormatTest, StringifyAttributeType) {
  EXPECT_EQ(absl::StrCat(AttrType::kFloat1Unpacked), "kFloat1Unpacked");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat2Unpacked), "kFloat2Unpacked");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat2PackedInOneFloat),
            "kFloat2PackedInOneFloat");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat3Unpacked), "kFloat3Unpacked");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat3PackedInOneFloat),
            "kFloat3PackedInOneFloat");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat3PackedInTwoFloats),
            "kFloat3PackedInTwoFloats");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat4Unpacked), "kFloat4Unpacked");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat4PackedInOneFloat),
            "kFloat4PackedInOneFloat");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat4PackedInTwoFloats),
            "kFloat4PackedInTwoFloats");
  EXPECT_EQ(absl::StrCat(AttrType::kFloat4PackedInThreeFloats),
            "kFloat4PackedInThreeFloats");
  EXPECT_EQ(absl::StrCat(static_cast<AttrType>(123)), "Invalid(123)");
}

TEST(MeshFormatTest, StringifyAttributeId) {
  EXPECT_EQ(absl::StrCat(AttrId::kPosition), "kPosition");
  EXPECT_EQ(absl::StrCat(AttrId::kColorShiftHsl), "kColorShiftHsl");
  EXPECT_EQ(absl::StrCat(AttrId::kOpacityShift), "kOpacityShift");
  EXPECT_EQ(absl::StrCat(AttrId::kTexture), "kTexture");
  EXPECT_EQ(absl::StrCat(AttrId::kSideDerivative), "kSideDerivative");
  EXPECT_EQ(absl::StrCat(AttrId::kSideLabel), "kSideLabel");
  EXPECT_EQ(absl::StrCat(AttrId::kForwardDerivative), "kForwardDerivative");
  EXPECT_EQ(absl::StrCat(AttrId::kForwardLabel), "kForwardLabel");
  EXPECT_EQ(absl::StrCat(AttrId::kSurfaceUv), "kSurfaceUv");
  EXPECT_EQ(absl::StrCat(AttrId::kAnimationOffset), "kAnimationOffset");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom0), "kCustom0");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom1), "kCustom1");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom2), "kCustom2");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom3), "kCustom3");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom4), "kCustom4");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom5), "kCustom5");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom6), "kCustom6");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom7), "kCustom7");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom8), "kCustom8");
  EXPECT_EQ(absl::StrCat(AttrId::kCustom9), "kCustom9");
  EXPECT_EQ(absl::StrCat(static_cast<AttrId>(255)), "Invalid(255)");
}

TEST(MeshFormatTest, StringifyIndexFormat) {
  EXPECT_EQ(absl::StrCat(MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            "k16BitUnpacked16BitPacked");
  EXPECT_EQ(absl::StrCat(MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
            "k32BitUnpacked16BitPacked");
  EXPECT_EQ(absl::StrCat(static_cast<MeshFormat::IndexFormat>(91)),
            "Invalid(91)");
}

TEST(MeshFormatTest, StringifyMeshFormat) {
  EXPECT_EQ(
      absl::StrCat(MeshFormat()),
      "MeshFormat({{kFloat2Unpacked, kPosition}}, position_attribute_index=0, "
      "index_format=k32BitUnpacked16BitPacked)");
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  EXPECT_EQ(
      absl::StrCat(*format),
      "MeshFormat({{kFloat4PackedInOneFloat, kColorShiftHsl}, "
      "{kFloat2PackedInOneFloat, kPosition}, "
      "{kFloat3PackedInTwoFloats, kCustom0}}, "
      "position_attribute_index=1, index_format=k16BitUnpacked16BitPacked)");
}

TEST(MeshFormatTest, DefaultCtor) {
  MeshFormat format;
  EXPECT_THAT(format.Attributes(),
              ElementsAre(FieldsAre(AttrType::kFloat2Unpacked,
                                    AttrId::kPosition, 0, 0, 8, 8)));
  EXPECT_EQ(format.PositionAttributeIndex(), 0);
  EXPECT_EQ(format.UnpackedVertexStride(), 8);
  EXPECT_EQ(format.PackedVertexStride(), 8);
  EXPECT_EQ(format.GetIndexFormat(),
            MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  EXPECT_EQ(format.UnpackedIndexStride(), 4);
}

TEST(MeshFormatTest, ConstructWithOneAttribute) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  EXPECT_THAT(format->Attributes(),
              ElementsAre(FieldsAre(AttrType::kFloat2PackedInOneFloat,
                                    AttrId::kPosition, 0, 0, 8, 4)));
  EXPECT_EQ(format->PositionAttributeIndex(), 0);
  EXPECT_EQ(format->UnpackedVertexStride(), 8);
  EXPECT_EQ(format->PackedVertexStride(), 4);
  EXPECT_EQ(format->GetIndexFormat(),
            MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  EXPECT_EQ(format->UnpackedIndexStride(), 2);
}

TEST(MeshFormatTest, ConstructWithMultipleAttributes) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  EXPECT_THAT(format->Attributes(),
              ElementsAre(FieldsAre(AttrType::kFloat4PackedInOneFloat,
                                    AttrId::kColorShiftHsl, 0, 0, 16, 4),
                          FieldsAre(AttrType::kFloat2PackedInOneFloat,
                                    AttrId::kPosition, 16, 4, 8, 4),
                          FieldsAre(AttrType::kFloat3PackedInTwoFloats,
                                    AttrId::kCustom0, 24, 8, 12, 8)));
  EXPECT_EQ(format->PositionAttributeIndex(), 1);
  EXPECT_EQ(format->UnpackedVertexStride(), 36);
  EXPECT_EQ(format->PackedVertexStride(), 16);
  EXPECT_EQ(format->GetIndexFormat(),
            MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  EXPECT_EQ(format->UnpackedIndexStride(), 4);
}

TEST(MeshFormatTest, ConstructionErrorEmptyAttributes) {
  absl::Status empty_attributes =
      MeshFormat::Create({}, MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(empty_attributes.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(empty_attributes.message(), HasSubstr("Empty"));
}

TEST(MeshFormatTest, ConstructionErrorTooManyAttributes) {
  std::vector<std::pair<AttrType, AttrId>> attrs(
      MeshFormat::MaxAttributes() + 1,
      std::make_pair(AttrType::kFloat2Unpacked, AttrId::kPosition));
  absl::Status too_many_attributes =
      MeshFormat::Create(absl::MakeSpan(attrs),
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(too_many_attributes.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(too_many_attributes.message(), HasSubstr("Maximum"));
}

TEST(MeshFormatTest, ConstructionErrorNoPosition) {
  absl::Status no_position =
      MeshFormat::Create({{AttrType::kFloat2Unpacked, AttrId::kColorShiftHsl}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(no_position.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(no_position.message(), HasSubstr("Missing a kPosition"));
}

TEST(MeshFormatTest, ConstructionErrorMultiplePositions) {
  absl::Status multiple_positions =
      MeshFormat::Create({{AttrType::kFloat2Unpacked, AttrId::kPosition},
                          {AttrType::kFloat2Unpacked, AttrId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(multiple_positions.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(multiple_positions.message(),
              HasSubstr("more than one kPosition"));
}

TEST(MeshFormatTest, ConstructionErrorMultipleSideLabels) {
  absl::Status multiple_side_labels =
      MeshFormat::Create({{AttrType::kFloat1Unpacked, AttrId::kSideLabel},
                          {AttrType::kFloat2Unpacked, AttrId::kPosition},
                          {AttrType::kFloat1Unpacked, AttrId::kSideLabel}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(multiple_side_labels.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(multiple_side_labels.message(),
              HasSubstr("more than one kSideLabel"));
}

TEST(MeshFormatTest, ConstructionErrorPositionDoesNotHaveTwoComponents) {
  absl::Status position_does_not_have_two_components =
      MeshFormat::Create({{AttrType::kFloat1Unpacked, AttrId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(position_does_not_have_two_components.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(position_does_not_have_two_components.message(),
              HasSubstr("components"));
}

TEST(MeshFormatTest, ConstructionErrorBadAttributeType) {
  absl::Status bad_attr_type =
      MeshFormat::Create({{static_cast<AttrType>(200), AttrId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(bad_attr_type.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(bad_attr_type.message(), HasSubstr("unrecognized AttributeType"));
}

TEST(MeshFormatTest, ConstructionErrorBadAttributeId) {
  absl::Status bad_attr_id =
      MeshFormat::Create(
          {{AttrType::kFloat2Unpacked, static_cast<AttrId>(100)}},
          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(bad_attr_id.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(bad_attr_id.message(), HasSubstr("unrecognized AttributeId"));
}

TEST(MeshFormatTest, WithoutAttributesRemoveOne) {
  absl::StatusOr<MeshFormat> original = MeshFormat::Create(
      {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(original.status(), absl::OkStatus());
  absl::StatusOr<MeshFormat> expected = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(expected.status(), absl::OkStatus());
  absl::StatusOr<MeshFormat> actual =
      original->WithoutAttributes({AttrId::kColorShiftHsl});
  EXPECT_EQ(actual, expected);
}

TEST(MeshFormatTest, WithoutAttributesRemoveMultiple) {
  absl::StatusOr<MeshFormat> original = MeshFormat::Create(
      {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom1}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(original.status(), absl::OkStatus());
  absl::StatusOr<MeshFormat> expected = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom1}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(expected.status(), absl::OkStatus());
  absl::StatusOr<MeshFormat> actual =
      original->WithoutAttributes({AttrId::kCustom0, AttrId::kColorShiftHsl});
  EXPECT_EQ(actual, expected);
}

TEST(MeshFormatTest, WithoutAttributesPosition) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  // You can't ever remove the position attribute.
  absl::Status can_not_remove_position =
      format->WithoutAttributes({AttrId::kPosition}).status();
  EXPECT_EQ(can_not_remove_position.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(can_not_remove_position.message(), HasSubstr("cannot remove"));
}

TEST(MeshFormatTest, WithoutAttributesMissing) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  // You can't remove an attribute that isn't in the original format.
  absl::Status missing_attr_removed =
      format->WithoutAttributes({AttrId::kOpacityShift}).status();
  EXPECT_EQ(missing_attr_removed.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(missing_attr_removed.message(), HasSubstr("cannot remove"));
}

TEST(MeshFormatTest, TotalComponentCount) {
  EXPECT_EQ(MeshFormat().TotalComponentCount(), 2);

  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_THAT(format, IsOk());
  EXPECT_EQ(format->TotalComponentCount(), 4 + 2 + 3);
}

void TotalComponentCountIsAtLeastTwo(MeshFormat format) {
  EXPECT_GE(format.TotalComponentCount(), 2);
}
FUZZ_TEST(MeshFormatTest, TotalComponentCountIsAtLeastTwo)
    .WithDomains(ArbitraryMeshFormat());

TEST(MeshFormatTest, ComponentCount) {
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat1Unpacked), 1);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat2Unpacked), 2);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat2PackedInOneFloat), 2);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat3Unpacked), 3);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat3PackedInOneFloat), 3);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat3PackedInTwoFloats), 3);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat4Unpacked), 4);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat4PackedInOneFloat), 4);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat4PackedInTwoFloats), 4);
  EXPECT_EQ(MeshFormat::ComponentCount(AttrType::kFloat4PackedInThreeFloats),
            4);
}

TEST(MeshFormatTest, PackedBitsPerComponent) {
  EXPECT_EQ(MeshFormat::PackedBitsPerComponent(AttrType::kFloat1Unpacked),
            std::nullopt);
  EXPECT_EQ(MeshFormat::PackedBitsPerComponent(AttrType::kFloat2Unpacked),
            std::nullopt);
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat2PackedInOneFloat)
          .value()
          .Values(),
      ElementsAre(12, 12));
  EXPECT_EQ(MeshFormat::PackedBitsPerComponent(AttrType::kFloat3Unpacked),
            std::nullopt);
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat3PackedInOneFloat)
          .value()
          .Values(),
      ElementsAre(8, 8, 8));
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat3PackedInTwoFloats)
          .value()
          .Values(),
      ElementsAre(16, 16, 16));
  EXPECT_EQ(MeshFormat::PackedBitsPerComponent(AttrType::kFloat4Unpacked),
            std::nullopt);
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat4PackedInOneFloat)
          .value()
          .Values(),
      ElementsAre(6, 6, 6, 6));
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat4PackedInTwoFloats)
          .value()
          .Values(),
      ElementsAre(12, 12, 12, 12));
  EXPECT_THAT(
      MeshFormat::PackedBitsPerComponent(AttrType::kFloat4PackedInThreeFloats)
          .value()
          .Values(),
      ElementsAre(18, 18, 18, 18));
}

TEST(MeshFormatTest, IsUnpackedType) {
  EXPECT_TRUE(MeshFormat::IsUnpackedType(AttrType::kFloat1Unpacked));
  EXPECT_TRUE(MeshFormat::IsUnpackedType(AttrType::kFloat2Unpacked));
  EXPECT_FALSE(MeshFormat::IsUnpackedType(AttrType::kFloat2PackedInOneFloat));
  EXPECT_TRUE(MeshFormat::IsUnpackedType(AttrType::kFloat3Unpacked));
  EXPECT_FALSE(MeshFormat::IsUnpackedType(AttrType::kFloat3PackedInOneFloat));
  EXPECT_FALSE(MeshFormat::IsUnpackedType(AttrType::kFloat3PackedInTwoFloats));
  EXPECT_TRUE(MeshFormat::IsUnpackedType(AttrType::kFloat4Unpacked));
  EXPECT_FALSE(MeshFormat::IsUnpackedType(AttrType::kFloat4PackedInOneFloat));
  EXPECT_FALSE(MeshFormat::IsUnpackedType(AttrType::kFloat4PackedInTwoFloats));
  EXPECT_FALSE(
      MeshFormat::IsUnpackedType(AttrType::kFloat4PackedInThreeFloats));
}

TEST(MeshFormatTest, UnpackedAttributeSize) {
  EXPECT_EQ(MeshFormat::UnpackedAttributeSize(AttrType::kFloat1Unpacked), 4);
  EXPECT_EQ(MeshFormat::UnpackedAttributeSize(AttrType::kFloat2Unpacked), 8);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat2PackedInOneFloat), 8);
  EXPECT_EQ(MeshFormat::UnpackedAttributeSize(AttrType::kFloat3Unpacked), 12);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat3PackedInOneFloat), 12);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat3PackedInTwoFloats),
      12);
  EXPECT_EQ(MeshFormat::UnpackedAttributeSize(AttrType::kFloat4Unpacked), 16);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat4PackedInOneFloat), 16);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat4PackedInTwoFloats),
      16);
  EXPECT_EQ(
      MeshFormat::UnpackedAttributeSize(AttrType::kFloat4PackedInThreeFloats),
      16);
}

TEST(MeshFormatTest, PackedAttributeSize) {
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat1Unpacked), 4);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat2Unpacked), 8);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat2PackedInOneFloat),
            4);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat3Unpacked), 12);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat3PackedInOneFloat),
            4);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat3PackedInTwoFloats),
            8);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat4Unpacked), 16);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat4PackedInOneFloat),
            4);
  EXPECT_EQ(MeshFormat::PackedAttributeSize(AttrType::kFloat4PackedInTwoFloats),
            8);
  EXPECT_EQ(
      MeshFormat::PackedAttributeSize(AttrType::kFloat4PackedInThreeFloats),
      12);
}

void PackedVertexStrideIsAtMostUnpackedVertexStride(MeshFormat format) {
  EXPECT_LE(format.PackedVertexStride(), format.UnpackedVertexStride());
}
FUZZ_TEST(MeshFormatTest, PackedVertexStrideIsAtMostUnpackedVertexStride)
    .WithDomains(ArbitraryMeshFormat());

TEST(MeshFormatTest, UnpackedIndexSize) {
  EXPECT_EQ(MeshFormat::UnpackedIndexSize(
                MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            2);
  EXPECT_EQ(MeshFormat::UnpackedIndexSize(
                MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
            4);
}

TEST(MeshFormatTest, MaxAttributes) {
  EXPECT_EQ(MeshFormat::MaxAttributes(), 16);
}

TEST(MeshFormatDeathTest, BadEnumValues) {
  EXPECT_DEATH_IF_SUPPORTED(
      MeshFormat::ComponentCount(static_cast<AttrType>(90)), "Unrecognized");
  EXPECT_DEATH_IF_SUPPORTED(
      MeshFormat::PackedBitsPerComponent(static_cast<AttrType>(150)),
      "Unrecognized");
  EXPECT_DEATH_IF_SUPPORTED(
      MeshFormat::UnpackedAttributeSize(static_cast<AttrType>(50)),
      "Unrecognized");
  EXPECT_DEATH_IF_SUPPORTED(
      MeshFormat::PackedAttributeSize(static_cast<AttrType>(73)),
      "Unrecognized");
  EXPECT_DEATH_IF_SUPPORTED(
      MeshFormat::UnpackedIndexSize(static_cast<MeshFormat::IndexFormat>(12)),
      "Unrecognized");
}

TEST(MeshFormatTest, IsPackedEquivalent) {
  absl::StatusOr<MeshFormat> original = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(original.status(), absl::OkStatus());

  absl::StatusOr<MeshFormat> exact = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(exact.status(), absl::OkStatus());

  EXPECT_TRUE(MeshFormat::IsPackedEquivalent(*original, *exact));

  absl::StatusOr<MeshFormat> position_type_changed = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2Unpacked, AttrId::kPosition},
          {AttrType::kFloat3Unpacked, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(position_type_changed.status(), absl::OkStatus());

  EXPECT_FALSE(
      MeshFormat::IsPackedEquivalent(*original, *position_type_changed));

  absl::StatusOr<MeshFormat> position_index_changed = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(position_index_changed.status(), absl::OkStatus());

  EXPECT_FALSE(
      MeshFormat::IsPackedEquivalent(*original, *position_index_changed));

  absl::StatusOr<MeshFormat> extra_attribute = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
          {AttrType::kFloat4PackedInThreeFloats, AttrId::kCustom1},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(extra_attribute.status(), absl::OkStatus());

  EXPECT_FALSE(MeshFormat::IsPackedEquivalent(*original, *extra_attribute));

  absl::StatusOr<MeshFormat> missing_attribute = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(missing_attribute.status(), absl::OkStatus());

  EXPECT_FALSE(MeshFormat::IsPackedEquivalent(*original, *missing_attribute));
}

TEST(MeshFormatTest, IsUnpackedEquivalent) {
  absl::StatusOr<MeshFormat> original = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(original.status(), absl::OkStatus());

  absl::StatusOr<MeshFormat> exact = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(exact.status(), absl::OkStatus());
  EXPECT_TRUE(MeshFormat::IsUnpackedEquivalent(*original, *exact));

  absl::StatusOr<MeshFormat> packing_scheme_change = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInTwoFloats, AttrId::kColorShiftHsl},
          {AttrType::kFloat2Unpacked, AttrId::kPosition},
          {AttrType::kFloat3Unpacked, AttrId::kCustom0},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(packing_scheme_change.status(), absl::OkStatus());
  EXPECT_TRUE(
      MeshFormat::IsUnpackedEquivalent(*original, *packing_scheme_change));

  absl::StatusOr<MeshFormat> position_index_changed = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(position_index_changed.status(), absl::OkStatus());
  EXPECT_FALSE(
      MeshFormat::IsUnpackedEquivalent(*original, *position_index_changed));

  absl::StatusOr<MeshFormat> extra_attribute = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
          {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0},
          {AttrType::kFloat4PackedInThreeFloats, AttrId::kCustom1},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(extra_attribute.status(), absl::OkStatus());
  EXPECT_FALSE(MeshFormat::IsUnpackedEquivalent(*original, *extra_attribute));

  absl::StatusOr<MeshFormat> missing_attribute = MeshFormat::Create(
      {
          {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
          {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(missing_attribute.status(), absl::OkStatus());
  EXPECT_FALSE(MeshFormat::IsUnpackedEquivalent(*original, *missing_attribute));
}

TEST(MeshFormatTest, Equality) {
  absl::StatusOr<MeshFormat> original =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kSideDerivative}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(original.status(), absl::OkStatus());

  EXPECT_TRUE(original == original);
  EXPECT_FALSE(original != original);

  absl::StatusOr<MeshFormat> with_different_index_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kSideDerivative}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(with_different_index_format.status(), absl::OkStatus());

  EXPECT_FALSE(original == with_different_index_format);
  EXPECT_TRUE(original != with_different_index_format);

  absl::StatusOr<MeshFormat> with_different_attribute_count =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kSideDerivative},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kSideLabel}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(with_different_attribute_count.status(), absl::OkStatus());

  EXPECT_FALSE(original == with_different_attribute_count);
  EXPECT_TRUE(original != with_different_attribute_count);

  absl::StatusOr<MeshFormat> with_different_attribute_type =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kSideDerivative}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(with_different_attribute_type.status(), absl::OkStatus());

  EXPECT_FALSE(original == with_different_attribute_type);
  EXPECT_TRUE(original != with_different_attribute_type);

  absl::StatusOr<MeshFormat> with_different_attribute_id =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kForwardDerivative}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(with_different_attribute_id.status(), absl::OkStatus());

  EXPECT_FALSE(original == with_different_attribute_id);
  EXPECT_TRUE(original != with_different_attribute_id);
}

TEST(MeshFormatTest, Hash) {
  absl::StatusOr<MeshFormat> with_always_16_bit_indices =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(with_always_16_bit_indices.status(), absl::OkStatus());

  absl::StatusOr<MeshFormat> with_always_unpacked_position =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(with_always_unpacked_position.status(), absl::OkStatus());

  absl::StatusOr<MeshFormat> with_non_position_attributes = MeshFormat::Create(
      {
          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
           MeshFormat::AttributeId::kPosition},
          {MeshFormat::AttributeType::kFloat3Unpacked,
           MeshFormat::AttributeId::kColorShiftHsl},
          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
           MeshFormat::AttributeId::kSideDerivative},
      },
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(with_non_position_attributes.status(), absl::OkStatus());

  absl::StatusOr<MeshFormat> with_non_position_attributes_reordered =
      MeshFormat::Create(
          {
              {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
               MeshFormat::AttributeId::kPosition},
              {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
               MeshFormat::AttributeId::kSideDerivative},
              {MeshFormat::AttributeType::kFloat3Unpacked,
               MeshFormat::AttributeId::kColorShiftHsl},
          },
          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(with_non_position_attributes_reordered.status(), absl::OkStatus());

  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      MeshFormat(),
      *with_always_16_bit_indices,
      *with_always_unpacked_position,
      *with_non_position_attributes,
      *with_non_position_attributes_reordered,
  }));
}

}  // namespace
}  // namespace ink
