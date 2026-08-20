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

#include "ink/strokes/internal/brush_tip_shape.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/internal/test_matchers.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/extrusion_points.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;
using ::ink::geometry_internal::CircleEq;
using ::ink::geometry_internal::CircleNear;
using ::testing::ElementsAre;

constexpr float kEpsilon = 1e-5;

BrushTipShape ShapeWithZeroMinRadiusAndSeparation(BrushTipState state) {
  return BrushTipShape(state, 0);
}

TEST(BrushTipShapeTest, ConstructedFormingCircle) {
  BrushTipShape shape = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {5, 3}, .width = 14, .height = 14, .percent_radius = 1});
  EXPECT_THAT(shape.Center(), PointEq({5, 3}));
  EXPECT_THAT(shape.PerimeterCircles(),
              ElementsAre(CircleEq(Circle({5, 3}, 7))));
  EXPECT_EQ(shape.GetNextPerimeterIndexCcw(0), 0);
  EXPECT_EQ(shape.GetNextPerimeterIndexCw(0), 0);
}

TEST(BrushTipShapeTest, ConstructedFormingStadium) {
  {
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 1},
                                             .width = 4,
                                             .height = 2,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    EXPECT_THAT(shape.Center(), PointEq({1, 1}));
    EXPECT_THAT(
        shape.PerimeterCircles(),
        ElementsAre(CircleEq(Circle({2, 1}, 1)), CircleEq(Circle({0, 1}, 1))));
  }
  {
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 1},
                                             .width = 4,
                                             .height = 16,
                                             .percent_radius = 1,
                                             .rotation = 3 * kQuarterTurn});
    EXPECT_THAT(shape.Center(), PointEq({1, 1}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({7, 1}, 2), 0.001),
                            CircleNear(Circle({-5, 1}, 2), 0.001)));
  }
}

TEST(BrushTipShapeTest, ConstructedFormingRectangle) {
  BrushTipShape shape =
      ShapeWithZeroMinRadiusAndSeparation({.position = {2, 3},
                                           .width = 8,
                                           .height = 8.f / 3,
                                           .percent_radius = 0,
                                           .rotation = kFullTurn / 6});
  EXPECT_THAT(shape.Center(), PointEq({2, 3}));
  EXPECT_THAT(shape.PerimeterCircles(),
              ElementsAre(CircleNear(Circle({2.85, 7.13}, 0), 0.01),
                          CircleNear(Circle({-1.15, .20}, 0), 0.01),
                          CircleNear(Circle({1.15, -1.13}, 0), 0.01),
                          CircleNear(Circle({5.15, 5.80}, 0), 0.01)));
}

TEST(BrushTipShapeTest, ConstructedFormingPinchedQuad) {
  {  // Shape with 0 min_radius_and_separation, and 0 percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0,
                                             .rotation = Angle(),
                                             .pinch = 0.3});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({-4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({-3.15, -1.5}, 0), 0.01),
                            CircleNear(Circle({3.15, -1.5}, 0), 0.01)));
  }
  {  // Shape with 0 min_radius_and_separation, and a non-zero percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0.2,
                                             .rotation = Angle(),
                                             .pinch = 0.3});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({4.2, 1.2}, 0.3), 0.01),
                            CircleNear(Circle({-4.2, 1.2}, 0.3), 0.01),
                            CircleNear(Circle({-2.94, -1.2}, 0.3), 0.01),
                            CircleNear(Circle({2.94, -1.2}, 0.3), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and 0 percent_radius,
     // pinch not creating overlap control circle.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0,
                                         .rotation = Angle(),
                                         .pinch = 0.3},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({-4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({-3.15, -1.5}, 0), 0.01),
                            CircleNear(Circle({3.15, -1.5}, 0), 0.01)));
  }
  {  // Shape with 0 min_radius_and_separation, and a non-zero percent_radius
     // not centered around 0.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {2, 3},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0.2,
                                             .rotation = Angle(),
                                             .pinch = 0.3});
    EXPECT_THAT(shape.Center(), PointEq({2, 3}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({6.2, 4.2}, 0.3), 0.01),
                            CircleNear(Circle({-2.2, 4.2}, 0.3), 0.01),
                            CircleNear(Circle({-0.94, 1.8}, 0.3), 0.01),
                            CircleNear(Circle({4.94, 1.8}, 0.3), 0.01)));
  }
}

TEST(BrushTipShapeTest, ConstructedFormingPinchedQuadToTriangle) {
  {  // Shape with non-zero min_radius_and_separation, and zero percent_radius,
     // pinch creating overlap control circle
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0,
                                         .rotation = Angle(),
                                         .pinch = 0.8},
                                        2.0);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({-4.5, 1.5}, 0), 0.01),
                            CircleNear(Circle({0, -1.5}, 0), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, pinch creating overlap control circle
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f,
                                         .percent_radius = 0.5,
                                         .rotation = Angle(),
                                         .pinch = 0.99},
                                        0.8);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({2.25, 2.25}, 2.25), 0.01),
                            CircleNear(Circle({-2.25, 2.25}, 2.25), 0.01),
                            CircleNear(Circle({0, -2.25}, 2.25), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, pinch creating overlap control circle not centered
     // around 0.
    BrushTipShape shape = BrushTipShape({.position = {2, 3},
                                         .width = 9,
                                         .height = 9.f,
                                         .percent_radius = 0.5,
                                         .rotation = Angle(),
                                         .pinch = 0.99},
                                        0.8);
    EXPECT_THAT(shape.Center(), PointEq({2, 3}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({4.25, 5.25}, 2.25), 0.01),
                            CircleNear(Circle({-0.25, 5.25}, 2.25), 0.01),
                            CircleNear(Circle({2, 0.75}, 2.25), 0.01)));
  }
}

