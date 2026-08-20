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

#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator_helpers.h"

#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::StrokeVertex;
using ::testing::Eq;
using ::testing::Optional;

// Local instances of label constants to make the test cases more concise:
constexpr StrokeVertex::Label kLeftExterior = StrokeVertex::kExteriorLeftLabel;
constexpr StrokeVertex::Label kRightExterior =
    StrokeVertex::kExteriorRightLabel;
constexpr StrokeVertex::Label kFrontExterior =
    StrokeVertex::kExteriorFrontLabel;
constexpr StrokeVertex::Label kBackExterior = StrokeVertex::kExteriorBackLabel;

void AppendVertex(
    MutableMeshView& mesh, Point position,
    StrokeVertex::Label side_label = StrokeVertex::kInteriorLabel,
    StrokeVertex::Label forward_label = StrokeVertex::kInteriorLabel) {
  mesh.AppendVertex(
      {.position = position,
       .new_non_position_attributes = {.side_label = side_label,
                                       .forward_label = forward_label}});
}

TEST(FindFirstExteriorVerticesTest, AllVerticesExterior) {
  // 0-1-2   Left: 0, 1, 2   Right: 3, 4, 5
  // |/|/|
  // 3-4-5

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 1}, kLeftExterior, kFrontExterior);
  AppendVertex(mesh_view, {1, 1}, kLeftExterior);
  AppendVertex(mesh_view, {2, 1}, kLeftExterior, kBackExterior);
  AppendVertex(mesh_view, {0, 0}, kRightExterior, kFrontExterior);
  AppendVertex(mesh_view, {1, 0}, kRightExterior);
  AppendVertex(mesh_view, {2, 0}, kRightExterior, kBackExterior);

  mesh_view.AppendTriangleIndices({0, 3, 1});
  mesh_view.AppendTriangleIndices({1, 3, 4});
  mesh_view.AppendTriangleIndices({1, 4, 2});
  mesh_view.AppendTriangleIndices({2, 4, 5});

  std::vector<SideId> vertex_side_ids = {
      SideId::kLeft,  SideId::kLeft,  SideId::kLeft,
      SideId::kRight, SideId::kRight, SideId::kRight,
  };

  OptionalSideIndexPair index_pair;

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 0);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 1);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 2);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(4)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 3);
  EXPECT_THAT(index_pair.left, Optional(Eq(2)));
  EXPECT_THAT(index_pair.right, Optional(Eq(4)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 4);
  EXPECT_THAT(index_pair.left, Eq(std::nullopt));
  EXPECT_THAT(index_pair.right, Eq(std::nullopt));
}

