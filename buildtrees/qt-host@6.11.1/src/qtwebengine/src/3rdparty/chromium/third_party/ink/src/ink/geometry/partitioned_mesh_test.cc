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

#include "ink/geometry/partitioned_mesh.h"

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

using ::absl_testing::IsOk;
using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatNear;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

MATCHER_P(TriangleIndexPairEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       absl::Substitute(" TriangleIndexPair (expected "
                                        "mesh_index = $0, triangle_index = $1)",
                                        expected.mesh_index,
                                        expected.triangle_index))) {
  return ExplainMatchResult(
      AllOf(Field("mesh_index", &PartitionedMesh::TriangleIndexPair::mesh_index,
                  expected.mesh_index),
            Field("triangle_index",
                  &PartitionedMesh::TriangleIndexPair::triangle_index,
                  expected.triangle_index)),
      arg, result_listener);
}
Matcher<PartitionedMesh::TriangleIndexPair> TriangleIndexPairEq(
    PartitionedMesh::TriangleIndexPair idx_pair) {
  return TriangleIndexPairEqMatcher(idx_pair);
}

TEST(PartitionedMeshTest, DefaultCtor) {
  PartitionedMesh shape;

  EXPECT_THAT(shape.Meshes(), IsEmpty());
  EXPECT_EQ(shape.RenderGroupCount(), 0);
  EXPECT_TRUE(shape.Bounds().IsEmpty());
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, WithZeroEmptyGroups) {
  PartitionedMesh shape = PartitionedMesh::WithEmptyGroups(0);

  EXPECT_THAT(shape.Meshes(), IsEmpty());
  EXPECT_EQ(shape.RenderGroupCount(), 0);
  EXPECT_TRUE(shape.Bounds().IsEmpty());
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, WithThreeEmptyGroups) {
  uint32_t num_groups = 3;
  PartitionedMesh shape = PartitionedMesh::WithEmptyGroups(num_groups);

  EXPECT_THAT(shape.Meshes(), IsEmpty());
  EXPECT_TRUE(shape.Bounds().IsEmpty());
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
  ASSERT_EQ(shape.RenderGroupCount(), num_groups);
  for (uint32_t group_index = 0; group_index < num_groups; ++group_index) {
    EXPECT_THAT(shape.RenderGroupMeshes(group_index), IsEmpty());
    EXPECT_EQ(shape.OutlineCount(group_index), 0);
  }
}

TEST(PartitionedMeshTest, FromMutableMesh) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(100, MakeSinglePackedPositionFormat());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = mutable_mesh.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  ASSERT_THAT(*meshes, SizeIs(1));
  const Mesh& mesh = (*meshes)[0];

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMesh(mutable_mesh);

  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh)));
  ASSERT_EQ(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh.Format()));
  EXPECT_THAT(shape->RenderGroupMeshes(0), ElementsAre(MeshEq(mesh)));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  EXPECT_EQ(shape->OutlineCount(0), 0);
}

TEST(PartitionedMeshTest, FromMutableMeshWithOutlines) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(8, MakeSinglePackedPositionFormat());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = mutable_mesh.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  ASSERT_THAT(*meshes, SizeIs(1));
  const Mesh& mesh = (*meshes)[0];

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      mutable_mesh, {{1, 5, 4, 0}, {5, 9, 8, 4}});

  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh)));
  ASSERT_EQ(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh.Format()));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  ASSERT_EQ(shape->OutlineCount(0), 2);
  EXPECT_THAT(
      shape->Outline(0, 0),
      ElementsAre(VertexIndexPairEq({0, 1}), VertexIndexPairEq({0, 5}),
                  VertexIndexPairEq({0, 4}), VertexIndexPairEq({0, 0})));
  EXPECT_EQ(shape->OutlineVertexCount(0, 0), 4);
  EXPECT_THAT(
      shape->Outline(0, 1),
      ElementsAre(VertexIndexPairEq({0, 5}), VertexIndexPairEq({0, 9}),
                  VertexIndexPairEq({0, 8}), VertexIndexPairEq({0, 4})));
  EXPECT_EQ(shape->OutlineVertexCount(0, 1), 4);
  EXPECT_THAT(shape->OutlinePosition(0, 0, 0), PointEq({1, -1}));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 1), PointEq({5, -1}));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 2), PointEq({4, 0}));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 3), PointEq({0, 0}));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 0), PointEq({5, -1}));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 1), PointEq({9, -1}));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 2), PointEq({8, 0}));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 3), PointEq({4, 0}));
}

TEST(PartitionedMeshTest, FromMutableMeshWithPackingParams) {
  MeshFormat packed_format =
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition}},
                          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  MutableMesh mutable_mesh = MakeStraightLineMutableMesh(2, packed_format);

  EXPECT_THAT(mutable_mesh.Format(), MeshFormatEq(packed_format));
  ASSERT_EQ(mutable_mesh.TriangleCount(), 2);
  ASSERT_EQ(mutable_mesh.VertexCount(), 4);

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      mutable_mesh, {}, {},
      {MeshAttributeCodingParams{
          {{.offset = -10, .scale = 1}, {.offset = -10, .scale = 1}}}});
  ASSERT_THAT(shape.status(), Eq(absl::OkStatus()));

  ASSERT_THAT(shape->Meshes(), SizeIs(1));
  const Mesh& packed_mesh = shape->Meshes()[0];
  EXPECT_EQ(packed_mesh.VertexCount(), 4);
  EXPECT_THAT(packed_mesh.VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(MeshAttributeCodingParams{
                  .components = {{.offset = -10, .scale = 1},
                                 {.offset = -10, .scale = 1}}}));
  EXPECT_THAT(packed_mesh.VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT(packed_mesh.VertexPosition(1), PointEq({1, -1}));
  EXPECT_THAT(packed_mesh.VertexPosition(2), PointEq({2, 0}));
  EXPECT_THAT(packed_mesh.VertexPosition(3), PointEq({3, -1}));
}

TEST(PartitionedMeshTest, FromMutableMeshThatRequiresPartitioning) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(1e5, MakeSinglePackedPositionFormat());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = mutable_mesh.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  ASSERT_THAT(*meshes, SizeIs(2));
  const Mesh& mesh0 = (*meshes)[0];
  const Mesh& mesh1 = (*meshes)[1];

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMesh(mutable_mesh);

  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh0), MeshEq(mesh1)));
  ASSERT_THAT(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh0.Format()));
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh1.Format()));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  EXPECT_EQ(shape->OutlineCount(0), 0);
}

TEST(PartitionedMeshTest, FromMutableMeshThatRequiresPartitioningWithOutlines) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(1e5, MakeSinglePackedPositionFormat());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = mutable_mesh.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  ASSERT_THAT(*meshes, SizeIs(2));
  const Mesh& mesh0 = (*meshes)[0];
  const Mesh& mesh1 = (*meshes)[1];

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      mutable_mesh, {{0, 1, 99999, 99998}, {2, 3, 99997, 99996}});

  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh0), MeshEq(mesh1)));
  ASSERT_THAT(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh0.Format()));
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh1.Format()));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  // Vertices 0 through 65535 get mapped to (0, 0) through (0, 65535), vertices
  // 65536 through 99999 get mapped to (1, 0) through (1, 34465).
  ASSERT_EQ(shape->OutlineCount(0), 2);
  EXPECT_THAT(shape->Outline(0, 0),
              ElementsAre(VertexIndexPairEq({0, 0}), VertexIndexPairEq({0, 1}),
                          VertexIndexPairEq({1, 34465}),
                          VertexIndexPairEq({1, 34464})));
  EXPECT_THAT(shape->Outline(0, 1),
              ElementsAre(VertexIndexPairEq({0, 2}), VertexIndexPairEq({0, 3}),
                          VertexIndexPairEq({1, 34463}),
                          VertexIndexPairEq({1, 34462})));
  // Because the bounds of the mesh are so enormous, we have a maximum error of
  // ~24.4 in the x-coordinate; the y-coordinate has no error, though.
  EXPECT_THAT(shape->OutlinePosition(0, 0, 0), PointNear({0, 0}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 1), PointNear({1, -1}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 2), PointNear({99999, -1}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 3), PointNear({99998, 0}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 0), PointNear({2, 0}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 1), PointNear({3, -1}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 2), PointNear({99997, -1}, 24.5));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 3), PointNear({99996, 0}, 24.5));
}

