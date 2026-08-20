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

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::StrokeVertex;

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::Optional;
using ::testing::PrintToString;

MATCHER_P(IndexOffsetRangeEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " IndexOffsetRange (expected: ", PrintToString(expected),
                       ")")) {
  return ExplainMatchResult(
      AllOf(Field("first", &Side::IndexOffsetRange::first, expected.first),
            Field("last", &Side::IndexOffsetRange::last, expected.last)),
      arg, result_listener);
}
Matcher<Side::IndexOffsetRange> IndexOffsetRangeEq(
    const Side::IndexOffsetRange& expected) {
  return IndexOffsetRangeEqMatcher(expected);
}

class GeometryExtrusionBreakTest : public ::testing::Test {
 protected:
  GeometryExtrusionBreakTest() : mesh_(StrokeVertex::FullMeshFormat()) {}

  void SetUp() override { geometry_.Reset(MutableMeshView(mesh_)); }

  void AppendAndProcessVertices(absl::Span<const Point> left_vertices,
                                absl::Span<const Point> right_vertices) {
    for (Point p : left_vertices) geometry_.AppendLeftVertex(p);
    for (Point p : right_vertices) geometry_.AppendRightVertex(p);
    geometry_.ProcessNewVertices(0, {});
  }

  MutableMesh mesh_;
  Geometry geometry_;
};

