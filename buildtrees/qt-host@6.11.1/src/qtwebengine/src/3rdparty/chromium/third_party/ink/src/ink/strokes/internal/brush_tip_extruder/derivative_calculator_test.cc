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

#include "ink/strokes/internal/brush_tip_extruder/derivative_calculator.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::StrokeVertex;
using ::testing::Each;
using ::testing::ElementsAreArray;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Not;

// Local instances of label constants to make the test cases more concise:
constexpr StrokeVertex::Label kLeftExterior = StrokeVertex::kExteriorLeftLabel;
constexpr StrokeVertex::Label kRightExterior =
    StrokeVertex::kExteriorRightLabel;
constexpr StrokeVertex::Label kFrontExterior =
    StrokeVertex::kExteriorFrontLabel;
constexpr StrokeVertex::Label kBackExterior = StrokeVertex::kExteriorBackLabel;
constexpr StrokeVertex::Label kInterior = StrokeVertex::kInteriorLabel;

struct StrokeDimensions {
  float width;
  float length;
};

struct SideIndices {
  std::vector<uint32_t> left;
  std::vector<uint32_t> right;
};

class DerivativeCalculatorTest : public ::testing::Test {
 protected:
  DerivativeCalculatorTest()
      : ::testing::Test(),
        mesh_(StrokeVertex::FullMeshFormat()),
        mesh_view_(mesh_) {}

  std::vector<Vec> GetSideDerivativeValues() const {
    std::vector<Vec> values;
    for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
      values.push_back(StrokeVertex::GetFromMesh(mesh_, i)
                           .non_position_attributes.side_derivative);
    }
    return values;
  }

  std::vector<Vec> GetForwardDerivativeValues() const {
    std::vector<Vec> values;
    for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
      values.push_back(StrokeVertex::GetFromMesh(mesh_, i)
                           .non_position_attributes.forward_derivative);
    }
    return values;
  }

  std::vector<float> GetSideMarginValues() const {
    std::vector<float> values;
    for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
      values.push_back(StrokeVertex::GetFromMesh(mesh_, i)
                           .non_position_attributes.side_label.DecodeMargin());
    }
    return values;
  }

  void AppendVertex(Point position, StrokeVertex::Label side_label,
                    StrokeVertex::Label forward_label = kInterior) {
    mesh_view_.AppendVertex(
        {.position = position,
         .new_non_position_attributes = {.side_label = side_label,
                                         .forward_label = forward_label}});
  }

  SideIndices AppendRectangularMesh(StrokeDimensions dimensions);

  MutableMesh mesh_;
  MutableMeshView mesh_view_;
  DerivativeCalculator calculator_;
};

// Fills `mesh_view_` with a rectangular mesh consisting of 6 vertices and 4
// triangles. The mesh is shaped so that any non-zero value for a type of
// derivative should have the same value.
//
//  left side  ->  0-2-4
//                 |/|/|   travel direction -->
//  right side ->  1-3-5
//
SideIndices DerivativeCalculatorTest::AppendRectangularMesh(
    StrokeDimensions dimensions) {
  AppendVertex({0, dimensions.width}, kLeftExterior, kFrontExterior);
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);
  AppendVertex({dimensions.length / 2, dimensions.width}, kLeftExterior);
  AppendVertex({dimensions.length / 2, 0}, kRightExterior);
  AppendVertex({dimensions.length, dimensions.width}, kLeftExterior,
               kBackExterior);
  AppendVertex({dimensions.length, 0}, kRightExterior, kBackExterior);

  mesh_view_.AppendTriangleIndices({0, 1, 2});
  mesh_view_.AppendTriangleIndices({2, 1, 3});
  mesh_view_.AppendTriangleIndices({2, 3, 4});
  mesh_view_.AppendTriangleIndices({4, 3, 5});

  return {.left = {0, 2, 4}, .right = {1, 3, 5}};
}

TEST_F(DerivativeCalculatorTest, CompleteUpdateForRectangularMesh) {
  SideIndices side_indices = AppendRectangularMesh({.width = 3, .length = 2});
  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  EXPECT_THAT(GetSideDerivativeValues(), Each(VecNear({0, -3}, 0.0001)));
  EXPECT_THAT(GetForwardDerivativeValues(), Each(VecNear({1, 0}, 0.0001)));

  EXPECT_THAT(GetSideMarginValues(), Each(StrokeVertex::kMaximumMargin));
}