TEST(PartitionedMeshTest, FromMutableMeshOmitAttribute) {
  absl::StatusOr<MeshFormat> original_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(original_format.status(), absl::OkStatus());
  MutableMesh mutable_mesh(*original_format);
  mutable_mesh.AppendVertex({0, 0});
  mutable_mesh.AppendVertex({4, 0});
  mutable_mesh.AppendVertex({0, 3});
  mutable_mesh.AppendTriangleIndices({0, 1, 2});
  absl::StatusOr<MeshFormat> expected_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(expected_format.status(), absl::OkStatus());

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      mutable_mesh, {{0, 1, 2}}, {MeshFormat::AttributeId::kColorShiftHsl});

  ASSERT_EQ(shape.status(), absl::OkStatus());
  ASSERT_EQ(shape->RenderGroupCount(), 1);
  EXPECT_EQ(shape->RenderGroupFormat(0), *expected_format);
  ASSERT_THAT(shape->Meshes(), SizeIs(1));
  const Mesh& mesh = shape->Meshes()[0];
  EXPECT_EQ(mesh.Format(), *expected_format);
  ASSERT_EQ(mesh.TriangleCount(), 1);
  EXPECT_EQ(mesh.GetTriangle(0), (Triangle{{0, 0}, {4, 0}, {0, 3}}));
}

TEST(PartitionedMeshTest, FromMutableMeshEmptyMesh) {
  // Construct a `PartitionedMesh` from an empty `MutableMesh`.
  MutableMesh mutable_mesh;
  absl::StatusOr<PartitionedMesh> no_triangles =
      PartitionedMesh::FromMutableMesh(mutable_mesh);
  // This should result in a `PartitionedMesh` with one render group, which
  // contains no meshes or outlines.
  ASSERT_THAT(no_triangles, IsOk());
  ASSERT_EQ(no_triangles->RenderGroupCount(), 1);
  EXPECT_THAT(no_triangles->RenderGroupMeshes(0), IsEmpty());
  EXPECT_EQ(no_triangles->OutlineCount(0), 0);
}

TEST(PartitionedMeshTest, FromMutableMeshPartitioningError) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(10, MakeSinglePackedPositionFormat());
  // Non-finite values cause `MutableMesh::AsMeshes` to fail.
  mutable_mesh.SetVertexPosition(0, {std::nanf(""), 0});

  absl::Status non_finite_value =
      PartitionedMesh::FromMutableMesh(mutable_mesh).status();
  EXPECT_EQ(non_finite_value.code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(non_finite_value.message(), HasSubstr("non-finite value"));
}

TEST(PartitionedMeshTest, FromMutableMeshOutlineIsEmpty) {
  // Construct a `PartitionedMesh` from a non-empty `MutableMesh` and an empty
  // outline.
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(10, MakeSinglePackedPositionFormat());
  absl::StatusOr<PartitionedMesh> empty_outline =
      PartitionedMesh::FromMutableMesh(mutable_mesh, {{}});
  // The mesh should be included in the `PartitionedMesh`, but the empty outline
  // should get filtered out.
  ASSERT_THAT(empty_outline, IsOk());
  ASSERT_EQ(empty_outline->RenderGroupCount(), 1);
  ASSERT_THAT(empty_outline->RenderGroupMeshes(0), SizeIs(1));
  ASSERT_EQ(empty_outline->OutlineCount(0), 0);
}

TEST(PartitionedMeshTest, FromMutableMeshOutlineRefersToNonExistentVertex) {
  MutableMesh mutable_mesh =
      MakeStraightLineMutableMesh(8, MakeSinglePackedPositionFormat());

  absl::Status missing_vertex =
      PartitionedMesh::FromMutableMesh(mutable_mesh, {{10}}).status();
  EXPECT_EQ(missing_vertex.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(missing_vertex.message(), HasSubstr("non-existent vertex"));
}

TEST(PartitionedMeshTest, FromMeshes) {
  constexpr int kMeshCount = 3;
  std::vector<Mesh> meshes(kMeshCount);
  for (int i = 0; i < kMeshCount; ++i) {
    MutableMesh mutable_mesh = MakeStraightLineMutableMesh(
        10 * (i + 1), MakeSinglePackedPositionFormat());
    absl::StatusOr<absl::InlinedVector<Mesh, 1>> partitions =
        mutable_mesh.AsMeshes();
    ASSERT_EQ(partitions.status(), absl::OkStatus());
    ASSERT_EQ(partitions->size(), 1);
    meshes[i] = std::move((*partitions)[0]);
  }

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeConstSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_THAT(shape->Meshes(), SizeIs(kMeshCount));
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(meshes[0]), MeshEq(meshes[1]),
                                           MeshEq(meshes[2])));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  EXPECT_EQ(shape->OutlineCount(0), 0);
  EXPECT_THAT(shape->Bounds(),
              EnvelopeNear(Rect::FromTwoPoints({0, -1}, {31, 0}), 0.001));
}

TEST(PartitionedMeshTest, FromMeshesWithOutlines) {
  constexpr int kMeshCount = 3;
  std::vector<Mesh> meshes(kMeshCount);
  for (int i = 0; i < kMeshCount; ++i) {
    MutableMesh mutable_mesh = MakeStraightLineMutableMesh(
        10 * (i + 1), MakeSinglePackedPositionFormat());
    absl::StatusOr<absl::InlinedVector<Mesh, 1>> partitions =
        mutable_mesh.AsMeshes();
    ASSERT_EQ(partitions.status(), absl::OkStatus());
    ASSERT_EQ(partitions->size(), 1);
    meshes[i] = std::move((*partitions)[0]);
  }

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshes(
      absl::MakeConstSpan(meshes),
      {{{0, 0}, {1, 5}, {2, 10}}, {{1, 19}, {2, 29}, {0, 9}}});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_THAT(shape->Meshes(), SizeIs(kMeshCount));
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(meshes[0]), MeshEq(meshes[1]),
                                           MeshEq(meshes[2])));
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  ASSERT_EQ(shape->RenderGroupCount(), 1u);
  ASSERT_EQ(shape->OutlineCount(0), 2u);
  EXPECT_THAT(shape->Outline(0, 0),
              ElementsAre(VertexIndexPairEq({0, 0}), VertexIndexPairEq({1, 5}),
                          VertexIndexPairEq({2, 10})));
  EXPECT_THAT(shape->Outline(0, 1), ElementsAre(VertexIndexPairEq({1, 19}),
                                                VertexIndexPairEq({2, 29}),
                                                VertexIndexPairEq({0, 9})));
  // The maximum error in these meshes is ~7.08e-3.
  EXPECT_THAT(shape->OutlinePosition(0, 0, 0), PointNear({0, 0}, 8e-3));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 1), PointNear({5, -1}, 8e-3));
  EXPECT_THAT(shape->OutlinePosition(0, 0, 2), PointNear({10, 0}, 8e-3));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 0), PointNear({19, -1}, 8e-3));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 1), PointNear({29, -1}, 8e-3));
  EXPECT_THAT(shape->OutlinePosition(0, 1, 2), PointNear({9, -1}, 8e-3));
}

