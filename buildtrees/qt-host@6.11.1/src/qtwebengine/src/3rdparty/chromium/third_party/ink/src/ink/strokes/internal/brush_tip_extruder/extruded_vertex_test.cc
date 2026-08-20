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

#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/color/type_matchers.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {
namespace {

using ::ink::ChannelStructEqChannelStruct;
using ::ink::ChannelStructNear;
using ::ink::strokes_internal::LegacyVertex;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::Eq;

TEST(ExtrudedVertexTest, EqualityOperators) {
  ExtrudedVertex a = {
      .position = {1, 2},
      .new_non_position_attributes = {.side_derivative = {-5, 4},
                                      .side_label =
                                          StrokeVertex::kExteriorRightLabel,
                                      .forward_derivative = {8, 1},
                                      .forward_label =
                                          StrokeVertex::kInteriorLabel},
      .color = {0, 0.5, 0.75, 1},
      .texture_coords = {2, 7},
      .secondary_texture_coords = {-2, 9}};
  ExtrudedVertex b = a;

  // The "NOLINT" comments help prevent suggestions to use EXPECT_EQ / NE
  // instead but we want to explicitly test the operators, and test them
  // returning both `true` and `false`.

  EXPECT_TRUE(a == b);   // NOLINT
  EXPECT_FALSE(a != b);  // NOLINT

  b.position = {2, 3};
  EXPECT_FALSE(a == b);  // NOLINT
  EXPECT_TRUE(a != b);   // NOLINT

  a = b;
  b.new_non_position_attributes = {
      .side_derivative = {0, 1},
      .side_label = StrokeVertex::kInteriorLabel,
      .forward_derivative = {3, 2},
      .forward_label = StrokeVertex::kExteriorBackLabel};
  EXPECT_FALSE(a == b);  // NOLINT
  EXPECT_TRUE(a != b);   // NOLINT

  a = b;
  b.color = {1, 0, 1, 1};
  EXPECT_FALSE(a == b);  // NOLINT
  EXPECT_TRUE(a != b);   // NOLINT

  a = b;
  b.texture_coords = {1, 7};
  EXPECT_FALSE(a == b);  // NOLINT
  EXPECT_TRUE(a != b);   // NOLINT

  a = b;
  b.secondary_texture_coords = {-8, 5};
  EXPECT_FALSE(a == b);  // NOLINT
  EXPECT_TRUE(a != b);   // NOLINT
}

TEST(ExtrudedVertexTest, CreatedFromLegacy) {
  LegacyVertex legacy_vertex = {.position = {3, 5},
                                .color = {0.2, 0.5, 0.3, 0.9},
                                .texture_coords = {5, 8},
                                .secondary_texture_coords = {9, -1}};
  ExtrudedVertex vertex = ExtrudedVertex::FromLegacy(legacy_vertex);

  EXPECT_THAT(vertex.position, PointEq(legacy_vertex.position));
  EXPECT_THAT(vertex.color, ChannelStructEqChannelStruct(legacy_vertex.color));
  EXPECT_THAT(vertex.texture_coords, PointEq(legacy_vertex.texture_coords));
  EXPECT_THAT(vertex.secondary_texture_coords,
              PointEq(legacy_vertex.secondary_texture_coords));
}

TEST(ExtrudedVertexTest, ConvertedToLegacy) {
  ExtrudedVertex vertex = {.position = {-15, 9},
                           .color = {1, 0, 0.5, 1},
                           .texture_coords = {-41, 25},
                           .secondary_texture_coords = {39, -84}};
  LegacyVertex legacy_vertex = vertex.ToLegacy();

  EXPECT_THAT(legacy_vertex.position,
              PointEq({vertex.position.x, vertex.position.y}));
  EXPECT_THAT(legacy_vertex.color, ChannelStructEqChannelStruct(vertex.color));
  EXPECT_THAT(legacy_vertex.texture_coords, PointEq(vertex.texture_coords));
  EXPECT_THAT(legacy_vertex.secondary_texture_coords,
              PointEq(vertex.secondary_texture_coords));
}

TEST(ExtrudedVertexTest, Interpolate) {
  StrokeVertex::Label shared_side_label = StrokeVertex::kExteriorLeftLabel;
  ExtrudedVertex a = {
      .position = {1, 2},
      .new_non_position_attributes = {.side_derivative = {1, 2},
                                      .side_label = shared_side_label,
                                      .forward_derivative = {4, 5},
                                      .forward_label =
                                          StrokeVertex::kExteriorFrontLabel},
      .color = {1, 0, 0, 1},
      .texture_coords = {5, -5},
      .secondary_texture_coords = {1, 0}};
  ExtrudedVertex b = {
      .position = {2, 3},
      .new_non_position_attributes = {.side_derivative = {1, 2},
                                      .side_label = shared_side_label,
                                      .forward_derivative = {4, 5},
                                      .forward_label =
                                          StrokeVertex::kExteriorBackLabel},
      .color = {0, 0, 1, 1},
      .texture_coords = {10, 0},
      .secondary_texture_coords = {5, 2}};
  ExtrudedVertex vertex = Lerp(a, b, 0.75);

  EXPECT_THAT(vertex.position, PointNear({1.75, 2.75}, 0.01));
  EXPECT_EQ(
      vertex.new_non_position_attributes,
      StrokeVertex::NonPositionAttributes{.side_label = shared_side_label});
  EXPECT_THAT(vertex.color, ChannelStructNear({0.25, 0, 0.75, 1}, 0.01));
  EXPECT_THAT(vertex.texture_coords, PointNear({8.75, -1.25}, 0.01));
  EXPECT_THAT(vertex.secondary_texture_coords, PointNear({4, 1.5}, 0.01));
}

TEST(ExtrudedVertexTest, Extrapolate) {
  ExtrudedVertex a = {
      .position = {1, 2},
      .new_non_position_attributes = {.side_derivative = {1, 2},
                                      .side_label =
                                          StrokeVertex::kExteriorLeftLabel,
                                      .forward_derivative = {4, 5},
                                      .forward_label =
                                          StrokeVertex::kExteriorFrontLabel},
      .color = {0.5, 0, 0.5, 1},
      .texture_coords = {5, -5},
      .secondary_texture_coords = {1, 0}};
  ExtrudedVertex b = {
      .position = {2, 3},
      .new_non_position_attributes = {.side_derivative = {7, 8},
                                      .side_label =
                                          StrokeVertex::kExteriorRightLabel,
                                      .forward_derivative = {10, 11},
                                      .forward_label =
                                          StrokeVertex::kExteriorBackLabel},
      .color = {0, 0, 1, 1},
      .texture_coords = {10, 0},
      .secondary_texture_coords = {5, 2}};
  ExtrudedVertex vertex = Lerp(a, b, -0.5);

  EXPECT_THAT(vertex.position, PointNear({0.5, 1.5}, 0.01));
  EXPECT_THAT(
      vertex.new_non_position_attributes,
      Eq(StrokeVertex::NonPositionAttributes{
          .side_label = a.new_non_position_attributes.side_label,
          .forward_label = a.new_non_position_attributes.forward_label}));
  EXPECT_THAT(vertex.color, ChannelStructNear({0.75, 0, 0.25, 1}, 0.01));
  EXPECT_THAT(vertex.texture_coords, PointNear({2.5, -7.5}, 0.01));
  EXPECT_THAT(vertex.secondary_texture_coords, PointNear({-1, -1}, 0.01));
}

TEST(ExtrudedVertexTest, BarycentricPositionInsideTriangle) {
  StrokeVertex::NonPositionAttributes shared_new_attributes = {
      .side_derivative = {1, 2},
      .side_label = StrokeVertex::kExteriorLeftLabel,
      .forward_derivative = {4, 5},
      .forward_label = StrokeVertex::kExteriorFrontLabel};

  ExtrudedVertex a = {.position = {0, 0},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0.5, 0, 0, 1},
                      .texture_coords = {1, 2},
                      .secondary_texture_coords = {3, 4}};
  ExtrudedVertex b = {.position = {1, 0},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0, 0.5, 0, 1},
                      .texture_coords = {2, 2},
                      .secondary_texture_coords = {4, 5}};
  ExtrudedVertex c = {.position = {0, 1},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0, 0, 0.5, 1},
                      .texture_coords = {2, 3},
                      .secondary_texture_coords = {4, 6}};
  Point position = {0.25, 0.25};
  ExtrudedVertex vertex = BarycentricLerp(a, b, c, position);

  EXPECT_THAT(vertex.position, PointEq({0.25, 0.25}));
  EXPECT_EQ(vertex.new_non_position_attributes,
            StrokeVertex::NonPositionAttributes{});
  EXPECT_THAT(vertex.color, ChannelStructNear({0.25, 0.12, 0.12, 1}, 0.01));
  EXPECT_THAT(vertex.texture_coords, PointNear({1.5, 2.25}, 0.01));
  EXPECT_THAT(vertex.secondary_texture_coords, PointNear({3.5, 4.75}, 0.01));
}

