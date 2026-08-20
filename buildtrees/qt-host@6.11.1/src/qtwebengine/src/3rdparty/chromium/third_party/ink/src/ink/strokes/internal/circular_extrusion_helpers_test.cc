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

#include "ink/strokes/internal/circular_extrusion_helpers.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/internal/extrusion_points.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;
using ::testing::ElementsAre;

ExtrusionPoints MakeCircularTurnExtrusionPoints(
    const geometry_internal::Circle& start,
    const geometry_internal::Circle& middle,
    const geometry_internal::Circle& end, float max_chord_height,
    AddCircularTangentIntersections add_intersections =
        AddCircularTangentIntersections::kYes) {
  ExtrusionPoints points;
  AppendCircularTurnExtrusionPoints(start, middle, end, max_chord_height,
                                    add_intersections, &points.left,
                                    &points.right);
  return points;
}

TEST(CircularTurnExtrusionPointsTest, SnapsNearlyCollinearTangents) {
  // In these cases, the tangents are just a hair off from collinear.
  // `AppendCircularExtrusionTurnPoints()` should treat them as close enough,
  // and create only one point instead of wrapping all the way around.

  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 0}, 1), Circle({0, 1}, 1), Circle({1e-6, 2}, 1), .012);
  EXPECT_THAT(turn.left.size(), 1);
  EXPECT_THAT(turn.right.size(), 1);

  turn = MakeCircularTurnExtrusionPoints(Circle({0, 0}, 1), Circle({0, 1}, 1),
                                         Circle({-1e-6, 2}, 1), .012);
  EXPECT_THAT(turn.left.size(), 1);
  EXPECT_THAT(turn.right.size(), 1);

  // This is the case that revealed b/225934641. It is three equally spaced
  // circles traveling in a straight line down and to the right. Approximately
  // the following, but needing in binary literals to be precise enough:
  //
  // ExtrusionPoints turn =
  //     MakeCircularTurnExtrusionPoints(
  //                              Circle({5.92482, 15.2368}, 0.338667),
  //                              Circle({6.11029, 15.034}, 0.338667),
  //                              Circle({6.29579, 14.8312}, 0.338667),
  //                              .004);
  turn = MakeCircularTurnExtrusionPoints(
      Circle({0x1.7b3036p+2, 0x1.e79384p+3}, 0x1.5acb7p-2),
      Circle({0x1.870f0ap+2, 0x1.e11666p+3}, 0x1.5acb7p-2),
      Circle({0x1.92ee48p+2, 0x1.da990ep+3}, 0x1.5acb7p-2), .004);
  EXPECT_THAT(turn.left.size(), 1);
  EXPECT_THAT(turn.right.size(), 1);
}