TEST(PartitionedMeshTest, FromMultipleMeshGroupsWithOutlines) {
  constexpr int kMeshCount = 3;
  std::vector<Mesh> meshes(kMeshCount);
  for (int i = 0; i < kMeshCount; ++i) {
    MutableMesh mutable_mesh = MakeStraightLineMutableMesh(
        10 * (i + 1), MakeSinglePackedPositionFormat());
    absl::StatusOr<absl::InlinedVector<Mesh, 1>> partitions =
        mutable_mesh.AsMeshes();
    ASSERT_EQ(partitions.status(), absl::OkStatus());
    ASSERT_EQ(partitions->size(), 1);
    meshes[i] = std::move((*partitions)[0]);
  }

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshGroups({
      PartitionedMesh::MeshGroup{
          .meshes = absl::MakeConstSpan(&meshes[0], 1),
          .outlines = {{{0, 0}, {0, 9}}},
      },
      PartitionedMesh::MeshGroup{
          .meshes = absl::MakeConstSpan(&meshes[1], 1),
          .outlines = {{{0, 5}, {0, 19}}},
      },
      PartitionedMesh::MeshGroup{
          .meshes = absl::MakeConstSpan(&meshes[2], 1),
          .outlines = {{{0, 10}, {0, 29}}},
      },
  });
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_THAT(shape->Meshes(), SizeIs(kMeshCount));
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(meshes[0]), MeshEq(meshes[1]),
                                           MeshEq(meshes[2])));
  ASSERT_EQ(shape->RenderGroupCount(), 3u);
  ASSERT_EQ(shape->OutlineCount(0), 1u);
  EXPECT_THAT(shape->Outline(0, 0), ElementsAre(VertexIndexPairEq({0, 0}),
                                                VertexIndexPairEq({0, 9})));
  ASSERT_EQ(shape->OutlineCount(1), 1u);
  EXPECT_THAT(shape->Outline(1, 0), ElementsAre(VertexIndexPairEq({0, 5}),
                                                VertexIndexPairEq({0, 19})));
  ASSERT_EQ(shape->OutlineCount(2), 1u);
  EXPECT_THAT(shape->Outline(2, 0), ElementsAre(VertexIndexPairEq({0, 10}),
                                                VertexIndexPairEq({0, 29})));
}

TEST(PartitionedMeshTest, FromMeshesEmptyMeshSpan) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshes({});
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(shape->Meshes(), IsEmpty());
  ASSERT_EQ(shape->RenderGroupCount(), 1);
  EXPECT_THAT(shape->RenderGroupMeshes(0), IsEmpty());
  EXPECT_TRUE(shape->Bounds().IsEmpty());
}

TEST(PartitionedMeshTest, FromMeshesTooManyMeshes) {
  std::vector<Mesh> too_many_meshes(65536);
  absl::Status has_too_many_meshes =
      PartitionedMesh::FromMeshes(absl::MakeSpan(too_many_meshes)).status();
  EXPECT_EQ(has_too_many_meshes.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(has_too_many_meshes.message(), HasSubstr("Too many meshes"));
}

TEST(PartitionedMeshTest, FromMeshesEmptyMesh) {
  Mesh empty;
  absl::Status no_triangles =
      PartitionedMesh::FromMeshes(absl::MakeSpan(&empty, 1)).status();
  EXPECT_EQ(no_triangles.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(no_triangles.message(), HasSubstr("contains no triangles"));
}

TEST(PartitionedMeshTest, FromMeshesWithDifferentFormats) {
  absl::StatusOr<MeshFormat> format_a =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat3Unpacked,
                           MeshFormat::AttributeId::kColorShiftHsl}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format_a.status(), absl::OkStatus());
  absl::StatusOr<MeshFormat> format_b =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kOpacityShift}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format_b.status(), absl::OkStatus());
  ASSERT_NE(format_a, format_b);

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes_a =
      MakeStraightLineMutableMesh(2, *format_a).AsMeshes();
  ASSERT_EQ(meshes_a.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes_b =
      MakeStraightLineMutableMesh(2, *format_b).AsMeshes();
  ASSERT_EQ(meshes_b.status(), absl::OkStatus());
  ASSERT_EQ(meshes_a->size(), 1);
  ASSERT_EQ(meshes_b->size(), 1);

  Mesh meshes[2] = {std::move((*meshes_a)[0]), std::move((*meshes_b)[0])};

  absl::Status inconsistent_format =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes)).status();
  EXPECT_EQ(inconsistent_format.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(inconsistent_format.message(),
              HasSubstr("must have the same format"));
}

TEST(PartitionedMeshTest, FromMeshesEmptyOutline) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      MakeStraightLineMutableMesh(20).AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  absl::Status no_points = PartitionedMesh::FromMeshes(absl::MakeSpan(*meshes),
                                                       {{{0, 1}, {0, 2}}, {}})
                               .status();
  EXPECT_EQ(no_points.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(no_points.message(), HasSubstr("contains no points"));
}

TEST(PartitionedMeshTest, FromMeshesOutlineRefersToNonExistentMesh) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      MakeStraightLineMutableMesh(5).AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  absl::Status missing_mesh =
      PartitionedMesh::FromMeshes(absl::MakeSpan(*meshes),
                                  {{{0, 1}, {1, 2}, {0, 1}, {0, 3}}})
          .status();
  EXPECT_EQ(missing_mesh.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(missing_mesh.message(), HasSubstr("non-existent mesh"));
}

TEST(PartitionedMeshTest, FromMeshesOutlineRefersToNonExistentVertex) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  absl::Status missing_vertex =
      PartitionedMesh::FromMeshes(absl::MakeSpan(*meshes),
                                  {{{0, 1}, {0, 2}, {0, 5}, {0, 3}}})
          .status();
  EXPECT_EQ(missing_vertex.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(missing_vertex.message(), HasSubstr("non-existent vertex"));
}