TEST(BrushTipShapeTest,
     ConstructedFormingStadiumFromPercentRadiusAndMinRadiusAndSeparation) {
  {  // Shape with percent radius big enough that `y` is set to zero resulting
     // in stadium.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.99,
                                         .rotation = Angle(),
                                         .pinch = 0.8},
                                        1.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.02, 0}, 1.49), 0.01),
                            CircleNear(Circle({-3.02, 0}, 1.49), 0.01)));
  }
  {  // Shape with percent radius big enough that `x` is set to zero resulting
     // in stadium.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9.f / 3,
                                         .height = 9,
                                         .percent_radius = 0.99,
                                         .rotation = Angle(),
                                         .pinch = 0.8},
                                        1.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({0, 3.02}, 1.49), 0.01),
                            CircleNear(Circle({0, -3.02}, 1.49), 0.01)));
  }
  {  // Shape with percent radius big enough that `x`  and `y` are set to zero
     // resulting in circle.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9,
                                         .percent_radius = 0.99,
                                         .rotation = Angle(),
                                         .pinch = 0.8},
                                        1.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({0, 0}, 4.46), 0.01)));
  }
}

TEST(BrushTipShapeTest, ConstructedFormingSlantedRectangle) {
  {  // Shape with 0 min_radius_and_separation, and 0 percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0,
                                             .rotation = Angle(),
                                             .slant = kFullTurn / 6});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.2, 0.75}, 0), 0.01),
                            CircleNear(Circle({-5.8, 0.75}, 0), 0.01),
                            CircleNear(Circle({-3.2, -0.75}, 0), 0.01),
                            CircleNear(Circle({5.8, -0.75}, 0), 0.01)));
  }
  {  // Shape with 0 min_radius_and_separation, and a non-zero percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0.2,
                                             .rotation = Angle(),
                                             .slant = kFullTurn / 6});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.16, 0.6}, 0.3), 0.01),
                            CircleNear(Circle({-5.24, 0.6}, 0.3), 0.01),
                            CircleNear(Circle({-3.16, -0.6}, 0.3), 0.01),
                            CircleNear(Circle({5.24, -0.6}, 0.3), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and 0 percent_radius.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.0,
                                         .rotation = Angle(),
                                         .slant = kFullTurn / 6},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.2, 0.75}, 0.0), 0.01),
                            CircleNear(Circle({-5.8, 0.75}, 0.0), 0.01),
                            CircleNear(Circle({-3.2, -0.75}, 0.0), 0.01),
                            CircleNear(Circle({5.8, -0.75}, 0.0), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, non-zero rotation.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.2,
                                         .rotation = kFullTurn / 8,
                                         .slant = kFullTurn / 6},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({1.81, 2.66}, 0.3), 0.01),
                            CircleNear(Circle({-4.13, -3.28}, 0.3), 0.01),
                            CircleNear(Circle({-1.81, -2.66}, 0.3), 0.01),
                            CircleNear(Circle({4.13, 3.28}, 0.3), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, non-zero rotation, and non-zero center.
    BrushTipShape shape = BrushTipShape({.position = {2, 3},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.2,
                                         .rotation = kFullTurn / 8,
                                         .slant = kFullTurn / 6},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({2, 3}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.81, 5.66}, 0.3), 0.01),
                            CircleNear(Circle({-2.13, -0.28}, 0.3), 0.01),
                            CircleNear(Circle({0.19, 0.34}, 0.3), 0.01),
                            CircleNear(Circle({6.13, 6.28}, 0.3), 0.01)));
  }
}

TEST(BrushTipShapeTest, ConstructedFormingSlantedPinchedQuad) {
  {  // Shape with 0 min_radius_and_separation, and 0 percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0,
                                             .rotation = Angle(),
                                             .slant = kFullTurn / 6,
                                             .pinch = 0.3});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.2, 0.75}, 0), 0.01),
                            CircleNear(Circle({-5.8, 0.75}, 0), 0.01),
                            CircleNear(Circle({-1.85, -0.75}, 0), 0.01),
                            CircleNear(Circle({4.45, -0.75}, 0), 0.01)));
  }
  {  // Shape with 0 min_radius_and_separation, and a non-zero percent_radius.
    BrushTipShape shape =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 9,
                                             .height = 9.f / 3,
                                             .percent_radius = 0.2,
                                             .rotation = Angle(),
                                             .slant = kFullTurn / 6,
                                             .pinch = 0.3});
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.16, 0.6}, 0.3), 0.01),
                            CircleNear(Circle({-5.24, 0.6}, 0.3), 0.01),
                            CircleNear(Circle({-1.9, -0.6}, 0.3), 0.01),
                            CircleNear(Circle({3.98, -0.6}, 0.3), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and 0 percent_radius.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.0,
                                         .rotation = Angle(),
                                         .slant = kFullTurn / 6,
                                         .pinch = 0.3},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.2, 0.75}, 0.0), 0.01),
                            CircleNear(Circle({-5.8, 0.75}, 0.0), 0.01),
                            CircleNear(Circle({-1.85, -0.75}, 0.0), 0.01),
                            CircleNear(Circle({4.45, -0.75}, 0.0), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, non-zero rotation.
    BrushTipShape shape = BrushTipShape({.position = {0, 0},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.2,
                                         .rotation = kFullTurn / 8,
                                         .slant = kFullTurn / 6,
                                         .pinch = 0.3},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({0, 0}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({1.81, 2.66}, 0.3), 0.01),
                            CircleNear(Circle({-4.13, -3.28}, 0.3), 0.01),
                            CircleNear(Circle({-0.92, -1.77}, 0.3), 0.01),
                            CircleNear(Circle({3.24, 2.39}, 0.3), 0.01)));
  }
  {  // Shape with non-zero min_radius_and_separation, and non-zero
     // percent_radius, non-zero rotation, and non-zero center.
    BrushTipShape shape = BrushTipShape({.position = {2, 3},
                                         .width = 9,
                                         .height = 9.f / 3,
                                         .percent_radius = 0.2,
                                         .rotation = kFullTurn / 8,
                                         .slant = kFullTurn / 6,
                                         .pinch = 0.3},
                                        0.2);
    EXPECT_THAT(shape.Center(), PointEq({2, 3}));
    EXPECT_THAT(shape.PerimeterCircles(),
                ElementsAre(CircleNear(Circle({3.81, 5.66}, 0.3), 0.01),
                            CircleNear(Circle({-2.13, -0.28}, 0.3), 0.01),
                            CircleNear(Circle({1.08, 1.23}, 0.3), 0.01),
                            CircleNear(Circle({5.24, 5.39}, 0.3), 0.01)));
  }
}