TEST(MakeCircularTurnExtrusionPointsTest, RapidlyChangingRadius) {
  // This test covers edge cases where the radius is increasing or decreasing
  // rapidly relative to distance traveled. It becomes possible to hit case 5 or
  // 6 from circular_turn_extrusion_points.svg even though the three points are
  // not making a sharp turn.

  enum class RadiusChange { kIncrease, kDecrease };
  auto make_turn_points = [](RadiusChange radius_change, Angle turn_angle) {
    // The travel distance from each point to the next is 1.0 and the radius
    // change is +/-0.99. This results in an `offset_angle` (inside
    // `GetTangentAngles()` in tip_utils.cc) of `acos(+/-0.99)`, which is ~172
    // degrees when radii are increasing and ~8 degrees when radii are
    // decreasing. This means that for each pair, the left and right tangent
    // angles will be separated by a total ~16 degrees, centered around the
    // travel direction between the given pair of circles. In this case, a
    // `turn_angle` with absolute value of 20 degrees or greater is guaranteed
    // to hit case 5 or 6.

    float dr = radius_change == RadiusChange::kIncrease ? 0.99 : -0.99;
    return MakeCircularTurnExtrusionPoints(
        Circle({0, 0}, 2), Circle({1, 0}, 2 + dr),
        Circle(Point{1, 0} + Vec::FromDirectionAndMagnitude(turn_angle, 1),
               2 + 2 * dr),
        .01);
  };
  ExtrusionPoints turn;

  // Rapidly increasing radius -> case #6 from
  // circular_turn_extrusion_points.svg: 20 degree turns - should only arc on
  // one side.
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(20));
  EXPECT_THAT(turn.left.size(), 2);
  EXPECT_THAT(turn.right.size(), 4);
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(-20));
  EXPECT_THAT(turn.left.size(), 4);
  EXPECT_THAT(turn.right.size(), 2);
  // 90 degree turns - should only arc on one side.
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(90));
  EXPECT_THAT(turn.left.size(), 2);
  EXPECT_THAT(turn.right.size(), 11);
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(-90));
  EXPECT_THAT(turn.left.size(), 11);
  EXPECT_THAT(turn.right.size(), 2);
  // 120 degree turns - deemed sharp enough to produce arcs on both sides.
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(120));
  EXPECT_THAT(turn.left.size(), 27);
  EXPECT_THAT(turn.right.size(), 14);
  turn = make_turn_points(RadiusChange::kIncrease, Angle::Degrees(-120));
  EXPECT_THAT(turn.left.size(), 14);
  EXPECT_THAT(turn.right.size(), 27);

  // Rapidly decreasing radius -> case #5 from
  // circular_turn_extrusion_points.svg: 20 degree turns - should only arc on
  // one side.
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(20));
  EXPECT_THAT(turn.left.size(), 2);
  EXPECT_THAT(turn.right.size(), 3);
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(-20));
  EXPECT_THAT(turn.left.size(), 3);
  EXPECT_THAT(turn.right.size(), 2);
  // 90 degree turns - should only arc on one side.
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(90));
  EXPECT_THAT(turn.left.size(), 2);
  EXPECT_THAT(turn.right.size(), 7);
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(-90));
  EXPECT_THAT(turn.left.size(), 7);
  EXPECT_THAT(turn.right.size(), 2);
  // 120 degree turns - deemed sharp enough to produce arcs on both sides.
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(120));
  EXPECT_THAT(turn.left.size(), 16);
  EXPECT_THAT(turn.right.size(), 9);
  turn = make_turn_points(RadiusChange::kDecrease, Angle::Degrees(-120));
  EXPECT_THAT(turn.left.size(), 9);
  EXPECT_THAT(turn.right.size(), 16);
}