TEST_F(GeometryExtrusionBreakTest, WithoutSelfIntersections) {
  AppendAndProcessVertices({{0, 0}, {0, 1}}, {{1, 0}, {1, 1}, {1, 2}});
  geometry_.ResetMutationTracking();

  ASSERT_EQ(mesh_.VertexCount(), 5);
  ASSERT_EQ(mesh_.TriangleCount(), 3);

  ASSERT_EQ(geometry_.LeftSide().indices.size(), 2);
  ASSERT_EQ(geometry_.LeftSide().first_simplifiable_index_offset, 0);
  ASSERT_EQ(geometry_.LeftSide().vertex_buffer.size(), 2);
  ASSERT_EQ(geometry_.LeftSide().next_buffered_vertex_offset, 2);

  ASSERT_EQ(geometry_.RightSide().indices.size(), 3);
  ASSERT_EQ(geometry_.RightSide().first_simplifiable_index_offset, 0);
  ASSERT_EQ(geometry_.RightSide().vertex_buffer.size(), 2);
  ASSERT_EQ(geometry_.RightSide().next_buffered_vertex_offset, 2);

  Side::MeshPartitionStart left_partition_start =
      geometry_.LeftSide().partition_start;
  ASSERT_EQ(left_partition_start.adjacent_first_index_offset, 0);
  ASSERT_EQ(left_partition_start.opposite_first_index_offset, 0);
  ASSERT_EQ(left_partition_start.first_triangle, 0);
  ASSERT_EQ(left_partition_start.opposite_side_initial_position, std::nullopt);
  ASSERT_EQ(left_partition_start.non_ccw_connection_index, std::nullopt);
  ASSERT_TRUE(left_partition_start.outline_connects_sides);
  ASSERT_TRUE(left_partition_start.is_forward_exterior);

  Side::MeshPartitionStart right_partition_start =
      geometry_.LeftSide().partition_start;
  ASSERT_EQ(right_partition_start.adjacent_first_index_offset, 0);
  ASSERT_EQ(right_partition_start.opposite_first_index_offset, 0);
  ASSERT_EQ(right_partition_start.first_triangle, 0);
  ASSERT_EQ(right_partition_start.opposite_side_initial_position, std::nullopt);
  ASSERT_EQ(right_partition_start.non_ccw_connection_index, std::nullopt);
  ASSERT_TRUE(right_partition_start.outline_connects_sides);
  ASSERT_TRUE(right_partition_start.is_forward_exterior);

  geometry_.AddExtrusionBreak();
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({0, 0}, {1, 2})));

  // Adding an extrusion break should not affect the mesh or existing side
  // indices:
  EXPECT_EQ(mesh_.VertexCount(), 5);
  EXPECT_EQ(mesh_.TriangleCount(), 3);
  EXPECT_EQ(geometry_.LeftSide().indices.size(), 2);
  EXPECT_EQ(geometry_.RightSide().indices.size(), 3);

  // Any buffered vertices should be cleared:
  EXPECT_TRUE(geometry_.LeftSide().vertex_buffer.empty());
  EXPECT_EQ(geometry_.LeftSide().first_simplifiable_index_offset, 2);
  EXPECT_EQ(geometry_.LeftSide().next_buffered_vertex_offset, 0);
  EXPECT_TRUE(geometry_.RightSide().vertex_buffer.empty());
  EXPECT_EQ(geometry_.RightSide().first_simplifiable_index_offset, 3);
  EXPECT_EQ(geometry_.RightSide().next_buffered_vertex_offset, 0);

  // The side partitions should start after all existing vertices and indices.
  left_partition_start = geometry_.LeftSide().partition_start;
  EXPECT_EQ(left_partition_start.adjacent_first_index_offset, 2);
  EXPECT_EQ(left_partition_start.opposite_first_index_offset, 3);
  EXPECT_EQ(left_partition_start.first_triangle, 3);
  EXPECT_EQ(left_partition_start.opposite_side_initial_position, std::nullopt);
  EXPECT_EQ(left_partition_start.non_ccw_connection_index, std::nullopt);
  EXPECT_TRUE(left_partition_start.outline_connects_sides);
  EXPECT_TRUE(left_partition_start.is_forward_exterior);

  right_partition_start = geometry_.RightSide().partition_start;
  EXPECT_EQ(right_partition_start.adjacent_first_index_offset, 3);
  EXPECT_EQ(right_partition_start.opposite_first_index_offset, 2);
  EXPECT_EQ(right_partition_start.first_triangle, 3);
  EXPECT_EQ(right_partition_start.opposite_side_initial_position, std::nullopt);
  EXPECT_EQ(right_partition_start.non_ccw_connection_index, std::nullopt);
  EXPECT_TRUE(right_partition_start.outline_connects_sides);
  EXPECT_TRUE(right_partition_start.is_forward_exterior);

  geometry_.ResetMutationTracking();

  // A second call to add an extrusion break before appending new vertices
  // should not make any modifications.
  geometry_.AddExtrusionBreak();
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  // The subsequent extrusion should be disconnected, and adding three new
  // vertices will add only one new triangle.
  AppendAndProcessVertices({{5, 5}}, {{5, 4}, {6, 4}});

  EXPECT_EQ(mesh_.VertexCount(), 8);
  EXPECT_EQ(mesh_.TriangleCount(), 4);

  EXPECT_EQ(geometry_.LeftSide().indices.size(), 3);
  EXPECT_EQ(geometry_.LeftSide().vertex_buffer.size(), 1);
  EXPECT_EQ(geometry_.LeftSide().next_buffered_vertex_offset, 1);

  EXPECT_EQ(geometry_.RightSide().indices.size(), 5);
  EXPECT_EQ(geometry_.RightSide().vertex_buffer.size(), 2);
  EXPECT_EQ(geometry_.RightSide().next_buffered_vertex_offset, 2);
}

TEST_F(GeometryExtrusionBreakTest, ResetsSelfIntersections) {
  AppendAndProcessVertices({{0, 0}, {0, 1}}, {{1, 0}, {1, 1}, {1, 2}});

  AppendAndProcessVertices({{0.25, 0.5}}, {{0.75, 0.5}});

  ASSERT_TRUE(geometry_.LeftSide().intersection.has_value());
  ASSERT_TRUE(geometry_.RightSide().intersection.has_value());
  ASSERT_TRUE(geometry_.LeftSide().intersection->retriangulation_started ||
              geometry_.RightSide().intersection->retriangulation_started);

  geometry_.ResetMutationTracking();
  geometry_.AddExtrusionBreak();
  // With retriangulation started, the last vertices should be unmodified.
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  EXPECT_FALSE(geometry_.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry_.RightSide().intersection.has_value());
}

