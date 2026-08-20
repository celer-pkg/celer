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

#include "ink/strokes/internal/brush_tip_extruder.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_shape_update.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::CalculateEnvelope;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Optional;

constexpr float kBrushEpsilon = 0.05;

BrushTipState MakeSquareTipState(Point position, float side_length) {
  return {.position = position,
          .width = side_length,
          .height = side_length,
          .percent_radius = 0};
}

BrushTipState MakeCircularTipState(Point position, float radius) {
  return {.position = position,
          .width = 2 * radius,
          .height = 2 * radius,
          .percent_radius = 1};
}

std::vector<BrushTipState> MakeUniformCircularTipStates(
    absl::Span<const Point> positions, float radius) {
  std::vector<BrushTipState> result;
  for (Point position : positions) {
    result.push_back(MakeCircularTipState(position, radius));
  }
  return result;
}

class BrushTipExtruderTest : public ::testing::Test {
 protected:
  // Storage for extruded geometry. Note that in this test we are generally not
  // inspecting the precise values of triangle indices and vertices as these are
  // covered in unit tests by `brush_tip_extruder_internal_test.cc` and
  // `brush_tip_shape_test.cc`.
  MutableMesh mesh_ = MutableMesh(StrokeVertex::FullMeshFormat());
};

TEST_F(BrushTipExtruderTest, StartStrokeEmptiesMeshBoundsAndOutline) {
  mesh_.Resize(15, 7);

  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_TRUE(extruder.GetBounds().IsEmpty());
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), IsEmpty());
}

TEST_F(BrushTipExtruderTest, ExtendNewStrokeWithEmptyStates) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke({}, {});

  EXPECT_THAT(update.region.AsRect(), Eq(std::nullopt));
  EXPECT_THAT(update.first_index_offset, Eq(std::nullopt));
  EXPECT_THAT(update.first_vertex_offset, Eq(std::nullopt));
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_TRUE(extruder.GetBounds().IsEmpty());
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), IsEmpty());
}

TEST_F(BrushTipExtruderTest, ExtendNewStrokeWithSingleFixedState) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update =
      extruder.ExtendStroke({MakeCircularTipState({0, 0}, 1)}, {});

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {1, 1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(),
              ElementsAreArray({8, 6, 4, 2, 1, 0, 3, 5, 7, 9}));
}

TEST_F(BrushTipExtruderTest, ExtendNewStrokeWithSingleVolatileState) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update =
      extruder.ExtendStroke({}, {MakeCircularTipState({0, 0}, 1)});

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {1, 1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(),
              ElementsAreArray({8, 6, 4, 2, 1, 0, 3, 5, 7, 9}));
}

TEST_F(BrushTipExtruderTest, ExtendNewStrokeWithMultipleFixedStates) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {1, 0}, {2, 1}}, 1), {});

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {3, 2}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(
      extruder.GetOutlines()[0].GetIndices(),
      ElementsAreArray({14, 12, 10, 9, 8, 6, 4, 2, 1, 0, 3, 5, 7, 11, 13, 15}));
}

TEST_F(BrushTipExtruderTest, ExtendNewStrokeWithMultipleVolatileStates) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {-1, 0}, {-2, 1}}, 1), {});

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-3, 2}, {1, -1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(
      extruder.GetOutlines()[0].GetIndices(),
      ElementsAreArray({14, 12, 10, 6, 4, 2, 1, 0, 3, 5, 7, 8, 9, 11, 13, 15}));
}

TEST_F(BrushTipExtruderTest, ExtendSingleFixedStateStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update =
      extruder.ExtendStroke({MakeCircularTipState({0, 0}, 1)}, {});

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {1, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update = extruder.ExtendStroke({MakeCircularTipState({-1, 0}, 1)}, {});

  // All of the geometry from a single state should be "volatile", so the update
  // from the second state should cover the entire stroke area and all of the
  // mesh data.
  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-2, -1}, {1, 1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));
}

