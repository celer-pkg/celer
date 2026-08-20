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

#include "ink/strokes/internal/brush_tip_extruder/geometry.h"

#include <cstdint>
#include <iterator>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_extruder/simplify.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/legacy_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::BrushTipState;
using ::testing::ElementsAre;
using ::testing::Field;
using ::testing::IsEmpty;

testing::Matcher<ExtrudedVertex> VertexPositionEq(Point p) {
  return Field("position", &ExtrudedVertex::position, PointEq(p));
}

struct MeshData {
  std::vector<strokes_internal::LegacyVertex> vertices;
  std::vector<IndexType> triangle_indices;
};

MutableMeshView MakeView(MeshData& data) {
  return MutableMeshView(data.vertices, data.triangle_indices);
}

BrushTipState PositionToTipState(Point p) {
  return BrushTipState{
      .position = p,
      .width = 1,
      .height = 1,
      .percent_radius = 1,
  };
}

BrushTipState PositionAndSizeToTipState(Point p, float size) {
  return BrushTipState{
      .position = p,
      .width = size,
      .height = size,
      .percent_radius = 1,
  };
}

TEST(GeometryTest, DefaultState) {
  Geometry line_geometry;
  EXPECT_FALSE(line_geometry.GetMeshView().HasMeshData());

  EXPECT_EQ(line_geometry.LeftSide().self_id, SideId::kLeft);
  EXPECT_EQ(line_geometry.LeftSide().first_triangle_vertex, 0);
  EXPECT_TRUE(line_geometry.LeftSide().indices.empty());
  EXPECT_TRUE(line_geometry.LeftSide().intersection_discontinuities.empty());
  EXPECT_FALSE(line_geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(
      line_geometry.LeftSide().last_simplified_vertex_positions.empty());
  EXPECT_TRUE(line_geometry.LeftSide().vertex_buffer.empty());
  EXPECT_EQ(line_geometry.LeftSide().next_buffered_vertex_offset, 0);

  EXPECT_EQ(line_geometry.RightSide().self_id, SideId::kRight);
  EXPECT_EQ(line_geometry.RightSide().first_triangle_vertex, 1);
  EXPECT_TRUE(line_geometry.RightSide().indices.empty());
  EXPECT_TRUE(line_geometry.RightSide().intersection_discontinuities.empty());
  EXPECT_FALSE(line_geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(
      line_geometry.LeftSide().last_simplified_vertex_positions.empty());
  EXPECT_TRUE(line_geometry.RightSide().vertex_buffer.empty());
  EXPECT_EQ(line_geometry.RightSide().next_buffered_vertex_offset, 0);

  line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
}

TEST(GeometryTest, ResetClearsMeshAndSideIndicesAndVertices) {
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 0}, 1));

  // Append and don't process a couple more vertices so that
  // `Side::vertex_buffer` is not empty:
  geometry.AppendLeftVertex(Point{0, 2});
  geometry.AppendRightVertex(Point{1, 2});

  ASSERT_FALSE(mesh_data.vertices.empty());
  ASSERT_FALSE(mesh_data.triangle_indices.empty());
  ASSERT_FALSE(geometry.LeftSide().indices.empty());
  ASSERT_FALSE(geometry.LeftSide().vertex_buffer.empty());
  ASSERT_NE(geometry.LeftSide().next_buffered_vertex_offset, 0);
  ASSERT_FALSE(geometry.RightSide().indices.empty());
  ASSERT_FALSE(geometry.RightSide().vertex_buffer.empty());
  ASSERT_NE(geometry.RightSide().next_buffered_vertex_offset, 0);

  geometry.Reset(MakeView(mesh_data));

  EXPECT_TRUE(mesh_data.vertices.empty());
  EXPECT_TRUE(mesh_data.triangle_indices.empty());
  EXPECT_TRUE(geometry.LeftSide().indices.empty());
  EXPECT_TRUE(geometry.LeftSide().vertex_buffer.empty());
  EXPECT_EQ(geometry.LeftSide().next_buffered_vertex_offset, 0);
  EXPECT_TRUE(geometry.RightSide().indices.empty());
  EXPECT_TRUE(geometry.RightSide().vertex_buffer.empty());
  EXPECT_EQ(geometry.RightSide().next_buffered_vertex_offset, 0);
}

TEST(GeometryTest, ResetClearsSideIntersectionState) {
  auto add_quad = [](Geometry& geometry) {
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 4});
    geometry.AppendRightVertex(Point{4, 0});
    geometry.AppendRightVertex(Point{4, 4});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 2));
    ASSERT_FALSE(geometry.LeftSide().intersection.has_value());
    ASSERT_FALSE(geometry.RightSide().intersection.has_value());
  };

  MeshData mesh_data;

  // Add a two-triangle quad, then start a self-intersection on the left side,
  // and check that `Reset()` clears the self-intersection state.
  Geometry geometry(MakeView(mesh_data));
  add_quad(geometry);
  geometry.AppendLeftVertex(Point{2, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 2));
  ASSERT_TRUE(geometry.LeftSide().intersection.has_value());
  ASSERT_FALSE(geometry.RightSide().intersection.has_value());

  geometry.Reset(MakeView(mesh_data));
  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  // Repeat above, but with a self-intersection on the right side.
  geometry = Geometry(MakeView(mesh_data));
  add_quad(geometry);
  geometry.AppendRightVertex(Point{2, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 2));
  ASSERT_FALSE(geometry.LeftSide().intersection.has_value());
  ASSERT_TRUE(geometry.RightSide().intersection.has_value());

  geometry.Reset(MakeView(mesh_data));
  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());
}

TEST(GeometryTest, ResetClearsSavedSimplificationPositions) {
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  // Append three vertices per side such that the middle positions are
  // simplified away and saved.
  //
  // L = left-side position, R = right-side position, S = simplified away
  //
  // L-----S-----L
  // |
  // |               ----> travel direction
  // |
  // R-----S-----R
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{1, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({1, 0}, 1));
  geometry.AppendLeftVertex(Point{2, 1});
  geometry.AppendRightVertex(Point{2, 0});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({2, 0}, 1));

  ASSERT_FALSE(geometry.LeftSide().last_simplified_vertex_positions.empty());
  ASSERT_FALSE(geometry.RightSide().last_simplified_vertex_positions.empty());

  geometry.Reset(MakeView(mesh_data));
  EXPECT_TRUE(geometry.LeftSide().last_simplified_vertex_positions.empty());
  EXPECT_TRUE(geometry.RightSide().last_simplified_vertex_positions.empty());
}

