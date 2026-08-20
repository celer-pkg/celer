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

#include "ink/geometry/partitioned_mesh.h"

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/type_matchers.h"
#include "ink/storage/mesh_format.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/partitioned_mesh.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/types/iterator_range.h"
#include "google/protobuf/text_format.h"

namespace ink {
namespace {

using ::ink::proto::CodedModeledShape;
using ::google::protobuf::TextFormat;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::SizeIs;

MATCHER_P2(VertexIndexPairEq, mesh_index, vertex_index, "") {
  return arg.mesh_index == mesh_index && arg.vertex_index == vertex_index;
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithGroupCountMismatch) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_outline_indices(0);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("group"));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithNonzeroFirstMeshIndex) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(1);
  shape_proto.add_group_first_outline_indices(0);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("start with zero"));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithNonzeroFirstOutlineIndex) {
  CodedModeledShape shape_proto;
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_outline_indices(1);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("start with zero"));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithOutOfBoundsMeshIndex) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_mesh_indices(3);
  shape_proto.add_group_first_outline_indices(0);
  shape_proto.add_group_first_outline_indices(0);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("meshes_size"));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithOutOfBoundsOutlineIndex) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_mesh_indices(1);
  shape_proto.add_group_first_outline_indices(0);
  shape_proto.add_group_first_outline_indices(1);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("outlines_size"));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithDecreasingFirstMeshIndices) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_mesh_indices(2);
  shape_proto.add_group_first_mesh_indices(1);
  shape_proto.add_group_first_outline_indices(0);
  shape_proto.add_group_first_outline_indices(0);
  shape_proto.add_group_first_outline_indices(0);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(), HasSubstr("group_first_mesh_indices"));
}

TEST(PartitionedMeshTest,
     DecodePartitionedMeshWithDecreasingFirstOutlineIndices) {
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  shape_proto.add_meshes();
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  EncodeMeshFormat(MeshFormat(), *shape_proto.add_group_formats());
  shape_proto.add_group_first_mesh_indices(0);
  shape_proto.add_group_first_mesh_indices(1);
  shape_proto.add_group_first_mesh_indices(2);
  shape_proto.add_group_first_outline_indices(0);
  shape_proto.add_group_first_outline_indices(2);
  shape_proto.add_group_first_outline_indices(1);

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  EXPECT_EQ(shape.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(shape.status().message(),
              HasSubstr("group_first_outline_indices"));
}

TEST(PartitionedMeshTest, EncodeEmptyPartitionedMesh) {
  // Start with a non-empty proto.
  CodedModeledShape shape_proto;
  shape_proto.add_meshes();
  shape_proto.add_outlines();

  EncodePartitionedMesh(PartitionedMesh(), shape_proto);

  // The previous proto contents should be overwritten.
  EXPECT_THAT(shape_proto.meshes(), IsEmpty());
  EXPECT_THAT(shape_proto.outlines(), IsEmpty());
}

TEST(PartitionedMeshTest, DecodeEmptyCodedModeledShape) {
  absl::StatusOr<PartitionedMesh> shape =
      DecodePartitionedMesh(CodedModeledShape());
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_THAT(*shape, PartitionedMeshDeepEq(PartitionedMesh()));
}

TEST(PartitionedMeshTest, EncodePartitionedMeshWithOneTriangleMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  absl::StatusOr<Mesh> mesh =
      Mesh::Create(*format, {{10, 30, 20}, {5, 5, 25}}, {0, 1, 2});
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  std::vector<PartitionedMesh::VertexIndexPair> outline = {
      {0, 0}, {0, 1}, {0, 2}};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(&(*mesh), 1), {outline});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  CodedModeledShape shape_proto;
  EncodePartitionedMesh(*shape, shape_proto);

  ASSERT_EQ(shape_proto.group_formats_size(), 1u);
  EXPECT_THAT(
      shape_proto.group_formats(0).attribute_types(),
      ElementsAre(proto::MeshFormat::ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT));
  ASSERT_EQ(shape_proto.meshes_size(), 1);
  ASSERT_EQ(shape_proto.outlines_size(), 1);
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
      DecodeFloatNumericRun(shape_proto.outlines(0));
  ASSERT_EQ(float_run.status(), absl::OkStatus());
  EXPECT_THAT(*float_run, ElementsAre(0, 1, 2));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithOneTriangleMesh) {
  CodedModeledShape shape_proto;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        group_formats {
          attribute_types: [ ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT ]
          attribute_ids: [ ATTR_ID_POSITION ]
        }
        group_first_mesh_indices: [ 0 ]
        group_first_outline_indices: [ 0 ]
        meshes {
          x_stroke_space { deltas: [ 10, 20, -10 ] }
          y_stroke_space { deltas: [ 5, 0, 20 ] }
          triangle_index { deltas: [ 0, 1, 1 ] }
        }
        outlines { deltas: [ 0, 1, 1 ] }
      )pb",
      &shape_proto));

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_EQ(shape->RenderGroupCount(), 1);
  const MeshFormat& format = shape->RenderGroupFormat(0);
  ASSERT_THAT(format.Attributes(), SizeIs(1));
  EXPECT_EQ(format.Attributes()[0].type,
            MeshFormat::AttributeType::kFloat2PackedInOneFloat);
  ASSERT_THAT(shape->Meshes(), SizeIs(1));
  EXPECT_EQ(shape->Meshes()[0].VertexCount(), 3);
  EXPECT_EQ(shape->Meshes()[0].TriangleCount(), 1);
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  ASSERT_EQ(shape->OutlineCount(0), 1u);
  EXPECT_THAT(shape->Outline(0, 0),
              ElementsAre(VertexIndexPairEq(0, 0), VertexIndexPairEq(0, 1),
                          VertexIndexPairEq(0, 2)));
}

