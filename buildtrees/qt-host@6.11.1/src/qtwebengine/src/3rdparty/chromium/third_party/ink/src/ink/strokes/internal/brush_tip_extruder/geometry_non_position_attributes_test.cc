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

#include <array>
#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/geometry.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_vertex.h"
#include "ink/types/small_array.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::BrushTipState;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::ElementsAreArray;
using ::testing::FloatEq;

float GetOpacityShift(const MutableMesh& mesh, uint32_t index) {
  const SmallArray<float, 4>& attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.opacity_shift);
  ABSL_CHECK_EQ(attribute.Size(), 1);
  return attribute[0];
}

std::array<float, 3> GetHslShift(const MutableMesh& mesh, uint32_t index) {
  const SmallArray<float, 4>& attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.hsl_shift);
  ABSL_CHECK_EQ(attribute.Size(), 3);
  return {attribute[0], attribute[1], attribute[2]};
}

Point GetSurfaceUv(const MutableMesh& mesh, uint32_t index) {
  const SmallArray<float, 4>& attribute = mesh.FloatVertexAttribute(
      index, StrokeVertex::kFullFormatAttributeIndices.surface_uv);
  ABSL_CHECK_EQ(attribute.Size(), 2);
  return {attribute[0], attribute[1]};
}

TEST(GeometryNonPositionAttributesTest,
     OpacityAndColorShiftValuesAreSetInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);
  Geometry geometry(mesh_view);

  std::array<float, 2> left_opacity_shifts = {-0.5, 0.5};
  std::array<float, 2> right_opacity_shifts = {-0.5, 0.5};

  std::array<std::array<float, 3>, 4> left_hsl_shifts = {
      std::array<float, 3>({-0.7, 0.2, 0.9}),
      std::array<float, 3>({-0.6, 0.4, 0.8})};
  std::array<std::array<float, 3>, 4> right_hsl_shifts = {
      std::array<float, 3>({-0.6, -0.4, -1}),
      std::array<float, 3>({-0.8, 0, 1})};

  geometry.AppendLeftVertex(Point{0, 0}, left_opacity_shifts[0],
                            left_hsl_shifts[0]);
  geometry.AppendLeftVertex(Point{0, 1}, left_opacity_shifts[1],
                            left_hsl_shifts[1]);
  geometry.AppendRightVertex(Point{1, 0}, right_opacity_shifts[0],
                             right_hsl_shifts[0]);
  geometry.AppendRightVertex(Point{1, 1}, right_opacity_shifts[1],
                             right_hsl_shifts[1]);
  geometry.ProcessNewVertices(0, BrushTipState{});

  ASSERT_EQ(geometry.LeftSide().indices.size(), 2);
  ASSERT_EQ(geometry.RightSide().indices.size(), 2);
  ASSERT_EQ(mesh.VertexCount(), 4);
  ASSERT_EQ(mesh.TriangleCount(), 2);

  EXPECT_THAT(GetOpacityShift(mesh, geometry.LeftSide().indices[0]),
              FloatEq(left_opacity_shifts[0]));
  EXPECT_THAT(GetOpacityShift(mesh, geometry.LeftSide().indices[1]),
              FloatEq(left_opacity_shifts[1]));

  EXPECT_THAT(GetOpacityShift(mesh, geometry.RightSide().indices[0]),
              FloatEq(right_opacity_shifts[0]));
  EXPECT_THAT(GetOpacityShift(mesh, geometry.RightSide().indices[1]),
              FloatEq(right_opacity_shifts[1]));

  EXPECT_THAT(GetHslShift(mesh, geometry.LeftSide().indices[0]),
              ElementsAreArray(left_hsl_shifts[0]));
  EXPECT_THAT(GetHslShift(mesh, geometry.LeftSide().indices[1]),
              ElementsAreArray(left_hsl_shifts[1]));

  EXPECT_THAT(GetHslShift(mesh, geometry.RightSide().indices[0]),
              ElementsAreArray(right_hsl_shifts[0]));
  EXPECT_THAT(GetHslShift(mesh, geometry.RightSide().indices[1]),
              ElementsAreArray(right_hsl_shifts[1]));
}

TEST(GeometryNonPositionAttributesTest, SurfaceUvValuesAreSetInMesh) {
  MutableMesh mesh(StrokeVertex::FullMeshFormat());
  MutableMeshView mesh_view(mesh);
  Geometry geometry(mesh_view);

  geometry.AppendLeftVertex(Point{0, 0},
                            /* opacity_shift = */ 0,
                            /* hsl_shift = */ {},
                            /* surface_uv = */ {0.1, 0.2});
  geometry.AppendLeftVertex(Point{0, 1},
                            /* opacity_shift = */ 0,
                            /* hsl_shift = */ {},
                            /* surface_uv = */ {0.3, 0.4});
  geometry.AppendRightVertex(Point{1, 0},
                             /* opacity_shift = */ 0,
                             /* hsl_shift = */ {},
                             /* surface_uv = */ {0.5, 0.6});
  geometry.AppendRightVertex(Point{1, 1},
                             /* opacity_shift = */ 0,
                             /* hsl_shift = */ {},
                             /* surface_uv = */ {0.7, 0.8});
  geometry.ProcessNewVertices(0, BrushTipState{});

  ASSERT_EQ(geometry.LeftSide().indices.size(), 2);
  ASSERT_EQ(geometry.RightSide().indices.size(), 2);
  ASSERT_EQ(mesh.VertexCount(), 4);
  ASSERT_EQ(mesh.TriangleCount(), 2);

  EXPECT_THAT(GetSurfaceUv(mesh, geometry.LeftSide().indices[0]),
              PointEq({0.1, 0.2}));
  EXPECT_THAT(GetSurfaceUv(mesh, geometry.LeftSide().indices[1]),
              PointEq({0.3, 0.4}));
  EXPECT_THAT(GetSurfaceUv(mesh, geometry.RightSide().indices[0]),
              PointEq({0.5, 0.6}));
  EXPECT_THAT(GetSurfaceUv(mesh, geometry.RightSide().indices[1]),
              PointEq({0.7, 0.8}));
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