TEST(GeometryTest, AppendVerticesAndProcess) {
  MeshData mesh_data;
  Geometry line_geometry(MakeView(mesh_data));
  line_geometry.AppendLeftVertex(Point{-1, 0});
  line_geometry.AppendLeftVertex(Point{-1, 1});

  EXPECT_EQ(line_geometry.GetMeshView().VertexCount(), 1);
  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 0);
  EXPECT_EQ(line_geometry.LeftSide().indices.size(), 1);
  EXPECT_EQ(line_geometry.LeftSide().vertex_buffer.size(), 2);
  EXPECT_EQ(line_geometry.LeftSide().next_buffered_vertex_offset, 1);
  EXPECT_TRUE(line_geometry.RightSide().indices.empty());
  EXPECT_TRUE(line_geometry.RightSide().vertex_buffer.empty());

  line_geometry.AppendRightVertex(Point{1, 0});
  line_geometry.AppendRightVertex(Point{1, 1});
  line_geometry.AppendRightVertex(Point{1, 2});

  EXPECT_EQ(line_geometry.GetMeshView().VertexCount(), 2);
  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 0);
  EXPECT_EQ(line_geometry.LeftSide().indices.size(), 1);
  EXPECT_EQ(line_geometry.LeftSide().vertex_buffer.size(), 2);
  EXPECT_EQ(line_geometry.LeftSide().next_buffered_vertex_offset, 1);
  EXPECT_EQ(line_geometry.RightSide().indices.size(), 1);
  EXPECT_EQ(line_geometry.RightSide().vertex_buffer.size(), 3);
  EXPECT_EQ(line_geometry.RightSide().next_buffered_vertex_offset, 1);

  line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
  EXPECT_THAT(line_geometry.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({0, 1}, 2, 2)));

  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 3);
  EXPECT_EQ(mesh_data.triangle_indices,
            std::vector<IndexType>({0, 1, 2, 0, 2, 3, 3, 2, 4}));

  // After processing new vertices, two vertices from each side will be part of
  // the buffered vertices for the next extrusion:
  EXPECT_EQ(line_geometry.LeftSide().vertex_buffer.size(), 2);
  EXPECT_EQ(line_geometry.RightSide().vertex_buffer.size(), 2);
}

TEST(GeometryTest, Reset) {
  MeshData initial_mesh_data;
  Geometry line_geometry(MakeView(initial_mesh_data));

  line_geometry.AppendLeftVertex(Point{-1, 0});
  line_geometry.AppendLeftVertex(Point{-1, 1});
  line_geometry.AppendRightVertex(Point{1, 0});
  line_geometry.AppendRightVertex(Point{1, 1});
  line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));

  MeshData final_mesh_data;
  line_geometry.Reset(MakeView(final_mesh_data));

  EXPECT_EQ(line_geometry.GetMeshView().VertexCount(), 0);
  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 0);

  EXPECT_TRUE(line_geometry.LeftSide().indices.empty());
  EXPECT_TRUE(line_geometry.LeftSide().intersection_discontinuities.empty());
  EXPECT_FALSE(line_geometry.LeftSide().intersection.has_value());
  EXPECT_EQ(line_geometry.LeftSide().vertex_buffer.size(), 0);

  EXPECT_TRUE(line_geometry.RightSide().indices.empty());
  EXPECT_TRUE(line_geometry.RightSide().intersection_discontinuities.empty());
  EXPECT_FALSE(line_geometry.RightSide().intersection.has_value());
  EXPECT_EQ(line_geometry.RightSide().vertex_buffer.size(), 0);

  line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
  EXPECT_TRUE(line_geometry.CalculateVisuallyUpdatedRegion().IsEmpty());

  line_geometry.AppendLeftVertex(Point{1, 1});
  line_geometry.AppendLeftVertex(Point{1, 3});
  line_geometry.AppendRightVertex(Point{3, 1});
  line_geometry.AppendRightVertex(Point{3, 3});

  line_geometry.ProcessNewVertices(0.f, PositionToTipState({2, 2}));
  EXPECT_THAT(line_geometry.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({2, 2}, 2, 2)));

  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 2);
  EXPECT_EQ(final_mesh_data.triangle_indices,
            std::vector<IndexType>({0, 1, 2, 0, 2, 3}));

  // After processing new vertices, two vertices from each side will be part of
  // the buffered vertices for the next extrusion:
  EXPECT_EQ(line_geometry.LeftSide().vertex_buffer.size(), 2);
  EXPECT_EQ(line_geometry.RightSide().vertex_buffer.size(), 2);
}

TEST(GeometryTest, Fans) {
  {
    MeshData mesh_data;
    Geometry line_geometry(MakeView(mesh_data));

    line_geometry.AppendLeftVertex(Point{-1, 0});
    line_geometry.AppendLeftVertex(Point{0, 1});
    line_geometry.AppendLeftVertex(Point{1, 0});
    line_geometry.AppendLeftVertex(Point{0, -1});

    line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
    EXPECT_TRUE(line_geometry.CalculateVisuallyUpdatedRegion().IsEmpty());
    line_geometry.ResetMutationTracking();

    line_geometry.AppendRightVertex(Point{0, 0});
    line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
    EXPECT_THAT(line_geometry.CalculateVisuallyUpdatedRegion(),
                EnvelopeEq(Rect::FromCenterAndDimensions({0, 0}, 2, 2)));

    EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 3);
    EXPECT_EQ(mesh_data.triangle_indices,
              std::vector<IndexType>({0, 1, 2, 2, 1, 3, 3, 1, 4}));
  }

  {
    MeshData mesh_data;
    Geometry line_geometry(MakeView(mesh_data));

    line_geometry.AppendRightVertex(Point{1, 0});
    line_geometry.AppendRightVertex(Point{0, 1});
    line_geometry.AppendRightVertex(Point{-1, 0});
    line_geometry.AppendRightVertex(Point{0, -1});

    line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
    EXPECT_TRUE(line_geometry.CalculateVisuallyUpdatedRegion().IsEmpty());
    line_geometry.ResetMutationTracking();

    line_geometry.AppendLeftVertex(Point{0, 0});
    line_geometry.ProcessNewVertices(0.f, PositionToTipState({0, 0}));
    EXPECT_THAT(line_geometry.CalculateVisuallyUpdatedRegion(),
                EnvelopeEq(Rect::FromCenterAndDimensions({0, 0}, 2, 2)));

    EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 3);
    EXPECT_EQ(mesh_data.triangle_indices,
              std::vector<IndexType>({1, 0, 2, 1, 2, 3, 1, 3, 4}));
  }
}