TEST(PartitionedMeshTest, FromMultipleMutableMeshGroups) {
  MutableMesh mutable_mesh0 = MakeStraightLineMutableMesh(8);
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes0 =
      mutable_mesh0.AsMeshes();
  ASSERT_EQ(meshes0.status(), absl::OkStatus());
  ASSERT_THAT(*meshes0, SizeIs(1));
  const Mesh& mesh0 = (*meshes0)[0];

  MutableMesh mutable_mesh1 =
      MakeStraightLineMutableMesh(3, MakeSinglePackedPositionFormat());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes1 =
      mutable_mesh1.AsMeshes();
  ASSERT_EQ(meshes1.status(), absl::OkStatus());
  ASSERT_THAT(*meshes1, SizeIs(1));
  const Mesh& mesh1 = (*meshes1)[0];

  // Different render groups can use different mesh formats.
  EXPECT_THAT(mesh0.Format(), Not(MeshFormatEq(mesh1.Format())));

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMeshGroups({
          PartitionedMesh::MutableMeshGroup{.mesh = &mutable_mesh0},
          PartitionedMesh::MutableMeshGroup{.mesh = &mutable_mesh1},
      });
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_EQ(shape->RenderGroupCount(), 2u);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh0.Format()));
  EXPECT_THAT(shape->RenderGroupFormat(1), MeshFormatEq(mesh1.Format()));
  EXPECT_THAT(shape->RenderGroupMeshes(0), ElementsAre(MeshEq(mesh0)));
  EXPECT_THAT(shape->RenderGroupMeshes(1), ElementsAre(MeshEq(mesh1)));
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh0), MeshEq(mesh1)));
  EXPECT_EQ(shape->OutlineCount(0), 0u);
  EXPECT_EQ(shape->OutlineCount(1), 0u);
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, FromMultipleMeshGroups) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes0 =
      MakeStraightLineMutableMesh(8).AsMeshes();
  ASSERT_EQ(meshes0.status(), absl::OkStatus());
  ASSERT_THAT(*meshes0, SizeIs(1));
  const Mesh& mesh0 = (*meshes0)[0];

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes1 =
      MakeStraightLineMutableMesh(3, MakeSinglePackedPositionFormat())
          .AsMeshes();
  ASSERT_EQ(meshes1.status(), absl::OkStatus());
  ASSERT_THAT(*meshes1, SizeIs(1));
  const Mesh& mesh1 = (*meshes1)[0];

  // Different render groups can use different mesh formats.
  EXPECT_THAT(mesh0.Format(), Not(MeshFormatEq(mesh1.Format())));

  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshGroups({
      PartitionedMesh::MeshGroup{.meshes = *meshes0},
      PartitionedMesh::MeshGroup{.meshes = *meshes1},
  });
  ASSERT_EQ(shape.status(), absl::OkStatus());

  ASSERT_EQ(shape->RenderGroupCount(), 2u);
  EXPECT_THAT(shape->RenderGroupFormat(0), MeshFormatEq(mesh0.Format()));
  EXPECT_THAT(shape->RenderGroupFormat(1), MeshFormatEq(mesh1.Format()));
  EXPECT_THAT(shape->RenderGroupMeshes(0), ElementsAre(MeshEq(mesh0)));
  EXPECT_THAT(shape->RenderGroupMeshes(1), ElementsAre(MeshEq(mesh1)));
  EXPECT_THAT(shape->Meshes(), ElementsAre(MeshEq(mesh0), MeshEq(mesh1)));
  EXPECT_EQ(shape->OutlineCount(0), 0u);
  EXPECT_EQ(shape->OutlineCount(1), 0u);
  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, InitializeSpatialIndex) {
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMesh(MakeStraightLineMutableMesh(100));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_FALSE(shape->IsSpatialIndexInitialized());

  shape->InitializeSpatialIndex();

  EXPECT_TRUE(shape->IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, InitializeSpatialIndexWithMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(10).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(10).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_FALSE(shape->IsSpatialIndexInitialized());

  shape->InitializeSpatialIndex();

  EXPECT_TRUE(shape->IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, SpatialIndexIsSharedBetweenCopies) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(100, MakeSinglePackedPositionFormat()));
  ASSERT_EQ(shape.status(), absl::OkStatus());
  PartitionedMesh copy = *shape;

  EXPECT_FALSE(shape->IsSpatialIndexInitialized());
  EXPECT_FALSE(copy.IsSpatialIndexInitialized());

  shape->InitializeSpatialIndex();

  EXPECT_TRUE(shape->IsSpatialIndexInitialized());
  EXPECT_TRUE(copy.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, InitializeSpatialIndexIsNoOpForEmptyPartitionedMesh) {
  PartitionedMesh shape;

  EXPECT_FALSE(shape.IsSpatialIndexInitialized());

  shape.InitializeSpatialIndex();

  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

// Helper function, visits all intersected triangles and returns them in a
// vector.
template <typename QueryType>
std::vector<PartitionedMesh::TriangleIndexPair> GetAllIntersectedTriangles(
    const PartitionedMesh& shape, const QueryType& query,
    const AffineTransform query_to_shape = {}) {
  std::vector<PartitionedMesh::TriangleIndexPair> tri_index_pairs;
  shape.VisitIntersectedTriangles(
      query,
      [&tri_index_pairs](PartitionedMesh::TriangleIndexPair idx) {
        tri_index_pairs.push_back(idx);
        return PartitionedMesh::FlowControl::kContinue;
      },
      query_to_shape);
  return tri_index_pairs;
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesPointQuery) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(GetAllIntersectedTriangles(shape, Point{2, 0}), IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(shape, Point{-.8, .1}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Point{.8, .1}),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12})));
  EXPECT_THAT(GetAllIntersectedTriangles(shape, Point{0, 0},
                                         AffineTransform::Translate({0, .8})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesPointQueryMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Point{0, -2}), IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Point{2, -.5}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1})));
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Point{-.5, -.5}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Point{.3, -.2}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Point{.1, .1},
                                         AffineTransform::Scale(3)),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 0})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesPointQueryEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(GetAllIntersectedTriangles(shape, Point{0, 0}), IsEmpty());
  // An empty shape never has a spatial index.
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesPointQueryExitEarly) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  shape.VisitIntersectedTriangles(
      Point{.8, .1}, [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      });

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query point.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesPointQueryInitializesTheSpatialIndex) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  shape.VisitIntersectedTriangles(Point{0, 0},
                                  [](PartitionedMesh::TriangleIndexPair) {
                                    // This doesn't actually need to do
                                    // anything.
                                    return PartitionedMesh::FlowControl::kBreak;
                                  });

  EXPECT_TRUE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesSegmentQuery) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(GetAllIntersectedTriangles(shape, Segment{{2, 0}, {2, 2}}),
              IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(shape, Segment{{-.5, .2}, {-1, .2}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 4}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Segment{{.5, .2}, {1, .2}}),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Segment{{0, 0}, {1, 1}},
                                 AffineTransform::Rotate(Angle::Degrees(45))),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesSegmentQueryMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Segment{{0, -2}, {3, -2}}),
              IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Segment{{2, -.5}, {3, -.8}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(*shape, Segment{{-.5, -.5}, {-.5, .5}}),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Segment{{-1, -.5}, {1, -.5}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(*shape, Segment{{-1, -1}, {1, 1}},
                                         AffineTransform::Translate({1, 0})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesSegmentQueryEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(GetAllIntersectedTriangles(shape, Segment{{0, 0}, {1, 1}}),
              IsEmpty());
  // An empty shape never has a spatial index.
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesSegmentQueryExitEarly) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  shape.VisitIntersectedTriangles(
      Segment{{.8, .1}, {0, 0}},
      [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      });

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesSegmentQueryInitializesTheSpatialIndex) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  shape.VisitIntersectedTriangles(Segment{{0, 0}, {1, 1}},
                                  [](PartitionedMesh::TriangleIndexPair) {
                                    // This doesn't actually need to do
                                    // anything.
                                    return PartitionedMesh::FlowControl::kBreak;
                                  });

  EXPECT_TRUE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesTriangleQuery) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Triangle{{2, 0}, {2, 2}, {1, 1}}),
      IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Triangle{{-.5, .2}, {-1, .2}, {-1, .5}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 4}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Triangle{{.5, .2}, {1, .2}, {1, 2}}),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Triangle{{0, 0}, {1, 1}, {0, .1}},
                                 AffineTransform::Rotate(Angle::Degrees(45))),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesTriangleQueryMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(
      GetAllIntersectedTriangles(*shape, Triangle{{0, -2}, {3, -2}, {1, -1.5}}),
      IsEmpty());
  EXPECT_THAT(
      GetAllIntersectedTriangles(*shape, Triangle{{2, -.5}, {3, -.8}, {3, .5}}),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Triangle{{-.5, -.5}, {-.5, .5}, {-1, 0}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Triangle{{-1, -.5}, {1, -.5}, {0, -1}}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(*shape, Triangle{{-1, -1}, {1, 1}, {-.1, .1}},
                                 AffineTransform::Translate({1, 0})),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesTriangleQueryEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Triangle{{0, 0}, {1, 1}, {1, 2}}),
      IsEmpty());
  // An empty shape never has a spatial index.
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesTriangleQueryExitEarly) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  shape.VisitIntersectedTriangles(
      Triangle{{.8, .1}, {0, 0}, {0, .1}},
      [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      });

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesTriangleQueryInitializesTheSpatialIndex) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  shape.VisitIntersectedTriangles(Triangle{{0, 0}, {1, 1}, {1, 2}},
                                  [](PartitionedMesh::TriangleIndexPair) {
                                    // This doesn't actually need to do
                                    // anything.
                                    return PartitionedMesh::FlowControl::kBreak;
                                  });

  EXPECT_TRUE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesRectQuery) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Rect::FromCenterAndDimensions({2, 0}, .5, .5)),
              IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Rect::FromTwoPoints({-.5, .2}, {-1, .2})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 4}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Rect::FromTwoPoints({.5, .2}, {1, .2})),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape, Rect::FromTwoPoints({0, 0}, {2, .1}),
                                 AffineTransform::Rotate(Angle::Degrees(90))),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesRectQueryHandlesNonAxisAlignedTransforms) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  // This `Rect` does not intersect the mesh when transformed, even the bounding
  // box of the transformed `Rect` would intersect the mesh.
  EXPECT_THAT(
      GetAllIntersectedTriangles(
          shape, Rect::FromCenterAndDimensions({1, 1}, .8, .8),
          AffineTransform::RotateAboutPoint(Angle::Degrees(45), {1, 1})),
      IsEmpty());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesRectQueryMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(
      GetAllIntersectedTriangles(*shape, Rect::FromTwoPoints({0, -3}, {3, -2})),
      IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Rect::FromTwoPoints({2, -.5}, {3, -.8})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Rect::FromCenterAndDimensions({-.5, 0}, .1, 1)),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Rect::FromTwoPoints({-1, -.5}, {1, -.5})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Rect::FromCenterAndDimensions({0, 0}, .2, .2),
                  AffineTransform::Translate({.5, -.5})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesRectQueryEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Rect::FromCenterAndDimensions({0, 0}, 1, 1)),
              IsEmpty());
  // An empty shape never has a spatial index.
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesRectQueryExitEarly) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  shape.VisitIntersectedTriangles(
      Rect::FromTwoPoints({.8, .1}, {.05, .05}),
      [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      });

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesRectQueryInitializesTheSpatialIndex) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  shape.VisitIntersectedTriangles(Rect::FromTwoPoints({0, 0}, {1, 1}),
                                  [](PartitionedMesh::TriangleIndexPair) {
                                    // This doesn't actually need to do
                                    // anything.
                                    return PartitionedMesh::FlowControl::kBreak;
                                  });

  EXPECT_TRUE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesQuadQuery) {
  // This mesh will wrap around and partially overlap itself.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Quad::FromCenterAndDimensions({2, 0}, .5, .5)),
              IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Quad::FromCenterDimensionsAndRotation(
                             {-.7, .3}, .5, .1, Angle::Degrees(-30))),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 4}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(shape,
                                 Quad::FromCenterDimensionsRotationAndShear(
                                     {1, .5}, .2, .5, Angle::Degrees(0), 2)),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Quad::FromCenterAndDimensions({0, 0}, .1, .5),
                  AffineTransform::Translate({0, .5})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesQuadQueryMultipleMeshes) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(shape.status(), absl::OkStatus());

  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Quad::FromCenterDimensionsRotationAndShear(
                              {5, 5}, 1, 2, Angle::Degrees(75), 1)),
              IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Quad::FromCenterDimensionsAndRotation(
                              {2.5, -.5}, 10, .5, Angle::Degrees(45))),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Quad::FromCenterAndDimensions({-.5, 0}, .1, 1)),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2})));
  EXPECT_THAT(GetAllIntersectedTriangles(
                  *shape, Quad::FromCenterAndDimensions({0, -.5}, 2, 0)),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(
      GetAllIntersectedTriangles(
          *shape, Quad::FromCenterAndDimensions({.5, -.5}, .2, 1),
          AffineTransform::RotateAboutPoint(Angle::Degrees(-45), {.5, -.5})),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesQuadQueryEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(GetAllIntersectedTriangles(
                  shape, Quad::FromCenterAndDimensions({0, 0}, 1, 1)),
              IsEmpty());
  // An empty shape never has a spatial index.
  EXPECT_FALSE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesQuadQueryExitEarly) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  shape.VisitIntersectedTriangles(
      Quad::FromCenterAndDimensions({.8, .1}, .01, .01),
      [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      });

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesQuadQueryInitializesTheSpatialIndex) {
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(14, 6);

  shape.VisitIntersectedTriangles(Quad::FromCenterAndDimensions({0, 0}, 10, 10),
                                  [](PartitionedMesh::TriangleIndexPair) {
                                    // This doesn't actually need to do
                                    // anything.
                                    return PartitionedMesh::FlowControl::kBreak;
                                  });

  EXPECT_TRUE(shape.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesPartitionedMeshQuery) {
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> first_mesh =
      MakeStraightLineMutableMesh(3).AsMeshes();
  ASSERT_EQ(first_mesh.status(), absl::OkStatus());
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> second_mesh =
      MakeStarMutableMesh(4).AsMeshes();
  ASSERT_EQ(second_mesh.status(), absl::OkStatus());
  ASSERT_EQ(first_mesh->size(), 1);
  ASSERT_EQ(second_mesh->size(), 1);
  Mesh meshes[2] = {std::move((*first_mesh)[0]), std::move((*second_mesh)[0])};
  absl::StatusOr<PartitionedMesh> star_and_line =
      PartitionedMesh::FromMeshes(absl::MakeSpan(meshes));
  ASSERT_EQ(star_and_line.status(), absl::OkStatus());
  PartitionedMesh ring = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(
      GetAllIntersectedTriangles(ring, *star_and_line),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 4}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 5}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 6}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 7}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 10}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 11}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(GetAllIntersectedTriangles(*star_and_line, ring),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(*star_and_line, ring,
                                         AffineTransform::Translate({0, 2})),
              IsEmpty());
  EXPECT_THAT(
      GetAllIntersectedTriangles(ring, *star_and_line,
                                 AffineTransform::Translate({1, 1})),
      UnorderedElementsAre(
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 12}),
          TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 13})));
  EXPECT_THAT(GetAllIntersectedTriangles(*star_and_line, ring,
                                         AffineTransform::Translate({-1, 1})),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 1, .triangle_index = 1})));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesPartitionedMeshQueryEmptyShape) {
  PartitionedMesh empty;
  PartitionedMesh ring = MakeCoiledRingPartitionedMesh(14, 6);

  EXPECT_THAT(GetAllIntersectedTriangles(ring, empty), IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(empty, ring), IsEmpty());
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesPartitionedMeshQueryExitEarly) {
  PartitionedMesh star = MakeStarPartitionedMesh(4);
  PartitionedMesh line = MakeStraightLinePartitionedMesh(3);
  std::vector<PartitionedMesh::TriangleIndexPair> visited_tris;

  star.VisitIntersectedTriangles(
      line,
      [&visited_tris](PartitionedMesh::TriangleIndexPair idx) {
        visited_tris.push_back(idx);
        return PartitionedMesh::FlowControl::kBreak;
      },
      AffineTransform::Translate({-2, 1.5}));

  // The visitor should find one triangle, then stop; but because visitation
  // order is arbitrary, the visited triangle could be either of the two that
  // intersect the query.
  EXPECT_THAT(
      visited_tris,
      UnorderedElementsAre(
          AnyOf(TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}))));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesPartitionedMeshQueryInitializesTheSpatialIndex) {
  PartitionedMesh star = MakeStarPartitionedMesh(4);
  PartitionedMesh line = MakeStraightLinePartitionedMesh(3);

  line.VisitIntersectedTriangles(star, [](PartitionedMesh::TriangleIndexPair) {
    // This doesn't actually need to do anything.
    return PartitionedMesh::FlowControl::kBreak;
  });

  EXPECT_TRUE(line.IsSpatialIndexInitialized());
  EXPECT_TRUE(star.IsSpatialIndexInitialized());
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesWithNonInvertibleTransformToPoint) {
  PartitionedMesh star = MakeStarPartitionedMesh(6);
  PartitionedMesh line = MakeStraightLinePartitionedMesh(3);

  // This transform collapses the query to the point (-1, 2);
  EXPECT_THAT(GetAllIntersectedTriangles(line, star,
                                         AffineTransform{0, 0, -1, 0, 0, 2}),
              IsEmpty());
  // This transform collapses the query to the point (2, -0.5);
  EXPECT_THAT(GetAllIntersectedTriangles(line, star,
                                         AffineTransform{0, 0, 2, 0, 0, -0.5}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1})));
}