TEST_F(BrushTipExtruderTest, ExtendSingleVolatileStateStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update =
      extruder.ExtendStroke({}, {MakeCircularTipState({0, 0}, 1)});

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {1, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update = extruder.ExtendStroke({MakeCircularTipState({-1, 0}, 1)}, {});

  // All of the geometry from a single state should be "volatile", so the update
  // from the second state should cover the entire stroke area and all of the
  // mesh data.
  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-2, -1}, {1, 1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));
}

TEST_F(BrushTipExtruderTest, ExtendManyFixedStateStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{-2, 0}, {2, 2}, {-1, 2}, {2, 0}}, 1), {});

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-3, -1}, {3, 3}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  ASSERT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update =
      extruder.ExtendStroke(MakeUniformCircularTipStates({{3, 0}, {4, 3}}, 1),
                            {MakeCircularTipState({5, 5}, 1)});

  // Most of the geometry for the previous extension is fixed, so the updated
  // region should be a subset of the stroke area, and not all of the mesh
  // vertices and indices should be in the update.
  EXPECT_THAT(update.region,
              EnvelopeNear(Rect::FromTwoPoints({-2, -1}, {6, 6}), 0.06));
  EXPECT_THAT(update.first_index_offset, Optional(Gt(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Gt(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));
}

TEST_F(BrushTipExtruderTest, ExtendCoversRemovedPreviousVolatileGeometry) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  // Make a stroke that first has volatile states extending in the positive x
  // direction, and then turns toward the positive y, which simulates an input
  // misprediction.

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {1, 0}, {2, 0}}, 1),
      MakeUniformCircularTipStates({{3, 0}, {4, 0}, {5, 0}}, 1));

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {6, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  ASSERT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update =
      extruder.ExtendStroke(MakeUniformCircularTipStates({{3, 1}, {3, 2}}, 1),
                            MakeUniformCircularTipStates({{3, 3}, {3, 4}}, 1));

  // The returned updated region should include the older volatile geometry that
  // has been cleared. Note the x lower bound of the rect stretches back to the
  // last positions in the startcap because of self-intersection handling.
  EXPECT_THAT(update.region,
              EnvelopeNear(Rect::FromTwoPoints({-0.6, -1}, {6, 5}), 0.05));
  EXPECT_THAT(update.first_index_offset, Optional(Gt(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Gt(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  // The vertices themselves should no longer include the positions of volatile
  // geometry.
  EXPECT_THAT(CalculateEnvelope(mesh_).AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {4, 5}), 0.06)));
}

TEST_F(BrushTipExtruderTest, EmptyExtendRemovesPreviousVolatileGeometry) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {1, 0}, {2, 0}}, 1),
      MakeUniformCircularTipStates({{3, 0}, {4, 0}, {5, 0}}, 1));

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {6, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  ASSERT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update = extruder.ExtendStroke({}, {});

  // The returned updated region should include the older volatile geometry that
  // has been cleared.
  EXPECT_THAT(update.region,
              EnvelopeNear(Rect::FromTwoPoints({0, -1}, {6, 1}), 0.06));
  EXPECT_THAT(update.first_index_offset, Optional(Gt(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Gt(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  // The vertices themselves should no longer include the positions of volatile
  // geometry.
  EXPECT_THAT(CalculateEnvelope(mesh_).AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {3, 1}), 0.06)));
}

TEST_F(BrushTipExtruderTest, EmptyExtendClearsCompletelyVolatileStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      {}, MakeUniformCircularTipStates({{3, 0}, {4, 0}, {5, 0}}, 1));

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({2, -1}, {6, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  ASSERT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));

  update = extruder.ExtendStroke({}, {});
  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({2, -1}, {6, 1}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_TRUE(extruder.GetBounds().IsEmpty());
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), IsEmpty());
}