TEST(PartitionedMeshTest, EncodePartitionedMeshWithTwoTriangleMeshes) {
  MeshFormat format;

  // Create two meshes, each with one triangle, that together form a square.
  absl::StatusOr<Mesh> mesh0 =
      Mesh::Create(format, {{0, 1, 1}, {0, 0, 1}}, {0, 1, 2});
  ASSERT_EQ(mesh0.status(), absl::OkStatus());

  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(format, {{1, 0, 0}, {1, 1, 0}}, {0, 1, 2});
  ASSERT_EQ(mesh1.status(), absl::OkStatus());

  std::vector<Mesh> meshes;
  meshes.push_back(*std::move(mesh0));
  meshes.push_back(*std::move(mesh1));
  // The outline goes around the square, with two vertices from each mesh.
  std::vector<PartitionedMesh::VertexIndexPair> outline = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1}};

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes), {outline});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  CodedModeledShape shape_proto;
  EncodePartitionedMesh(*shape, shape_proto);

  ASSERT_EQ(shape_proto.meshes_size(), 2);
  ASSERT_EQ(shape_proto.outlines_size(), 1);
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>> float_run =
      DecodeFloatNumericRun(shape_proto.outlines(0));
  ASSERT_EQ(float_run.status(), absl::OkStatus());
  EXPECT_THAT(*float_run, ElementsAre(0x00000, 0x00001, 0x10000, 0x10001));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithTwoTriangleMeshes) {
  CodedModeledShape shape_proto;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        meshes {
          x_stroke_space { deltas: [ 0, 1, 0 ] }
          y_stroke_space { deltas: [ 0, 0, 1 ] }
          triangle_index { deltas: [ 0, 1, 1 ] }
        }
        meshes {
          x_stroke_space { deltas: [ 1, -1, 0 ] }
          y_stroke_space { deltas: [ 1, 0, -1 ] }
          triangle_index { deltas: [ 0, 1, 1 ] }
        }
        outlines { deltas: [ 0, 1, 65535, 1 ] }
      )pb",
      &shape_proto));

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_EQ(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(MeshFormat()));
  ASSERT_THAT(shape->Meshes(), SizeIs(2));
  EXPECT_EQ(shape->Meshes()[0].VertexCount(), 3);
  EXPECT_EQ(shape->Meshes()[0].TriangleCount(), 1);
  EXPECT_EQ(shape->Meshes()[1].VertexCount(), 3);
  EXPECT_EQ(shape->Meshes()[1].TriangleCount(), 1);
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  ASSERT_EQ(shape->OutlineCount(0), 1u);
  EXPECT_THAT(shape->Outline(0, 0),
              ElementsAre(VertexIndexPairEq(0, 0), VertexIndexPairEq(0, 1),
                          VertexIndexPairEq(1, 0), VertexIndexPairEq(1, 1)));
}