TEST_F(DerivativeCalculatorTest, PartialUpdateForRectangularMesh) {
  SideIndices side_indices = AppendRectangularMesh({.width = 2, .length = 5});

  ASSERT_THAT(absl::MakeSpan(side_indices.left).last(1), ElementsAreArray({4}));
  ASSERT_THAT(absl::MakeSpan(side_indices.right).last(1),
              ElementsAreArray({5}));

  calculator_.UpdateMesh({4}, {5}, mesh_view_);

  // Only the vertices at 4, and 5 should have their values updated:
  EXPECT_THAT(GetSideDerivativeValues(), ElementsAreArray({
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecNear({0, -2}, 0.0001),
                                             VecNear({0, -2}, 0.0001),
                                         }));
  EXPECT_THAT(GetForwardDerivativeValues(), ElementsAreArray({
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecNear({2.5, 0}, 0.0001),
                                                VecNear({2.5, 0}, 0.0001),
                                            }));
}

TEST_F(DerivativeCalculatorTest, UpdateForRectangularMeshWithoutLeftIndices) {
  SideIndices side_indices = AppendRectangularMesh({.width = 2, .length = 5});

  ASSERT_THAT(absl::MakeSpan(side_indices.right).last(2),
              ElementsAreArray({3, 5}));
  calculator_.UpdateMesh({}, {3, 5}, mesh_view_);

  // Only the vertices at 3 and 5 should have their values updated:
  EXPECT_THAT(GetSideDerivativeValues(), ElementsAreArray({
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecNear({0, -2}, 0.0001),
                                             VecEq({0, 0}),
                                             VecNear({0, -2}, 0.0001),
                                         }));
  EXPECT_THAT(GetForwardDerivativeValues(), ElementsAreArray({
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecNear({2.5, 0}, 0.0001),
                                                VecEq({0, 0}),
                                                VecNear({2.5, 0}, 0.0001),
                                            }));
}

TEST_F(DerivativeCalculatorTest, UpdateForRectangularMeshWithoutRightIndices) {
  SideIndices side_indices = AppendRectangularMesh({.width = 2, .length = 5});

  ASSERT_THAT(absl::MakeSpan(side_indices.left).last(2),
              ElementsAreArray({2, 4}));
  calculator_.UpdateMesh({2, 4}, {}, mesh_view_);

  // Only the vertices at 2 and 4 should have their values updated:
  EXPECT_THAT(GetSideDerivativeValues(), ElementsAreArray({
                                             VecEq({0, 0}),
                                             VecEq({0, 0}),
                                             VecNear({0, -2}, 0.0001),
                                             VecEq({0, 0}),
                                             VecNear({0, -2}, 0.0001),
                                             VecEq({0, 0}),
                                         }));
  EXPECT_THAT(GetForwardDerivativeValues(), ElementsAreArray({
                                                VecEq({0, 0}),
                                                VecEq({0, 0}),
                                                VecNear({2.5, 0}, 0.0001),
                                                VecEq({0, 0}),
                                                VecNear({2.5, 0}, 0.0001),
                                                VecEq({0, 0}),
                                            }));
}

TEST_F(DerivativeCalculatorTest, UpdateForRectangularMeshWithEmptyIndices) {
  SideIndices side_indices = AppendRectangularMesh({.width = 2, .length = 5});
  calculator_.UpdateMesh({}, {}, mesh_view_);

  EXPECT_THAT(GetSideDerivativeValues(), Each(VecEq({0, 0})));
  EXPECT_THAT(GetForwardDerivativeValues(), Each(VecEq({0, 0})));
}