TEST_F(BrushTipExtruderTest, StartSecondStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  StrokeShapeUpdate update = extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {1, 0}, {2, 0}}, 1),
      MakeUniformCircularTipStates({{3, 0}, {4, 0}, {5, 0}}, 1));

  ASSERT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({-1, -1}, {6, 1}), 0.06)));
  ASSERT_THAT(update.first_index_offset, Optional(Eq(0)));
  ASSERT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  ASSERT_NE(mesh_.VertexCount(), 0);
  ASSERT_NE(mesh_.TriangleCount(), 0);
  ASSERT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(),
              ElementsAreArray({12, 10, 8, 6, 4, 2, 1, 0, 3, 5, 7, 9, 11, 13}));

  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);

  // Starting a new stroke should clear the geometry
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_EQ(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetOutlines().size(), Eq(1));
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), IsEmpty());

  update = extruder.ExtendStroke({MakeCircularTipState({10, 10}, 1)}, {});

  // The updated region should not include any of the last stroke.
  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({9, 9}, {11, 11}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
  EXPECT_THAT(extruder.GetBounds().AsRect(),
              Optional(RectNear(*CalculateEnvelope(mesh_).AsRect(), 0.0001)));
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_THAT(extruder.GetOutlines()[0].GetIndices(), Not(IsEmpty()));
}

TEST_F(BrushTipExtruderTest, WidthAndHeightLessThanEpsilonCreatesBreakPoint) {
  float tip_radius = kBrushEpsilon * 0.4;

  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  StrokeShapeUpdate update =
      extruder.ExtendStroke({MakeCircularTipState({10, 10}, tip_radius)}, {});

  // There should be no update or geometry added since the tip dimensions are
  // less than the epsilon.
  EXPECT_TRUE(update.region.IsEmpty());
  EXPECT_THAT(update.first_index_offset, Eq(std::nullopt));
  EXPECT_THAT(update.first_vertex_offset, Eq(std::nullopt));
  EXPECT_EQ(mesh_.VertexCount(), 0);
  EXPECT_EQ(mesh_.TriangleCount(), 0);
}

TEST_F(BrushTipExtruderTest, OneDimensionLessThanEpsilonIsNotABreakPoint) {
  float brush_epsilon = 0.1;

  BrushTipExtruder extruder;
  extruder.StartStroke(brush_epsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  StrokeShapeUpdate update = extruder.ExtendStroke(
      {
          {.position = {0, 0}, .width = 0.09, .height = 1},
          {.position = {1, 0}, .width = 0.5, .height = 0.09},
      },
      {});

  // The update and geometry should not be empty, because only one dimension of
  // each tip state was less than epsilon.
  EXPECT_FALSE(update.region.IsEmpty());
  EXPECT_THAT(update.first_index_offset, Optional(Eq(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(0)));
  EXPECT_NE(mesh_.VertexCount(), 0);
  EXPECT_NE(mesh_.TriangleCount(), 0);
}

TEST_F(BrushTipExtruderTest, AddBreakPointsToNonEmptyStroke) {
  BrushTipExtruder extruder;
  extruder.StartStroke(/* brush_epsilon = */ 0.06,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      MakeUniformCircularTipStates({{0, 0}, {1, 0}, {2, 1}}, 1), {});

  BrushTipState break_point = {.width = 0, .height = 0};

  // Adding the first break-point should cause a non-empty update that covers
  // the "end" of the current stroke segment.
  StrokeShapeUpdate update = extruder.ExtendStroke({break_point}, {});
  EXPECT_THAT(update.region,
              EnvelopeNear(Rect::FromTwoPoints({0, -1}, {3, 2}), 0.06));
  EXPECT_THAT(update.first_index_offset, Optional(Gt(0)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Gt(0)));

  // A second break-point in a row should be a no-op.
  update = extruder.ExtendStroke({}, {break_point});
  EXPECT_TRUE(update.region.IsEmpty());
  EXPECT_EQ(update.first_index_offset, std::nullopt);
  EXPECT_EQ(update.first_vertex_offset, std::nullopt);

  uint32_t last_index_count = 3 * mesh_.TriangleCount();
  uint32_t last_vertex_count = mesh_.VertexCount();

  // A new non-break-point extrusion should be disconnected from the segment
  // that prececed the break-point.
  update = extruder.ExtendStroke({MakeCircularTipState({10, 10}, 1)}, {});
  EXPECT_THAT(update.region.AsRect(),
              Optional(RectNear(Rect::FromTwoPoints({9, 9}, {11, 11}), 0.06)));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(last_index_count)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(last_vertex_count)));
}

