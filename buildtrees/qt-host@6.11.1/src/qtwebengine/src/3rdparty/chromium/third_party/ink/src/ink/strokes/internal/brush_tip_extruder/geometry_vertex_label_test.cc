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

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::BrushTipState;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::Each;
using ::testing::ElementsAreArray;
using ::testing::Eq;

class GeometryVertexLabelTest : public ::testing::Test {
 protected:
  GeometryVertexLabelTest() : mesh_(StrokeVertex::FullMeshFormat()) {}

  void SetUp() override { geometry_.Reset(MutableMeshView(mesh_)); }

  std::vector<StrokeVertex::SideCategory> GetSideLabelCategories(
      const Side& side) const {
    std::vector<StrokeVertex::SideCategory> categories;
    for (uint32_t index : side.indices) {
      categories.push_back(StrokeVertex::Label{
          .encoded_value = mesh_.FloatVertexAttribute(
              index, StrokeVertex::kFullFormatAttributeIndices.side_label)[0]}
                               .DecodeSideCategory());
    }
    return categories;
  }

  std::vector<StrokeVertex::ForwardCategory> GetForwardLabelCategories(
      const Side& side) const {
    std::vector<StrokeVertex::ForwardCategory> categories;
    for (uint32_t index : side.indices) {
      categories.push_back(StrokeVertex::Label{
          .encoded_value = mesh_.FloatVertexAttribute(
              index,
              StrokeVertex::kFullFormatAttributeIndices.forward_label)[0]}
                               .DecodeForwardCategory());
    }
    return categories;
  }

  MutableMesh mesh_;
  Geometry geometry_;
};

TEST_F(GeometryVertexLabelTest, WithoutSelfIntersections) {
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 1});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendRightVertex(Point{1, 0});
  geometry_.AppendRightVertex(Point{1, 1});
  geometry_.AppendRightVertex(Point{1, 2});
  geometry_.AppendRightVertex(Point{1, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  // Without self-intersections, all left and right side vertices should be
  // labeled as exterior on their respective sides.
  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorLeft)));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));

  // At the start of the stroke, the first vertex on each side should be labeled
  // as forward-exterior, with the rest of the vertices being unlabeled.
  EXPECT_THAT(GetForwardLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
              }));
  EXPECT_THAT(GetForwardLabelCategories(geometry_.RightSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
              }));

  geometry_.ResetMutationTracking();
  geometry_.AddExtrusionBreak();
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({0, 1}, {1, 3})));

  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorLeft)));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));

  // Adding an extrusion break should have set the last left and right vertex as
  // forward-exterior.
  EXPECT_THAT(GetForwardLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
              }));
  EXPECT_THAT(GetForwardLabelCategories(geometry_.RightSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
              }));

  // Repeat with more vertices to verify behavior after the extrusion break.

  geometry_.AppendLeftVertex(Point{5, 5});
  geometry_.AppendLeftVertex(Point{5, 6});
  geometry_.AppendLeftVertex(Point{5, 7});
  geometry_.AppendRightVertex(Point{6, 5});
  geometry_.AppendRightVertex(Point{6, 6});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorLeft)));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));

  EXPECT_THAT(GetForwardLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
              }));
  EXPECT_THAT(GetForwardLabelCategories(geometry_.RightSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
              }));

  geometry_.ResetMutationTracking();
  geometry_.AddExtrusionBreak();
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({5, 5}, {6, 7})));

  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorLeft)));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));

  EXPECT_THAT(GetForwardLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
              }));
  EXPECT_THAT(GetForwardLabelCategories(geometry_.RightSide()),
              ElementsAreArray({
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kInterior,
                  StrokeVertex::ForwardCategory::kExteriorBack,
                  StrokeVertex::ForwardCategory::kExteriorFront,
                  StrokeVertex::ForwardCategory::kExteriorBack,
              }));
}