TEST(BrushTipShapeTest, ConstructedFormingRoundedSquare) {
  BrushTipShape shape =
      ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                           .width = 4,
                                           .height = 4,
                                           .percent_radius = 0.5,
                                           .rotation = kFullTurn / 8});
  EXPECT_THAT(shape.Center(), PointEq({0, 0}));
  EXPECT_THAT(shape.PerimeterCircles(),
              ElementsAre(CircleNear(Circle({0, 1.414}, 1), 0.001),
                          CircleNear(Circle({-1.414, 0}, 1), 0.001),
                          CircleNear(Circle({0, -1.414}, 1), 0.001),
                          CircleNear(Circle({1.414, 0}, 1), 0.001)));
}

TEST(BrushTipShapeTest, ConstructedWithZeroWidthAndHeight) {
  BrushTipShape shape = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {5, 3}, .width = 0, .height = 0, .percent_radius = 0});
  EXPECT_THAT(shape.Center(), PointEq({5, 3}));
  EXPECT_THAT(shape.PerimeterCircles(),
              ElementsAre(CircleEq(Circle(shape.Center(), 0))));
  EXPECT_EQ(shape.GetNextPerimeterIndexCcw(0), 0);
  EXPECT_EQ(shape.GetNextPerimeterIndexCw(0), 0);
}

TEST(BrushTipShapeTest, ConstructedWithZeroWidth) {
  BrushTipShape shape = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 0, .height = 4, .percent_radius = 0.5});

  EXPECT_THAT(shape.Center(), PointEq({0, 0}));
  EXPECT_THAT(
      shape.PerimeterCircles(),
      ElementsAre(CircleEq(Circle({0, 2}, 0)), CircleEq(Circle({0, -2}, 0))));
}

TEST(BrushTipShapeTest, ConstructedWithZeroHeight) {
  BrushTipShape shape(
      {.position = {0, 0}, .width = 4, .height = 0, .percent_radius = 0.5}, 0);

  EXPECT_THAT(shape.Center(), PointEq({0, 0}));
  EXPECT_THAT(
      shape.PerimeterCircles(),
      ElementsAre(CircleEq(Circle({2, 0}, 0)), CircleEq(Circle({-2, 0}, 0))));
}

TEST(BrushTipShapeDeathTest, ConstructedWithPercentRadiusLessThanZero) {
  EXPECT_DEATH_IF_SUPPORTED(BrushTipShape(BrushTipState{.position = {0, 0},
                                                        .width = 2,
                                                        .height = 2,
                                                        .percent_radius = -1},
                                          0),
                            "");
}

TEST(BrushTipShapeDeathTest, ConstructedWithPercentRadiusGreaterThanOne) {
  EXPECT_DEATH_IF_SUPPORTED(
      BrushTipShape(
          BrushTipState{
              .position = {0, 0}, .width = 2, .height = 2, .percent_radius = 2},
          0),
      "");
}

TEST(BrushTipShapeDeathTest, ConstructedWithNegativeWidth) {
  EXPECT_DEATH_IF_SUPPORTED(BrushTipShape(BrushTipState{.position = {0, 0},
                                                        .width = -1,
                                                        .height = 2,
                                                        .percent_radius = 0.5},
                                          0),
                            "");
}

TEST(BrushTipShapeDeathTest, ConstructedWithNegativeHeight) {
  EXPECT_DEATH_IF_SUPPORTED(BrushTipShape(BrushTipState{.position = {0, 0},
                                                        .width = 3,
                                                        .height = -5,
                                                        .percent_radius = 0.5},
                                          0),
                            "");
}

TEST(BrushTipShapeTest, Circles) {
  // See brush_tip_shape_tests.svg Circles
  BrushTipShape circle_1 = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
  BrushTipShape circle_2 = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {1, 0}, .width = 4, .height = 4, .percent_radius = 1});
  BrushTipShape::TangentCircleIndices indices =
      BrushTipShape::GetTangentCircleIndices(circle_1, circle_2);
  EXPECT_EQ(indices.left.first, 0);
  EXPECT_EQ(indices.left.second, 0);
  EXPECT_EQ(indices.right.first, 0);
  EXPECT_EQ(indices.right.second, 0);
}