TEST_F(BrushTipExtruderTest, RejectTipStateContainedInPrevious) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      {MakeCircularTipState({0, 0}, 10), MakeCircularTipState({5, 0}, 10)}, {});
  uint32_t n_verts = mesh_.VertexCount();
  uint32_t n_tris = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  uint32_t n_outline_indices = extruder.GetOutlines()[0].GetIndices().size();

  extruder.ExtendStroke({MakeCircularTipState({10, 0}, 1)}, {});

  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(), n_outline_indices);
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-10, -10}, {15, 10})));
}

TEST_F(BrushTipExtruderTest, DontRejectTipStateAfterBreakPoint) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      {MakeCircularTipState({0, 0}, 10), MakeCircularTipState({5, 0}, 10),
       MakeCircularTipState({15, 0}, 0.1 * kBrushEpsilon)},
      {});
  uint32_t n_verts = mesh_.VertexCount();
  uint32_t n_tris = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  uint32_t n_outline_indices_before_break =
      extruder.GetOutlines()[0].GetIndices().size();

  extruder.ExtendStroke({MakeCircularTipState({20, 0}, 1)}, {});

  EXPECT_GT(mesh_.VertexCount(), n_verts);
  EXPECT_GT(mesh_.TriangleCount(), n_tris);
  ASSERT_EQ(extruder.GetOutlines().size(), 2);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(),
            n_outline_indices_before_break);
  EXPECT_GT(extruder.GetOutlines()[1].GetIndices().size(), 0);
  EXPECT_THAT(
      extruder.GetBounds(),
      EnvelopeNear(Rect::FromTwoPoints({-10, -10}, {21, 10}), kBrushEpsilon));
}

TEST_F(BrushTipExtruderTest,
       StartNewPartitionIfCurrentPartitionIsPartiallyContained) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  // We stagger the extrusions vertically so that vertices don't get simplified
  // away.
  extruder.ExtendStroke(
      {MakeSquareTipState({0, 0}, 0.5), MakeSquareTipState({1, 0.2}, 0.5),
       MakeSquareTipState({2, 0}, 0.5), MakeSquareTipState({3, 0.2}, 0.5),
       MakeSquareTipState({4, 0}, 0.5)},
      {});
  uint32_t n_verts = mesh_.VertexCount();
  uint32_t n_tris = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  uint32_t n_outline_indices = extruder.GetOutlines()[0].GetIndices().size();

  extruder.ExtendStroke({MakeSquareTipState({5, 0}, 8)}, {});

  // The last extrusion should have started a new partition, with just the
  // square in it.
  EXPECT_EQ(mesh_.VertexCount(), n_verts + 4);
  EXPECT_EQ(mesh_.TriangleCount(), n_tris + 2);
  ASSERT_EQ(extruder.GetOutlines().size(), 2);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(), n_outline_indices);
  EXPECT_EQ(extruder.GetOutlines()[1].GetIndices().size(), 4);
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-0.25, -4}, {9, 4})));
}