TEST(GeometryTest, VisuallyUpdatedRegionWithUnchangedIntersectingTriangle) {
  // Cover an edge case for visually updated region calculation where
  // retriangulation has begun and the point of intersection remains in the same
  // triangle over multiple extrusions. This has a chance of not reporting the
  // complete bounds of changed geometry if not handled properly.

  // Extrusion travels up and then sharply to the left.
  MeshData mesh_data;
  Geometry line_geometry(MakeView(mesh_data));
  line_geometry.AppendLeftVertex(Point{-1, 0});
  line_geometry.AppendLeftVertex(Point{-1, 2});
  line_geometry.AppendRightVertex(Point{1, 0});
  line_geometry.AppendRightVertex(Point{1, 2});
  line_geometry.ProcessNewVertices(0.f, PositionAndSizeToTipState({0, 0}, 2));
  line_geometry.AppendLeftVertex(Point{-0.5, 1.5});
  line_geometry.AppendRightVertex(Point{0.5, 2.5});
  line_geometry.ProcessNewVertices(0.f, PositionAndSizeToTipState({0, 0}, 2));
  line_geometry.AppendLeftVertex(Point{0, 1.5});
  line_geometry.AppendRightVertex(Point{0, 2.5});
  line_geometry.ProcessNewVertices(0.f, PositionAndSizeToTipState({0, 0}, 2));

  line_geometry.ResetMutationTracking();
  line_geometry.AppendLeftVertex(Point{-0.25, 1.5});
  line_geometry.AppendRightVertex(Point{-0.25, 2.5});
  line_geometry.ProcessNewVertices(0.f, PositionAndSizeToTipState({0, 0}, 2));

  Envelope visually_mutated_region =
      line_geometry.CalculateVisuallyUpdatedRegion();
  EXPECT_THAT(visually_mutated_region,
              EnvelopeEq(Rect::FromCenterAndDimensions({0, 1.25}, 2, 2.5)));

  // The region should include all of the triangles in the intersection because
  // the central pivot vertex has moved:
  EXPECT_TRUE(line_geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(line_geometry.LeftSide().intersection->retriangulation_started);
  Envelope bounds_starting_with_intersection;
  for (uint32_t i = 3 * line_geometry.LeftSide()
                            .intersection->oldest_retriangulation_triangle;
       i < mesh_data.triangle_indices.size(); ++i) {
    bounds_starting_with_intersection.Add(
        line_geometry.GetMeshView().GetPosition(mesh_data.triangle_indices[i]));
  }
  EXPECT_FALSE(bounds_starting_with_intersection.IsEmpty());
  EXPECT_THAT(visually_mutated_region,
              EnvelopeEq(bounds_starting_with_intersection));
}

MATCHER(TrianglesAreNotCw, "") {
  for (int i = 0; i < arg.TriangleCount(); ++i) {
    float signed_area = arg.GetTriangle(i).SignedArea();
    if (signed_area < 0) {
      *result_listener << "Triangle " << i << " is oriented clockwise";
      return false;
    }
  }
  return true;
}

TEST(GeometryTest, RetryRejectedIntersectionVertex) {
  {
    // This case creates an intersection where the order of processing the last
    // two vertices makes a difference for finding clockwise triangles.
    //   * The last right vertex is appended first. It is rejected because it
    //     would create a CW triangle because of the shape of the right side.
    //   * Then the last left vertex is appended. It makes the last right
    //     vertex viable again because it and the last right vertex create
    //     triangles that cover up the problematic part of the right side.
    //
    //  L----------L
    //  |          |
    //  |  R----R  |
    //  |  |    |  |
    //  L  | R--R  |
    //  |  R |     L
    //  L----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 1});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1.25, 1.01});
    geometry.AppendRightVertex(Point{1.25, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{1.5, 2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendRightVertex(Point{0.75, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_GT(
        geometry.RightSide().intersection->outline_reposition_budget,
        geometry.RightSide().intersection->initial_outline_reposition_budget);

    // Append the last right vertex before the last left vertex. Accepting this
    // vertex would cause clockwise triangles, so it gets a corrected position.
    geometry.AppendRightVertex(Point{0.75, 0.8});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_THAT(
        geometry.RightSide().intersection->last_proposed_vertex.position,
        PointEq({0.75, 0.8}));
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointNear({0.88, 1.01}, 0.01));
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());

    // Append the last left vertex, which causes the rejected right vertex to be
    // retried. With the new left position the portion of the right outline that
    // would have caused a clockwise triangle last time is compressed. Those
    // triangles are now degenerate and not clockwise, so the vertex can be
    // appended.
    geometry.AppendLeftVertex(Point{1.5, 0.8});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointEq({0.75, 0.8}));
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }

  {
    // This case begins an intersection on the left side, and then also begins
    // an intersection on the right side before the left side intersection
    // completes. When the left side intersection is completed on the last
    // processed vertex, the last attempted vertex of the right side should be
    // retried.
    //
    //      R-----------R
    //      |           |
    //      |    L--L   |
    //      | R  |  |   |
    //      |/   L------R
    //      R       |
    //              L
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 0.5});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0.5, 0.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);

    geometry.AppendRightVertex(Point{-0.5, 1.5});
    geometry.AppendRightVertex(Point{-0.5, -0.25});
    geometry.AppendRightVertex(Point{-0.25, 0.25});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
    // Intersection has started on the right side, but is not able to break up
    // triangles because the left side owns mesh mutation for now.
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_FALSE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_THAT(
        geometry.RightSide().intersection->last_proposed_vertex.position,
        PointEq({-0.25, 0.25}));
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointEq({-0.5, -0.25}));
    geometry.AppendLeftVertex(Point{0.5, -0.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_FALSE(geometry.LeftSide().intersection.has_value());

    // Now that the left side intersection is finished, the current intersection
    // vertex on the right side should be retried and begin modifying triangles.
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_THAT(
        geometry.RightSide().intersection->last_proposed_vertex.position,
        PointEq({-0.25, 0.25}));
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointEq({-0.25, 0.25}));
  }
}