TEST(BrushTipShapeTest, TangentIndicesWithCircleStadium) {
  {
    // See brush_tip_shape_tests.svg Circle + Stadium #1
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape stadium =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 1},
                                             .width = 4,
                                             .height = 2,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(circle, stadium);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 0);
    EXPECT_EQ(indices.right.second, 0);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Stadium #2
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape stadium =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 1},
                                             .width = 4,
                                             .height = 2,
                                             .percent_radius = 1,
                                             .rotation = -kQuarterTurn});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(circle, stadium);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 0);
    EXPECT_EQ(indices.right.second, 1);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Stadium #3
    BrushTipShape stadium =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = kQuarterTurn});
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 2}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(stadium, circle);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 1);
    EXPECT_EQ(indices.right.second, 0);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Stadium #4
    BrushTipShape stadium =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 2}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(stadium, circle);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 1);
    EXPECT_EQ(indices.right.second, 0);
  }
}

TEST(BrushTipShapeTest, TangentIndicesWithCircleSquare) {
  {
    // See brush_tip_shape_tests.svg Circle + Square #1
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape square = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {-3, 0}, .width = 4, .height = 4, .percent_radius = 0.25});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(circle, square);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 3);
    EXPECT_EQ(indices.right.first, 0);
    EXPECT_EQ(indices.right.second, 0);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Square #2
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {3, 0},
                                             .width = 4,
                                             .height = 4,
                                             .percent_radius = 0.25,
                                             .rotation = kFullTurn / 8});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(circle, square);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 0);
    EXPECT_EQ(indices.right.second, 2);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Square #3
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {0, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {2, 0},
                                             .width = 2,
                                             .height = 2,
                                             .percent_radius = 0.25,
                                             .rotation = -kQuarterTurn});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(circle, square);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 0);
    EXPECT_EQ(indices.right.second, 0);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Square #4
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 2,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {2, 0}, .width = 4, .height = 4, .percent_radius = 1});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(square, circle);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 2);
    EXPECT_EQ(indices.right.second, 0);
  }
  {
    // See brush_tip_shape_tests.svg Circle + Square #5
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 4,
                                             .height = 4,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation(
        {.position = {1, 0}, .width = 2.2, .height = 2.2, .percent_radius = 1});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(square, circle);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 3);
    EXPECT_EQ(indices.right.second, 0);
  }
}

TEST(BrushTipShapeTest, TangentIndicesWithStadia) {
  {
    // See brush_tip_shape_tests.svg Stadium + Stadium #1
    BrushTipShape stadium_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    BrushTipShape stadium_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(stadium_1, stadium_2);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 1);
    EXPECT_EQ(indices.right.second, 1);
  }
  {
    // See brush_tip_shape_tests.svg Stadium + Stadium #2
    BrushTipShape stadium_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = Angle()});
    BrushTipShape stadium_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {2, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 1,
                                             .rotation = kQuarterTurn});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(stadium_1, stadium_2);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 1);
    EXPECT_EQ(indices.right.second, 1);
  }
}

TEST(BrushTipShapeTest, TangentIndicesWithRectangles) {
  {
    // See brush_tip_shape_tests.svg Rectangle + Rectangle #1
    BrushTipShape rectangle_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 6,
                                             .height = 8,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape rectangle_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 0},
                                             .width = 6,
                                             .height = 8,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(rectangle_1, rectangle_2);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 2);
    EXPECT_EQ(indices.right.second, 2);
  }
  {
    // See brush_tip_shape_tests.svg Rectangle + Rectangle #2
    BrushTipShape rectangle_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 6,
                                             .height = 8,
                                             .percent_radius = 0,
                                             .rotation = Angle()});
    BrushTipShape rectangle_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 1},
                                             .width = 6,
                                             .height = 8,
                                             .percent_radius = 0,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(rectangle_1, rectangle_2);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 3);
    EXPECT_EQ(indices.right.second, 3);
  }
  {
    // See brush_tip_shape_tests.svg Rectangle + Rectangle #3
    BrushTipShape square_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 6,
                                             .height = 6,
                                             .percent_radius = 0.25,
                                             .rotation = kFullTurn / 8});
    BrushTipShape square_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 0},
                                             .width = 6,
                                             .height = 6,
                                             .percent_radius = 0.25,
                                             .rotation = -kFullTurn / 8});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(square_1, square_2);
    EXPECT_EQ(indices.left.first, 0);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 2);
    EXPECT_EQ(indices.right.second, 3);
  }
}

TEST(BrushTipShapeTest, TangentIndicesWithCoincidentControlPoints) {
  {
    // See brush_tip_shape_tests.svg Coincident Points #1
    // Two pairs of shape coincident
    BrushTipShape rectangle_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 6,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape rectangle_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1.75, 0},
                                             .width = 2,
                                             .height = 6,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(rectangle_1, rectangle_2);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 2);
    EXPECT_EQ(indices.right.second, 2);
  }
  {
    // See brush_tip_shape_tests.svg Coincident Points #2
    // One pair of shape coincident
    BrushTipShape rectangle_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 4,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape rectangle_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 1},
                                             .width = 4,
                                             .height = 2,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(rectangle_1, rectangle_2);
    EXPECT_EQ(indices.left.first, 2);
    EXPECT_EQ(indices.left.second, 2);
    EXPECT_EQ(indices.right.first, 3);
    EXPECT_EQ(indices.right.second, 3);
  }
}