TEST_F(BrushTipExtruderTest,
       RestartPartitionIfCurrentPartitionIsFullyContained) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  // We stagger the extrusions vertically so that vertices don't get simplified
  // away.
  extruder.ExtendStroke(
      {MakeSquareTipState({0, 0}, 0.5), MakeSquareTipState({1, 0.2}, 0.5),
       MakeSquareTipState({2, 0}, 0.5), MakeSquareTipState({3, 0.2}, 0)},
      {});
  uint32_t n_verts_before_break = mesh_.VertexCount();
  uint32_t n_tris_before_break = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  uint32_t n_outline_indices_before_break =
      extruder.GetOutlines()[0].GetIndices().size();
  extruder.ExtendStroke(
      {MakeSquareTipState({4, 0}, 0.5), MakeSquareTipState({5, 0.2}, 0.5)}, {});
  uint32_t n_verts_after_break = mesh_.VertexCount();
  uint32_t n_tris_after_break = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 2);
  EXPECT_EQ(extruder.GetOutlines()[1].GetIndices().size(), 6);

  extruder.ExtendStroke({MakeSquareTipState({6, 0}, 5)}, {});

  // The last extrusion should have cleared and restarted the current partition,
  // with just the square in it.
  EXPECT_EQ(mesh_.VertexCount(), n_verts_before_break + 4);
  EXPECT_EQ(mesh_.TriangleCount(), n_tris_before_break + 2);
  ASSERT_EQ(extruder.GetOutlines().size(), 2);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(),
            n_outline_indices_before_break);
  EXPECT_EQ(extruder.GetOutlines()[1].GetIndices().size(), 4);

  EXPECT_NE(mesh_.VertexCount(), n_verts_after_break);
  EXPECT_NE(mesh_.TriangleCount(), n_tris_after_break);
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-0.25, -2.5}, {8.5, 2.5})));
}

TEST_F(BrushTipExtruderTest, RestartPartitionIfWholeStrokeIsContained) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  // We stagger the extrusions vertically so that vertices don't get simplified
  // away.
  extruder.ExtendStroke(
      {MakeSquareTipState({0, 0}, 0.5), MakeSquareTipState({1, 0.2}, 0.5),
       MakeSquareTipState({2, 0}, 0.5)},
      {});

  extruder.ExtendStroke({MakeSquareTipState({3, 0}, 8)}, {});

  // The last extrusion should have cleared and restarted partition, which was
  // the whole stroke so far; it should be just a single square now.
  EXPECT_EQ(mesh_.VertexCount(), 4);
  EXPECT_EQ(mesh_.TriangleCount(), 2);
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(), 4);
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-1, -4}, {7, 4})));
}

TEST_F(BrushTipExtruderTest, RejectTipStateIfWeCannotConstrainIt) {
  // Case 1) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red), and the same is true for all intermediate shapes,
  // because the red and blue shapes have the same center, and any rotation
  // cause opposite corners to leave the previous shape (blue). So we discard
  // the proposed shape (red).
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke({{.position = {0, 0}, .width = 1, .height = 3},
                         {.position = {2, 0}, .width = 1, .height = 3}},
                        {});
  uint32_t n_verts = mesh_.VertexCount();
  uint32_t n_tris = mesh_.TriangleCount();
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  uint32_t n_outline_indices = extruder.GetOutlines()[0].GetIndices().size();
  auto expected_bounds = Rect::FromTwoPoints({-0.5, -1.5}, {2.5, 1.5});
  ASSERT_THAT(extruder.GetBounds(), EnvelopeEq(expected_bounds));

  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {2, 0},
                                 .width = 1,
                                 .height = 3,
                                 .rotation = Angle::Degrees(90)}},
      /* volatile_states = */ {});

  EXPECT_EQ(mesh_.VertexCount(), n_verts);
  EXPECT_EQ(mesh_.TriangleCount(), n_tris);
  ASSERT_EQ(extruder.GetOutlines().size(), 1);
  EXPECT_EQ(extruder.GetOutlines()[0].GetIndices().size(), n_outline_indices);
  EXPECT_THAT(extruder.GetBounds(), EnvelopeEq(expected_bounds));
}

