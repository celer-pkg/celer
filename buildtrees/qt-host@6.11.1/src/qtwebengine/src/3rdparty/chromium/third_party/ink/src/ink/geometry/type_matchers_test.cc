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

#include "ink/geometry/type_matchers.h"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;

using AttrType = ::ink::MeshFormat::AttributeType;
using AttrId = ::ink::MeshFormat::AttributeId;

TEST(PointNearTest, DifferentTolerances) {
  EXPECT_THAT((Point{3, 2}), PointNear({2.9, 2.1}, 0.15, 0.15));
  EXPECT_THAT((Point{3, 2}), Not(PointNear({2.9, 2.1}, 0.05, 0.15)));
  EXPECT_THAT((Point{3, 2}), Not(PointNear({2.9, 2.1}, 0.15, 0.05)));
}

TEST(TriangleEqTest, Equal) {
  Triangle triangle = {.p0 = {1, 2}, .p1 = {3, 4}, .p2 = {5, 6}};
  EXPECT_THAT(triangle, TriangleEq(triangle));
}

TEST(TriangleEqTest, Unequal) {
  Triangle a = {.p0 = {1, 2}, .p1 = {3, 4}, .p2 = {5, 6}};

  Triangle b = a;
  b.p0.x += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));

  b = a;
  b.p0.y += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));

  b = a;
  b.p1.x += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));

  b = a;
  b.p1.y += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));

  b = a;
  b.p2.x += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));

  b = a;
  b.p2.y += 1;
  EXPECT_THAT(b, Not(TriangleEq(a)));
}
TEST(QuadEqTest, Equal) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 7.0f}, 8.0f, 6.0f, kQuarterTurn, 0.8f);
  EXPECT_THAT(quad, QuadEq(quad));
}

TEST(QuadEqTest, Unequal) {
  Quad base_quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 7.0f}, 8.0f, 6.0f, kQuarterTurn, 0.8f);

  Quad changed_center = base_quad;
  changed_center.SetCenter({1.0f, 1.0f});
  EXPECT_THAT(base_quad, Not(QuadEq(changed_center)));
  EXPECT_THAT(changed_center, Not(QuadEq(base_quad)));

  Quad changed_width = base_quad;
  changed_width.SetWidth(-1.99f);
  EXPECT_THAT(base_quad, Not(QuadEq(changed_width)));
  EXPECT_THAT(changed_width, Not(QuadEq(base_quad)));

  Quad changed_height = base_quad;
  changed_height.SetHeight(-1.99f);
  EXPECT_THAT(base_quad, Not(QuadEq(changed_height)));
  EXPECT_THAT(changed_height, Not(QuadEq(base_quad)));

  Quad changed_rotation = base_quad;
  changed_rotation.SetRotation(kFullTurn);
  EXPECT_THAT(base_quad, Not(QuadEq(changed_rotation)));
  EXPECT_THAT(changed_rotation, Not(QuadEq(base_quad)));

  Quad changed_shear = base_quad;
  changed_shear.SetShearFactor(0.23f);
  EXPECT_THAT(base_quad, Not(QuadEq(changed_shear)));
  EXPECT_THAT(changed_shear, Not(QuadEq(base_quad)));
}

TEST(TriangleNearTest, Equal) {
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              TriangleNear({{1, 2}, {3, 4}, {5, 6}}, 1));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              TriangleNear({{1.2, 1.9}, {2.6, 4.3}, {5.4, 5.6}}, 0.5));
}

TEST(TriangleNearTest, NotEqual) {
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              Not(TriangleNear({{1.1, 2}, {3, 4}, {5, 6}}, 0.05)));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              Not(TriangleNear({{1, 1.9}, {3, 4}, {5, 6}}, 0.05)));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              Not(TriangleNear({{1, 2}, {2.8, 4}, {5, 6}}, 0.05)));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 6}}),
              Not(TriangleNear({{1, 2}, {3, 4.1}, {5, 6}}, 0.05)));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5.1, 6}}),
              Not(TriangleNear({{1, 2}, {3, 4}, {5, 6}}, 0.05)));
  EXPECT_THAT((Triangle{{1, 2}, {3, 4}, {5, 5.9}}),
              Not(TriangleNear({{1, 2}, {3, 4}, {5, 6}}, 0.05)));
}

