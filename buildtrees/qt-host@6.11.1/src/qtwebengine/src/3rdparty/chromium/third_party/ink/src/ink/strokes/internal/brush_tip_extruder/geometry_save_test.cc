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
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/base/nullability.h"
#include "absl/types/span.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/legacy_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::BrushTipState;
using ::ink::strokes_internal::LegacyVertex;
using ::testing::ElementsAreArray;
using ::testing::ExplainMatchResult;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::Value;

struct MeshData {
  std::vector<LegacyVertex> vertices;
  std::vector<IndexType> triangle_indices;
};

MutableMeshView MakeView(MeshData& data) {
  return MutableMeshView(data.vertices, data.triangle_indices);
}

BrushTipState PositionAndSizeToTipState(Point p, float size) {
  return BrushTipState{
      .position = p,
      .width = size,
      .height = size,
      .percent_radius = 1,
  };
}

MATCHER_P(VerticesAndIndicesEq, mesh_data, "") {
  if (!Value(arg.vertices, ElementsAreArray(mesh_data.vertices))) {
    *result_listener << "\nMeshData::vertices do not match: ";
    return ExplainMatchResult(ElementsAreArray(mesh_data.vertices),
                              arg.vertices, result_listener);
  }
  if (!Value(arg.triangle_indices,
             ElementsAreArray(mesh_data.triangle_indices))) {
    *result_listener << "\nMeshData::triangle_indices do not match: ";
    return ExplainMatchResult(ElementsAreArray(mesh_data.triangle_indices),
                              arg.triangle_indices, result_listener);
  }
  return true;
}

bool SelfIntersectionsEqual(const Side::SelfIntersection& a,
                            const Side::SelfIntersection& b) {
  return a.starting_position == b.starting_position &&
         a.last_proposed_vertex == b.last_proposed_vertex &&
         a.last_proposed_vertex_triangle == b.last_proposed_vertex_triangle &&
         a.starting_offset == b.starting_offset &&
         a.retriangulation_started == b.retriangulation_started &&
         a.undo_stack_starting_triangle == b.undo_stack_starting_triangle &&
         a.undo_triangulation_stack == b.undo_triangulation_stack &&
         a.outline_reposition_budget == b.outline_reposition_budget &&
         a.initial_outline_reposition_budget ==
             b.initial_outline_reposition_budget &&
         a.travel_limit_from_starting_position ==
             b.travel_limit_from_starting_position;
}

bool MeshPartitionStartsEqual(const Side::MeshPartitionStart& a,
                              const Side::MeshPartitionStart& b) {
  return a.adjacent_first_index_offset == b.adjacent_first_index_offset &&
         a.opposite_first_index_offset == b.opposite_first_index_offset &&
         a.first_triangle == b.first_triangle &&
         a.opposite_side_initial_position == b.opposite_side_initial_position &&
         a.non_ccw_connection_index == b.non_ccw_connection_index &&
         a.outline_connects_sides == b.outline_connects_sides &&
         a.is_forward_exterior == b.is_forward_exterior;
}

MATCHER(IndexOffsetRangesMatch, "") {
  const Side::IndexOffsetRange& a = std::get<0>(arg);
  const Side::IndexOffsetRange& b = std::get<1>(arg);
  return a.first == b.first && a.first == b.first;
}

MATCHER_P(SideEq, side, "") {
  if (!Value(arg.indices, ElementsAreArray(side.indices))) {
    *result_listener << "\nSide::indices do not match: ";
    return ExplainMatchResult(ElementsAreArray(side.indices), arg.indices,
                              result_listener);
  }
  if (!Value(arg.intersection_discontinuities,
             ::testing::Pointwise(IndexOffsetRangesMatch(),
                                  side.intersection_discontinuities))) {
    *result_listener << "\nSide::intersection_discontinuities do not match.";
    return false;
  }
  if (arg.first_simplifiable_index_offset !=
      side.first_simplifiable_index_offset) {
    *result_listener
        << "\nSide::first_simplifiable_index_offset do not match: expected "
        << side.first_simplifiable_index_offset << "; got "
        << arg.first_simplifiable_index_offset;
    return false;
  }
  if (!Value(arg.vertex_buffer, ElementsAreArray(side.vertex_buffer))) {
    *result_listener << "\nSide::vertex_buffer do not match: ";
    return ExplainMatchResult(ElementsAreArray(side.vertex_buffer),
                              arg.vertex_buffer, result_listener);
  }
  if (arg.next_buffered_vertex_offset != side.next_buffered_vertex_offset) {
    *result_listener
        << "\nSide::next_buffered_vertex_offset do not match: expected "
        << side.next_buffered_vertex_offset << "; got "
        << arg.next_buffered_vertex_offset;
    return false;
  }
  // Forego more verbose result explanation and do a simple equality (at least
  // for now), because this matcher is for testing Save/Revert where the
  // intersection and partition_start are each copied as a whole.
  if (!MeshPartitionStartsEqual(arg.partition_start, side.partition_start)) {
    *result_listener << "\nSide::partition_start do not match.";
    return false;
  }
  if (arg.intersection.has_value() != side.intersection.has_value() ||
      (arg.intersection.has_value() &&
       !SelfIntersectionsEqual(*arg.intersection, *side.intersection))) {
    *result_listener << "\nSide::intersection do no match";
    return false;
  }
  if (!Value(arg.last_simplified_vertex_positions,
             ElementsAreArray(side.last_simplified_vertex_positions))) {
    *result_listener
        << "\nSide::last_simplified_vertex_positions do not match: ";
    return ExplainMatchResult(
        ElementsAreArray(side.last_simplified_vertex_positions),
        arg.last_simplified_vertex_positions, result_listener);
  }
  return true;
}