TEST(GeometryTest, BeginRetriangulationOnCcwTriangle) {
  // This case exercises the path where intersection handling begins to
  // retriangulate the existing mesh with a proposed CCW winding triangle.
  // Usually this happens on a proposed CW triangle instead. We check that
  // starting retriangulation in this case does not introduce incorrectly
  // winding triangles.

  // Initial geometry outline:
  //
  //       X------X
  //      /       |
  //     X X      |
  //       |      X
  //       |      |
  //       |      |
  //       X------X
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1.5});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.AppendRightVertex(Point{1, 2});
  geometry.AppendRightVertex(Point{0, 2});
  geometry.AppendRightVertex(Point{-0.5, 1.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 5);
  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  // Add first self intersecting position that will not exceed the travel
  // threshold to start retriangulation:
  geometry.AppendLeftVertex(Point{0.1, 1.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 5);
  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.LeftSide().intersection->retriangulation_started);
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  // Add second self-intersection position that proposes a CCW triangle:
  geometry.AppendLeftVertex(Point{0.5, 0.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 1));
  // Only one new triangle should be introduced by starting retriangulation, as
  // opposed to two in the case of starting on a CW triangle.
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 6);
  EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());
}

TEST(GeometryTest, BeginRetriangulationGiveUpOnCcwTriangle) {
  // This case exercises the path where intersection handling cannot begin to
  // retriangulate on a proposed CCW winding triangle. In this case it should
  // give up instead of rejecting the vertex.

  //  X-----------X
  //  |           |
  //  X--X X-X    |
  //       | |    X
  //       | X    |
  //       |      |
  //       X------X
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 2));

  // Start an intersection on the left side, but don't exceed the
  // retriangulation threshold.
  geometry.AppendLeftVertex(Point{0.1, 0.9});
  geometry.AppendRightVertex(Point{-1, 2});
  geometry.AppendRightVertex(Point{-1, 1});
  geometry.AppendRightVertex(Point{-0.5, 0.9});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 2));

  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.LeftSide().intersection->retriangulation_started);

  // Exceed the threshold on a CCW triangle proposing vertex that would cause a
  // bad winding triangle with the outer triangle fan.
  geometry.AppendLeftVertex(Point{0.5, 0.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 1}, 2));

  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
}

TEST(GeometryTest, BeginRetriangulationWouldCauseCwTriangle) {
  // Tests that retriangulation does not begin if it would cause a CW triangle
  // right away.

  // Initial geometry:
  //
  //   X         X
  //   |       /
  //   X     X
  //   |     |
  //   |     |
  //   X-----X
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 2);

  geometry.AppendLeftVertex(Point{1, 1.25});
  geometry.AppendRightVertex(Point{2, 1.25});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 4);

  // Add a left vertex that would cause a CW triangle with the right side if
  // intersecton started.
  //
  //   X         X
  //   |       /
  //   X     X
  //   |     |
  //   |  O  |
  //   X-----X
  geometry.AppendLeftVertex(Point{0.5, 0.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 4);
  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.LeftSide().intersection->retriangulation_started);

  // Add a left vertex that won't cause a CW triangle, triangulation should be
  // updated.
  //
  //   X         X
  //   |       /
  //   X  O  X
  //   |     |
  //   |     |
  //   X-----X
  geometry.AppendLeftVertex(Point{0.5, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 6);
  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
}

TEST(GeometryTest, NewProposedVertexWouldCauseClockwiseTriangle) {
  {
    // The last proposed intersection position on the left side is proposing
    // a CCW triangle, but would turn an older triangle CW. Intersection
    // handling should be given up, because a correction cannot be found.
    //
    //    R------------R
    //    |            |
    //    R-R    L--L  R
    //           |  |  |
    //           |  L  |
    //           |     |
    //           L-----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0.5, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);

    geometry.AppendRightVertex(Point{1, 3});
    geometry.AppendRightVertex(Point{-1, 3});
    geometry.AppendRightVertex(Point{-1, 1.5});
    geometry.AppendRightVertex(Point{-0.5, 1.4});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);

    geometry.AppendLeftVertex(Point{0.5, 0.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    // Check that geometry gave up intersection handling and accepted the
    // newest left vertex.
    EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices.back())
                    .position,
                PointEq({0.5, 0.5}));
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }

  {
    // The last proposed intersection position on the left side is proposing
    // a CW triangle, and would turn an old triangle CW. The vertex should be
    // rejected because a working correction cannot be found. Intersection
    // handling should continue.
    //
    //   L--
    //   |  L
    //   L  |  R--R
    //   |  L  |
    //   |     |
    //   L-----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 1.5});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1.5, 1.6});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0.5, 1.75});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);

    geometry.AppendLeftVertex(Point{0.5, 1.25});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    // Intersection handling should be ongoing, but the last proposed vertex
    // should be rejected.
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices.back())
                    .position,
                PointEq({0.5, 1.75}));
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }

  {
    // Similar to the case above with an extra right vertex to work with:
    //
    // The last proposed intersection position on the left side is proposing
    // a CW triangle, and would turn an old triangle CW. The vertex should be
    // corrected and intersection handling should continue.
    //
    //   L--      R
    //   |  L     |
    //   L  |  R--R
    //   |  L  |
    //   |     |
    //   L-----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 1.5});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1.5, 1.6});
    geometry.AppendRightVertex(Point{1.5, 2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0.5, 1.75});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);

    geometry.AppendLeftVertex(Point{0.5, 1.25});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    // Intersection handling should be ongoing, and the intersection position
    // should have been updated.
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices.back())
                    .position,
                PointNear({0.78, 1.46}, 0.01));
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }
}