TEST_F(GeometryExtrusionBreakTest, ProcessOnlyLeftVerticesAfterExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  EXPECT_EQ(mesh_.TriangleCount(), 2);
  geometry_.AddExtrusionBreak();
  geometry_.ResetMutationTracking();

  AppendAndProcessVertices({{2, 1}, {3, 1}}, {});

  EXPECT_EQ(mesh_.TriangleCount(), 2);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  AppendAndProcessVertices({}, {{2, 0}, {3, 0}});

  EXPECT_EQ(mesh_.TriangleCount(), 4);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({2, 0}, {3, 1})));
}

TEST_F(GeometryExtrusionBreakTest,
       ProcessOnlyRightVerticesAfterExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  EXPECT_EQ(mesh_.TriangleCount(), 2);
  geometry_.AddExtrusionBreak();
  geometry_.ResetMutationTracking();

  AppendAndProcessVertices({}, {{2, 0}, {3, 0}});

  EXPECT_EQ(mesh_.TriangleCount(), 2);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  AppendAndProcessVertices({{2, 1}, {3, 1}}, {});

  EXPECT_EQ(mesh_.TriangleCount(), 4);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({2, 0}, {3, 1})));
}

TEST_F(GeometryExtrusionBreakTest, ClearIsNoOpWhenEmpty) {
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());
}

TEST_F(GeometryExtrusionBreakTest, ClearIsNoOpWhenNewBreakWasJustStarted) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  geometry_.AddExtrusionBreak();
  int n_tris = mesh_.TriangleCount();
  int n_verts = mesh_.VertexCount();

  geometry_.ResetMutationTracking();
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());
}

TEST_F(GeometryExtrusionBreakTest, ClearWithoutAddingExtrusionBreaks) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});

  geometry_.ResetMutationTracking();
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({0, 0}, {1, 1}))));
}

TEST_F(GeometryExtrusionBreakTest, ClearAfterExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  int n_tris = mesh_.TriangleCount();
  int n_verts = mesh_.VertexCount();
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{2, 1}, {3, 1}}, {{2, 0}, {3, 0}});

  geometry_.ResetMutationTracking();
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({2, 0}, {3, 1}))));
}

TEST_F(GeometryExtrusionBreakTest, ClearWithOnlyNewLeftVertices) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  int n_tris = mesh_.TriangleCount();
  int n_verts = mesh_.VertexCount();
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{2, 1}, {3, 1}}, {});

  geometry_.ResetMutationTracking();
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  AppendAndProcessVertices({{5, 5}, {6, 5}}, {{5, 4}, {6, 4}});
  EXPECT_EQ(mesh_.TriangleCount(), n_tris + 2);
  EXPECT_EQ(mesh_.VertexCount(), n_verts + 4);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({5, 4}, {6, 5})));
}

TEST_F(GeometryExtrusionBreakTest, ClearWithOnlyNewRightVertices) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  int n_tris = mesh_.TriangleCount();
  int n_verts = mesh_.VertexCount();
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({}, {{2, 0}, {3, 0}});

  geometry_.ResetMutationTracking();
  geometry_.ClearSinceLastExtrusionBreak();

  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_TRUE(geometry_.CalculateVisuallyUpdatedRegion().IsEmpty());

  AppendAndProcessVertices({{5, 5}, {6, 5}}, {{5, 4}, {6, 4}});
  EXPECT_EQ(mesh_.TriangleCount(), n_tris + 2);
  EXPECT_EQ(mesh_.VertexCount(), n_verts + 4);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({5, 4}, {6, 5})));
}

TEST_F(GeometryExtrusionBreakTest,
       ClearAndRevertToSavePointBeforeExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  geometry_.SetSavePoint();
  int n_saved_tris = mesh_.TriangleCount();
  int n_saved_verts = mesh_.VertexCount();
  AppendAndProcessVertices({{2, 1}}, {{2, 0}});
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{3, 1}, {4, 1}, {5, 1}}, {{3, 0}, {4, 0}, {5, 0}});

  geometry_.ClearSinceLastExtrusionBreak();
  AppendAndProcessVertices({{3, 2}, {4, 2}}, {{3, 1}, {4, 1}});
  geometry_.ResetMutationTracking();
  geometry_.RevertToSavePoint();

  EXPECT_EQ(mesh_.TriangleCount(), n_saved_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_saved_verts);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({1, 0}, {4, 2}))));
}