TEST(FindFirstIndexBelongingToEachSide, WithInteriorVertices) {
  // 0---1---5  Left: 0, 1, 4, 5   Right: 2, 3, 6, 7
  // |\ /|\ /|
  // | 4 | 7 |
  // |/ \|/ \|
  // 2---3---6

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 2}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex(mesh_view, {2, 2}, kLeftExterior);                   // 1
  AppendVertex(mesh_view, {0, 0}, kRightExterior, kFrontExterior);  // 2
  AppendVertex(mesh_view, {1, 0}, kRightExterior);                  // 3
  AppendVertex(mesh_view, {1, 1});                                  // 4
  AppendVertex(mesh_view, {4, 2}, kLeftExterior, kBackExterior);    // 5
  AppendVertex(mesh_view, {4, 0}, kRightExterior, kBackExterior);   // 6
  AppendVertex(mesh_view, {3, 1});                                  // 7

  mesh_view.AppendTriangleIndices({0, 2, 4});  // 0 L-R-L
  mesh_view.AppendTriangleIndices({0, 4, 1});  // 1 L-L-L
  mesh_view.AppendTriangleIndices({4, 2, 3});  // 2 L-R-R
  mesh_view.AppendTriangleIndices({1, 4, 3});  // 3 L-L-R
  mesh_view.AppendTriangleIndices({1, 3, 7});  // 4 L-R-R
  mesh_view.AppendTriangleIndices({1, 7, 5});  // 5 L-R-L
  mesh_view.AppendTriangleIndices({7, 3, 6});  // 6 R-R-R
  mesh_view.AppendTriangleIndices({5, 7, 6});  // 7 L-R-R

  std::vector<SideId> vertex_side_ids(8);
  vertex_side_ids[0] = SideId::kLeft;
  vertex_side_ids[1] = SideId::kLeft;
  vertex_side_ids[2] = SideId::kRight;
  vertex_side_ids[3] = SideId::kRight;
  vertex_side_ids[4] = SideId::kLeft;
  vertex_side_ids[5] = SideId::kLeft;
  vertex_side_ids[6] = SideId::kRight;
  vertex_side_ids[7] = SideId::kRight;

  OptionalSideIndexPair index_pair;

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 0);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 1);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 2);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 3);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 4);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 5);
  EXPECT_THAT(index_pair.left, Optional(Eq(1)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 6);
  EXPECT_THAT(index_pair.left, Optional(Eq(5)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 7);
  EXPECT_THAT(index_pair.left, Optional(Eq(5)));
  EXPECT_THAT(index_pair.right, Optional(Eq(6)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 8);
  EXPECT_THAT(index_pair.left, Eq(std::nullopt));
  EXPECT_THAT(index_pair.right, Eq(std::nullopt));
}

TEST(FindFirstExteriorVerticesTest, ForwardExteriorSideInterior) {
  // Somewhat contrived case to double check that we detect vertices that are
  // only labeled as "forward-exterior" as exterior. Current stroke mesh
  // generation should only output "forward" exterior vertices that are also
  // "side" exterior.
  //
  // 0-1-2   Left: 0, 1, 2   Right: 3, 4, 5
  // |/|/|
  // 3-4-5

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 1}, StrokeVertex::kInteriorLabel, kFrontExterior);
  AppendVertex(mesh_view, {1, 1});
  AppendVertex(mesh_view, {2, 1}, StrokeVertex::kInteriorLabel, kBackExterior);
  AppendVertex(mesh_view, {0, 0}, StrokeVertex::kInteriorLabel, kFrontExterior);
  AppendVertex(mesh_view, {1, 0});
  AppendVertex(mesh_view, {2, 0}, StrokeVertex::kInteriorLabel, kBackExterior);

  mesh_view.AppendTriangleIndices({0, 3, 1});
  mesh_view.AppendTriangleIndices({1, 3, 4});
  mesh_view.AppendTriangleIndices({1, 4, 2});
  mesh_view.AppendTriangleIndices({2, 4, 5});

  std::vector<SideId> vertex_side_ids = {
      SideId::kLeft,  SideId::kLeft,  SideId::kLeft,
      SideId::kRight, SideId::kRight, SideId::kRight,
  };

  OptionalSideIndexPair index_pair;

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 0);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 1);
  EXPECT_THAT(index_pair.left, Optional(Eq(2)));
  EXPECT_THAT(index_pair.right, Optional(Eq(3)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 2);
  EXPECT_THAT(index_pair.left, Optional(Eq(2)));
  EXPECT_THAT(index_pair.right, Optional(Eq(5)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 3);
  EXPECT_THAT(index_pair.left, Optional(Eq(2)));
  EXPECT_THAT(index_pair.right, Optional(Eq(5)));
}

TEST(FindFirstExteriorVerticesTest, NoLeftExteriorVerticesInLastTriangle) {
  // Somewhat contrived case to check that we can return nullopt on a per-side
  // basis.
  //
  // 0-1   Left: 0, 1   Right: 2, 3
  // |/|
  // 2-3

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 1}, kLeftExterior);
  AppendVertex(mesh_view, {1, 1});
  AppendVertex(mesh_view, {0, 0}, kRightExterior);
  AppendVertex(mesh_view, {1, 0});

  mesh_view.AppendTriangleIndices({0, 2, 1});
  mesh_view.AppendTriangleIndices({1, 2, 3});

  std::vector<SideId> vertex_side_ids = {SideId::kLeft, SideId::kLeft,
                                         SideId::kRight, SideId::kRight};

  OptionalSideIndexPair index_pair;

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 0);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 1);
  EXPECT_THAT(index_pair.left, Eq(std::nullopt));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));
}

