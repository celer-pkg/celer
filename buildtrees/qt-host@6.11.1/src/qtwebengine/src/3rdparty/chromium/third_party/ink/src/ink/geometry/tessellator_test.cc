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

#include "ink/geometry/tessellator.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"

namespace ink {

namespace {

using ::absl_testing::IsOk;
using ::absl_testing::StatusIs;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;

TEST(TessellatorTest, ReturnsErrorForEmptyPolyline) {
  EXPECT_THAT(
      CreateMeshFromPolyline({}),
      StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("size: 0")));
}

TEST(TessellatorTest, ReturnsErrorForSinglePointPolyline) {
  EXPECT_THAT(
      CreateMeshFromPolyline({Point{0, 0}}),
      StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("size: 1")));
}

TEST(TessellatorTest, ReturnsErrorForTwoPointPolyline) {
  EXPECT_THAT(
      CreateMeshFromPolyline({Point{0, 0}, Point{10, 10}}),
      StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr("size: 2")));
}

TEST(TessellatorTest, ReturnsErrorForCollinearPoints) {
  EXPECT_THAT(CreateMeshFromPolyline(
                  {Point{0, 0}, Point{1, 2}, Point{2, 4}, Point{3, 6}}),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("tessellate")));
}

TEST(TessellatorFuzzTest, ReturnsErrorForOverflowingBoundingBox) {
  // Each point is finite, but some are too big for the tessellator to handle
  // without a risk of overflow errors.
  EXPECT_THAT(CreateMeshFromPolyline(
                  {Point{-2e+38f, 0}, Point{0, 0}, Point{2e+38f, -1}}),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("tessellate")));
  EXPECT_THAT(CreateMeshFromPolyline(
                  {Point{0, 0}, Point{0, 2e+38f}, Point{-1, -2e+38f}}),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("tessellate")));
  EXPECT_THAT(CreateMeshFromPolyline(
                  {Point{-1e+17f, 0}, Point{0, 1e+21f}, Point{1e17f, -1e+21f}}),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("tessellate")));

  // The bounding box doesn't overflow, but some coordinates are still too big.
  // The tessellator allows coordinates between -2^23 and 2^23.
  EXPECT_THAT(CreateMeshFromPolyline(
                  {Point{-2e+37f, 0}, Point{0, 5}, Point{1e37f, -5}}),
              StatusIs(absl::StatusCode::kInternal, HasSubstr("tessellate")));
}

TEST(TessellatorTest, ReturnsMeshForSingleTriangle) {
  absl::StatusOr<Mesh> mesh =
      CreateMeshFromPolyline({Point{0, 0}, Point{10, 0}, Point{0, 10}});
  ASSERT_THAT(mesh, IsOk());

  EXPECT_THAT(mesh->VertexCount(), Eq(3));
  EXPECT_THAT(mesh->TriangleCount(), Eq(1));

  EXPECT_THAT(mesh->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(mesh->VertexPositionAttributeIndex(), 0);
  EXPECT_EQ(mesh->VertexStride(), 8);
  EXPECT_EQ(mesh->IndexStride(), 2);
  EXPECT_THAT(mesh->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));

  EXPECT_EQ(mesh->VertexPosition(0), (Point{0, 0}));
  EXPECT_EQ(mesh->VertexPosition(1), (Point{10, 0}));
  EXPECT_EQ(mesh->VertexPosition(2), (Point{0, 10}));

  EXPECT_THAT(mesh->TriangleIndices(0), ElementsAre(1, 2, 0));

  EXPECT_THAT(mesh->GetTriangle(0), TriangleEq({{10, 0}, {0, 10}, {0, 0}}));
}

TEST(TessellatorTest, ReturnsMeshForConcaveLoop) {
  absl::StatusOr<Mesh> mesh = CreateMeshFromPolyline(
      {Point{0, 0}, Point{10, 0}, Point{2, 2}, Point{0, 10}});
  ASSERT_THAT(mesh, IsOk());

  EXPECT_THAT(mesh->VertexCount(), Eq(4));
  EXPECT_THAT(mesh->TriangleCount(), Eq(2));

  EXPECT_THAT(mesh->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(mesh->VertexPositionAttributeIndex(), 0);
  EXPECT_EQ(mesh->VertexStride(), 8);
  EXPECT_EQ(mesh->IndexStride(), 2);
  EXPECT_THAT(mesh->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));

  EXPECT_EQ(mesh->VertexPosition(0), (Point{0, 0}));
  EXPECT_EQ(mesh->VertexPosition(1), (Point{10, 0}));
  EXPECT_EQ(mesh->VertexPosition(2), (Point{2, 2}));
  EXPECT_EQ(mesh->VertexPosition(3), (Point{0, 10}));

  EXPECT_THAT(mesh->TriangleIndices(0), ElementsAre(2, 0, 1));
  EXPECT_THAT(mesh->TriangleIndices(1), ElementsAre(0, 2, 3));

  EXPECT_THAT(mesh->GetTriangle(0), TriangleEq({{2, 2}, {0, 0}, {10, 0}}));
  EXPECT_THAT(mesh->GetTriangle(1), TriangleEq({{0, 0}, {2, 2}, {0, 10}}));
}