TEST_F(BrushTipExtruderTest, ConstrainTipStateIfItDoesNotHaveGoodTangents) {
  // Case 2) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red), so we find an intermediate shape (green) that we can
  // construct tangents to, and use that instead of the proposed shape (red).
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 1, .height = 4},
                                {.position = {2, 0}, .width = 1, .height = 4}},
      /* volatile_states = */ {});

  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {3, 0}, .width = 4, .height = 1}},
      /* volatile_states = */ {});

  // If the tip was not constrained, this would be (-0.5, -2)->(5, 2).
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeNear(Rect::FromTwoPoints({-0.5, -2}, {4.5, 2}), 0.005));
}

TEST_F(BrushTipExtruderTest,
       RejectNonFinalFixedTipStateIfConstrainedStateIsTooSimilar) {
  // Case 3) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red); we can construct an intermediate shape (green) that has
  // good tangents, but it's very close to the intermediate shape at
  // `lerp_amount` = 0 (magenta). Since we have another shape after it (teal),
  // we reject the proposed shape (red), discarding both it and the intermediate
  // shape (green).
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 2, .height = 15}},
      {});

  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0.5, 0.25},
                                 .width = 2,
                                 .height = 15,
                                 .rotation = Angle::Degrees(60)},
                                {.position = {2, 0}, .width = 2, .height = 15}},
      /* volatile_states = */ {});

  // If the middle tip was not rejected, this would be (-1, -7.5)->(3, 7.799).
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-1, -7.5}, {3, 7.5})));
}

TEST_F(BrushTipExtruderTest, DontRejectFinalFixedTipStateIfNoVolatileState) {
  // Case 4) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red); we can construct an intermediate shape (green) that has
  // good tangents. It's very close to the intermediate shape at `lerp_amount` =
  // 0 (magenta), but since we have no shapes after it, we keep it anyway, using
  // it instead of the proposed shape (red), to avoid lagging behind the stylus.
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 2, .height = 15}},
      /* volatile_states = */ {});

  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {2, 0}, .width = 2, .height = 15},
                                {.position = {2.5, 0.25},
                                 .width = 2,
                                 .height = 15,
                                 .rotation = Angle::Degrees(60)}},
      /* volatile_states = */ {});

  // If the final tip was rejected, this would be (-1, -7.5)->(3, 7.5).
  EXPECT_THAT(
      extruder.GetBounds(),
      EnvelopeNear(Rect::FromTwoPoints({-1, -7.5}, {3.988, 7.799}), 0.005));
}

TEST_F(
    BrushTipExtruderTest,
    // NOLINTNEXTLINE(whitespace/line_length)
    RejectFinalFixedTipStateIfConstrainedStateIsTooSimilarAndThereAreVolatileStates) {
  // Case 5) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red); we can construct an intermediate shape (green) that has
  // good tangents, but it's very close to the intermediate shape at
  // `lerp_amount` = 0 (magenta). Since we have another shape after it (teal),
  // we reject the proposed state (red) and discard the intermediate shape
  // (green), even though the subsequent shape (teal) is a volatile state.
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 2, .height = 15}},
      /* volatile_states = */ {});

  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {2, 0}, .width = 2, .height = 15},
                                {.position = {2.5, 0.25},
                                 .width = 2,
                                 .height = 15,
                                 .rotation = Angle::Degrees(60)}},
      /* volatile_states = */ {{.position = {6, 0}, .width = 2, .height = 2}});

  // If the final fixed tip was not rejected, this would be
  // (-1, -7.5)->(7, 7.799).
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeNear(Rect::FromTwoPoints({-1, -7.5}, {7, 7.5}), 0.005));
}