TEST(BrushTipShapeDeathTest, TangentIndicesWithOneInsideTheOther) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  BrushTipShape large_rectangle =
      ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                           .width = 8,
                                           .height = 6,
                                           .percent_radius = 0.25,
                                           .rotation = Angle()});
  BrushTipShape small_rectangle =
      ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                           .width = 2,
                                           .height = 4,
                                           .percent_radius = 0.25,
                                           .rotation = Angle()});
  // See brush_tip_shape_tests.svg One Inside The Other #1
  EXPECT_DEATH_IF_SUPPORTED(
      BrushTipShape::GetTangentCircleIndices(small_rectangle, large_rectangle),
      "");
  // See brush_tip_shape_tests.svg One Inside The Other #2
  EXPECT_DEATH_IF_SUPPORTED(
      BrushTipShape::GetTangentCircleIndices(large_rectangle, small_rectangle),
      "");
  {
    // See brush_tip_shape_tests.svg One Inside The Other #3
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 2,
                                             .percent_radius = 0.5,
                                             .rotation = Angle()});
    EXPECT_DEATH_IF_SUPPORTED(
        BrushTipShape::GetTangentCircleIndices(square, square), "");
  }
  {
    // See brush_tip_shape_tests.svg One Inside The Other #4
    BrushTipShape square =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 2,
                                             .percent_radius = 0.5,
                                             .rotation = kFullTurn / 8});
    EXPECT_DEATH_IF_SUPPORTED(
        BrushTipShape::GetTangentCircleIndices(square, square), "");
  }
  {
    // See brush_tip_shape_tests.svg One Inside The Other #4
    BrushTipShape rectangle_1 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                             .width = 2,
                                             .height = 6,
                                             .percent_radius = 0.25,
                                             .rotation = Angle()});
    BrushTipShape rectangle_2 =
        ShapeWithZeroMinRadiusAndSeparation({.position = {1, 0},
                                             .width = 4,
                                             .height = 6,
                                             .percent_radius = 0.125,
                                             .rotation = Angle()});
    EXPECT_DEATH_IF_SUPPORTED(
        BrushTipShape::GetTangentCircleIndices(rectangle_1, rectangle_2), "");
  }
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(BrushTipShapeTest, TangentIndicesWithMoreThanTwoPointsOfIntersection) {
  BrushTipShape square_1 =
      ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                           .width = 2,
                                           .height = 2,
                                           .percent_radius = 0,
                                           .rotation = Angle()});
  BrushTipShape square_2 =
      ShapeWithZeroMinRadiusAndSeparation({.position = {0, 0},
                                           .width = 2,
                                           .height = 2,
                                           .percent_radius = 0,
                                           .rotation = kFullTurn / 8});
  {
    // See brush_tip_shape_tests.svg More Than Two Points Of
    // Intersection #1
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(square_1, square_2);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 0);
    EXPECT_EQ(indices.right.first, 2);
    EXPECT_EQ(indices.right.second, 2);
  }
  {
    // See brush_tip_shape_tests.svg More Than Two Points Of
    // Intersection #2
    BrushTipShape::TangentCircleIndices indices =
        BrushTipShape::GetTangentCircleIndices(square_2, square_1);
    EXPECT_EQ(indices.left.first, 1);
    EXPECT_EQ(indices.left.second, 1);
    EXPECT_EQ(indices.right.first, 1);
    EXPECT_EQ(indices.right.second, 2);
  }
}

TEST(BrushTipShapeTest,
     TangentIndicesReturnsCorrectIndicesWhenOneCornerIsInsidePreviousShape) {
  // See brush_tip_shape_tests.svg One Corner Inside
  // The upper-left corner (index 1) of `small_square` is inside `large_circle`.
  // The other corners lie just outside it.
  BrushTipShape large_circle{
      {.position = {0, 0}, .width = 2, .height = 2, .percent_radius = 1},
      kEpsilon};
  BrushTipShape small_square{
      {.position = {0.8, -0.8}, .width = 0.25, .height = 0.25}, kEpsilon};

  BrushTipShape::TangentCircleIndices indices =
      BrushTipShape::GetTangentCircleIndices(large_circle, small_square);

  EXPECT_EQ(indices.left.first, 0);
  EXPECT_EQ(indices.left.second, 3);
  EXPECT_EQ(indices.right.first, 0);
  EXPECT_EQ(indices.right.second, 3);
}

ExtrusionPoints AppendTipExtrusionPointsHelper(const BrushTipState &start,
                                               const BrushTipState &middle,
                                               const BrushTipState &end,
                                               float max_chord_height) {
  ExtrusionPoints result;
  BrushTipShape::AppendTurnExtrusionPoints(
      BrushTipShape(start, 0), BrushTipShape(middle, 0), BrushTipShape(end, 0),
      max_chord_height, result);
  return result;
}