// Verifies that the tessellation succeeds and CreateMeshForPolyline() preserves
// the duplicate vertices for polyline:
//   \  |\
//    \ | \
//     \|  \
// ----------
//    (10, 0) -> Duplicated point
TEST(TessellatorTest, PreservesDuplicatePointsAndReturnsMesh) {
  absl::StatusOr<Mesh> mesh =
      CreateMeshFromPolyline({Point{0, 0}, Point{10, 0}, Point{20, 0},
                              Point{15, 5}, Point{10, 0}, Point{5, 5}});
  ASSERT_THAT(mesh, IsOk());
  EXPECT_THAT(mesh->VertexCount(), Eq(6));
  EXPECT_THAT(mesh->TriangleCount(), Eq(2));

  EXPECT_THAT(mesh->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(mesh->VertexPositionAttributeIndex(), 0);
  EXPECT_EQ(mesh->VertexStride(), 8);
  EXPECT_EQ(mesh->IndexStride(), 2);
  EXPECT_THAT(mesh->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));

  EXPECT_EQ(mesh->VertexPosition(0), (Point{0, 0}));
  EXPECT_EQ(mesh->VertexPosition(1), (Point{10, 0}));
  EXPECT_EQ(mesh->VertexPosition(2), (Point{20, 0}));
  EXPECT_EQ(mesh->VertexPosition(3), (Point{15, 5}));
  EXPECT_EQ(mesh->VertexPosition(4), (Point{10, 0}));
  EXPECT_EQ(mesh->VertexPosition(5), (Point{5, 5}));

  EXPECT_THAT(mesh->TriangleIndices(0), ElementsAre(1, 5, 0));
  EXPECT_THAT(mesh->TriangleIndices(1), ElementsAre(2, 3, 1));

  EXPECT_THAT(mesh->GetTriangle(0), TriangleEq({{10, 0}, {5, 5}, {0, 0}}));
  EXPECT_THAT(mesh->GetTriangle(1), TriangleEq({{20, 0}, {15, 5}, {10, 0}}));
}

// Verifies that the tessellation succeeds, and CreateMeshFromPolyline() creates
// a mesh with extra vertex at the intersection, for self-intersecting polyline:
// \   /|
//  \ / |
//  /\  |
// /  \ |
///    \|
TEST(TessellatorTest, ReturnsMeshForSelfIntersectingPolyline) {
  absl::StatusOr<Mesh> mesh = CreateMeshFromPolyline(
      {Point{0, 0}, Point{10, 10}, Point{10, 0}, Point{0, 10}});
  ASSERT_THAT(mesh, IsOk());
  EXPECT_THAT(mesh->VertexCount(), Eq(5));
  EXPECT_THAT(mesh->TriangleCount(), Eq(2));

  EXPECT_THAT(mesh->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(mesh->VertexPositionAttributeIndex(), 0);
  EXPECT_EQ(mesh->VertexStride(), 8);
  EXPECT_EQ(mesh->IndexStride(), 2);
  EXPECT_THAT(mesh->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));

  // Verify that the tessellator introduces an extra vertex (5, 5) at the
  // self-intersection of the input polyline.
  EXPECT_EQ(mesh->VertexPosition(0), (Point{0, 0}));
  EXPECT_EQ(mesh->VertexPosition(1), (Point{10, 10}));
  EXPECT_EQ(mesh->VertexPosition(2), (Point{10, 0}));
  EXPECT_EQ(mesh->VertexPosition(3), (Point{0, 10}));
  EXPECT_EQ(mesh->VertexPosition(4), (Point{5, 5}));

  EXPECT_THAT(mesh->TriangleIndices(0), ElementsAre(1, 4, 2));
  EXPECT_THAT(mesh->TriangleIndices(1), ElementsAre(4, 3, 0));

  EXPECT_THAT(mesh->GetTriangle(0), TriangleEq({{10, 10}, {5, 5}, {10, 0}}));
  EXPECT_THAT(mesh->GetTriangle(1), TriangleEq({{5, 5}, {0, 10}, {0, 0}}));
}

}  // namespace
}  // namespace ink