class GeometrySaveTest : public testing::Test {
 protected:
  struct Extrusion {
    std::vector<Point> left;
    std::vector<Point> right;
    float simplification_threshold;
  };

  void AppendVertices(Geometry* absl_nonnull geometry,
                      const Extrusion& extrusion) {
    for (Point p : extrusion.left) {
      geometry->AppendLeftVertex(p);
    }
    for (Point p : extrusion.right) {
      geometry->AppendRightVertex(p);
    }
  }

  void Extrude(Geometry* absl_nonnull geometry,
               absl::Span<const Extrusion> extrusions) {
    for (const Extrusion& extrusion : extrusions) {
      AppendVertices(geometry, extrusion);
      (void)geometry->ProcessNewVertices(extrusion.simplification_threshold,
                                         PositionAndSizeToTipState({0, 0}, 2));
    }
  }

  // Performs an identical extrusion on multiple Geometry objects. This is
  // useful for having a known correct copy of Geometry from prior to setting a
  // save point.
  void Extrude(absl::Span<Geometry* absl_nonnull const> geometries,
               absl::Span<const Extrusion> extrusions) {
    for (Geometry* absl_nonnull g : geometries) {
      Extrude(g, extrusions);
    }
  }
};

TEST_F(GeometrySaveTest, AfterSavePoint) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });
  g1.SetSavePoint();
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
}