TEST(AddRoundedQuadTurnPoints, AppendTurnExtrusionPoints) {
  {
    // Slight left turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.5,
                                        .rotation = Angle()},
                                       {.position = {2, -1},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.25,
                                        .rotation = kQuarterTurn},
                                       {.position = {4, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.5,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(points.left, ElementsAre(PointNear({2, 0.37}, 0.01)));
    EXPECT_THAT(points.right, ElementsAre(PointNear({1.13, -1.97}, 0.01),
                                          PointNear({1.25, -2}, 0.01),
                                          PointNear({2.75, -2}, 0.01),
                                          PointNear({2.88, -1.97}, 0.01)));
  }
  {
    // Slight right turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.5,
                                        .rotation = Angle()},
                                       {.position = {2, 1},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.25,
                                        .rotation = kQuarterTurn},
                                       {.position = {4, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0.5,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(
        points.left,
        ElementsAre(PointNear({1.13, 1.97}, 0.01), PointNear({1.25, 2}, 0.01),
                    PointNear({2.75, 2}, 0.01), PointNear({2.88, 1.97}, 0.01)));
    EXPECT_THAT(points.right, ElementsAre(PointNear({2, -0.37}, 0.01)));
  }
  {
    // Right-angle left turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, 2},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(points.left, ElementsAre(PointNear({1, 1}, 0.01)));
    EXPECT_THAT(points.right, ElementsAre(PointNear({1, -1}, 0.01),
                                          PointNear({3, -1}, 0.01)));
  }
  {
    // Right-angle right turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, 0},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, -2},
                                        .width = 2,
                                        .height = 2,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(points.left,
                ElementsAre(PointNear({1, 1}, 0.01), PointNear({3, 1}, 0.01)));
    EXPECT_THAT(points.right, ElementsAre(PointNear({1, -1}, 0.01)));
  }
  {
    // Sharp left turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 0},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, 1},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {0, 2},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(points.left,
                ElementsAre(PointNear({0, 3}, 0.01), PointNear({0, -1}, 0.01)));
    EXPECT_THAT(points.right,
                ElementsAre(PointNear({4, -1}, 0.01), PointNear({4, 3}, 0.01)));
  }
  {
    // Sharp right turn
    ExtrusionPoints points =
        AppendTipExtrusionPointsHelper({.position = {0, 2},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {2, 1},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       {.position = {0, 0},
                                        .width = 4,
                                        .height = 4,
                                        .percent_radius = 0,
                                        .rotation = Angle()},
                                       0.01);
    EXPECT_THAT(points.left,
                ElementsAre(PointNear({4, 3}, 0.01), PointNear({4, -1}, 0.01)));
    EXPECT_THAT(points.right,
                ElementsAre(PointNear({0, -1}, 0.01), PointNear({0, 3}, 0.01)));
  }
}

// TODO(b/279156264): Add BrushTipShape cases for dynamic sizes and
// orientations.

TEST(BrushTipShapeTest, AppendRoundedSquareStartcapExtrusionPoints) {
  BrushTipShape first = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 20, .height = 20, .percent_radius = 0.5});
  BrushTipShape second = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {5, 5}, .width = 20, .height = 20, .percent_radius = 0.5});

  // Fill the points with some starting values to check they are not modified.
  ExtrusionPoints startcap = {
      .left = {{8, 7}, {6, 5}, {4, 3}},
      .right = {{2, 1}, {0, -1}},
  };

  BrushTipShape::AppendStartcapExtrusionPoints(first, second, 5, startcap);
  EXPECT_THAT(startcap.left,
              ElementsAre(PointEq({8, 7}), PointEq({6, 5}), PointEq({4, 3}),
                          PointNear({-10, -5}, 0.01), PointNear({-10, 5}, 0.01),
                          PointNear({-8.54, 8.54}, 0.01)));
  EXPECT_THAT(
      startcap.right,
      ElementsAre(PointEq({2, 1}), PointEq({0, -1}), PointNear({-5, -10}, 0.01),
                  PointNear({5, -10}, 0.01), PointNear({8.54, -8.54}, 0.01)));
}

TEST(BrushTipShapeTest, AppendRoundedSquareEndcapExtrusionPoints) {
  BrushTipShape second_to_last = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {-5, 5}, .width = 20, .height = 20, .percent_radius = 0.5});
  BrushTipShape last = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 20, .height = 20, .percent_radius = 0.5});

  // Fill the points with some starting values to check they are not modified.
  ExtrusionPoints endcap = {
      .left = {{8, 7}, {6, 5}, {4, 3}},
      .right = {{2, 1}, {0, -1}},
  };

  BrushTipShape::AppendEndcapExtrusionPoints(second_to_last, last, 5, endcap);
  EXPECT_THAT(endcap.left,
              ElementsAre(PointEq({8, 7}), PointEq({6, 5}), PointEq({4, 3}),
                          PointNear({8.54, 8.54}, 0.01),
                          PointNear({10, 5}, 0.01), PointNear({10, -5}, 0.01)));
  EXPECT_THAT(endcap.right, ElementsAre(PointEq({2, 1}), PointEq({0, -1}),
                                        PointNear({-8.54, -8.54}, 0.01),
                                        PointNear({-5, -10}, 0.01),
                                        PointNear({5, -10}, 0.01)));
}

TEST(BrushTipShapeTest, AppendCircularStartcapExtrusionPoints) {
  BrushTipShape first = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 20, .height = 20, .percent_radius = 1});
  BrushTipShape second = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {10, 10}, .width = 10, .height = 10, .percent_radius = 1});

  ExtrusionPoints startcap;
  BrushTipShape::AppendStartcapExtrusionPoints(first, second, .5, startcap);

  EXPECT_THAT(startcap.left, ElementsAre(PointNear({-8.73, -4.88}, 0.01),
                                         PointNear({-9.99, 0.43}, 0.01),
                                         PointNear({-8.28, 5.60}, 0.01),
                                         PointNear({-4.11, 9.11}, 0.01)));
  EXPECT_THAT(startcap.right, ElementsAre(PointNear({-4.88, -8.73}, 0.01),
                                          PointNear({0.43, -9.99}, 0.01),
                                          PointNear({5.60, -8.28}, 0.01),
                                          PointNear({9.11, -4.11}, 0.01)));
}

TEST(BrushTipShapeTest, AppendCircularEndcapExtrusionPoints) {
  BrushTipShape second_to_last = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {0, 0}, .width = 10, .height = 10, .percent_radius = 1});
  BrushTipShape last = ShapeWithZeroMinRadiusAndSeparation(
      {.position = {10, -10}, .width = 20, .height = 20, .percent_radius = 1});

  ExtrusionPoints endcap;
  BrushTipShape::AppendEndcapExtrusionPoints(second_to_last, last, .5, endcap);
  EXPECT_THAT(endcap.left, ElementsAre(PointNear({14.11, -0.89}, 0.01),
                                       PointNear({18.28, -4.40}, 0.01),
                                       PointNear({19.99, -9.57}, 0.01),
                                       PointNear({18.73, -14.88}, 0.01)));
  EXPECT_THAT(endcap.right, ElementsAre(PointNear({0.89, -14.11}, 0.01),
                                        PointNear({4.40, -18.28}, 0.01),
                                        PointNear({9.57, -19.99}, 0.01),
                                        PointNear({14.88, -18.73}, 0.01)));
}