TEST_F(GeometryExtrusionBreakTest,
       ClearTwiceAndRevertToSavePointBeforeExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  geometry_.SetSavePoint();
  int n_saved_tris = mesh_.TriangleCount();
  int n_saved_verts = mesh_.VertexCount();
  AppendAndProcessVertices({{2, 1}}, {{2, 0}});
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{3, 1}, {4, 1}}, {{3, 0}, {4, 0}});

  geometry_.ClearSinceLastExtrusionBreak();
  AppendAndProcessVertices({{3, 2}, {4, 2}, {5, 2}}, {{3, 1}, {4, 1}, {5, 1}});
  geometry_.ResetMutationTracking();
  geometry_.RevertToSavePoint();

  EXPECT_EQ(mesh_.TriangleCount(), n_saved_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_saved_verts);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({1, 0}, {5, 2}))));
}

TEST_F(GeometryExtrusionBreakTest,
       ClearAndRevertToSavePointAfterExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{3, 1}, {4, 1}, {5, 1}}, {{3, 0}, {4, 0}, {5, 0}});
  geometry_.SetSavePoint();
  int n_saved_tris = mesh_.TriangleCount();
  int n_saved_verts = mesh_.VertexCount();

  geometry_.ClearSinceLastExtrusionBreak();
  AppendAndProcessVertices({{3, 2}, {4, 2}}, {{3, 1}, {4, 1}});
  geometry_.ResetMutationTracking();
  geometry_.RevertToSavePoint();

  EXPECT_EQ(mesh_.TriangleCount(), n_saved_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_saved_verts);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({3, 0}, {5, 2}))));
}

TEST_F(GeometryExtrusionBreakTest,
       ClearTwiceAndRevertToSavePointAfterExtrusionBreak) {
  AppendAndProcessVertices({{0, 1}, {1, 1}}, {{0, 0}, {1, 0}});
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{3, 1}, {4, 1}}, {{3, 0}, {4, 0}});
  geometry_.SetSavePoint();
  int n_saved_tris = mesh_.TriangleCount();
  int n_saved_verts = mesh_.VertexCount();

  geometry_.ClearSinceLastExtrusionBreak();
  AppendAndProcessVertices({{3, 2}, {4, 2}, {5, 2}}, {{3, 1}, {4, 1}, {5, 1}});
  geometry_.ResetMutationTracking();
  geometry_.RevertToSavePoint();

  EXPECT_EQ(mesh_.TriangleCount(), n_saved_tris);
  EXPECT_EQ(mesh_.VertexCount(), n_saved_verts);
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion().AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({3, 0}, {5, 2}))));
}

TEST_F(GeometryExtrusionBreakTest, ClearAndRestoreIntersectionDiscontinuities) {
  AppendAndProcessVertices({{-1, -2}, {-1, -1}}, {{1, -2}, {1, -1}});
  geometry_.AddExtrusionBreak();
  AppendAndProcessVertices({{-1, 0},
                            {-1, 1},
                            {-1, 2},
                            {-1, 3},
                            {-1, 4},
                            {-0.5, 3.5},
                            {0, 3.5},
                            {0, 3},
                            {0, 2},
                            {0, 1}},
                           {{1, 0},
                            {1, 1},
                            {1, 2},
                            {1, 3},
                            {1, 4},
                            {-0.5, 4.5},
                            {-2, 3.5},
                            {-2, 3},
                            {-2, 2},
                            {-2, 1}});
  EXPECT_THAT(geometry_.LeftSide().intersection_discontinuities,
              ElementsAre(IndexOffsetRangeEq({5, 8})));

  geometry_.SetSavePoint();

  geometry_.ClearSinceLastExtrusionBreak();
  EXPECT_THAT(geometry_.LeftSide().intersection_discontinuities, IsEmpty());

  geometry_.RevertToSavePoint();
  EXPECT_THAT(geometry_.LeftSide().intersection_discontinuities,
              ElementsAre(IndexOffsetRangeEq({5, 8})));
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