TEST(GeometryTest, OutlineShiftWouldCauseClockwiseTriangle) {
  {
    // As intersection progresses, the outline on the right side is pushed down.
    // In this case, the last proposed vertex is the second intersecting one on
    // the right side. This would cause a clockwise triangle between the result
    // outline position and two of the vertices on the left side. Notably, the
    // resulting CW triangle in this case is not part of the triangle fan
    // around the intersection position. Intersection handling should be given
    // up, because the proposed triangle is CCW.
    //
    //  L------------L
    //  |            |
    //  L     R--R   L
    //  |     R  |   |
    //  L--L     R   |
    //     |     |   |
    //     |     |   L
    //     L-----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 1});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{-0.5, 1.05});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{-0.5, 1.5});
    geometry.AppendLeftVertex(Point{-0.5, 2});
    geometry.AppendLeftVertex(Point{1.5, 2});
    geometry.AppendRightVertex(Point{1, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendRightVertex(Point{0.5, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

    geometry.AppendLeftVertex(Point{1.5, 1.5});
    geometry.AppendLeftVertex(Point{1.5, 0.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

    uint32_t triangles_before_giving_up =
        geometry.GetMeshView().TriangleCount();
    geometry.AppendRightVertex(Point{0.5, 1.2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    // Check that geometry gave up intersection handling and accepted the
    // newest right vertex.
    EXPECT_FALSE(geometry.RightSide().intersection.has_value());
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointEq({0.5, 1.2}));
    EXPECT_EQ(geometry.GetMeshView().TriangleCount(),
              triangles_before_giving_up + 1);
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }

  {
    // This case is identical to the one above, except the order of processing
    // the last two vertices is swapped. This time, the second intersection
    // position is accepted, and the last processed vertex is on the left side.
    // It is the new left vertex that would cause a an old triangle to be CW.
    //
    //  L------------L
    //  |            |
    //  L     R--R   L
    //  |     R  |   |
    //  L--L     R   |
    //     |     |   |
    //     |     |   L
    //     L-----R
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 1});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 1});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{-0.5, 1.05});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendLeftVertex(Point{-0.5, 1.5});
    geometry.AppendLeftVertex(Point{-0.5, 2});
    geometry.AppendLeftVertex(Point{1.5, 2});
    geometry.AppendRightVertex(Point{1, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    geometry.AppendRightVertex(Point{0.5, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

    geometry.AppendLeftVertex(Point{1.5, 1.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

    geometry.AppendRightVertex(Point{0.5, 1.2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

    uint32_t triangles_before_giving_up =
        geometry.GetMeshView().TriangleCount();
    geometry.AppendLeftVertex(Point{1.5, 0.5});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

    // Check that geometry gave up intersection handling
    EXPECT_FALSE(geometry.RightSide().intersection.has_value());
    EXPECT_EQ(geometry.GetMeshView().TriangleCount(),
              triangles_before_giving_up + 1);
    EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
  }
}

TEST(GeometryTest, FinishIntersectionWouldCauseClockwiseTriangle) {
  // Finishing the intersection handling on the right side by using the outline
  // intersection position would cause a CW triangle. Intersection handling
  // should be given up and the CCW triangle should be appended the same way as
  // if the intersection ran out of `outline_reposition_budget`.
  //
  //  L--------L
  //  |        |
  //  |        L-L
  //  |
  //  |          R
  //  |      R--/---R
  //  |      | /    |
  //  |      R      |
  //  |             |
  //  L-------------L
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{2, 0});
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 2});
  geometry.AppendRightVertex(Point{2, 1});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

  geometry.AppendRightVertex(Point{1, 0.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  geometry.AppendLeftVertex(Point{1, 2});
  geometry.AppendLeftVertex(Point{1.01, 1.75});
  geometry.AppendLeftVertex(Point{1.5, 1.75});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  uint32_t triangles_before_giving_up = geometry.GetMeshView().TriangleCount();
  geometry.AppendRightVertex(Point{1.5, 1.25});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

  // Check that intersection handling finished, and gave up instead of exiting
  // cleanly and undoing the retriangulation. It should append one new vertex
  // and triangle at the outline intersection, and another at the new exterior
  // position.
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());
  uint32_t n_indices = geometry.RightSide().indices.size();
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(geometry.RightSide().indices[n_indices - 3])
                  .position,
              PointEq({1, 0.5}));
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(geometry.RightSide().indices[n_indices - 2])
                  .position,
              PointNear({1.33, 1.0}, 0.01));
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(geometry.RightSide().indices[n_indices - 1])
                  .position,
              PointEq({1.5, 1.25}));
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(),
            triangles_before_giving_up + 2);
  EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
}

TEST(GeometryTest, FinishIntersectionBudgetGreaterThanInitial) {
  // Finish intersection handling by exiting the outline through the leading
  // edge of the stroke.
  //
  //      R            R             L            L
  //      |             \            |           /
  //  L   | R      L     R         L |   R      L     R
  //  |   |/|      |     |         |\|   |      |     |
  //  |   R |  =>  |     |   and   | L   |  =>  |     |
  //  |     |      |     |         |     |      |     |
  //  L-----R      L-----R         L-----R      L-----R

  {
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 0.5}, 1));

    geometry.AppendRightVertex(Point{0.75, 1});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 0.5}, 1));
    EXPECT_TRUE(geometry.RightSide().intersection.has_value());
    EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
    EXPECT_GT(
        geometry.RightSide().intersection->outline_reposition_budget,
        geometry.RightSide().intersection->initial_outline_reposition_budget);
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices.back())
                    .position,
                PointEq({0.75, 1}));

    geometry.AppendRightVertex(Point{0.75, 3});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 1));
    EXPECT_FALSE(geometry.RightSide().intersection.has_value());

    uint32_t n_indices = geometry.RightSide().indices.size();
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices[n_indices - 2])
                    .position,
                PointEq({1, 2}));
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.RightSide().indices[n_indices - 1])
                    .position,
                PointEq({0.75, 3}));
  }

  {
    MeshData mesh_data;
    Geometry geometry(MakeView(mesh_data));
    geometry.AppendLeftVertex(Point{0, 0});
    geometry.AppendLeftVertex(Point{0, 2});
    geometry.AppendRightVertex(Point{1, 0});
    geometry.AppendRightVertex(Point{1, 2});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 0.5}, 1));

    geometry.AppendLeftVertex(Point{0.25, 1});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 0.5}, 1));
    EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
    EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
    EXPECT_GT(
        geometry.LeftSide().intersection->outline_reposition_budget,
        geometry.LeftSide().intersection->initial_outline_reposition_budget);
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices.back())
                    .position,
                PointEq({0.25, 1}));

    geometry.AppendLeftVertex(Point{0.25, 3});
    geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 1));
    EXPECT_FALSE(geometry.LeftSide().intersection.has_value());

    uint32_t n_indices = geometry.LeftSide().indices.size();
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices[n_indices - 2])
                    .position,
                PointEq({0, 2}));
    EXPECT_THAT(geometry.GetMeshView()
                    .GetVertex(geometry.LeftSide().indices[n_indices - 1])
                    .position,
                PointEq({0.25, 3}));
  }
}