TEST_F(BrushTipExtruderTest,
       RejectNonFinalVolatileTipStateIfConstrainedStateDoesNotLerpEnough) {
  // Case 3) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red); we can construct an intermediate shape (green) that has
  // good tangents, but it's very close to the intermediate shape at
  // `lerp_amount` = 0 (magenta). Since we have another shape after it (teal),
  // we reject the proposed shape (red), discarding both it and the intermediate
  // shape (green).
  // This is identical to the test
  // `RejectNonFinalFixedTipStateIfConstrainedStateIsTooSimilar` above, save
  // that only the first shape (blue) is a fixed state.
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 2, .height = 15}},
      /* volatile_states = */ {});

  extruder.ExtendStroke(/* new_fixed_states = */ {}, /* volatile_states = */ {
                            {.position = {0.5, 0.25},
                             .width = 2,
                             .height = 15,
                             .rotation = Angle::Degrees(60)},
                            {.position = {2, 0}, .width = 2, .height = 15}});

  // If the middle tip was not rejected, this would be (-1, -7.5)->(3, 7.799).
  EXPECT_THAT(extruder.GetBounds(),
              EnvelopeEq(Rect::FromTwoPoints({-1, -7.5}, {3, 7.5})));
}

TEST_F(BrushTipExtruderTest,
       DontRejectFinalVolatileTipStateIfConstrainedStateDoesNotLerpEnough) {
  // Case 4) in brush_tip_extruder_test_cases.svg.
  // We can't construct tangents between the previous shape (blue) and the
  // proposed one (red); we can construct an intermediate shape (green) that has
  // good tangents. It's very close to the intermediate shape at `lerp_amount` =
  // 0 (magenta), but since we have no shapes after it, we keep it anyway, using
  // it instead of the proposed shape (red), to avoid lagging behind the stylus.
  // This is identical to the test
  // `DontRejectFinalFixedTipStateIfNoVolatileState` above, save that only the
  // first two shapes (black and blue) are fixed states.
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0}, .width = 2, .height = 15}},
      /* volatile_states = */ {});

  extruder.ExtendStroke(/* new_fixed_states = */ {}, /* volatile_states = */ {
                            {.position = {2, 0}, .width = 2, .height = 15},
                            {.position = {2.5, 0.25},
                             .width = 2,
                             .height = 15,
                             .rotation = Angle::Degrees(60)}});

  // If the final tip was rejected, this would be (-1, -7.5)->(3, 7.5).
  EXPECT_THAT(
      extruder.GetBounds(),
      EnvelopeNear(Rect::FromTwoPoints({-1, -7.5}, {3.988, 7.799}), 0.005));
}

TEST(BrushTipExtruderDeathTest, ExtendWithoutStart) {
  BrushTipExtruder extruder;
  EXPECT_DEATH_IF_SUPPORTED(extruder.ExtendStroke({}, {}), "");
}

TEST(BrushTipExtruderDeathTest, ZeroBrushEpsilon) {
  BrushTipExtruder extruder;
  MutableMesh mesh;
  EXPECT_DEATH_IF_SUPPORTED(
      extruder.StartStroke(/* brush_epsilon = */ 0,
                           /* is_winding_texture_particle_brush = */ false,
                           mesh),
      "");
}