TEST(BrushTipShapeTest, AppendCircularWholeShapeExtrusionPoints) {
  BrushTipShape shape = ShapeWithZeroMinRadiusAndSeparation(
      {.position{2, 2}, .width = 5, .height = 5, .percent_radius = 1});

  // Fill the points with some starting values to check they are not modified.
  ExtrusionPoints points = {
      .left = {{8, 7}, {6, 5}, {4, 3}},
      .right = {{2, 1}, {0, -1}},
  };

  BrushTipShape::AppendWholeShapeExtrusionPoints(shape, 0.5, {-2, 1}, points);
  EXPECT_THAT(points.left,
              ElementsAre(PointEq({8, 7}), PointEq({6, 5}), PointEq({4, 3}),
                          PointNear({3.12, -0.23}, 0.01),
                          PointNear({0.88, -0.24}, 0.01),
                          PointNear({-0.46, 1.55}, 0.01)));
  EXPECT_THAT(points.right, ElementsAre(PointEq({2, 1}), PointEq({0, -1}),
                                        PointNear({4.46, 2.45}, 0.01),
                                        PointNear({3.12, 4.24}, 0.01),
                                        PointNear({0.88, 4.24}, 0.01)));
}

TEST(BrushTipShapeTest, AppendRoundedRectangleWholeShapeExtrusionPoints) {
  BrushTipShape shape =
      ShapeWithZeroMinRadiusAndSeparation({.position{3, 4},
                                           .width = 4,
                                           .height = 5,
                                           .percent_radius = 0.5,
                                           .rotation = kFullTurn / 3});

  // Fill the points with some starting values to check they are not modified.
  ExtrusionPoints points = {
      .left = {{8, 7}, {6, 5}, {4, 3}},
      .right = {{2, 1}, {0, -1}},
  };

  BrushTipShape::AppendWholeShapeExtrusionPoints(shape, 0.5, {1, -5}, points);
  EXPECT_THAT(
      points.left,
      ElementsAre(PointEq({8, 7}), PointEq({6, 5}), PointEq({4, 3}),
                  PointNear({3.30, 6.48}, 0.01), PointNear({4.67, 6.12}, 0.01),
                  PointNear({5.67, 4.38}, 0.01),
                  PointNear({5.30, 3.02}, 0.01)));
  EXPECT_THAT(points.right, ElementsAre(PointEq({2, 1}), PointEq({0, -1}),
                                        PointNear({0.70, 4.98}, 0.01),
                                        PointNear({0.33, 3.62}, 0.01),
                                        PointNear({1.33, 1.89}, 0.01),
                                        PointNear({2.70, 1.52}, 0.01)));
}

TEST(BrushTipShapeTest, AppendSquareWholeShapeExtrusionPoints) {
  BrushTipShape shape = ShapeWithZeroMinRadiusAndSeparation(
      {.position{0, 0}, .width = 4, .height = 4, .percent_radius = 0});

  // Fill the points with some starting values to check they are not modified.
  ExtrusionPoints points = {
      .left = {{8, 7}, {6, 5}, {4, 3}},
      .right = {{2, 1}, {0, -1}},
  };

  BrushTipShape::AppendWholeShapeExtrusionPoints(shape, 0.5, {0, -1}, points);
  EXPECT_THAT(points.left,
              ElementsAre(PointEq({8, 7}), PointEq({6, 5}), PointEq({4, 3}),
                          PointNear({2, 2}, 0.01), PointNear({2, -2}, 0.01)));
  EXPECT_THAT(points.right,
              ElementsAre(PointEq({2, 1}), PointEq({0, -1}),
                          PointNear({-2, 2}, 0.01), PointNear({-2, -2}, 0.01)));
}

TEST(BrushTipShapeTest, ContainsSelf) {
  BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {1, 2},
      .width = 5,
      .height = 5,
      .percent_radius = 1,
  });
  EXPECT_TRUE(circle.Contains(circle));

  BrushTipShape stadium = ShapeWithZeroMinRadiusAndSeparation({
      .position = {-3, 5},
      .width = 2,
      .height = 8,
      .percent_radius = 1,
  });
  EXPECT_TRUE(stadium.Contains(stadium));

  BrushTipShape rounded_rectangle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {5, 7},
      .width = 4,
      .height = 8,
      .percent_radius = 0.5,
      .rotation = kFullTurn / 6,
  });
  EXPECT_TRUE(rounded_rectangle.Contains(rounded_rectangle));

  BrushTipShape rectangle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {5, 7},
      .width = 4,
      .height = 8,
      .percent_radius = 0,
      .rotation = -kQuarterTurn,
  });
  EXPECT_TRUE(rectangle.Contains(rectangle));
}

TEST(BrushTipShapeTest, ContainsWithDistantShapes) {
  BrushTipShape shape1 = ShapeWithZeroMinRadiusAndSeparation({
      .position = {5, 7},
      .width = 4,
      .height = 8,
      .percent_radius = 0.5,
      .rotation = kFullTurn / 6,
  });
  BrushTipShape shape2 = ShapeWithZeroMinRadiusAndSeparation({
      .position = {20, -8},
      .width = 7,
      .height = 3,
      .percent_radius = 1,
      .rotation = kFullTurn / 10,
  });
  EXPECT_FALSE(shape1.Contains(shape2));
  EXPECT_FALSE(shape2.Contains(shape1));
}