TEST(ExtrudedVertexTest, BarycentricPositionOutsideTriangle) {
  StrokeVertex::NonPositionAttributes shared_new_attributes = {
      .side_derivative = {1, 2},
      .side_label = StrokeVertex::kExteriorLeftLabel,
      .forward_derivative = {4, 5},
      .forward_label = StrokeVertex::kExteriorFrontLabel};

  ExtrudedVertex a = {.position = {0, 0},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0.5, 0, 0, 1},
                      .texture_coords = {1, 2},
                      .secondary_texture_coords = {3, 4}};
  ExtrudedVertex b = {.position = {1, 0},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0, 0.5, 0, 1},
                      .texture_coords = {2, 2},
                      .secondary_texture_coords = {4, 5}};
  ExtrudedVertex c = {.position = {0, 1},
                      .new_non_position_attributes = shared_new_attributes,
                      .color = {0, 0, 0.5, 1},
                      .texture_coords = {2, 3},
                      .secondary_texture_coords = {4, 6}};
  Point position = {1, 1};
  ExtrudedVertex vertex = BarycentricLerp(a, b, c, position);

  EXPECT_THAT(vertex.position, PointEq({1, 1}));
  EXPECT_EQ(vertex.new_non_position_attributes,
            StrokeVertex::NonPositionAttributes{});
  EXPECT_THAT(vertex.color, ChannelStructNear({-0.5, 0.5, 0.5, 1}, 0.01));
  EXPECT_THAT(vertex.texture_coords, PointNear({3, 3}, 0.01));
  EXPECT_THAT(vertex.secondary_texture_coords, PointNear({5, 7}, 0.01));
}

}  // namespace
}  // namespace ink::brush_tip_extruder_internal
