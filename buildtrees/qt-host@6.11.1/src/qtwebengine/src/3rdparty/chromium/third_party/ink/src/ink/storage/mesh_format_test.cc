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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/type_matchers.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/storage/proto_matchers.h"

namespace ink {
namespace {

using ::testing::HasSubstr;

TEST(MeshTest, DecodeMeshFormatValid) {
  proto::MeshFormat format_proto;
  format_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED);
  format_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_POSITION);

  absl::StatusOr<MeshFormat> mesh_format = DecodeMeshFormat(
      format_proto, MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(mesh_format.status(), absl::OkStatus());
  EXPECT_THAT(*mesh_format, MeshFormatEq(MeshFormat()));
}

TEST(MeshTest, DecodeMeshFormatWithTooManyAttributes) {
  // Create a MeshFormat proto with (max_attributes + 1) attributes.
  proto::MeshFormat format_proto;
  for (int i = 0; i < MeshFormat::MaxAttributes() + 1; ++i) {
    format_proto.add_attribute_types(
        proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED);
    format_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_POSITION);
  }

  absl::Status status =
      DecodeMeshFormat(format_proto,
                       MeshFormat::IndexFormat::k32BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("too many attributes"));
}

TEST(MeshTest, DecodeMeshFormatWithAttributeCountMismatch) {
  // Create a MeshFormat proto with two attribute types, but only one attribute
  // ID.
  proto::MeshFormat format_proto;
  format_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED);
  format_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT3_UNPACKED);
  format_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_POSITION);

  absl::Status attribute_count_mismatch =
      DecodeMeshFormat(format_proto,
                       MeshFormat::IndexFormat::k32BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(attribute_count_mismatch.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(attribute_count_mismatch.message(),
              HasSubstr("attribute count mismatch"));
}

TEST(MeshTest, DecodeMeshFormatWithInvalidAttributeIdValue) {
  proto::MeshFormat format_proto;
  format_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED);
  format_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_UNSPECIFIED);

  absl::Status invalid_attr_id =
      DecodeMeshFormat(format_proto,
                       MeshFormat::IndexFormat::k32BitUnpacked16BitPacked)
          .status();
  EXPECT_EQ(invalid_attr_id.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(invalid_attr_id.message(),
              HasSubstr("invalid ink.proto.MeshFormat.AttributeId value: 0"));
}

TEST(MeshTest, EncodeMeshFormatClearsExistingProto) {
  proto::MeshFormat format_proto;
  format_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT2_UNPACKED);
  format_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_POSITION);

  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  EncodeMeshFormat(*format, format_proto);

  proto::MeshFormat expected_proto;
  expected_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT4_PACKED_IN_ONE_FLOAT);
  expected_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_COLOR_SHIFT_HSL);
  expected_proto.add_attribute_types(
      proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT);
  expected_proto.add_attribute_ids(proto::MeshFormat::ATTR_ID_POSITION);
  EXPECT_THAT(format_proto, EqualsProto(expected_proto));
}

void DecodeMeshFormatDoesNotCrashOnArbitraryInput(
    const proto::MeshFormat& format_proto,
    MeshFormat::IndexFormat index_format) {
  DecodeMeshFormat(format_proto, index_format).IgnoreError();
}
FUZZ_TEST(MeshTest, DecodeMeshFormatDoesNotCrashOnArbitraryInput)
    .WithDomains(fuzztest::Arbitrary<proto::MeshFormat>(),
                 ArbitraryMeshIndexFormat());

void EncodeMeshFormatRoundTrip(const MeshFormat& format) {
  MeshFormat::IndexFormat index_format = format.GetIndexFormat();
  proto::MeshFormat format_proto;
  EncodeMeshFormat(format, format_proto);
  absl::StatusOr<MeshFormat> mesh_format =
      DecodeMeshFormat(format_proto, index_format);
  ASSERT_EQ(mesh_format.status(), absl::OkStatus());
  EXPECT_THAT(*mesh_format, MeshFormatEq(format));
}
FUZZ_TEST(MeshTest, EncodeMeshFormatRoundTrip)
    .WithDomains(ArbitraryMeshFormat());

}  // namespace
}  // namespace ink