TEST(PartitionedMeshTest,
     VisitIntersectedTrianglesWithNonInvertibleTransformToSegment) {
  PartitionedMesh star = MakeStarPartitionedMesh(6);
  PartitionedMesh line = MakeStraightLinePartitionedMesh(3);

  // This transform collapses the query to the segment from (1.634, -0.683) to
  // (4.366, 0.683).
  EXPECT_THAT(GetAllIntersectedTriangles(line, star,
                                         AffineTransform{1, 1, 3, 0.5, 0.5, 0}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2})));
  // This transform collapses the query to the segment from (-1, -1) to (-1, 1).
  EXPECT_THAT(GetAllIntersectedTriangles(line, star,
                                         AffineTransform{0, 0, -1, 0, 1, 0}),
              IsEmpty());
  // This transform collapses the query to the segment from (-1.092, 0.820) to
  // (1.092, -0.820). Note that it does not intersect the triangle at index 1,
  // even though the transformed diagonal segment would have.
  EXPECT_THAT(GetAllIntersectedTriangles(
                  line, star, AffineTransform{0.8, 0.8, 0, 0.6, 0.6, 0}),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0})));
}

TEST(PartitionedMeshTest, VisitIntersectedTrianglesWithReentrantVisitor) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Rect query = Rect::FromTwoPoints({3, -2}, {6, 2});
  auto visitor = [&shape, &query](PartitionedMesh::TriangleIndexPair idx) {
    GetAllIntersectedTriangles(shape, query);
    return PartitionedMesh::FlowControl::kContinue;
  };

  // We don't actually care about the results here; we just want to validate
  // that this does not crash or deadlock.
  shape.VisitIntersectedTriangles(query, visitor);
}