TEST(MakeCircularTurnExtrusionPointsTest, RightTurn) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 0}, 15), Circle({50, 0}, 20), Circle({100, -50}, 40), 0.35);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({48, 19.9}, 0.01),
                                     PointNear({53.96, 19.6}, 0.01),
                                     PointNear({59.56, 17.56}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({37.49, -18.84}, .01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, LeftTurn) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 0}, 60), Circle({50, -50}, 55), Circle({120, 0}, 50), 1.5);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({60.62, 21.69}, .01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({13.96, -91.54}, 0.01),
                                      PointNear({36.38, -103.29}, 0.01),
                                      PointNear({61.68, -103.74}, 0.01),
                                      PointNear({84.52, -92.82}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, SmallCircleBetweenBigCircles) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, -50}, 40), Circle({0, 0}, 10), Circle({-20, 40}, 40), 0.5);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({-15.90, -4.53}, .01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({10.95, 2.06}, .01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, BigCircleBetweenSmallCircles) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({-10, 0}, 10), Circle({50, 0}, 50), Circle({100, 15}, 10), 1.25);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({16.67, 37.27}, 0.01),
                                     PointNear({35.58, 47.87}, 0.01),
                                     PointNear({57.20, 49.48}, 0.01),
                                     PointNear({77.47, 41.78}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({16.67, -37.27}, 0.01),
                                      PointNear({32.81, -46.95}, 0.01),
                                      PointNear({51.40, -49.98}, 0.01),
                                      PointNear({69.78, -45.92}, 0.01),
                                      PointNear({85.36, -35.35}, 0.01),
                                      PointNear({95.93, -19.76}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, BacktrackDecreasingSize) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({50, -50}, 40), Circle({0, 0}, 30), Circle({30, -30}, 20), 1.75);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({-24.00, -18.00}, 0.01),
                                     PointNear({-30.00, 0.12}, 0.01),
                                     PointNear({-23.85, 18.19}, 0.01),
                                     PointNear({-8.05, 28.90}, 0.01),
                                     PointNear({11.01, 27.91}, 0.01),
                                     PointNear({25.62, 15.62}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({18.00, 24.00}, 0.01),
                                      PointNear({-0.12, 30.00}, 0.01),
                                      PointNear({-18.19, 23.85}, 0.01),
                                      PointNear({-28.90, 8.05}, 0.01),
                                      PointNear({-27.91, -11.01}, 0.01),
                                      PointNear({-15.62, -25.62}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, BacktrackIncreasingSize) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({50, 0}, 10), Circle({0, 0}, 50), Circle({80, 0}, 60), 2);
  EXPECT_THAT(
      turn.left,
      ElementsAre(
          PointNear({40.00, -30.00}, 0.01), PointNear({17.80, -46.73}, 0.01),
          PointNear({-9.91, -49.01}, 0.01), PointNear({-34.55, -36.15}, 0.01),
          PointNear({-48.51, -12.11}, 0.01), PointNear({-47.48, 15.67}, 0.01),
          PointNear({-31.78, 38.60}, 0.01), PointNear({-6.25, 49.61}, 0.01)));
  EXPECT_THAT(
      turn.right,
      ElementsAre(
          PointNear({40.00, 30.00}, 0.01), PointNear({17.80, 46.73}, 0.01),
          PointNear({-9.91, 49.01}, 0.01), PointNear({-34.55, 36.15}, 0.01),
          PointNear({-48.51, 12.11}, 0.01), PointNear({-47.48, -15.67}, 0.01),
          PointNear({-31.78, -38.60}, 0.01), PointNear({-6.25, -49.61}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, LeftTurnNonIntersectingTangents) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 0}, 15), Circle({50, 10}, 15), Circle({0, 20}, 15), 0.9);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({47.06, 24.71}, 0.01),
                                     PointNear({47.06, -4.71}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({52.94, -4.71}, 0.01),
                                      PointNear({61.60, 0.49}, 0.01),
                                      PointNear({65.00, 10.00}, 0.01),
                                      PointNear({61.60, 19.51}, 0.01),
                                      PointNear({52.94, 24.71}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, RightTurnNonIntersectingTangents) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 20}, 15), Circle({50, 10}, 15), Circle({0, 0}, 15), 0.9);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({52.94, 24.71}, 0.01),
                                     PointNear({61.60, 19.51}, 0.01),
                                     PointNear({65.00, 10.00}, 0.01),
                                     PointNear({61.60, 0.49}, 0.01),
                                     PointNear({52.94, -4.71}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({47.06, -4.71}, 0.01),
                                      PointNear({47.06, 24.71}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, ParallelRightTangents) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 10}, 10), Circle({50, 20}, 20), Circle({100, 5}, 5), 0.8);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({42.31, 38.46}, 0.01),
                                     PointNear({51.88, 39.91}, 0.01),
                                     PointNear({61.01, 36.70}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({50, 0}, .01)));
}

TEST(MakeCircularTurnExtrusionPointsTest, ParallelLeftTangents) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({20, -40}, 20), Circle({15, 0}, 15), Circle({5, 30}, 5), 0.7);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({0, 0}, .01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({29.54, 3.69}, 0.01),
                                      PointNear({27.00, 9.00}, 0.01)));
}

TEST(MakeCircularTurnExtrusionPointsDeathTest,
     MiddlePointContainedInStartPoint) {
  EXPECT_DEATH_IF_SUPPORTED(
      MakeCircularTurnExtrusionPoints(Circle({0, 0}, 30), Circle({10, 0}, 10),
                                      Circle({50, 20}, 10), 0.4),
      "");
}

TEST(MakeCircularTurnExtrusionPointsDeathTest, MiddlePointContainedInEndPoint) {
  EXPECT_DEATH_IF_SUPPORTED(
      MakeCircularTurnExtrusionPoints(Circle({0, 0}, 5), Circle({30, 0}, 10),
                                      Circle({40, 0}, 25), 0.25),
      "");
}

TEST(MakeCircularTurnExtrusionPointsDeathTest,
     MiddlePointContainedInBothAdjacentPoints) {
  EXPECT_DEATH_IF_SUPPORTED(
      MakeCircularTurnExtrusionPoints(Circle({0, 0}, 30), Circle({20, 0}, 5),
                                      Circle({40, 0}, 30), 0.1),
      "");
}