TEST_F(GeometrySaveTest, RevertWithoutSavePoint) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });
  g1.ResetMutationTracking();
  g1.RevertToSavePoint();
  EXPECT_TRUE(g1.CalculateVisuallyUpdatedRegion().IsEmpty());
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, ImmediateRevert) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });
  g1.SetSavePoint();
  g1.ResetMutationTracking();
  g1.RevertToSavePoint();
  EXPECT_TRUE(g1.CalculateVisuallyUpdatedRegion().IsEmpty());
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, UnprocessedVertices) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  AppendVertices(&g1, {.left = {{-1, 2}, {-1, 3}}, .right = {{1, 2}, {1, 3}}});

  g1.RevertToSavePoint();
  EXPECT_TRUE(g1.CalculateVisuallyUpdatedRegion().IsEmpty());
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, ExtrudeWithoutIntersection) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {
                   {.left = {{-1, 2}, {-1, 3}}, .right = {{1, 2}, {1, 3}}},
               });

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({0, 2}, 2, 2)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, SimplificationThreshold) {
  // TODO(b/201002500): Until fixed, simplification should not reach across the
  // save point.
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}}, .right = {{1, 0}, {1, 1}}},
          });

  g1.SetSavePoint();
  g1.ResetMutationTracking();

  // After save point, vertex buffers should only have one element since they
  // shouldn't be able to simplify pre-save-point vertices:
  EXPECT_EQ(g1.LeftSide().vertex_buffer.size(), 1);
  EXPECT_THAT(g1.LeftSide().vertex_buffer[0].position, PointEq({-1, 1}));
  EXPECT_EQ(g1.RightSide().vertex_buffer.size(), 1);
  EXPECT_THAT(g1.RightSide().vertex_buffer[0].position, PointEq({1, 1}));

  Extrude(&g1, {
                   {.left = {{-1, 2}},
                    .right = {{1, 2}},
                    .simplification_threshold = 0.1},
               });
  EXPECT_EQ(g1.GetMeshView().TriangleCount(), 4);
  EXPECT_EQ(g1.GetMeshView().VertexCount(), 6);

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({0, 1.5}, 2, 1)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, ContinueIntersection) {
  // Extrusion travels up and then sharply to the left and back down.
  // Intersection is ongoing prior to the save point and still exists before
  // reverting.

  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2}, {
                          {.left = {{-1, 0}, {-1, 1}, {-1, 2}},
                           .right = {{1, 0}, {1, 1}, {1, 2}}},
                          {.left = {{-0.5, 1.5}}, .right = {{0.5, 2.5}}},
                          {.left = {{0, 1.5}}, .right = {{0, 2.5}}},
                      });

  EXPECT_TRUE(g1.LeftSide().intersection.has_value());
  EXPECT_TRUE(g1.LeftSide().intersection->retriangulation_started);

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1,
          {
              {.left = {{0, 0.5}}, .right = {{-1, 2.5}, {-2, 1}, {-2, 0.5}}},
          });
  EXPECT_TRUE(g1.LeftSide().intersection.has_value());
  EXPECT_TRUE(g1.LeftSide().intersection->retriangulation_started);

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({-0.5, 1.25}, 3, 2.5)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, EndIntersection) {
  // Extrusion travels up and then sharply to the left. Intersection is ongoing
  // prior to the save point and is finished prior to reverting.

  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 2}}, .right = {{1, 0}, {1, 2}}},
              {.left = {{-0.5, 1.5}}, .right = {{0.5, 2.5}}},
              {.left = {{0, 1.5}}, .right = {{0, 2.5}}},
          });

  EXPECT_TRUE(g1.LeftSide().intersection.has_value());
  EXPECT_TRUE(g1.LeftSide().intersection->retriangulation_started);

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {
                   {.left = {{-1.5, 1.5}}, .right = {{-1.5, 2.5}}},
               });
  EXPECT_FALSE(g1.LeftSide().intersection.has_value());

  g1.RevertToSavePoint();
  EXPECT_THAT(
      g1.CalculateVisuallyUpdatedRegion(),
      EnvelopeEq(Rect::FromCenterAndDimensions({-0.25, 1.25}, 2.5, 2.5)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, BeginIntersection) {
  // Extrusion travels up and then sharply to the right. Intersection begins
  // after save point and still exists before reverting.

  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 2}}, .right = {{1, 0}, {1, 2}}},
          });

  EXPECT_FALSE(g1.RightSide().intersection.has_value());

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {
                   {.left = {{-0.5, 2.5}}, .right = {{0.5, 1.5}}},
                   {.left = {{0, 2.5}}, .right = {{0, 1.5}}},
               });

  EXPECT_TRUE(g1.RightSide().intersection.has_value());
  EXPECT_TRUE(g1.RightSide().intersection->retriangulation_started);

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({0, 1.25}, 2, 2.5)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, BeginAndEndIntersection) {
  // Extrusion travels up and then sharply to the right. Intersection begins and
  // ends after the save point.

  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 2}}, .right = {{1, 0}, {1, 2}}},
          });

  EXPECT_FALSE(g1.RightSide().intersection.has_value());

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {
                   {.left = {{-0.5, 2.5}}, .right = {{0.5, 1.5}}},
                   {.left = {{0, 2.5}}, .right = {{0, 1.5}}},
               });
  EXPECT_TRUE(g1.RightSide().intersection.has_value());
  EXPECT_TRUE(g1.RightSide().intersection->retriangulation_started);
  Extrude(&g1, {
                   {.left = {{1.5, 2.5}}, .right = {{1.5, 1.5}}},
               });
  EXPECT_FALSE(g1.RightSide().intersection.has_value());

  g1.RevertToSavePoint();
  EXPECT_THAT(
      g1.CalculateVisuallyUpdatedRegion(),
      EnvelopeEq(Rect::FromCenterAndDimensions({0.25, 1.25}, 2.5, 2.5)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, StableTriangles) {
  MeshData mesh_data;
  Geometry line_geometry(MakeView(mesh_data));
  line_geometry.SetIntersectionHandling(
      Geometry::IntersectionHandling::kDisabled);
  Extrude(&line_geometry, {
                              {.left = {{-1, 0}, {-1, 1}, {-1, 2}},
                               .right = {{1, 0}, {1, 1}, {1, 2}}},
                          });

  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 4);
  EXPECT_EQ(line_geometry.NStableTriangles(), 2);

  line_geometry.SetSavePoint();
  Extrude(&line_geometry,
          {
              {.left = {{-1, 3}, {-1, 4}}, .right = {{1, 3}, {1, 4}}},
          });

  EXPECT_EQ(line_geometry.GetMeshView().TriangleCount(), 8);
  EXPECT_EQ(line_geometry.NStableTriangles(), 2);
}