// Returns a `PartitionedMesh` with four triangles in a row along the x-axis,
// each with a base of one unit, and with heights of 1, 2, 3, and 4 units. Each
// triangle has a different area (to facilitate testing `Coverage` and
// `CoverageIsGreaterThan`), and are 10%, 20%, 30%, and 40% of the total area of
// the shape, respectively.
// The vertices of the mesh are laid out like so:
//         8
//       6
//     4
//   2
// 0 1 3 5 7
PartitionedMesh MakeRisingSawtoothShape() {
  MutableMesh mesh;
  mesh.AppendVertex({0, 0});
  mesh.AppendVertex({1, 0});
  mesh.AppendVertex({1, 1});
  mesh.AppendVertex({2, 0});
  mesh.AppendVertex({2, 2});
  mesh.AppendVertex({3, 0});
  mesh.AppendVertex({3, 3});
  mesh.AppendVertex({4, 0});
  mesh.AppendVertex({4, 4});
  mesh.AppendTriangleIndices({0, 1, 2});
  mesh.AppendTriangleIndices({1, 3, 4});
  mesh.AppendTriangleIndices({3, 5, 6});
  mesh.AppendTriangleIndices({5, 7, 8});

  absl::StatusOr<PartitionedMesh> shape =
      PartitionedMesh::FromMutableMesh(mesh);
  ABSL_CHECK_OK(shape);
  return *shape;
}

TEST(PartitionedMeshTest, CoverageWithTriangleMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{-1, 0}, {-2, 1}, {-5, 3}}), 0);
}

TEST(PartitionedMeshTest, CoverageWithTriangleSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{0.5, 0}, {0.5, 5}, {0.6, 2}}), 0.1);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{1.5, 0}, {1.5, 5}, {1.6, 2}}), 0.2);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{2.5, 0}, {2.5, 5}, {2.6, 2}}), 0.3);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{3.5, 0}, {3.5, 5}, {3.6, 2}}), 0.4);
}

TEST(PartitionedMeshTest, CoverageWithTriangleMultipleTriangles) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{0.5, 0}, {1.5, 0}, {1, 1}}), 0.3);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{1.5, 0}, {2.5, 0}, {2, 1}}), 0.5);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{2.5, 0}, {3.5, 0}, {3, 1}}), 0.7);
}

TEST(PartitionedMeshTest, CoverageWithTriangleOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);

  // This point hits two overlapping triangles each of area 0.0812 unit^2.
  EXPECT_THAT(shape.Coverage(Triangle{{0, 0}, {0.6, 0.3}, {0, 0.1}}),
              FloatNear(0.1071, 1e-4));
}

TEST(PartitionedMeshTest, CoverageWithTriangleWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{0.5, 0}, {0.5, 1}, {0.6, 1}},
                                 AffineTransform::Translate({3, 0})),
                  0.4);
  EXPECT_FLOAT_EQ(shape.Coverage(Triangle{{0.5, 0}, {0.5, 1}, {0.6, 1}},
                                 AffineTransform::Translate({-5, -5})),
                  0);
}

TEST(PartitionedMeshTest, CoverageWithRectMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Rect::FromCenterAndDimensions({10, 10}, 1, 1)),
                  0);
}

TEST(PartitionedMeshTest, CoverageWithRectSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({0.5, 0.5}, 0.1, 0.1)), 0.1);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({1.5, 0.5}, 0.1, 0.1)), 0.2);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({2.5, 0.5}, 0.1, 0.1)), 0.3);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({3.5, 0.5}, 0.1, 0.1)), 0.4);
}

TEST(PartitionedMeshTest, CoverageWithRectMultipleTriangles) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Rect::FromCenterAndDimensions({1, 0}, 1, 1)),
                  0.3);
  EXPECT_FLOAT_EQ(shape.Coverage(Rect::FromCenterAndDimensions({2, 0}, 1, 1)),
                  0.5);
  EXPECT_FLOAT_EQ(shape.Coverage(Rect::FromCenterAndDimensions({3, 0}, 1, 1)),
                  0.7);
}

TEST(PartitionedMeshTest, CoverageWithRectOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);

  // This point hits two overlapping triangles each of area 0.0812 unit^2.
  EXPECT_THAT(shape.Coverage(Rect::FromTwoPoints({0, 0}, {0.6, 0.3})),
              FloatNear(0.1071, 1e-4));
}