TEST(MeshFormatEqTest, Equal) {
  EXPECT_THAT(MeshFormat(), MeshFormatEq(MeshFormat()));
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
              MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked)));
}

TEST(MeshFormatEqTest, DifferentNumberOfAttrs) {
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
              Not(MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked))));
}

TEST(MeshFormatEqTest, DifferentAttrType) {
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3Unpacked, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
              Not(MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked))));
}

TEST(MeshFormatEqTest, DifferentAttrName) {
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
              Not(MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kTexture}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked))));
}

TEST(MeshFormatEqTest, DifferentPositionAttrIndex) {
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat2PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
              Not(MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked))));
}

TEST(MeshFormatEqTest, DifferentIndexFormat) {
  EXPECT_THAT(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
              Not(MeshFormatEq(*MeshFormat::Create(
                  {{AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
                   {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                   {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
                  MeshFormat::IndexFormat::k32BitUnpacked16BitPacked))));
}

TEST(MeshAttributeCodingParamsEqTest, Equal) {
  EXPECT_THAT(MeshAttributeCodingParams(),
              MeshAttributeCodingParamsEq(MeshAttributeCodingParams()));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}}),
              MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}}));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                          {.offset = 1, .scale = 2},
                                          {.offset = 2, .scale = 3}}}),
              MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                             {.offset = 1, .scale = 2},
                                             {.offset = 2, .scale = 3}}}));
}

TEST(MeshAttributeCodingParamsEqTest, DifferentNumberOfComponents) {
  EXPECT_THAT(MeshAttributeCodingParams(),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}})));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}}),
              Not(MeshAttributeCodingParamsEq(MeshAttributeCodingParams())));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 0, .scale = 1}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                             {.offset = 1, .scale = 2},
                                             {.offset = 2, .scale = 3}}})));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                          {.offset = 1, .scale = 2},
                                          {.offset = 2, .scale = 3}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 0, .scale = 1}}})));
}

TEST(MeshAttributeCodingParamsEqTest, DifferentOffset) {
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 2.1, .scale = 0.5}}})));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                          {.offset = 1, .scale = 2},
                                          {.offset = 2, .scale = 3}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                             {.offset = 0.9, .scale = 2},
                                             {.offset = 2, .scale = 3}}})));
}

TEST(MeshAttributeCodingParamsEqTest, DifferentScale) {
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 2, .scale = 0.5}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 2, .scale = 0.4}}})));
  EXPECT_THAT((MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                          {.offset = 1, .scale = 2},
                                          {.offset = 2, .scale = 3}}}),
              Not(MeshAttributeCodingParamsEq(
                  MeshAttributeCodingParams{{{.offset = 0, .scale = 1},
                                             {.offset = 1, .scale = 2.1},
                                             {.offset = 2, .scale = 3}}})));
}

TEST(MeshEqTest, Equal) {
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  Mesh clone = *mesh;

  EXPECT_THAT(*mesh, MeshEq(*mesh));
  EXPECT_THAT(*mesh, MeshEq(clone));
  EXPECT_THAT(clone, MeshEq(*mesh));
}