TEST_F(DerivativeCalculatorTest, VaryingWidthMesh) {
  // Fill with a mesh consisting of 6 vertices and 4 triangles that has
  // non-uniform separation between vertices:
  //
  //                     4
  //                    /|
  //  left side  ->  0-2 |    travel direction -->
  //                 |\|\|
  //                 | 3-5
  //                 |/
  //  right side ->  1
  //
  AppendVertex({0, 3}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({2, 3}, kLeftExterior);                   // 2
  AppendVertex({6, 1}, kRightExterior);                  // 3
  AppendVertex({8, 4}, kLeftExterior, kBackExterior);    // 4
  AppendVertex({8, 1}, kRightExterior, kBackExterior);   // 5

  mesh_view_.AppendTriangleIndices({0, 1, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 2});
  mesh_view_.AppendTriangleIndices({2, 3, 5});
  mesh_view_.AppendTriangleIndices({2, 5, 4});

  SideIndices side_indices = {.left = {0, 2, 4}, .right = {1, 3, 5}};

  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  EXPECT_THAT(GetSideDerivativeValues(), ElementsAreArray({
                                             VecNear({0.208, -2.47}, 0.005),
                                             VecNear({0.49, -2.92}, 0.005),
                                             VecNear({0.13, -2.32}, 0.005),
                                             VecNear({0.13, -2.32}, 0.005),
                                             VecNear({0.49, -2.92}, 0.005),
                                             VecNear({0.20, -2.47}, 0.005),
                                         }));
  EXPECT_THAT(GetForwardDerivativeValues(), ElementsAreArray({
                                                VecNear({2.93, 1.81}, 0.005),
                                                VecNear({6, 0}, 0.005),
                                                VecNear({1.89, 1.78}, 0.005),
                                                VecNear({1.89, 1.78}, 0.005),
                                                VecNear({6, 0}, 0.005),
                                                VecNear({2.93, 1.81}, 0.005),
                                            }));

  EXPECT_THAT(GetSideMarginValues(), ElementsAreArray({
                                         FloatNear(0.35, 0.005),
                                         FloatEq(StrokeVertex::kMaximumMargin),
                                         FloatEq(StrokeVertex::kMaximumMargin),
                                         FloatEq(StrokeVertex::kMaximumMargin),
                                         FloatEq(StrokeVertex::kMaximumMargin),
                                         FloatNear(0.35, 0.005),
                                     }));
}

TEST_F(DerivativeCalculatorTest, MeshWithInteriorSideVertices) {
  // Fill with a mesh consisting of 5 vertices and 4 triangles where one of the
  // vertices has an interior side label.
  //
  // left side  ->  0---3
  //                |\ /|
  //                | 2 |    travel direction -->
  //                |/ \|
  // right side ->  1---4
  //
  AppendVertex({0, 2}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({1, 1}, kInterior);                       // 2
  AppendVertex({2, 2}, kLeftExterior, kBackExterior);    // 3
  AppendVertex({2, 0}, kRightExterior, kBackExterior);   // 4

  mesh_view_.AppendTriangleIndices({0, 1, 2});
  mesh_view_.AppendTriangleIndices({0, 2, 3});
  mesh_view_.AppendTriangleIndices({2, 1, 4});
  mesh_view_.AppendTriangleIndices({3, 2, 4});

  SideIndices side_indices = {.left = {0, 2, 3}, .right = {1, 4}};
  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  EXPECT_THAT(GetSideDerivativeValues(), Each(VecNear({0, -1}, 0.005)));
  EXPECT_THAT(GetForwardDerivativeValues(), Each(VecNear({1, 0}, 0.005)));

  EXPECT_THAT(GetSideMarginValues(), ElementsAreArray({
                                         StrokeVertex::kMaximumMargin,
                                         StrokeVertex::kMaximumMargin,
                                         0.f,
                                         StrokeVertex::kMaximumMargin,
                                         StrokeVertex::kMaximumMargin,
                                     }));
}

TEST_F(DerivativeCalculatorTest, SharpTurnMaintainsDerivativeMagnitudes) {
  // We are interested in the side derivative value at index 3 of the mesh
  // shaped approximately as:
  //
  //       4---5
  //      / \ / \           /\    travel
  //     2---3---6         /  \    direction
  //    / \ / \ / \       /   \/
  //   0----1 7----8
  //
  AppendVertex({0, 0}, kLeftExterior, kFrontExterior);      // 0
  AppendVertex({1.49, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({0.5, 1}, kLeftExterior);                    // 2
  AppendVertex({1.5, 1}, kRightExterior);                   // 3
  AppendVertex({1, 2}, kLeftExterior);                      // 4
  AppendVertex({2, 2}, kLeftExterior);                      // 5
  AppendVertex({2.5, 1}, kLeftExterior);                    // 6
  AppendVertex({1.51, 0}, kRightExterior, kBackExterior);   // 7
  AppendVertex({3, 0}, kLeftExterior, kBackExterior);       // 8

  mesh_view_.AppendTriangleIndices({0, 1, 2});
  mesh_view_.AppendTriangleIndices({2, 1, 3});
  mesh_view_.AppendTriangleIndices({2, 3, 4});
  mesh_view_.AppendTriangleIndices({4, 3, 5});
  mesh_view_.AppendTriangleIndices({5, 3, 6});
  mesh_view_.AppendTriangleIndices({6, 3, 7});
  mesh_view_.AppendTriangleIndices({6, 7, 8});

  SideIndices side_indices = {.left = {0, 2, 4, 5, 6, 8}, .right = {1, 3, 7}};

  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  // The side derivative at vertex 3 should be completely in the negative-y
  // direction due to symmetry. The magnitude of the resulting "average" should
  // be maintained rather than canceled out as would be by normal component-wise
  // arithmetic averaging of vectors.
  EXPECT_THAT(StrokeVertex::GetFromMesh(mesh_, 3)
                  .non_position_attributes.side_derivative,
              VecNear({0, -1}, 0.05));
}

TEST_F(DerivativeCalculatorTest,
       CoincidentVerticesWithSameCategoryGetEqualValues) {
  // Fill with a mesh consisting of 9 vertices and 7 triangles where two sets of
  // vertices are coincident, also making three of the triangles trivially
  // degenerate.
  //
  //  left side  -> 0--2,4---7
  //                 |\|   / |
  //                 1-3,5,6 |   travel direction -->
  //                     \   |
  //  right side ->        \ 8
  //
  AppendVertex({-1, 1}, kLeftExterior, kFrontExterior);  // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({1, 1}, kLeftExterior);                   // 2
  AppendVertex({1, 0}, kRightExterior);                  // 3
  AppendVertex({1, 1}, kLeftExterior);                   // 4
  AppendVertex({1, 0}, kRightExterior);                  // 5
  AppendVertex({1, 0}, kRightExterior);                  // 6
  AppendVertex({3, 1}, kLeftExterior, kBackExterior);    // 7
  AppendVertex({3, -2}, kRightExterior, kBackExterior);  // 8

  ASSERT_EQ(mesh_view_.GetPosition(2), mesh_view_.GetPosition(4));
  ASSERT_EQ(mesh_view_.GetPosition(3), mesh_view_.GetPosition(5));
  ASSERT_EQ(mesh_view_.GetPosition(3), mesh_view_.GetPosition(6));

  mesh_view_.AppendTriangleIndices({0, 1, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 2});
  mesh_view_.AppendTriangleIndices({2, 3, 4});
  mesh_view_.AppendTriangleIndices({4, 3, 5});
  mesh_view_.AppendTriangleIndices({4, 5, 6});
  mesh_view_.AppendTriangleIndices({4, 6, 7});
  mesh_view_.AppendTriangleIndices({7, 6, 8});

  SideIndices side_indices = {.left = {0, 2, 4, 7}, .right = {1, 3, 5, 6, 8}};

  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  std::vector<Vec> side_derivatives = GetSideDerivativeValues();
  EXPECT_THAT(GetSideDerivativeValues(), Each(Not(VecNear({0, 0}, 0.005))));

  // Check exact equality for coincident vertices:
  EXPECT_THAT(side_derivatives[4], VecEq(side_derivatives[2]));
  EXPECT_THAT(side_derivatives[5], VecEq(side_derivatives[3]));
  EXPECT_THAT(side_derivatives[6], VecEq(side_derivatives[3]));

  std::vector<Vec> forward_derivatives = GetForwardDerivativeValues();
  EXPECT_THAT(GetForwardDerivativeValues(), Each(Not(VecNear({0, 0}, 0.005))));

  // Check exact equality for coincident vertices:
  EXPECT_THAT(forward_derivatives[4], VecEq(forward_derivatives[2]));
  EXPECT_THAT(forward_derivatives[5], VecEq(forward_derivatives[3]));
  EXPECT_THAT(forward_derivatives[6], VecEq(forward_derivatives[3]));
}

TEST_F(DerivativeCalculatorTest,
       CoincidentVerticesWithDifferentForwardCategoriesGetDifferentValues) {
  // Fill with a mesh consisting of 8 vertices and 4 triangles where two pairs
  // of vertices are coincident, but have different label categories.
  //
  //  left side  ->  0-2,4---6
  //                 |\|   / |
  //  right side ->  1-3,5---7   travel direction -->
  //
  AppendVertex({0, 1}, kLeftExterior, kFrontExterior);     // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);    // 1
  AppendVertex({1, 1}, kLeftExterior, kBackExterior);      // 2
  AppendVertex({1, 0}, kRightExterior, kBackExterior);     // 3
  AppendVertex({1, 1}, kLeftExterior, kFrontExterior);     // 4
  AppendVertex({1, 0}, kRightExterior, kFrontExterior);    // 5
  AppendVertex({3, 0.75}, kLeftExterior, kBackExterior);   // 6
  AppendVertex({3, 0.25}, kRightExterior, kBackExterior);  // 7

  ASSERT_EQ(mesh_view_.GetPosition(2), mesh_view_.GetPosition(4));
  ASSERT_EQ(mesh_view_.GetPosition(3), mesh_view_.GetPosition(5));

  mesh_view_.AppendTriangleIndices({0, 1, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 2});
  mesh_view_.AppendTriangleIndices({4, 5, 6});
  mesh_view_.AppendTriangleIndices({6, 5, 7});

  SideIndices side_indices = {.left = {0, 2, 4, 6}, .right = {1, 3, 5, 7}};

  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  // The concident vertices should not get the average value of derivatives
  // because they have different label categories.

  std::vector<Vec> side_derivatives = GetSideDerivativeValues();
  EXPECT_THAT(side_derivatives[4], Not(VecEq(side_derivatives[2])));
  EXPECT_THAT(side_derivatives[5], Not(VecEq(side_derivatives[3])));

  std::vector<Vec> forward_derivatives = GetForwardDerivativeValues();
  EXPECT_THAT(forward_derivatives[4], Not(VecEq(forward_derivatives[2])));
  EXPECT_THAT(forward_derivatives[5], Not(VecEq(forward_derivatives[3])));
}

TEST_F(DerivativeCalculatorTest,
       CoincidentVerticesWithDifferentSideCategoriesGetDifferentValues) {
  // Fill with a mesh consisting of 9 vertices and 6 triangles that mimicks part
  // of the stroke mesh after `Geometry::GiveUpIntersectionHandling()` on the
  // left side of the stroke. Vertices 3 and 5 will share positions, but vertex
  // 2 is side-interior and vertex 5 is side-exterior.
  //
  //                  7---8
  //                  |  /|                       ^
  // left side  ->  0-2 / |                       |
  //                |\|/  |                       |
  //                |3,5--6    travel direction --/
  //                |/| /
  // right side ->  1-4
  //
  AppendVertex({0, 2}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({1, 1}, kInterior);                       // 2
  AppendVertex({1, 2}, kLeftExterior);                   // 3
  AppendVertex({1, 0}, kRightExterior);                  // 4
  AppendVertex({1, 1}, kLeftExterior);                   // 5
  AppendVertex({2, 1}, kRightExterior);                  // 6
  AppendVertex({1, 3}, kLeftExterior, kBackExterior);    // 7
  AppendVertex({2, 3}, kRightExterior, kBackExterior);   // 8

  mesh_view_.AppendTriangleIndices({0, 1, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 2});
  mesh_view_.AppendTriangleIndices({3, 1, 4});
  mesh_view_.AppendTriangleIndices({3, 4, 6});
  mesh_view_.AppendTriangleIndices({5, 6, 8});
  mesh_view_.AppendTriangleIndices({5, 8, 7});

  SideIndices side_indices = {.left = {0, 2, 3, 5, 7}, .right = {1, 4, 6, 8}};
  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  // The concident vertices should not get the average value of derivatives
  // because they have different label categories.

  std::vector<Vec> side_derivatives = GetSideDerivativeValues();
  EXPECT_THAT(side_derivatives[5], Not(VecEq(side_derivatives[3])));

  std::vector<Vec> forward_derivatives = GetForwardDerivativeValues();
  EXPECT_THAT(forward_derivatives[5], Not(VecEq(forward_derivatives[3])));
}

TEST_F(DerivativeCalculatorTest,
       MeshTriangleHasTwoForwardInteriorSameSideVertices) {
  // Fill with a mesh consisting of 6 vertices and 4 triangles that starts
  // traveling to the right and quickly turns toward the positive-y. This
  // exercises the edge case where a triangle (with indices {0, 2, 3}) has
  // of one vertex that is forward-exterior (with index 0) and two vertices that
  // are forward-interior and belong to the same side (with indices {2, 3}).
  //
  //                   4----5
  //                  /  \ /
  //  left side  ->  0----3                      ^
  //                 | \ /   travel direction --/
  //  right side ->  1--2
  //
  AppendVertex({0, 2}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex({0, 0}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({1, 0}, kRightExterior);                  // 2
  AppendVertex({2, 2}, kRightExterior);                  // 3
  AppendVertex({1, 3}, kLeftExterior, kBackExterior);    // 4
  AppendVertex({3, 3}, kRightExterior, kBackExterior);   // 4

  mesh_view_.AppendTriangleIndices({0, 1, 2});
  mesh_view_.AppendTriangleIndices({0, 2, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 4});
  mesh_view_.AppendTriangleIndices({4, 3, 5});

  SideIndices side_indices = {.left = {0, 4}, .right = {1, 2, 3, 5}};
  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  // We expect for none of the vertices to be skipped in having a non-zero value
  // for forward-derivative assigned:
  EXPECT_THAT(GetForwardDerivativeValues(), Each(Not(VecNear({0, 0}, 0.005))));
}

TEST_F(DerivativeCalculatorTest,
       NonTriviallyDegenerateTriangleGetsZeroMargins) {
  // Fill with a mesh with 7 vertices and 5 triangles that travels to the right
  // and suddenly changes width. With vertices {2, 3, 4} forming a degenerate
  // triangle, this exercises the edge case where the three vertices should get
  // zero margin values to ensure they stay collinear.
  //
  //  left side  ->  0-2---5
  //                 |\|\  |
  //  right side ->  1-3 \ |    travel direction -->
  //                   |  \|
  //                   4---6
  AppendVertex({0, 2}, kLeftExterior, kFrontExterior);   // 0
  AppendVertex({0, 1}, kRightExterior, kFrontExterior);  // 1
  AppendVertex({1, 2}, kLeftExterior);                   // 2
  AppendVertex({1, 1}, kRightExterior);                  // 3
  AppendVertex({1, 0}, kRightExterior);                  // 4
  AppendVertex({3, 2}, kLeftExterior, kBackExterior);    // 5
  AppendVertex({3, 0}, kRightExterior, kBackExterior);   // 6

  mesh_view_.AppendTriangleIndices({0, 1, 3});
  mesh_view_.AppendTriangleIndices({0, 3, 2});
  mesh_view_.AppendTriangleIndices({2, 3, 4});
  mesh_view_.AppendTriangleIndices({2, 4, 6});
  mesh_view_.AppendTriangleIndices({2, 6, 5});

  SideIndices side_indices = {.left = {0, 2, 5}, .right = {1, 3, 4, 6}};
  calculator_.UpdateMesh(side_indices.left, side_indices.right, mesh_view_);

  EXPECT_THAT(GetSideMarginValues(), ElementsAreArray({
                                         StrokeVertex::kMaximumMargin,
                                         StrokeVertex::kMaximumMargin,
                                         0.f,
                                         0.f,
                                         0.f,
                                         StrokeVertex::kMaximumMargin,
                                         StrokeVertex::kMaximumMargin,
                                     }));
}

TEST(DerivativeCalculatorDeathTest, NonEmptyIndicesWithNoDataMeshView) {
  MutableMeshView mesh_view;
  ASSERT_FALSE(mesh_view.HasMeshData());
  DerivativeCalculator calculator;
  EXPECT_DEATH_IF_SUPPORTED(calculator.UpdateMesh({0, 1}, {2, 3}, mesh_view),
                            "");
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