TEST_F(GeometrySaveTest, MeshPartition) {
  // Extrusion travels up and sharply to the left. It travels back down long
  // enough for intersection handling to give up, which triggers a logical
  // partition of the mesh and sides.

  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2},
          {
              {.left = {{-1, 0}, {-1, 1}, {-1, 2}, {-1, 3}, {-1, 4}},
               .right = {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}}},
              {.left = {{-0.5, 3.5}}, .right = {{-0.5, 4.5}}},
              {.left = {{0, 3.5}}, .right = {{-2, 3.5}}},
          });

  EXPECT_TRUE(g1.LeftSide().intersection.has_value());
  EXPECT_TRUE(g1.LeftSide().intersection->retriangulation_started);

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {
                   {.left = {{0, 3}, {0, 2}, {0, 1}},
                    .right = {{-2, 3}, {-2, 2}, {-2, 1}}},
               });
  EXPECT_FALSE(g1.LeftSide().intersection.has_value());

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromCenterAndDimensions({-0.5, 2.75}, 3, 3.5)));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, RestoresLastSimplifiedVertexPositions) {
  MeshData m1, m2;
  Geometry g1(MakeView(m1)), g2(MakeView(m2));
  Extrude({&g1, &g2}, {
                          {.left = {{-1, 0}, {-1, 1}},
                           .right = {{1, 0}, {1, 1}},
                           .simplification_threshold = 0.1},
                          {.left = {{-1, 2}},
                           .right = {{1, 2}},
                           .simplification_threshold = 0.1},
                      });

  ASSERT_THAT(g2.LeftSide().last_simplified_vertex_positions,
              ElementsAre(PointEq({-1, 1})));
  ASSERT_THAT(g2.RightSide().last_simplified_vertex_positions,
              ElementsAre(PointEq({1, 1})));

  g1.SetSavePoint();
  g1.ResetMutationTracking();
  Extrude(&g1, {{.left = {{-2, 3}},
                 .right = {{0, 3}},
                 .simplification_threshold = 0.1}});

  ASSERT_THAT(g1.LeftSide().last_simplified_vertex_positions, IsEmpty());
  ASSERT_THAT(g1.RightSide().last_simplified_vertex_positions, IsEmpty());

  g1.RevertToSavePoint();
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
}

TEST_F(GeometrySaveTest, DebugMeshAfterSavePoint) {
  MeshData mesh_data;
  Geometry line_geometry(MakeView(mesh_data));

  MeshData mesh_data_after_save;
  line_geometry.DebugMakeMeshAfterSavePoint(MakeView(mesh_data_after_save));
  EXPECT_EQ(mesh_data_after_save.vertices.size(), 0);
  EXPECT_EQ(mesh_data_after_save.triangle_indices.size(), 0);

  Extrude(&line_geometry, {
                              {.left = {{-1, 0}, {-1, 1}, {-1, 2}},
                               .right = {{1, 0}, {1, 1}, {1, 2}}},
                          });

  line_geometry.DebugMakeMeshAfterSavePoint(MakeView(mesh_data_after_save));
  EXPECT_EQ(mesh_data_after_save.vertices.size(), 0);
  EXPECT_EQ(mesh_data_after_save.triangle_indices.size(), 0);

  line_geometry.SetSavePoint();

  line_geometry.DebugMakeMeshAfterSavePoint(MakeView(mesh_data_after_save));
  EXPECT_EQ(mesh_data_after_save.vertices.size(), 0);
  EXPECT_EQ(mesh_data_after_save.triangle_indices.size(), 0);

  Extrude(&line_geometry, {
                              {.left = {{-1, 3}}, .right = {{1, 3}, {1, 4}}},
                          });

  line_geometry.DebugMakeMeshAfterSavePoint(MakeView(mesh_data_after_save));
  EXPECT_EQ(mesh_data_after_save.vertices.size(), 5);
  EXPECT_THAT(mesh_data_after_save.triangle_indices,
              ElementsAreArray({1, 0, 2, 1, 2, 3, 3, 2, 4}));
  Envelope envelope;
  for (const LegacyVertex v : mesh_data_after_save.vertices) {
    envelope.Add(Point{v.position.x, v.position.y});
  }
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromCenterAndDimensions({0, 3}, 2, 2))));
}