TEST_F(BrushTipExtruderTest, TextureUVsAreSetForWindingTextureParticles) {
  constexpr float kTol = 1e-5;

  BrushTipExtruder extruder;
  // We choose a larger value for brush_epsilon so that the rounded corners are
  // reduced to line segments, giving us (irregular) octagons for each particle.
  extruder.StartStroke(/* brush_epsilon = */ 1,
                       /* is_winding_texture_particle_brush = */ true, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0},
                                 .width = 10,
                                 .height = 10,
                                 .percent_radius = 0.4}},
      /* volatile_states = */ {});

  ASSERT_EQ(mesh_.VertexCount(), 8);
  EXPECT_THAT(mesh_.VertexPosition(0), PointEq({-5, 3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 0),
              PointNear({0, 0.8}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(1), PointEq({-5, -3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 1),
              PointNear({0, 0.2}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(2), PointEq({-3, -5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 2),
              PointNear({0.2, 0}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(3), PointEq({-3, 5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 3),
              PointNear({0.2, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(4), PointEq({3, -5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 4),
              PointNear({0.8, 0}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(5), PointEq({3, 5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 5),
              PointNear({0.8, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(6), PointEq({5, -3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 6),
              PointNear({1, 0.2}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(7), PointEq({5, 3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 7),
              PointNear({1, 0.8}, kTol));

  // We need to extrude a zero-area tip state between the two particles -- this
  // is the break-point that indicates that the extrusions are separate.
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {20, 0}, .width = 0, .height = 0},
                                {.position = {20, 0},
                                 .width = 10,
                                 .height = 10,
                                 .percent_radius = 0.4}},
      /* volatile_states = */ {});

  ASSERT_EQ(mesh_.VertexCount(), 16);
  EXPECT_THAT(mesh_.VertexPosition(8), PointEq({15, 3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 8),
              PointNear({0, 0.8}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(9), PointEq({15, -3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 9),
              PointNear({0, 0.2}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(10), PointEq({17, -5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 10),
              PointNear({0.2, 0}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(11), PointEq({17, 5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 11),
              PointNear({0.2, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(12), PointEq({23, -5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 12),
              PointNear({0.8, 0}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(13), PointEq({23, 5}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 13),
              PointNear({0.8, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(14), PointEq({25, -3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 14),
              PointNear({1, 0.2}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(15), PointEq({25, 3}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 15),
              PointNear({1, 0.8}, kTol));
}

TEST_F(BrushTipExtruderTest, TextureUVsAreNotSetForNonWindingTextureParticles) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ false, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {0, 0},
                                 .width = 10,
                                 .height = 10,
                                 .percent_radius = 0.4}},
      /* volatile_states = */ {});

  for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
    // Texture surface UV defaults to (0, 0) if not needed.
    EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, i), PointEq({0, 0}));
  }
}

TEST_F(BrushTipExtruderTest, WindingTextureParticleUVsAreClamped) {
  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ true, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {1669.30981, 761.19311},
                                 .width = 109.525535,
                                 .height = 109.525535,
                                 .percent_radius = 0}},
      /* volatile_states = */ {});

  for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
    // Texture surface UV defaults to (0, 0) if not needed.
    Point uv = StrokeVertex::GetSurfaceUvFromMesh(mesh_, i);
    EXPECT_GE(uv.x, 0);
    EXPECT_LE(uv.x, 1);
    EXPECT_GE(uv.y, 0);
    EXPECT_LE(uv.y, 1);
  }
}

TEST_F(BrushTipExtruderTest, TextureUVsFollowTipRotation) {
  constexpr float kTol = 1e-5;

  BrushTipExtruder extruder;
  extruder.StartStroke(kBrushEpsilon,
                       /* is_winding_texture_particle_brush = */ true, mesh_);
  extruder.ExtendStroke(
      /* new_fixed_states = */ {{.position = {5, 5},
                                 .width = 2,
                                 .height = 2,
                                 .percent_radius = 0,
                                 .rotation = kQuarterTurn}},
      /* volatile_states = */ {});

  ASSERT_EQ(mesh_.VertexCount(), 4);
  EXPECT_THAT(mesh_.VertexPosition(0), PointEq({4, 6}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 0),
              PointNear({1, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(1), PointEq({4, 4}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 1),
              PointNear({0, 1}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(2), PointEq({6, 4}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 2),
              PointNear({0, 0}, kTol));
  EXPECT_THAT(mesh_.VertexPosition(3), PointEq({6, 6}));
  EXPECT_THAT(StrokeVertex::GetSurfaceUvFromMesh(mesh_, 3),
              PointNear({1, 0}, kTol));
}

}  // namespace
}  // namespace ink::strokes_internal