TEST(BrushTipShapeTest, ContainsWithCircleAndRoundedRectangle) {
  BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {1, 2},
      .width = 10,
      .height = 10,
      .percent_radius = 1,
  });

  EXPECT_TRUE(circle.Contains(
      ShapeWithZeroMinRadiusAndSeparation({.position = {1, 2},
                                           .width = 6,
                                           .height = 6,
                                           .percent_radius = 0.1,
                                           .rotation = kQuarterTurn})));
  EXPECT_FALSE(circle.Contains(
      ShapeWithZeroMinRadiusAndSeparation({.position = {1, 4},
                                           .width = 6,
                                           .height = 6,
                                           .percent_radius = 0.2,
                                           .rotation = -kQuarterTurn})));
}

TEST(BrushTipShapeTest, ContainsWithRoundedRectangleAndCircleEdgeCases) {
  BrushTipShape rounded_rectangle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {0, 0},
      .width = 20,
      .height = 10,
      .percent_radius = 0.5,
  });

  absl::Span<const Circle> circles = rounded_rectangle.PerimeterCircles();

  // Make circular tip states that are strictly larger than, the same size as,
  // and strictly smaller than, the size of the perimenter circles in the
  // rounded rectangle.
  BrushTipState larger_circle = {.width = 6, .height = 6, .percent_radius = 1};
  BrushTipState same_sized_circle = {
      .width = 5, .height = 5, .percent_radius = 1};
  BrushTipState smaller_circle = {.width = 4, .height = 4, .percent_radius = 1};

  float rounded_rectangle_radius = circles.front().Radius();
  ASSERT_GT(ShapeWithZeroMinRadiusAndSeparation(larger_circle)
                .PerimeterCircles()[0]
                .Radius(),
            rounded_rectangle_radius);
  ASSERT_EQ(ShapeWithZeroMinRadiusAndSeparation(same_sized_circle)
                .PerimeterCircles()[0]
                .Radius(),
            rounded_rectangle_radius);
  ASSERT_LT(ShapeWithZeroMinRadiusAndSeparation(smaller_circle)
                .PerimeterCircles()[0]
                .Radius(),
            rounded_rectangle_radius);

  for (int i = 0; i < static_cast<int>(circles.size()); ++i) {
    SCOPED_TRACE(absl::StrCat("i = ", i));

    larger_circle.position = circles[i].Center();
    EXPECT_FALSE(rounded_rectangle.Contains(
        ShapeWithZeroMinRadiusAndSeparation(larger_circle)));

    same_sized_circle.position = circles[i].Center();
    EXPECT_TRUE(rounded_rectangle.Contains(
        ShapeWithZeroMinRadiusAndSeparation(same_sized_circle)));

    smaller_circle.position = circles[i].Center();
    EXPECT_TRUE(rounded_rectangle.Contains(
        ShapeWithZeroMinRadiusAndSeparation(smaller_circle)));
  }
}

TEST(BrushTipShapeTest, ContainsWithRoundedSquares) {
  BrushTipShape large_rounded_square = ShapeWithZeroMinRadiusAndSeparation({
      .position = {0, 0},
      .width = 10,
      .height = 10,
      .percent_radius = 0.25,
      .rotation = kFullTurn / 8,
  });

  BrushTipState small_rounded_square = {
      .position = {0, 0},
      .width = 5,
      .height = 5,
      .percent_radius = 0.25,
  };

  EXPECT_TRUE(large_rounded_square.Contains(
      ShapeWithZeroMinRadiusAndSeparation(small_rounded_square)));

  small_rounded_square.position = {2, 2};
  EXPECT_FALSE(large_rounded_square.Contains(
      ShapeWithZeroMinRadiusAndSeparation(small_rounded_square)));

  small_rounded_square.position = {-2, 2};
  EXPECT_FALSE(large_rounded_square.Contains(
      ShapeWithZeroMinRadiusAndSeparation(small_rounded_square)));

  small_rounded_square.position = {-2, -2};
  EXPECT_FALSE(large_rounded_square.Contains(
      ShapeWithZeroMinRadiusAndSeparation(small_rounded_square)));

  small_rounded_square.position = {2, -2};
  EXPECT_FALSE(large_rounded_square.Contains(
      ShapeWithZeroMinRadiusAndSeparation(small_rounded_square)));
}

TEST(BrushTipShapeTest, ContainsWithRoundedTriangleEdgeCase) {
  // This test case exercises the edge case where the first perimeter circle of
  // one shape contains the first perimeter circle of the other shape while the
  // bounds of the first shape are still contained in the bounds of the second.
  //
  // This can happen when a triangular tip shape also has slant and rotation so
  // that the first perimeter circle of the shape is strictly in the interior of
  // the shape's bounds.
  BrushTipShape rounded_triangle = ShapeWithZeroMinRadiusAndSeparation({
      .position = {0, 0},
      .width = 16,
      .height = 20,
      .percent_radius = 0.1,
      .rotation = Angle::Degrees(75),
      .slant = Angle::Degrees(60),
      .pinch = 1,
  });

  ASSERT_EQ(rounded_triangle.PerimeterCircles().size(), 3u);
  Circle first_circle = rounded_triangle.PerimeterCircles()[0];

  BrushTipShape circle = ShapeWithZeroMinRadiusAndSeparation({
      .position = first_circle.Center(),
      .width = 3 * first_circle.Radius(),
      .height = 3 * first_circle.Radius(),
      .percent_radius = 1,
  });

  EXPECT_FALSE(rounded_triangle.Contains(circle));
}

}  // namespace
}  // namespace ink::strokes_internal