TEST(PartitionedMeshTest, CoverageWithRectWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({0.5, 0}, 0.1, 0.1),
                     AffineTransform::Translate({3, 0})),
      0.4);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Rect::FromCenterAndDimensions({0.5, 0}, 0.1, 0.1),
                     AffineTransform::Translate({-5, -5})),
      0);
}

TEST(PartitionedMeshTest, CoverageWithQuadMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Quad::FromCenterAndDimensions({10, 10}, 1, 1)),
                  0);
}

TEST(PartitionedMeshTest, CoverageWithQuadSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({0.5, 0.5}, 0.1, 0.1)), 0.1);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({1.5, 0.5}, 0.1, 0.1)), 0.2);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({2.5, 0.5}, 0.1, 0.1)), 0.3);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({3.5, 0.5}, 0.1, 0.1)), 0.4);
}

TEST(PartitionedMeshTest, CoverageWithQuadMultipleTriangles) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(shape.Coverage(Quad::FromCenterAndDimensions({1, 0}, 1, 1)),
                  0.3);
  EXPECT_FLOAT_EQ(shape.Coverage(Quad::FromCenterAndDimensions({2, 0}, 1, 1)),
                  0.5);
  EXPECT_FLOAT_EQ(shape.Coverage(Quad::FromCenterAndDimensions({3, 0}, 1, 1)),
                  0.7);
}

TEST(PartitionedMeshTest, CoverageWithQuadOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);

  // This point hits two overlapping triangles each of area 0.0812 unit^2.
  EXPECT_THAT(shape.Coverage(Quad::FromCenterAndDimensions({0.6, 0.3}, .1, .1)),
              FloatNear(0.1071, 1e-4));
}

TEST(PartitionedMeshTest, CoverageWithQuadWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();

  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({0.5, 0}, 0.1, 0.1),
                     AffineTransform::Translate({3, 0})),
      0.4);
  EXPECT_FLOAT_EQ(
      shape.Coverage(Quad::FromCenterAndDimensions({0.5, 0}, 0.1, 0.1),
                     AffineTransform::Translate({-5, -5})),
      0);
}

TEST(PartitionedMeshTest, CoverageWithPartitionedMeshMissesShape) {
  PartitionedMesh target_shape = MakeRisingSawtoothShape();
  PartitionedMesh query_shape = MakeStraightLinePartitionedMesh(
      3, {}, AffineTransform::Translate({10, 10}));

  EXPECT_FLOAT_EQ(target_shape.Coverage(query_shape), 0);
}

TEST(PartitionedMeshTest, CoverageWithPartitionedMeshSingleTriangle) {
  PartitionedMesh target_shape = MakeRisingSawtoothShape();
  // This makes a ring with radius 0.1 centered at (0.5, 0).
  PartitionedMesh query_shape = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({0.5, 0}) * AffineTransform::Scale(0.1));

  EXPECT_FLOAT_EQ(target_shape.Coverage(query_shape), 0.1);
}

TEST(PartitionedMeshTest, CoverageWithPartitionedMeshMultipleTriangles) {
  PartitionedMesh target_shape = MakeRisingSawtoothShape();
  // This makes a ring with radius 0.1 centered at (1, 0).
  PartitionedMesh query_shape = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({1, 0}) * AffineTransform::Scale(0.1));

  EXPECT_FLOAT_EQ(target_shape.Coverage(query_shape), 0.3);
}

TEST(PartitionedMeshTest, CoverageWithPartitionedMeshOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh target_shape = MakeCoiledRingPartitionedMesh(16, 6);
  // This makes a ring with radius 0.05 centered at (0.6, 0.3), which hits two
  // overlapping triangles in `target_shape`, each of which has area 0.0812
  // unit^2.
  PartitionedMesh query_shape = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({0.6, 0.3}) * AffineTransform::Scale(0.05));

  EXPECT_THAT(target_shape.Coverage(query_shape), FloatNear(0.1071, 1e-4));
}

TEST(PartitionedMeshTest, CoverageWithPartitionedMeshWithTransform) {
  PartitionedMesh target_shape = MakeRisingSawtoothShape();
  PartitionedMesh query_shape = MakeStraightLinePartitionedMesh(3);

  EXPECT_FLOAT_EQ(
      target_shape.Coverage(query_shape, AffineTransform::Translate({0, 1})),
      1);
  EXPECT_FLOAT_EQ(
      target_shape.Coverage(query_shape, AffineTransform::Translate({-5, -5})),
      0);
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithTriangleMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Triangle query{{-5, 5}, {-10, 10}, {-10, 0}};

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0);
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithTriangleSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Triangle query{{1.5, 0}, {1.5, -1}, {1.6, 0.5}};

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0.2);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.19));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.21));
}

TEST(PartitionedMeshTest,
     CoverageIsGreaterThanWithTriangleOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);
  // This query hits two overlapping triangles each of area 0.0812 unit^2.
  Triangle query{{0.6, 0.3}, {0, 0}, {0, 0.1}};

  EXPECT_THAT(shape.Coverage(query), FloatNear(0.1071, 1e-4));
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.1));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.11));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithTriangleWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Triangle query{{0.5, 0}, {0.5, 5}, {0.6, 2}};
  AffineTransform transform = AffineTransform::Translate({3, 0});

  EXPECT_FLOAT_EQ(shape.Coverage(query, transform), 0.4);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.39, transform));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.41, transform));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithRectMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Rect query = Rect::FromCenterAndDimensions({-10, 10}, 5, 5);

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0);
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithRectSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Rect query = Rect::FromCenterAndDimensions({1.5, 0.5}, 0.2, 0.2);

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0.2);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.19));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.21));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithRectOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);
  // This query hits two overlapping triangles each of area 0.0812 unit^2.
  Rect query = Rect::FromCenterAndDimensions({0.6, 0.3}, 0.1, 0.1);

  EXPECT_THAT(shape.Coverage(query), FloatNear(0.1071, 1e-4));
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.1));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.11));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithRectWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Rect query = Rect::FromCenterAndDimensions({0.5, 0.5}, 0.2, 0.2);
  AffineTransform transform = AffineTransform::Translate({3, 0});

  EXPECT_FLOAT_EQ(shape.Coverage(query, transform), 0.4);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.39, transform));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.41, transform));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithQuadMissesShape) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Quad query = Quad::FromCenterAndDimensions({-10, 10}, 5, 5);

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0);
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithQuadSingleTriangle) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Quad query = Quad::FromCenterAndDimensions({1.5, 0.5}, 0.2, 0.2);

  EXPECT_FLOAT_EQ(shape.Coverage(query), 0.2);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.19));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.21));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithQuadOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh shape = MakeCoiledRingPartitionedMesh(16, 6);
  // This query hits two overlapping triangles each of area 0.0812 unit^2.
  Quad query = Quad::FromCenterAndDimensions({0.6, 0.3}, 0.1, 0.1);

  EXPECT_THAT(shape.Coverage(query), FloatNear(0.1071, 1e-4));
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.1));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.11));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithQuadWithTransform) {
  PartitionedMesh shape = MakeRisingSawtoothShape();
  Quad query = Quad::FromCenterAndDimensions({0.5, 0.5}, 0.2, 0.2);
  AffineTransform transform = AffineTransform::Translate({3, 0});

  EXPECT_FLOAT_EQ(shape.Coverage(query, transform), 0.4);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(query, 0.39, transform));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(query, 0.41, transform));
}