TEST_F(GeometryVertexLabelTest, WithSelfIntersection) {
  // Start a mesh that travels straight in the positive y direction.
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendLeftVertex(Point{0, 4});
  geometry_.AppendRightVertex(Point{2, 0});
  geometry_.AppendRightVertex(Point{2, 2});
  geometry_.AppendRightVertex(Point{2, 4});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  // Make a left turn that starts self-intersection handling.
  geometry_.AppendLeftVertex(Point{1, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  geometry_.AppendLeftVertex(Point{0.75, 3});
  geometry_.AppendRightVertex(Point{0.75, 5});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  ASSERT_TRUE(geometry_.LeftSide().intersection.has_value());
  ASSERT_TRUE(geometry_.LeftSide().intersection->retriangulation_started);

  // Current expected mesh:
  //
  //            R
  //            | \     <---  travel
  //         L-L|  R       |  direction
  //         | \|/ |       |
  //         L--I--R       |
  //         | / \ |
  //         L-----R
  //
  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kInterior,
              }));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));

  // Continue the turn and travel into the negative y direction until the
  // intersection reaches the reposition limit.

  geometry_.AppendRightVertex(Point{0, 5});
  geometry_.AppendRightVertex(Point{-1.25, 4});
  geometry_.AppendRightVertex(Point{-1.25, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  geometry_.AppendLeftVertex(Point{0.75, 1});
  geometry_.AppendRightVertex(Point{-1.25, 1});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  ASSERT_FALSE(geometry_.LeftSide().intersection.has_value());

  // Generated points (instead of current mesh for better clarity):
  //
  //         R--R
  //       /      \     ----  travel
  //      R  L     R    |  |  direction
  //      |  |\    |   \/  |
  //      R  L  L  R       |
  //      |  |  |  |
  //      R  |  L  |
  //         L-----R
  //
  EXPECT_THAT(GetSideLabelCategories(geometry_.LeftSide()),
              ElementsAreArray({
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kInterior,
                  StrokeVertex::SideCategory::kExteriorLeft,
                  StrokeVertex::SideCategory::kExteriorLeft,
              }));
  EXPECT_THAT(GetSideLabelCategories(geometry_.RightSide()),
              Each(Eq(StrokeVertex::SideCategory::kExteriorRight)));
  // TODO: b/290231022 - Check the values of "forward" labels once they are
  // correctly set when giving up self-intersection.
}

TEST_F(GeometryVertexLabelTest,
       AddingBreakPointDuringRetriangulationRemainsForwardInterior) {
  // Start a mesh that travels straight in the positive y direction.
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendLeftVertex(Point{0, 4});
  geometry_.AppendRightVertex(Point{2, 0});
  geometry_.AppendRightVertex(Point{2, 2});
  geometry_.AppendRightVertex(Point{2, 4});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  // Make a left turn that starts self-intersection handling.
  geometry_.AppendLeftVertex(Point{1, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  geometry_.AppendLeftVertex(Point{0.75, 3});
  geometry_.AppendRightVertex(Point{0.75, 5});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  ASSERT_TRUE(geometry_.LeftSide().intersection.has_value());
  ASSERT_TRUE(geometry_.LeftSide().intersection->retriangulation_started);

  geometry_.AddExtrusionBreak();

  EXPECT_EQ(geometry_.GetMeshView()
                .GetForwardLabel(geometry_.LeftSide().indices.back())
                .DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kInterior);
  EXPECT_EQ(geometry_.GetMeshView()
                .GetForwardLabel(geometry_.RightSide().indices.back())
                .DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kInterior);

  // Repeat with a turn to the right:
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendLeftVertex(Point{0, 4});
  geometry_.AppendRightVertex(Point{2, 0});
  geometry_.AppendRightVertex(Point{2, 2});
  geometry_.AppendRightVertex(Point{2, 4});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  // Make a left turn that starts self-intersection handling.
  geometry_.AppendRightVertex(Point{1, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  geometry_.AppendLeftVertex(Point{0.75, 5});
  geometry_.AppendRightVertex(Point{0.75, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{.width = 2, .height = 2});

  ASSERT_TRUE(geometry_.RightSide().intersection.has_value());
  ASSERT_TRUE(geometry_.RightSide().intersection->retriangulation_started);

  geometry_.AddExtrusionBreak();

  EXPECT_EQ(geometry_.GetMeshView()
                .GetForwardLabel(geometry_.LeftSide().indices.back())
                .DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kInterior);
  EXPECT_EQ(geometry_.GetMeshView()
                .GetForwardLabel(geometry_.RightSide().indices.back())
                .DecodeForwardCategory(),
            StrokeVertex::ForwardCategory::kInterior);
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