TEST(PartitionedMeshTest, EncodePartitionedMeshWithTwoGroups) {
  absl::StatusOr<Mesh> mesh0 =
      Mesh::Create(MeshFormat(), {{0, 1, 1}, {0, 0, 1}}, {0, 1, 2});
  ASSERT_EQ(mesh0.status(), absl::OkStatus());

  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {{1, 0, 0}, {1, 1, 0}}, {0, 1, 2});
  ASSERT_EQ(mesh1.status(), absl::OkStatus());

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshGroups(
      {{.meshes = absl::MakeConstSpan(&*mesh0, 1)},
       {.meshes = absl::MakeConstSpan(&*mesh1, 1)}});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  CodedModeledShape shape_proto;
  EncodePartitionedMesh(*shape, shape_proto);

  EXPECT_EQ(shape_proto.meshes_size(), 2u);
  EXPECT_EQ(shape_proto.outlines_size(), 0u);
  EXPECT_EQ(shape_proto.group_formats_size(), 2u);
  EXPECT_THAT(shape_proto.group_first_mesh_indices(), ElementsAre(0, 1));
  EXPECT_THAT(shape_proto.group_first_outline_indices(), ElementsAre(0, 0));
}

TEST(PartitionedMeshTest, DecodePartitionedMeshWithTwoGroups) {
  CodedModeledShape shape_proto;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        meshes {
          x_stroke_space { deltas: [ 0, 1, 0 ] }
          y_stroke_space { deltas: [ 0, 0, 1 ] }
          triangle_index { deltas: [ 0, 1, 1 ] }
        }
        meshes {
          x_stroke_space { deltas: [ 1, -1, 0 ] }
          y_stroke_space { deltas: [ 1, 0, -1 ] }
          triangle_index { deltas: [ 0, 1, 1 ] }
        }
        outlines { deltas: [ 0, 1, 1 ] }
        outlines { deltas: [ 0, 2, -1 ] }
        group_formats {
          attribute_types: [ ATTR_TYPE_FLOAT2_PACKED_IN_ONE_FLOAT ]
          attribute_ids: [ ATTR_ID_POSITION ]
        }
        group_formats {
          attribute_types: [ ATTR_TYPE_FLOAT2_UNPACKED ]
          attribute_ids: [ ATTR_ID_POSITION ]
        }
        group_first_mesh_indices: [ 0, 1 ]
        group_first_outline_indices: [ 0, 1 ]
      )pb",
      &shape_proto));

  absl::StatusOr<PartitionedMesh> shape = DecodePartitionedMesh(shape_proto);
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_EQ(shape->RenderGroupCount(), 2u);
  ASSERT_THAT(shape->RenderGroupFormat(0).Attributes(), SizeIs(1));
  EXPECT_EQ(shape->RenderGroupFormat(0).Attributes()[0].type,
            MeshFormat::AttributeType::kFloat2PackedInOneFloat);
  ASSERT_THAT(shape->RenderGroupFormat(1).Attributes(), SizeIs(1));
  EXPECT_EQ(shape->RenderGroupFormat(1).Attributes()[0].type,
            MeshFormat::AttributeType::kFloat2Unpacked);
  ASSERT_THAT(shape->RenderGroupMeshes(0), SizeIs(1));
  EXPECT_EQ(shape->RenderGroupMeshes(0)[0].VertexCount(), 3);
  EXPECT_EQ(shape->RenderGroupMeshes(0)[0].TriangleCount(), 1);
  ASSERT_THAT(shape->RenderGroupMeshes(1), SizeIs(1));
  EXPECT_EQ(shape->RenderGroupMeshes(1)[0].VertexCount(), 3);
  EXPECT_EQ(shape->RenderGroupMeshes(1)[0].TriangleCount(), 1);
  ASSERT_EQ(shape->OutlineCount(0), 1u);
  EXPECT_THAT(shape->Outline(0, 0),
              ElementsAre(VertexIndexPairEq(0, 0), VertexIndexPairEq(0, 1),
                          VertexIndexPairEq(0, 2)));
  ASSERT_EQ(shape->OutlineCount(1), 1u);
  EXPECT_THAT(shape->Outline(1, 0),
              ElementsAre(VertexIndexPairEq(0, 0), VertexIndexPairEq(0, 2),
                          VertexIndexPairEq(0, 1)));
}

void DecodePartitionedMeshDoesNotCrashOnArbitraryInput(
    const CodedModeledShape& shape_proto) {
  DecodePartitionedMesh(shape_proto).IgnoreError();
}
FUZZ_TEST(PartitionedMeshTest,
          DecodePartitionedMeshDoesNotCrashOnArbitraryInput);

// TODO: b/294865374 - Once we can reliably round-trip arbitrary packed-format
// meshes through serialization, add a fuzz test for round-tripping arbitrary
// packed-format PartitionedMesh instances.

}  // namespace
}  // namespace ink