TEST(PartitionedMeshTest, CoverageIsGreaterThanWithPartitionedMeshMissesShape) {
  PartitionedMesh target = MakeRisingSawtoothShape();
  PartitionedMesh query = MakeStraightLinePartitionedMesh(
      3, {}, AffineTransform::Translate({-20, 20}));

  EXPECT_FLOAT_EQ(target.Coverage(query), 0);
  EXPECT_FALSE(target.CoverageIsGreaterThan(query, 0));
}

TEST(PartitionedMeshTest,
     CoverageIsGreaterThanWithPartitionedMeshSingleTriangle) {
  PartitionedMesh target = MakeRisingSawtoothShape();
  // This makes a ring with radius 0.1 centered at (1.5, 0.5).
  PartitionedMesh query = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({1.5, 0.5}) * AffineTransform::Scale(0.1));

  EXPECT_FLOAT_EQ(target.Coverage(query), 0.2);
  EXPECT_TRUE(target.CoverageIsGreaterThan(query, 0.19));
  EXPECT_FALSE(target.CoverageIsGreaterThan(query, 0.21));
}

TEST(PartitionedMeshTest,
     CoverageIsGreaterThanWithPartitionedMeshOverlappingTriangles) {
  // This shape has 16 triangles, half of which have area of 0.0812 unit^2 and
  // the other half of which have area of 0.1083 unit^2, and a total area of
  // 1.5155 unit^2.
  PartitionedMesh target = MakeCoiledRingPartitionedMesh(16, 6);
  // This makes a ring with radius 0.05 centered at (0.6, 0.3), which hits two
  // overlapping triangles each of area 0.0812 unit^2.
  PartitionedMesh query = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({0.6, 0.3}) * AffineTransform::Scale(0.05));

  EXPECT_THAT(target.Coverage(query), FloatNear(0.1071, 1e-4));
  EXPECT_TRUE(target.CoverageIsGreaterThan(query, 0.1));
  EXPECT_FALSE(target.CoverageIsGreaterThan(query, 0.11));
}

TEST(PartitionedMeshTest,
     CoverageIsGreaterThanWithPartitionedMeshWithTransform) {
  PartitionedMesh target = MakeRisingSawtoothShape();
  // This makes a ring with radius 0.1 centered at (0.5, 0.5).
  PartitionedMesh query = MakeCoiledRingPartitionedMesh(
      12, 6, {},
      AffineTransform::Translate({0.5, 0.5}) * AffineTransform::Scale(0.1));
  AffineTransform transform = AffineTransform::Translate({3, 0});

  EXPECT_FLOAT_EQ(target.Coverage(query, transform), 0.4);
  EXPECT_TRUE(target.CoverageIsGreaterThan(query, 0.39, transform));
  EXPECT_FALSE(target.CoverageIsGreaterThan(query, 0.41, transform));
}

TEST(PartitionedMeshTest, QueryAgainstSelf) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(4);

  EXPECT_THAT(GetAllIntersectedTriangles(shape, shape),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_FLOAT_EQ(shape.Coverage(shape), 1);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(shape, 0.99));
}

TEST(PartitionedMeshTest, QueryAgainstSelfWithTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(4);
  AffineTransform transform = AffineTransform::Translate({2.5, 0});

  EXPECT_THAT(GetAllIntersectedTriangles(shape, shape, transform),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_FLOAT_EQ(shape.Coverage(shape, transform), 0.5);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(shape, 0.49, transform));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(shape, 0.51, transform));
}

TEST(PartitionedMeshTest, QueryAgainstSelfEmptyShape) {
  PartitionedMesh shape;

  EXPECT_THAT(GetAllIntersectedTriangles(shape, shape), IsEmpty());
  EXPECT_FLOAT_EQ(shape.Coverage(shape), 0);
  EXPECT_FALSE(shape.CoverageIsGreaterThan(shape, 0));
}

TEST(PartitionedMeshTest, QueryAgainstCopy) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(4);
  PartitionedMesh copy = shape;

  EXPECT_THAT(GetAllIntersectedTriangles(shape, copy),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(copy, shape),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 0}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 1}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_FLOAT_EQ(shape.Coverage(copy), 1);
  EXPECT_FLOAT_EQ(copy.Coverage(shape), 1);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(copy, 0.99));
  EXPECT_TRUE(copy.CoverageIsGreaterThan(shape, 0.99));
}

TEST(PartitionedMeshTest, QueryAgainstCopyWithTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(4);
  PartitionedMesh copy = shape;
  AffineTransform transform = AffineTransform::Translate({2.5, 0});

  EXPECT_THAT(GetAllIntersectedTriangles(shape, copy, transform),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_THAT(GetAllIntersectedTriangles(copy, shape, transform),
              UnorderedElementsAre(
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 2}),
                  TriangleIndexPairEq({.mesh_index = 0, .triangle_index = 3})));
  EXPECT_FLOAT_EQ(shape.Coverage(copy, transform), 0.5);
  EXPECT_FLOAT_EQ(copy.Coverage(shape, transform), 0.5);
  EXPECT_TRUE(shape.CoverageIsGreaterThan(copy, 0.49, transform));
  EXPECT_TRUE(copy.CoverageIsGreaterThan(shape, 0.49, transform));
  EXPECT_FALSE(shape.CoverageIsGreaterThan(copy, 0.51, transform));
  EXPECT_FALSE(copy.CoverageIsGreaterThan(shape, 0.51, transform));
}

TEST(PartitionedMeshTest, QueryAgainstCopyEmptyShape) {
  PartitionedMesh shape;
  PartitionedMesh copy = shape;

  EXPECT_THAT(GetAllIntersectedTriangles(shape, copy), IsEmpty());
  EXPECT_THAT(GetAllIntersectedTriangles(copy, shape), IsEmpty());
  EXPECT_FLOAT_EQ(shape.Coverage(copy), 0);
  EXPECT_FLOAT_EQ(copy.Coverage(shape), 0);
  EXPECT_FALSE(shape.CoverageIsGreaterThan(copy, 0));
  EXPECT_FALSE(copy.CoverageIsGreaterThan(shape, 0));
}

TEST(PartitionedMeshDeathTest, OutlineGroupIndexOutOfBounds) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(10), {{1, 5, 4, 0}, {5, 9, 4}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(shape->Outline(2, 0), "");
}

TEST(PartitionedMeshDeathTest, OutlineOutlineIndexOutOfBounds) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(10), {{1, 5, 4, 0}, {5, 9, 4}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(shape->Outline(0, 2), "");
}

TEST(PartitionedMeshDeathTest, OutlinePositionGroupIndexOutOfBounds) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(10), {{1, 5, 4, 0}, {5, 9, 4}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(shape->OutlinePosition(2, 0, 0), "");
}

TEST(PartitionedMeshDeathTest, OutlinePositionOutlineIndexOutOfBounds) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(10), {{1, 5, 4, 0}, {5, 9, 4}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(shape->OutlinePosition(0, 2, 0), "");
}

TEST(PartitionedMeshDeathTest, OutlinePositionVertexIndexOutOfBounds) {
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMutableMesh(
      MakeStraightLineMutableMesh(10), {{1, 5, 4, 0}, {5, 9, 4}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  EXPECT_DEATH_IF_SUPPORTED(shape->OutlinePosition(0, 0, 4), "");
  EXPECT_DEATH_IF_SUPPORTED(shape->OutlinePosition(0, 1, 3), "");
}

}  // namespace
}  // namespace ink
