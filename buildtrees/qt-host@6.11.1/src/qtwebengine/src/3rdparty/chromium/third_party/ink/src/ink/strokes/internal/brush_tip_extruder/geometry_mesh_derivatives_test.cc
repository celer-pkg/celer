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
#include "absl/log/absl_check.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/types/small_array.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::BrushTipState;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::Each;
using ::testing::Not;

Vec ToVec(const SmallArray<float, 4>& attribute) {
  ABSL_CHECK_EQ(attribute.Size(), 2);
  return {attribute[0], attribute[1]};
}

class GeometryMeshDerivativesTest : public ::testing::Test {
 protected:
  GeometryMeshDerivativesTest()
      : mesh_(StrokeVertex::FullMeshFormat()),
        geometry_(MutableMeshView(mesh_)) {}

  std::vector<Vec> GetDerivativeValues(int derivative_attribute_index) const {
    std::vector<Vec> derivatives;
    for (uint32_t i = 0; i < mesh_.VertexCount(); ++i) {
      derivatives.push_back(
          ToVec(mesh_.FloatVertexAttribute(i, derivative_attribute_index)));
    }
    return derivatives;
  }

  std::vector<Vec> GetSideDerivatives() const {
    return GetDerivativeValues(
        StrokeVertex::kFullFormatAttributeIndices.side_derivative);
  }

  MutableMesh mesh_;
  Geometry geometry_;
};

TEST_F(GeometryMeshDerivativesTest, ContiguousExtrusions) {
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 1});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendRightVertex(Point{1, 0});
  geometry_.AppendRightVertex(Point{1, 1});
  geometry_.AppendRightVertex(Point{1, 2});
  geometry_.AppendRightVertex(Point{1, 3});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  geometry_.UpdateMeshDerivatives();
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));

  geometry_.ResetMutationTracking();
  geometry_.AppendLeftVertex(Point{1, 4});
  geometry_.AppendLeftVertex(Point{2, 6});
  geometry_.AppendRightVertex(Point{2, 4});
  geometry_.AppendRightVertex(Point{3, 6});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  // Without updating the mesh derivatives, the visually updated region includes
  // the last left and right vertex of the previous extrusion only.
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({0, 2}, {3, 6})));

  geometry_.UpdateMeshDerivatives();
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));

  // The last left and right vertex of the previous extrusion should have needed
  // derivative values to get updated. That means the visually updated region
  // should now extend farther back into the last extrusion to cover the
  // triangles connected to the modified vertices.
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({0, 1}, {3, 6})));
}

TEST_F(GeometryMeshDerivativesTest, WithExtrusionBreak) {
  geometry_.AppendLeftVertex(Point{0, 0});
  geometry_.AppendLeftVertex(Point{0, 1});
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendRightVertex(Point{1, 0});
  geometry_.AppendRightVertex(Point{1, 1});
  geometry_.AppendRightVertex(Point{1, 2});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  geometry_.UpdateMeshDerivatives();
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));

  geometry_.AddExtrusionBreak();
  geometry_.ResetMutationTracking();

  // Make the first vertex on each side have the same position as the last
  // vertex from the previous extrusion:
  geometry_.AppendLeftVertex(Point{0, 2});
  geometry_.AppendLeftVertex(Point{2, 6});
  geometry_.AppendRightVertex(Point{1, 2});
  geometry_.AppendRightVertex(Point{3, 6});
  geometry_.ProcessNewVertices(0, BrushTipState{});

  geometry_.UpdateMeshDerivatives();
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));
  EXPECT_THAT(GetSideDerivatives(), Each(Not(VecEq({0, 0}))));

  // Updating mesh derivatives should not affect the vertices prior to the call
  // to `ResetMutationTracking()`. So even though the geometry before and after
  // the extrusion break share positions, they should not share derivative
  // values. The visually updated region should not extend to cover any
  // triangles of the last extrusion.
  EXPECT_THAT(geometry_.CalculateVisuallyUpdatedRegion(),
              EnvelopeEq(Rect::FromTwoPoints({0, 2}, {3, 6})));
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