TEST(MeshEqTest, DifferentMeshFormat) {
  absl::StatusOr<MeshFormat> alternate_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(alternate_format.status(), absl::OkStatus());
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 =
      Mesh::Create(*alternate_format, {position_x, position_y}, triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, DifferentUnpackingParams) {
  absl::StatusOr<MeshFormat> packed_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(packed_format.status(), absl::OkStatus());
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<float> alternate_position_y = {0, 0, 5};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(*packed_format, {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 = Mesh::Create(
      *packed_format, {position_x, alternate_position_y}, triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, DifferentAttributeValues) {
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<float> alternate_position_y = {0, 2, 10};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 =
      Mesh::Create(MeshFormat(), {position_x, alternate_position_y}, triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, DifferentVertexCount) {
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<float> alternate_position_x = {0, 10, 10, 10};
  std::vector<float> alternate_position_y = {0, 0, 10, 15};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 = Mesh::Create(
      MeshFormat(), {alternate_position_x, alternate_position_y}, triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, DifferentTriangleIndices) {
  std::vector<float> position_x = {0, 10, 10, 10};
  std::vector<float> position_y = {0, 0, 10, 15};
  std::vector<uint32_t> triangles = {0, 1, 2};
  std::vector<uint32_t> alternate_triangles = {0, 1, 3};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, alternate_triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, DifferentTriangleCount) {
  std::vector<float> position_x = {0, 10, 10, 10};
  std::vector<float> position_y = {0, 0, 10, 15};
  std::vector<uint32_t> triangles = {0, 1, 2};
  std::vector<uint32_t> alternate_triangles = {0, 1, 2, 0, 2, 3};
  absl::StatusOr<Mesh> mesh1 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh1.status(), absl::OkStatus());
  absl::StatusOr<Mesh> mesh2 =
      Mesh::Create(MeshFormat(), {position_x, position_y}, alternate_triangles);
  ASSERT_EQ(mesh2.status(), absl::OkStatus());

  EXPECT_THAT(*mesh1, Not(MeshEq(*mesh2)));
  EXPECT_THAT(*mesh2, Not(MeshEq(*mesh1)));
}

TEST(MeshEqTest, Describe) {
  std::vector<float> position_x = {0, 10, 10};
  std::vector<float> position_y = {0, 0, 10};
  std::vector<uint32_t> triangles = {0, 1, 2};
  absl::StatusOr<Mesh> mesh =
      Mesh::Create(MeshFormat(), {position_x, position_y}, triangles);
  ASSERT_EQ(mesh.status(), absl::OkStatus());
  testing::Matcher<Mesh> matcher = MeshEq(*mesh);

  std::stringstream s;
  matcher.DescribeTo(&s);
  EXPECT_THAT(s.str(), HasSubstr("equals Mesh"));

  s.str("");
  matcher.DescribeNegationTo(&s);
  EXPECT_THAT(s.str(), HasSubstr("does not equal Mesh"));
}

TEST(PartitionedMeshEqTest, OutlineIndexPairEquality) {
  PartitionedMesh::VertexIndexPair pair{.mesh_index = 3, .vertex_index = 5};

  EXPECT_THAT(pair, VertexIndexPairEq(PartitionedMesh::VertexIndexPair{
                        .mesh_index = 3, .vertex_index = 5}));
  EXPECT_THAT(pair, Not(VertexIndexPairEq(PartitionedMesh::VertexIndexPair{
                        .mesh_index = 999, .vertex_index = 5})));
  EXPECT_THAT(pair, Not(VertexIndexPairEq(PartitionedMesh::VertexIndexPair{
                        .mesh_index = 3, .vertex_index = 999})));
}

TEST(PartitionedMeshEqTest, DeepVsShallowEquality) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(20, MakeSinglePackedPositionFormat()),
      {{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  PartitionedMesh shape_with_same_meshes = *shape;
  EXPECT_THAT(*shape, PartitionedMeshDeepEq(shape_with_same_meshes));
  EXPECT_THAT(*shape, PartitionedMeshShallowEq(shape_with_same_meshes));

  absl::StatusOr<PartitionedMesh> shape_with_equivalent_meshes =
      PartitionedMesh::FromMutableMesh(
          MakeStraightLineMutableMesh(20, MakeSinglePackedPositionFormat()),
          {{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}});
  ASSERT_EQ(shape_with_equivalent_meshes.status(), absl::OkStatus());
  EXPECT_THAT(*shape, PartitionedMeshDeepEq(*shape_with_equivalent_meshes));
  EXPECT_THAT(*shape,
              Not(PartitionedMeshShallowEq(*shape_with_equivalent_meshes)));
}

TEST(PartitionedMeshEqTest, DifferentMeshes) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(20, MakeSinglePackedPositionFormat()),
      {{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  // Equivalent outlines, different number of meshes.

  std::vector<absl::Span<const PartitionedMesh::VertexIndexPair>> outlines;
  PartitionedMesh::VertexIndexPair index_pair = {.mesh_index = 0,
                                                 .vertex_index = 7};
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  for (size_t i = 0; i < shape->OutlineCount(0); ++i) {
    absl::Span<PartitionedMesh::VertexIndexPair> outline =
        absl::MakeSpan(&index_pair, 1);
    outlines.push_back(outline);
  }
  std::vector<Mesh> meshes_twice;  // Add all the meshes twice.
  for (size_t i = 0; i < shape->Meshes().size(); ++i) {
    meshes_twice.push_back(shape->Meshes()[i]);
  }
  for (size_t i = 0; i < shape->Meshes().size(); ++i) {
    meshes_twice.push_back(shape->Meshes()[i]);
  }
  absl::StatusOr<PartitionedMesh>
      shape_with_equivalent_outlines_but_different_mesh_count =
          PartitionedMesh::FromMeshes(absl::MakeSpan(meshes_twice),
                                      absl::MakeSpan(outlines));
  ASSERT_EQ(shape_with_equivalent_outlines_but_different_mesh_count.status(),
            absl::OkStatus());

  EXPECT_THAT(*shape_with_equivalent_outlines_but_different_mesh_count,
              Not(PartitionedMeshShallowEq(*shape)));
  EXPECT_THAT(*shape_with_equivalent_outlines_but_different_mesh_count,
              Not(PartitionedMeshDeepEq(*shape)));

  // Equivalent outlines, same number of meshes, but different mesh contents.

  absl::StatusOr<PartitionedMesh> other_shape =
      PartitionedMesh::FromMutableMesh(
          MakeStraightLineMutableMesh(11, MakeSinglePackedPositionFormat()),
          {{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}});
  ASSERT_EQ(other_shape.status(), absl::OkStatus());
  std::vector<Mesh> other_meshes;
  for (size_t i = 0; i < shape->Meshes().size(); ++i) {
    // Add the same number of meshes in the original shape, but with a mesh from
    // the other shape.
    other_meshes.push_back(other_shape->Meshes()[0]);
  }
  absl::StatusOr<PartitionedMesh>
      shape_with_equivalent_outlines_but_different_meshes =
          PartitionedMesh::FromMeshes(absl::MakeSpan(other_meshes),
                                      absl::MakeSpan(outlines));
  ASSERT_EQ(shape_with_equivalent_outlines_but_different_meshes.status(),
            absl::OkStatus());

  EXPECT_THAT(*shape_with_equivalent_outlines_but_different_meshes,
              Not(PartitionedMeshShallowEq(*shape)));
  EXPECT_THAT(*shape_with_equivalent_outlines_but_different_meshes,
              Not(PartitionedMeshDeepEq(*shape)));
}

TEST(PartitionedMeshEqTest, DifferentOutlines) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(20, MakeSinglePackedPositionFormat()),
      {{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  // Equivalent meshes but different number of outlines.

  absl::StatusOr<PartitionedMesh> shape_with_equivalent_meshes_but_no_outline =
      PartitionedMesh::FromMeshes(shape->Meshes());
  ASSERT_EQ(shape_with_equivalent_meshes_but_no_outline.status(),
            absl::OkStatus());
  EXPECT_THAT(*shape_with_equivalent_meshes_but_no_outline,
              Not(PartitionedMeshShallowEq(*shape)));
  EXPECT_THAT(*shape_with_equivalent_meshes_but_no_outline,
              Not(PartitionedMeshDeepEq(*shape)));

  // Equivalent meshes, same number of outlines, but different outline contents.
  PartitionedMesh::VertexIndexPair index_pair = {.mesh_index = 0,
                                                 .vertex_index = 7};
  std::vector<absl::Span<const PartitionedMesh::VertexIndexPair>> outlines;
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  for (size_t i = 0; i < shape->OutlineCount(0); ++i) {
    absl::Span<PartitionedMesh::VertexIndexPair> outline =
        absl::MakeSpan(&index_pair, 1);
    outlines.push_back(outline);
  }
  absl::StatusOr<PartitionedMesh>
      shape_with_equivalent_meshes_but_different_outlines =
          PartitionedMesh::FromMeshes(shape->Meshes(), outlines);
  ASSERT_EQ(shape_with_equivalent_meshes_but_different_outlines.status(),
            absl::OkStatus());

  EXPECT_THAT(*shape_with_equivalent_meshes_but_different_outlines,
              Not(PartitionedMeshShallowEq(*shape)));
  EXPECT_THAT(*shape_with_equivalent_meshes_but_different_outlines,
              Not(PartitionedMeshDeepEq(*shape)));
}

}  // namespace
}  // namespace ink
