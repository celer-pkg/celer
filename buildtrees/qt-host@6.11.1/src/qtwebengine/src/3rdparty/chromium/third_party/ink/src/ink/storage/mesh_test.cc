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

#include "ink/storage/mesh.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/types/iterator_range.h"
#include "google/protobuf/text_format.h"

namespace ink {
namespace {

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::ink::proto::CodedMesh;
using ::google::protobuf::TextFormat;
using ::testing::ElementsAre;
using ::testing::FloatNear;

TEST(MeshTest, EncodeEmptyMesh) {
  // Start with a non-empty CodedMesh.
  CodedMesh coded_mesh;
  ASSERT_TRUE(TextFormat::ParseFromString(R"pb(
                                            triangle_index { deltas: [ 0 ] }
                                            x_stroke_space { deltas: [ 0 ] }
                                            y_stroke_space { deltas: [ 0 ] }
                                          )pb",
                                          &coded_mesh));
  // Encoding an empty mesh should clear the CodedMesh.
  Mesh mesh;
  EncodeMesh(mesh, coded_mesh);
  EXPECT_FALSE(coded_mesh.has_triangle_index());
  EXPECT_FALSE(coded_mesh.has_x_stroke_space());
  EXPECT_FALSE(coded_mesh.has_y_stroke_space());
}

TEST(MeshTest, EncodeTriangleMesh) {
  // Start with a non-empty CodedMesh.
  CodedMesh coded_mesh;
  ASSERT_TRUE(TextFormat::ParseFromString(R"pb(
                                            triangle_index { deltas: [ 0 ] }
                                            x_stroke_space { deltas: [ 17 ] }
                                            y_stroke_space { deltas: [ 42 ] }
                                          )pb",
                                          &coded_mesh));

  std::vector<float> position_x = {1, 3, 5};
  std::vector<float> position_y = {2, 4, 6};
  std::vector<uint32_t> triangles = {0, 1, 2};

  absl::StatusOr<Mesh> mesh =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  EncodeMesh(*mesh, coded_mesh);

  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>> int_run =
        DecodeIntNumericRun(coded_mesh.triangle_index());
    ASSERT_EQ(int_run.status(), absl::OkStatus());
    EXPECT_THAT(*int_run, ElementsAre(0, 1, 2));
  }
  const float epsilon = 1e-3;
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(1, epsilon), FloatNear(3, epsilon),
                            FloatNear(5, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(2, epsilon), FloatNear(4, epsilon),
                            FloatNear(6, epsilon)));
  }
}

TEST(MeshTest, EncodeTriangleMeshOmittingFormat) {
  // Start with a non-empty format.
  CodedMesh coded_mesh;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        format {
          attribute_types: [ ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT ]
          attribute_ids: [ ATTR_ID_POSITION ]
        }
      )pb",
      &coded_mesh));

  absl::StatusOr<Mesh> mesh =
      Mesh::Create(MeshFormat(), {{1, 3, 5}, {2, 4, 6}}, {0, 1, 2});
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  EncodeMeshOmittingFormat(*mesh, coded_mesh);

  EXPECT_FALSE(coded_mesh.has_format());
  absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>> int_run =
      DecodeIntNumericRun(coded_mesh.triangle_index());
  ASSERT_EQ(int_run.status(), absl::OkStatus());
  EXPECT_THAT(*int_run, ElementsAre(0, 1, 2));
}

TEST(MeshTest, EncodePackedTriangleMesh) {
  std::vector<float> position_x = {1, 3, 5};
  std::vector<float> position_y = {2, 4, 6};
  std::vector<uint32_t> triangles = {0, 1, 2};

  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh =
      Mesh::Create(*format, {position_x, position_y}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  CodedMesh coded_mesh;
  EncodeMesh(*mesh, coded_mesh);

  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>> int_run =
        DecodeIntNumericRun(coded_mesh.triangle_index());
    ASSERT_EQ(int_run.status(), absl::OkStatus());
    EXPECT_THAT(*int_run, ElementsAre(0, 1, 2));
  }
  const float epsilon = 1e-3;
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(1, epsilon), FloatNear(3, epsilon),
                            FloatNear(5, epsilon)));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run,
                ElementsAre(FloatNear(2, epsilon), FloatNear(4, epsilon),
                            FloatNear(6, epsilon)));
  }
}