TEST(GeometryTest, SimplificationCausesCwTriangle) {
  // Tests an edge case where `geometry::Simplify` can behave correctly, but
  // replacing the last vertex of the previous extrusion actually creates a
  // missed clockwise winding triangle.
  //
  // This can happen when three vertices are clustered together relative to the
  // simplification threshold. Because the simplification algorithm does not
  // have awareness of the overall direction of the stroke, it can remove the
  // middle vertex as if it is slightly off to one side. But actually what has
  // happened is the vertices have made a u-turn.
  //
  // E.g.
  //      X <--- Simplify can try to remove this vertex because the direction of
  //     / \     travel changes too rapidly.
  //    X   \
  //         X

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  // Start with a two-triangle quad:
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionToTipState({0, 0}));
  // Add one skinny triangle on top:
  geometry.AppendRightVertex(Point{1.05, 1.05});
  geometry.ProcessNewVertices(0, PositionToTipState({0, 0}));

  // Add the problematic vertex:
  geometry.AppendRightVertex(Point{1.1, 0.95});

  // Verify that the current setup for the right side would cause the
  // simplification algorithm to remove the {1.05, 1.05} vertex:
  float simplification_threshold = 0.1;
  const std::vector<ExtrudedVertex>& buffered_vertices =
      geometry.RightSide().vertex_buffer;
  EXPECT_THAT(buffered_vertices, ElementsAre(VertexPositionEq({1, 1}),
                                             VertexPositionEq({1.05, 1.05}),
                                             VertexPositionEq({1.1, 0.95})));
  std::vector<ExtrudedVertex> simplified_vertices;
  SimplifyPolyline(buffered_vertices.cbegin(), buffered_vertices.cend(),
                   simplification_threshold, simplified_vertices);
  EXPECT_THAT(simplified_vertices, ElementsAre(VertexPositionEq({1, 1}),
                                               VertexPositionEq({1.1, 0.95})));

  // But ProcessNewVertices should not be allowed to replace the {1.05, 1.05}
  // vertex with the one at {1.1, 0.95} because that would cause a CW triangle:
  geometry.ProcessNewVertices(simplification_threshold,
                              PositionToTipState({0, 0}));
  EXPECT_THAT(geometry.GetMeshView(), TrianglesAreNotCw());
}

TEST(GeometryTest, LargeSimplificationDuringIntersection) {
  // Tests an edge case where `geometry::Simplify` removes the last processed
  // vertex on the side opposite of an ongoing intersection. If the distance
  // between the vertex and its replacement is large enough, this can cause the
  // exanded triangle to contain whole segments of the intersecting side
  // outline. This could cause intersection handling to give up incorrectly,
  // because the start of the outline would not be contained in a new triangle
  // during segment-outline intersection search.

  BrushTipState tip_state = PositionAndSizeToTipState({0, 0}, 1);
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  // Start with a four-triangle rectangle going up:
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 1.8});
  geometry.AppendLeftVertex(Point{0, 2});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 1.8});
  geometry.AppendRightVertex(Point{1, 2});
  geometry.ProcessNewVertices(0, tip_state);

  // Turn to the left and down to start an intersection:
  geometry.AppendLeftVertex(Point{0.5, 1.9});
  geometry.AppendRightVertex(Point{0.5, 2.5});
  geometry.AppendRightVertex(Point{-0.5, 2.5});
  geometry.AppendRightVertex(Point{-0.5, 1.9});
  geometry.ProcessNewVertices(0, tip_state);

  const Side& left_side = geometry.LeftSide();
  EXPECT_TRUE(left_side.intersection.has_value());
  EXPECT_TRUE(left_side.intersection->retriangulation_started);

  // Check positions for the last segment of the outline on the left side before
  // the intersection:
  EXPECT_THAT(
      geometry.GetMeshView()
          .GetVertex(
              left_side.indices[left_side.intersection->starting_offset - 1])
          .position,
      PointEq({0, 1.9}));
  EXPECT_THAT(
      geometry.GetMeshView()
          .GetVertex(
              left_side.indices[left_side.intersection->starting_offset - 2])
          .position,
      PointEq({0, 1.8}));

  uint32_t n_triangles = geometry.GetMeshView().TriangleCount();
  uint32_t n_right_indices = geometry.RightSide().indices.size();

  // Process just one new vertex on the outside of the turn. This should only
  // trigger simplification: the number of triangles and right-side indices
  // should not change.
  geometry.AppendRightVertex(Point{-0.5, 1.5});
  geometry.ProcessNewVertices(1, tip_state);
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), n_triangles);
  EXPECT_EQ(geometry.RightSide().indices.size(), n_right_indices);

  // The left side should still be intersecting and the outline should have been
  // moved by simplification.
  EXPECT_TRUE(left_side.intersection.has_value());
  EXPECT_TRUE(left_side.intersection->retriangulation_started);
  EXPECT_THAT(
      geometry.GetMeshView()
          .GetVertex(
              left_side.indices[left_side.intersection->starting_offset - 1])
          .position,
      PointEq({0, 1.7}));
}

TEST(GeometryTest, SimplificationGivesUpIntersection) {
  // Tests the case where simplification opposite an ongoing intersection causes
  // the intersection to run out of `outline_reposition_budget` and give up.
  //
  //  L---------L
  //  |         |
  //  L  R--R   |
  //  |  R  |   L
  //  |     |   |
  //  |     |   L
  //  L-----R

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 2});
  geometry.AppendLeftVertex(Point{0, 2.5});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.AppendRightVertex(Point{1, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));

  geometry.AppendRightVertex(Point{0.5, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  geometry.AppendLeftVertex(Point{1.5, 2.5});
  geometry.AppendLeftVertex(Point{1.5, 1.5});
  geometry.AppendRightVertex(Point{0.5, 1.5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0.5, 0.5}, 1));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  // Append one left vertex that would cause the intersection to run out of
  // `outline_reposition_budget`. Enable simplification, which would usually try
  // to remove the last left vertex.
  uint32_t triangles_before = geometry.GetMeshView().TriangleCount();
  uint32_t left_indices_before = geometry.LeftSide().indices.size();
  geometry.AppendLeftVertex(Point{0.5, 0.5});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({0.5, 0.5}, 1));

  EXPECT_FALSE(geometry.RightSide().intersection.has_value());
  // Since last vertex replacement shouldn't have taken place, there should be a
  // new triangle instead of the last triangle becoming larger:
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), triangles_before + 1);
  // Giving up intersection handling means an extra duplicate vertex should have
  // been appended before appending the newly processed one:
  EXPECT_EQ(geometry.LeftSide().indices.size(), left_indices_before + 2);
  // The second to last left side position should not have been simplified away.
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(*(geometry.LeftSide().indices.rbegin() + 1))
                  .position,
              PointEq({1.5, 1.5}));
}