TEST_F(GeometrySaveTest,
       SaveAndRevertWithBreakPointFollowedBySelfIntersections) {
  MeshData m1, m2, m3;
  Geometry g1(MakeView(m1));
  Geometry g2(MakeView(m2));
  Geometry g3(MakeView(m3));

  Extrude({&g1, &g2, &g3}, {
                               {
                                   .left = {{-1, -2}, {-1, -1}},
                                   .right = {{1, -2}, {1, -1}},
                                   .simplification_threshold = 0,
                               },
                           });
  g1.AddExtrusionBreak();
  g2.AddExtrusionBreak();
  g3.AddExtrusionBreak();

  // Create two versions of subsequent extrusions:
  std::vector<Extrusion> extrusion_list_a = {
      // Start by continuing to travel upward after the gap.
      {
          .left = {{-1, 0}, {-1, 2}},
          .right = {{1, 0}, {1, 2}},
          .simplification_threshold = 0,
      },
      // Make a counter-clockwise loop with self-intersection.
      {
          .left = {{0, 1}},
          .right = {{1, 3}, {-1, 3}, {-1, -1}, {2, -1}, {2, 1}},
          .simplification_threshold = 0,
      },
      // Extend upward a little bit to give room for the next extrusion.
      {
          .left = {{0, 2}},
          .right = {{2, 2}},
          .simplification_threshold = 0,
      },
      // Make a second loop that is clockwise with self-intersection.
      {
          .left = {{0, 3}, {2, 3}, {2, -1}, {-1, -1}, {-1, 1}},
          .right = {{1, 1}},
          .simplification_threshold = 0,
      },
  };
  // This time, make a clockwise loop first followed by a counter-clockwise loop
  // to swap the order of left and right vertices in the mesh as well as the
  // order of intersection handling.
  std::vector<Extrusion> extrusion_list_b = {
      {
          .left = {{-1, 0}, {-1, 2}},
          .right = {{1, 0}, {1, 2}},
          .simplification_threshold = 0,
      },
      {
          .left = {{-1, 3}, {1, 3}, {1, -1}, {-2, -1}, {-2, 1}},
          .right = {{0, 1}},
          .simplification_threshold = 0,
      },
      {
          .left = {{-2, 2}},
          .right = {{0, 2}},
          .simplification_threshold = 0,
      },
      {
          .left = {{-1, 1}},
          .right = {{2, 3}, {0, 3}, {0, -1}, {1, -1}, {1, 1}},
          .simplification_threshold = 0,
      },
  };

  // Leave the third state as-is for later comparison and extrude the first and
  // second geometries with list A.
  Extrude({&g1, &g2}, extrusion_list_a);

  // Set a save point on the first state and leave the second state unchanged
  // from this point on. We will use the second state for comparison later.
  g1.SetSavePoint();

  // The states for 1 and 3 should be the same after clearing everything since
  // the last extrusion break:
  g1.ClearSinceLastExtrusionBreak();
  EXPECT_THAT(g1.LeftSide(), SideEq(g3.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g3.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m3));

  // Repeat the extrusion and clearing using list A to make sure multiple clears
  // continue to work.
  Extrude(&g1, extrusion_list_a);
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
  g1.ClearSinceLastExtrusionBreak();
  EXPECT_THAT(g1.LeftSide(), SideEq(g3.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g3.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m3));

  // Next, extrude states 1 and 3 together with list B and check that they are
  // still equivalent.
  Extrude({&g1, &g3}, extrusion_list_b);
  EXPECT_THAT(g1.LeftSide(), SideEq(g3.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g3.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m3));

  g1.RevertToSavePoint();

  // Now the states for 1 and 2 should be the same again:
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));

  // Set a save point again so that we can check interaction of save and
  // extrusion break clearing on the same object multiple times.
  g1.SetSavePoint();

  // Clear since the extrusion break and redo the `extrusions` on g1 to make
  // sure 1 and 3 are equivalent once again:
  g1.ClearSinceLastExtrusionBreak();
  Extrude(&g1, extrusion_list_b);
  EXPECT_THAT(g1.LeftSide(), SideEq(g3.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g3.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m3));

  // Lastly, revert to the most recent save so that 1 and 2 should be the same:
  g1.RevertToSavePoint();
  EXPECT_THAT(g1.LeftSide(), SideEq(g2.LeftSide()));
  EXPECT_THAT(g1.RightSide(), SideEq(g2.RightSide()));
  EXPECT_THAT(m1, VerticesAndIndicesEq(m2));
}

}  // namespace
}  // namespace brush_tip_extruder_internal
}  // namespace ink