TEST(MeshTest, EncodeSingleVertexMesh) {
  // Create a degenerate mesh that has only one vertex.
  std::vector<float> position_x = {-1.75};
  std::vector<float> position_y = {2.25};
  std::vector<uint32_t> triangles = {};
  absl::StatusOr<Mesh> mesh =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  // Encode the mesh.
  CodedMesh coded_mesh;
  EncodeMesh(*mesh, coded_mesh);
  // The CodedMesh should be valid even though the envelope of vertices is empty
  // (we should avoid calculating an infinite scale).
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.x_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(-1.75f));
  }
  {
    absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
        DecodeFloatNumericRun(coded_mesh.y_stroke_space());
    ASSERT_EQ(float_run.status(), absl::OkStatus());
    EXPECT_THAT(*float_run, ElementsAre(2.25f));
  }
}

TEST(MeshTest, EncodeAndDecodeMeshWithCustomFormat) {
  // Create a mesh format with custom, non-position attributes.
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_THAT(format, IsOk());

  // Create a mesh using that format.
  std::vector<float> position_x = {1, 3, 5};
  std::vector<float> position_y = {2, 4, 6};
  std::vector<float> custom_attr = {-1, 7, 3};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh =
      Mesh::Create(*format, {position_x, position_y, custom_attr}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  // Encode the mesh and ensure that the custom attribute data is included.
  CodedMesh coded_mesh;
  EncodeMesh(*mesh, coded_mesh);
  ASSERT_EQ(coded_mesh.other_attribute_components_size(), 1);
  EXPECT_THAT(DecodeFloatNumericRun(coded_mesh.other_attribute_components(0)),
              IsOkAndHolds(ElementsAre(-1, 7, 3)));

  // Decode the coded mesh, and ensure that the format is the same and that the
  // custom attribute data is still there.
  absl::StatusOr<Mesh> decoded = DecodeMesh(coded_mesh);
  ASSERT_THAT(decoded, IsOk());
  ASSERT_THAT(decoded->Format(), MeshFormatEq(*format));
  ASSERT_EQ(decoded->VertexCount(), 3);
  EXPECT_EQ(decoded->FloatVertexAttribute(0, 1)[0], -1);
  EXPECT_EQ(decoded->FloatVertexAttribute(1, 1)[0], 7);
  EXPECT_EQ(decoded->FloatVertexAttribute(2, 1)[0], 3);
}

TEST(MeshTest, DecodeEmptyMesh) {
  CodedMesh coded_mesh;
  absl::StatusOr<Mesh> mesh = DecodeMesh(coded_mesh);
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  EXPECT_THAT(mesh->VertexCount(), 0);
  EXPECT_THAT(mesh->TriangleCount(), 0);
}

TEST(MeshTest, DecodeTriangleMesh) {
  CodedMesh coded_mesh;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        triangle_index { deltas: [ 0, 1, 1 ] }
        x_stroke_space {
          scale: 2.5
          deltas: [ 1, 2, 1 ]
        }
        y_stroke_space {
          scale: 2.5
          deltas: [ 1, -1, 1 ]
        }
      )pb",
      &coded_mesh));

  absl::StatusOr<Mesh> mesh = DecodeMesh(coded_mesh);
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  ASSERT_EQ(mesh->VertexCount(), 3);
  EXPECT_EQ(mesh->VertexPosition(0), (Point{2.5f, 2.5f}));
  EXPECT_EQ(mesh->VertexPosition(1), (Point{7.5f, 0.0f}));
  EXPECT_EQ(mesh->VertexPosition(2), (Point{10.0f, 2.5f}));

  ASSERT_EQ(mesh->TriangleCount(), 1);
  EXPECT_THAT(mesh->TriangleIndices(0), ElementsAre(0, 1, 2));
}

void DecodeMeshDoesNotCrashOnArbitraryInput(const CodedMesh& coded_mesh) {
  DecodeMesh(coded_mesh).IgnoreError();
}
FUZZ_TEST(MeshTest, DecodeMeshDoesNotCrashOnArbitraryInput);

// TODO: b/294865374 - Replace this test with a less-trivial one once we can
// round-trip a wider class of meshes.
void EncodeEmptyPositionOnlyMeshRoundTrip(
    MeshFormat::AttributeType position_type) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{position_type, MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh = Mesh::Create(*format, {{}, {}}, {});
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  CodedMesh mesh_proto;
  EncodeMesh(*mesh, mesh_proto);

  absl::StatusOr<Mesh> mesh_out = DecodeMesh(mesh_proto);
  ASSERT_EQ(mesh_out.status(), absl::OkStatus());

  EXPECT_THAT(*mesh_out, MeshEq(*mesh));
}
FUZZ_TEST(MeshTest, EncodeEmptyPositionOnlyMeshRoundTrip)
    .WithDomains(PositionMeshAttributeType());

}  // namespace
}  // namespace ink