TEST(GeometryTest, OverlappingIntersectionDisconnectsPartition) {
  // Test that the partition boundary of an old intersection is updated properly
  // when an intersection on the opposite side gives up while overlapping the
  // partition start.
  //
  //    END
  //  L-----R
  //  |     |
  //  |  R--|-------R      |
  //  |  |  |       |      |  *--*
  //  |  |  | L--L  |      |  |  |
  //  |  R--R |  |  |      *--*  |
  //  |       |  |  |            |
  //  L-------|--L  |
  //          L-----R
  //           START
  //
  //      Geometry       Overall Travel

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  // Start traveling up.
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{0, 5});
  geometry.AppendRightVertex(Point{2, 0});
  geometry.AppendRightVertex(Point{2, 8});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 3}, 2));

  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());
  EXPECT_EQ(geometry.GetMeshView().TriangleCount(), 2);

  // Turn left and down to start a self-intersection on the left side.
  geometry.AppendLeftVertex(Point{1, 5});
  geometry.AppendLeftVertex(Point{1, 4});
  geometry.AppendRightVertex(Point{-1, 8});
  geometry.AppendRightVertex(Point{-1, 4});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 3}, 2));

  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  // Travel down far enough before turning so the intersection has to give up.
  geometry.AppendLeftVertex(Point{1, 2});
  geometry.AppendLeftVertex(Point{-2, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 3}, 2));

  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  EXPECT_NE(geometry.LeftSide().partition_start.first_triangle, 0);
  EXPECT_TRUE(geometry.LeftSide().partition_start.outline_connects_sides);
  EXPECT_FALSE(geometry.LeftSide().partition_start.is_forward_exterior);

  EXPECT_EQ(geometry.RightSide().partition_start.first_triangle, 0);
  EXPECT_TRUE(geometry.RightSide().partition_start.outline_connects_sides);
  EXPECT_TRUE(geometry.RightSide().partition_start.is_forward_exterior);

  uint32_t left_partition_first_triangle =
      geometry.LeftSide().partition_start.first_triangle;

  // Turn up to start a self-intersection on the right side.
  geometry.AppendRightVertex(Point{0, 4});
  geometry.AppendRightVertex(Point{0, 5});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 3}, 2));

  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  // The right-side intersection should have traveled backwards in the mesh past
  // the first triangle in the left side partition:
  EXPECT_LT(geometry.RightSide().intersection->oldest_retriangulation_triangle,
            left_partition_first_triangle);

  // Travel up far enough for the right side intersection to give up.
  geometry.AppendLeftVertex(Point{-2, 7});
  geometry.AppendRightVertex(Point{0, 7});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 3}, 2));

  EXPECT_EQ(geometry.LeftSide().partition_start.first_triangle, 0);
  EXPECT_FALSE(geometry.LeftSide().partition_start.outline_connects_sides);
  EXPECT_FALSE(geometry.LeftSide().partition_start.is_forward_exterior);

  EXPECT_NE(geometry.RightSide().partition_start.first_triangle, 0);
  EXPECT_TRUE(geometry.RightSide().partition_start.outline_connects_sides);
  EXPECT_FALSE(geometry.RightSide().partition_start.is_forward_exterior);
}

TEST(GeometryTest, BothSidesFinishIntersection) {
  // Edge case where both sides finish intersection handling on the same
  // proposed vertex. The right side begins an intersection and is allowed to
  // modify the mesh. Then the left side begin intersection handling. On the
  // last proposed vertex, which is on the left side, the right side must give
  // up intersection handling because it will exceed the repositioning budget
  // and the left side must finish intersection handling because the vertex is
  // exterior and proposes a CCW triangle.
  //
  //    Vertices:                 Overall travel:
  //        L
  //       /
  //  L------L
  //  |  L---/                        End
  //  |                               /
  //  |     --------R            -----
  //  |     |   R   |            |
  //  |     R--/    |            ---------- Start
  //  L-------------L

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  geometry.AppendLeftVertex(Point{5, 0});
  geometry.AppendLeftVertex(Point{0, 0});
  geometry.AppendRightVertex(Point{5, 2});
  geometry.AppendRightVertex(Point{1, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({3, 1}, 2));

  // Start a turn that begins a self-intersection on the right side.
  geometry.AppendLeftVertex(Point{0, 3});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({1, 2}, 2));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  // Continue the self-intersection by traveling to the right.
  geometry.AppendLeftVertex(Point{2, 3});
  geometry.AppendRightVertex(Point{4, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({2, 2}, 2));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);

  // Start turning upward, which begins a self-intersection on the left side.
  // Because the right side is modifying the triangulation, the left side does
  // not.
  geometry.AppendLeftVertex(Point{1, 2});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({2, 2}, 2));
  EXPECT_TRUE(geometry.RightSide().intersection.has_value());
  EXPECT_TRUE(geometry.RightSide().intersection->retriangulation_started);
  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_FALSE(geometry.LeftSide().intersection->retriangulation_started);

  uint32_t n_left_before_end = geometry.LeftSide().indices.size();

  // Append an exterior left vertex.
  geometry.AppendLeftVertex(Point{2, 4});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({3, 3}, 2));

  // The right side must give up intersection handling because the repositioning
  // budget has been exceeded.
  EXPECT_FALSE(geometry.RightSide().intersection.has_value());

  // The left side finishes intersection handling.
  EXPECT_FALSE(geometry.LeftSide().intersection.has_value());

  // When the right side gave up, it appended a new vertex on the left side.
  // Then, when the left side finished intersection handling, it moved the start
  // of its outline to the position where the stroke exits self-intersection,
  // and then appended the new exterior vertex and triangle. Check that the
  // vertex appended on the left side when the right side gave up is also
  // repositioned:
  EXPECT_EQ(geometry.LeftSide().indices.size(), n_left_before_end + 2);
  EXPECT_EQ(geometry.RightSide().partition_start.opposite_first_index_offset,
            n_left_before_end);
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(geometry.LeftSide().indices[n_left_before_end])
                  .position,
              PointNear({1.5, 3}, 0.01));
  EXPECT_THAT(geometry.GetMeshView()
                  .GetVertex(geometry.LeftSide().indices[n_left_before_end + 1])
                  .position,
              PointEq({2, 4}));
}