TEST(FindFirstExteriorVerticesTest, NoRightExteriorVerticesInLastTriangle) {
  // Somewhat contrived case to check that we can return nullopt on a per-side
  // basis.
  //
  // 0-1   Left: 0, 1   Right: 2, 3
  // |\|
  // 2-3

  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 1}, kLeftExterior);
  AppendVertex(mesh_view, {1, 1});
  AppendVertex(mesh_view, {0, 0}, kRightExterior);
  AppendVertex(mesh_view, {1, 0});

  mesh_view.AppendTriangleIndices({0, 2, 3});
  mesh_view.AppendTriangleIndices({0, 3, 1});

  std::vector<SideId> vertex_side_ids = {SideId::kLeft, SideId::kLeft,
                                         SideId::kRight, SideId::kRight};

  OptionalSideIndexPair index_pair;

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 0);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Optional(Eq(2)));

  index_pair = FindFirstExteriorVertices(mesh_view, vertex_side_ids, 1);
  EXPECT_THAT(index_pair.left, Optional(Eq(0)));
  EXPECT_THAT(index_pair.right, Eq(std::nullopt));
}

TEST(StartingOffsetForCoincidentConnectedVerticesTest, AllPositionsAreUnique) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  AppendVertex(mesh_view, {0, 0});
  AppendVertex(mesh_view, {0, 1});
  AppendVertex(mesh_view, {1, 0});
  AppendVertex(mesh_view, {1, 1});
  AppendVertex(mesh_view, {2, 0});

  std::vector<IndexType> side_indices = {0, 2, 4};

  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 0),
      0);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 1),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 2),
      2);
}

TEST(StartingOffsetForCoincidentConnectedVerticesTest,
     EqualPositionsWithSameCategories) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  Point shared_point = {1, 0};

  AppendVertex(mesh_view, {0, 0});        // 0
  AppendVertex(mesh_view, shared_point);  // 1
  AppendVertex(mesh_view, {0, 1});        // 2
  AppendVertex(mesh_view, shared_point);  // 3
  AppendVertex(mesh_view, {1, 1});        // 4
  AppendVertex(mesh_view, shared_point);  // 5
  AppendVertex(mesh_view, {2, 0});        // 6

  std::vector<IndexType> side_indices = {0, 1, 3, 5, 6};

  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 0),
      0);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 1),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 2),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 3),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 4),
      4);
}

TEST(StartingOffsetForCoincidentConnectedVerticesTest,
     EqualPositionsWithChangingSideCategory) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  Point shared_point = {1, 0};

  AppendVertex(mesh_view, {0, 0});                       // 0
  AppendVertex(mesh_view, shared_point);                 // 1
  AppendVertex(mesh_view, {0, 1});                       // 2
  AppendVertex(mesh_view, shared_point, kLeftExterior);  // 3
  AppendVertex(mesh_view, {1, 1});                       // 4
  AppendVertex(mesh_view, shared_point, kLeftExterior);  // 5
  AppendVertex(mesh_view, {2, 0});                       // 6

  std::vector<IndexType> side_indices = {0, 1, 3, 5, 6};

  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 0),
      0);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 1),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 2),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 3),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 4),
      4);
}

TEST(StartingOffsetForCoincidentConnectedVerticesTest,
     EqualPositionsWithChangingForwardCategories) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);

  Point shared_point = {1, 0};

  AppendVertex(mesh_view, {0, 0}, kLeftExterior);                        // 0
  AppendVertex(mesh_view, shared_point, kLeftExterior, kBackExterior);   // 1
  AppendVertex(mesh_view, {0, 1});                                       // 2
  AppendVertex(mesh_view, shared_point, kLeftExterior, kBackExterior);   // 3
  AppendVertex(mesh_view, {1, 1});                                       // 4
  AppendVertex(mesh_view, shared_point, kLeftExterior, kFrontExterior);  // 5
  AppendVertex(mesh_view, {2, 0}, kLeftExterior);                        // 6

  std::vector<IndexType> side_indices = {0, 1, 3, 5, 6};

  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 0),
      0);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 1),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 2),
      1);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 3),
      3);
  EXPECT_EQ(
      StartingOffsetForCoincidentConnectedVertices(mesh_view, side_indices, 4),
      4);
}

TEST(StartingOffsetForCoincidentConnectedVerticesDeathTest,
     OutOfBoundsOffsetParameter) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  mesh_view.AppendVertex({});
  std::vector<IndexType> side_indices = {0, 1, 2};

  EXPECT_DEATH_IF_SUPPORTED(StartingOffsetForCoincidentConnectedVertices(
                                mesh_view, side_indices, side_indices.size()),
                            "");
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