TEST(AppendCircularTurnExtrusionPointsTest, NullptrPointsParameters) {
  ExtrusionPoints turn;
  AppendCircularTurnExtrusionPoints(
      Circle({0, 20}, 15), Circle({50, 10}, 15), Circle({0, 0}, 15), 0.9,
      AddCircularTangentIntersections::kYes, &turn.left, nullptr);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({52.94, 24.71}, 0.01),
                                     PointNear({61.60, 19.51}, 0.01),
                                     PointNear({65.00, 10.00}, 0.01),
                                     PointNear({61.60, 0.49}, 0.01),
                                     PointNear({52.94, -4.71}, 0.01)));

  AppendCircularTurnExtrusionPoints(
      Circle({0, 0}, 15), Circle({50, 10}, 15), Circle({0, 20}, 15), 0.9,
      AddCircularTangentIntersections::kYes, nullptr, &turn.right);
  EXPECT_THAT(turn.right, ElementsAre(PointNear({52.94, -4.71}, 0.01),
                                      PointNear({61.60, 0.49}, 0.01),
                                      PointNear({65.00, 10.00}, 0.01),
                                      PointNear({61.60, 19.51}, 0.01),
                                      PointNear({52.94, 24.71}, 0.01)));
}

TEST(AppendCircularTurnExtrusionPointsTest, KeepsExistingPoints) {
  ExtrusionPoints turn = MakeCircularTurnExtrusionPoints(
      Circle({0, 10}, 10), Circle({50, 20}, 20), Circle({100, 5}, 5), 0.8);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({42.31, 38.46}, 0.01),
                                     PointNear({51.88, 39.91}, 0.01),
                                     PointNear({61.01, 36.70}, 0.01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({50, 0}, .01)));

  AppendCircularTurnExtrusionPoints(
      Circle({20, -40}, 20), Circle({15, 0}, 15), Circle({5, 30}, 5), 0.7,
      AddCircularTangentIntersections::kYes, &turn.left, &turn.right);
  EXPECT_THAT(turn.left, ElementsAre(PointNear({42.31, 38.46}, 0.01),
                                     PointNear({51.88, 39.91}, 0.01),
                                     PointNear({61.01, 36.70}, 0.01),
                                     PointNear({0, 0}, .01)));
  EXPECT_THAT(turn.right, ElementsAre(PointNear({50, 0}, .01),
                                      PointNear({29.54, 3.69}, 0.01),
                                      PointNear({27.00, 9.00}, 0.01)));
}

TEST(AppendCircularTurnExtrusionPointsTest, DisabledTangentIntersections) {
  Circle circles[3] = {
      Circle({0, 0}, 10),
      Circle({10, 0}, 5),
      Circle({20, 0}, 10),
  };

  // These circles should create case #3 from
  // circular_turn_extrusion_points.svg, which should result in adding
  // intersections points on both sides if enabled.
  ExtrusionPoints turn_with_intersections =
      MakeCircularTurnExtrusionPoints(circles[0], circles[1], circles[2], 1,
                                      AddCircularTangentIntersections::kYes);
  ASSERT_EQ(turn_with_intersections.left.size(), 1);
  ASSERT_EQ(turn_with_intersections.right.size(), 1);

  ExtrusionPoints turn_without_intersections =
      MakeCircularTurnExtrusionPoints(circles[0], circles[1], circles[2], 1,
                                      AddCircularTangentIntersections::kNo);
  EXPECT_THAT(
      turn_without_intersections.left,
      ElementsAre(PointNear({12.5, 4.33}, 0.01), PointNear({15, 0}, 0.01),
                  PointNear({12.5, -4.33}, 0.01), PointNear({7.5, -4.33}, 0.01),
                  PointNear({5, 0}, 0.01), PointNear({7.5, 4.33}, 0.01)));
  EXPECT_THAT(
      turn_without_intersections.right,
      ElementsAre(PointNear({12.5, -4.33}, 0.01), PointNear({15, 0}, 0.01),
                  PointNear({12.5, 4.33}, 0.01), PointNear({7.5, 4.33}, 0.01),
                  PointNear({5, 0}, 0.01), PointNear({7.5, -4.33}, 0.01)));
}

}  // namespace
}  // namespace ink::strokes_internal