TEST(GeometryTest, AvoidSimplificationThatInvalidatesPreviousSimplification) {
  std::vector<Point> positions = {
      {0, 0}, {1, 0}, {2, -1}, {2.1, -1.1}, {2.11, -1.11}};
  float simplification_threshold = 0.46f;
  // Considering 0-2, 1 would get simplified away.
  EXPECT_LT(Distance(Segment{positions[0], positions[2]}, positions[1]),
            simplification_threshold);
  // Considering 0-3, 2 would get simplified away.
  EXPECT_LT(Distance(Segment{positions[0], positions[3]}, positions[2]),
            simplification_threshold);
  // But 1 actually _is_ far enough from 0-3!
  EXPECT_GT(Distance(Segment{positions[0], positions[3]}, positions[1]),
            simplification_threshold);
  // If 2 is kept, 3 will be simplified away.
  EXPECT_LT(Distance(Segment{positions[2], positions[4]}, positions[3]),
            simplification_threshold);
  // If 2 is not kept, 3 will also be simplified away.
  EXPECT_LT(Distance(Segment{positions[0], positions[4]}, positions[3]),
            simplification_threshold);

  auto append_vertices = [&](Geometry& geometry, const Point& point) {
    geometry.AppendLeftVertex(point);
    geometry.AppendRightVertex(Point{point.x, point.y - 0.5f});
    geometry.ProcessNewVertices(simplification_threshold,
                                PositionAndSizeToTipState({0, 0}, 2));
  };

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  append_vertices(geometry, positions[0]);
  append_vertices(geometry, positions[1]);
  // No opportunity for simplification here, we get 4 vertices after first
  // two updates.
  EXPECT_EQ(geometry.GetMeshView().VertexCount(), 4);
  // The third update doesn't increase the vertex count because 1 is close
  // to 0-2 and simplified away.
  append_vertices(geometry, positions[2]);
  EXPECT_EQ(geometry.GetMeshView().VertexCount(), 4);
  // The fourth update increases the vertex count. 2 is close to 0-3, but if
  // that were removed 1 would no longer be an irrelevant vertex, since it's
  // farther from 0-3.
  append_vertices(geometry, positions[3]);
  EXPECT_EQ(geometry.GetMeshView().VertexCount(), 6);
  // After 2 is kept, 3 is simplified away when 4 is added.
  append_vertices(geometry, positions[4]);
  EXPECT_EQ(geometry.GetMeshView().VertexCount(), 6);
}

TEST(GeometryTest, ClearSavedSimplifiedPositionsAfterAppendingNewVertices) {
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  // Begin by extruding a 4-vertex quad traveling in the positive-x direction:
  //
  // L = left-side position, R = right-side position
  //
  // L-----L
  // |
  // |         ----> travel direction
  // |
  // R-----R
  geometry.AppendLeftVertex(Point{0, 1});
  geometry.AppendRightVertex(Point{0, 0});
  geometry.AppendLeftVertex(Point{1, 1});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({1, 0}, 1));

  // Append one more vertex per side such that each is perfectly in line with
  // the first two positions on each side, causing a simplification of an
  // existing position.
  //
  // L = left-side position, R = right-side position, S = simplified away
  //
  // L-----S-----L
  // |
  // |               ----> travel direction
  // |
  // R-----S-----R
  geometry.AppendLeftVertex(Point{2, 1});
  geometry.AppendRightVertex(Point{2, 0});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({2, 0}, 1));

  EXPECT_THAT(geometry.LeftSide().last_simplified_vertex_positions,
              ElementsAre(PointEq({1, 1})));
  EXPECT_THAT(geometry.RightSide().last_simplified_vertex_positions,
              ElementsAre(PointEq({1, 0})));

  // Append one more vertex per side such that each is out of line with the
  // first two positions on each side, causing no simplification of an existing
  // position.
  //
  // L = left-side position, R = right-side position, S = simplified away
  //
  //                L
  //               /
  //              /
  // L-----S-----L          ^
  // |              R      /
  // |             /  ----   travel direction
  // |            /
  // R-----S-----R
  geometry.AppendLeftVertex(Point{3, 2});
  geometry.AppendRightVertex(Point{3, 1});
  geometry.ProcessNewVertices(0.1, PositionAndSizeToTipState({3, 1}, 1));

  // Since new vertices were appended, the simplified positions should have been
  // cleared:
  EXPECT_THAT(geometry.LeftSide().last_simplified_vertex_positions, IsEmpty());
  EXPECT_THAT(geometry.RightSide().last_simplified_vertex_positions, IsEmpty());
}

TEST(GeometryTest, SelfIntersectionFromTipStatesWithZeroWidth) {
  // Extrude vertices using tip states that have zero width and nonzero height,
  // making sure that self-intersection handling is still active.
  //
  // Create a mesh traveling to the right, and then sharply turning up to start
  // a self-intersecting sharp turn:
  //
  // L = left vertex and R = right vertex in the diagram below
  //
  //   L-------L         travel     ^
  //   |      /          direction  |
  //   |     L   R             -----
  //   |        /
  //   R-------R

  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));
  auto zero_width_tip_state = [](Point p) {
    return BrushTipState{.position = p, .width = 0, .height = 1};
  };

  geometry.AppendLeftVertex(Point{.x = 0, .y = 1});
  geometry.AppendRightVertex(Point{.x = 0, .y = 0});
  geometry.ProcessNewVertices(0, zero_width_tip_state({0, 0.5}));

  geometry.AppendLeftVertex(Point{.x = 2, .y = 1});
  geometry.AppendRightVertex(Point{.x = 2, .y = 0});
  geometry.ProcessNewVertices(0, zero_width_tip_state({1, 0.5}));

  // The mesh should have a 4 vertex and 2 triangle quad with no
  // self-intersection on the left side.
  ASSERT_EQ(geometry.GetMeshView().VertexCount(), 4);
  ASSERT_EQ(geometry.GetMeshView().TriangleCount(), 2);
  ASSERT_FALSE(geometry.LeftSide().intersection.has_value());

  geometry.AppendLeftVertex(Point{.x = 1, .y = 0.5});
  geometry.AppendRightVertex(Point{.x = 3, .y = 0.5});
  geometry.ProcessNewVertices(0, zero_width_tip_state({1, 0.5}));

  EXPECT_TRUE(geometry.LeftSide().intersection.has_value());
  EXPECT_TRUE(geometry.LeftSide().intersection->retriangulation_started);
}

TEST(GeometryTest, FirstMutatedIndexOffsets) {
  MeshData mesh_data;
  Geometry geometry(MakeView(mesh_data));

  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 0);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 0);

  geometry.AppendLeftVertex(Point{-1, 0});
  geometry.AppendRightVertex(Point{1, 0});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 0}, 1));

  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 0);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 0);

  // Resetting the mutation tracking should set all of the indices to clean.
  geometry.ResetMutationTracking();
  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 1);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 1);

  // Set a save point to check that mutation tracking takes it into account when
  // we revert.
  geometry.SetSavePoint();

  geometry.AppendLeftVertex(Point{-1, 1});
  geometry.AppendRightVertex(Point{1, 1});
  geometry.ProcessNewVertices(0, PositionAndSizeToTipState({0, 1}, 1));

  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 1);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 1);

  geometry.ResetMutationTracking();
  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 2);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 2);

  geometry.RevertToSavePoint();
  EXPECT_EQ(geometry.FirstMutatedLeftIndexOffsetInCurrentPartition(), 1);
  EXPECT_EQ(geometry.FirstMutatedRightIndexOffsetInCurrentPartition(), 1);
}

}  // namespace
}  // namespace brush_tip_extruder_internal
}  // namespace ink
