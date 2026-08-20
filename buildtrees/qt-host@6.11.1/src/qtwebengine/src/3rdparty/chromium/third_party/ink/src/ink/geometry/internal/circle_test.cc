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

#include "ink/geometry/internal/circle.h"

#include <cmath>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_format.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::FloatNear;

// Matcher for checking that every `Point` in the `arg` array lies on the
// `circle`.
MATCHER_P(PointsLieOnCircle, circle, "") {
  for (int i = 0; i < arg.size(); ++i) {
    float distance_to_center = (arg[i] - circle.Center()).Magnitude();
    if (!ExplainMatchResult(FloatNear(circle.Radius(), 0.0001),
                            distance_to_center, result_listener)) {
      *result_listener << absl::StrFormat(
          "Point arg[%d] = (%f, %f) does not lie on the circle with center = "
          "(%f, %f), radius = %f. Distance to center = %f",
          i, arg[i].x, arg[i].y, circle.Center().x, circle.Center().y,
          circle.Radius(), distance_to_center);
      return false;
    }
  }
  return true;
}

// Matcher for checking that the results of `Circle::AppendArcToPolyline()` are
// equally spaced apart.
MATCHER(PointsAreEquallySpaced, "") {
  if (arg.size() < 3) return true;

  float first_segment_length = (arg[1] - arg[0]).Magnitude();
  for (int i = 1; i < arg.size(); ++i) {
    float current_segment_length = (arg[i] - arg[i - 1]).Magnitude();
    if (!ExplainMatchResult(FloatNear(first_segment_length, 0.0001),
                            current_segment_length, result_listener)) {
      *result_listener << absl::StrFormat(
          "Distance between arg[%d] = (%f, %f) and arg[%d] = (%f, %f) is not "
          "the same as the distance between the first two points.",
          i - 1, arg[i - 1].x, arg[i - 1].y, i, arg[i].x, arg[i].y);
      return false;
    }
  }

  return true;
}

// Matcher for iterating over the results of `Circle::AppendArcToPolyline()` and
// checking that each segment of the polyline has chord height not exceeding the
// specified value.
MATCHER_P2(ChordHeightsAreLessThan, circle, max_chord_height, "") {
  for (int i = 1; i < arg.size(); ++i) {
    Point chord_midpoint = Segment{arg[i - 1], arg[i]}.Midpoint();
    Vec center_to_midpoint = chord_midpoint - circle.Center();
    float chord_height;
    if (center_to_midpoint == Vec{0, 0}) {
      chord_height = circle.Radius();
    } else {
      chord_height =
          (circle.Center() + circle.Radius() * center_to_midpoint.AsUnitVec() -
           chord_midpoint)
              .Magnitude();
    }
    if (chord_height > max_chord_height) {
      *result_listener << absl::StrFormat(
          "Segment from arg[%d] = (%f, %f) to arg[%d] = (%f, %f) has "
          "chord_height = %f, which exceeds max_cord_height = %f",
          i - 1, arg[i - 1].x, arg[i - 1].y, i, arg[i].x, arg[i].y,
          chord_height, max_chord_height);
      return false;
    }
  }
  return true;
}

TEST(CircleTest, DefaultConstructed) {
  Circle c;
  EXPECT_THAT(c.Center(), PointEq({0, 0}));
  EXPECT_EQ(c.Radius(), 1);
}

TEST(CircleTest, ConstructedFromCenterAndRadius) {
  Circle c({-4, 7}, 5);
  EXPECT_THAT(c.Center(), PointEq({-4, 7}));
  EXPECT_EQ(c.Radius(), 5);
}

TEST(CircleTest, ConstructWithZeroRadius) {
  Circle c({-4, 7}, 0);
  EXPECT_THAT(c.Center(), PointEq({-4, 7}));
  EXPECT_EQ(c.Radius(), 0);
}

TEST(CircleTest, GetPoint) {
  Circle circle({1, 2}, 3.f);

  EXPECT_THAT(circle.GetPoint(Angle()), PointNear({4, 2}, 0.001));
  EXPECT_THAT(circle.GetPoint(kQuarterTurn), PointNear({1, 5}, 0.001));
  EXPECT_THAT(circle.GetPoint(kHalfTurn), PointNear({-2, 2}, 0.001));
  EXPECT_THAT(circle.GetPoint(kFullTurn), PointNear({4, 2}, 0.001));
  EXPECT_THAT(circle.GetPoint(-kFullTurn / 6),
              PointNear({2.5f, 2 - 1.5f * std::sqrt(3.f)}, 0.001));
}

TEST(CircleTest, GetTangentAnglesWithCoincidentCircles) {
  Point center = {15, 10};
  float radius = 7;
  Circle a(center, radius);
  Circle b(center, radius);
  EXPECT_THAT(a.GetTangentAngles(b), Eq(std::nullopt));
}

TEST(CircleTest, GetTangentAnglesSinglePointOfContact) {
  Circle a({0, 0}, 20);
  Circle b({10, 0}, 10);
  EXPECT_THAT(a.GetTangentAngles(b), Eq(std::nullopt));
  EXPECT_THAT(b.GetTangentAngles(a), Eq(std::nullopt));
}

TEST(CircleTest, GetTangentAnglesOneCircleContainsTheOther) {
  Circle a({-20, 5}, 30);
  Circle b({-15, 0}, 15);
  EXPECT_THAT(a.GetTangentAngles(b), Eq(std::nullopt));
  EXPECT_THAT(b.GetTangentAngles(a), Eq(std::nullopt));
}

TEST(CircleTest, GetTangentAnglesSameRadius) {
  Circle a({0, 0}, 2);
  Circle b({5, 5}, 2);
  auto tangents = a.GetTangentAngles(b);
  EXPECT_TRUE(tangents.has_value());
  EXPECT_THAT(tangents->left, AngleEq(3 * kFullTurn / 8));
  EXPECT_THAT(tangents->right, AngleEq(-kFullTurn / 8));
}

TEST(CircleTest, GetTangentAnglesDifferentRadii) {
  Circle a({0, 0}, 2);
  Circle b({-2, 0}, 1);
  auto tangents = a.GetTangentAngles(b);
  EXPECT_THAT(tangents->left, AngleEq(-kFullTurn / 3));
  EXPECT_THAT(tangents->right, AngleEq(kFullTurn / 3));
}

TEST(CircleTest, GetGuaranteedRightTangentAngle) {
  Circle a({0, 0}, 2);
  Circle b({5, 5}, 2);
  auto tangents = a.GetTangentAngles(b);
  ASSERT_TRUE(tangents.has_value());
  EXPECT_THAT(a.GuaranteedRightTangentAngle(b), AngleEq(tangents->right));

  a = Circle({0, 0}, 2);
  b = Circle({-2, 0}, 1);
  tangents = a.GetTangentAngles(b);
  ASSERT_TRUE(tangents.has_value());
  EXPECT_THAT(a.GuaranteedRightTangentAngle(b), AngleEq(tangents->right));
}

TEST(CircleTest, AppendArcToPolylinePositiveArcAngle) {
  Circle circle({10, 30}, 10);
  std::vector<Point> polyline;
  float max_chord_height = 1;
  Angle starting_angle = Angle();
  Angle arc_angle = kQuarterTurn;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 3);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineNegativeArcAngle) {
  Circle circle({5, 8}, 2);
  std::vector<Point> polyline;
  float max_chord_height = .1;
  Angle starting_angle = kQuarterTurn;
  Angle arc_angle = -kHalfTurn;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 6);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineFullCircle) {
  Circle circle({10, -5}, 4);
  std::vector<Point> polyline;
  float max_chord_height = .1;
  Angle starting_angle = Angle();
  Angle arc_angle = kFullTurn;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 16);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineZeroArcAngle) {
  Circle circle({5, 8}, 1);
  std::vector<Point> polyline;
  float max_chord_height = .05;
  Angle starting_angle = kFullTurn * 0.75f;
  Angle arc_angle = Angle();
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 2);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineMoreThanFullCircle) {
  Circle circle({50, 88}, 20);
  std::vector<Point> polyline;
  float max_chord_height = 5;
  Angle starting_angle = Angle::Radians(1);
  Angle arc_angle = 3 * kFullTurn;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 15);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineMaxChordHeightGreaterThanRadius) {
  Circle circle({5, 2}, 2);
  std::vector<Point> polyline;
  float max_chord_height = 2.5;
  Angle starting_angle = Angle::Radians(1);
  Angle arc_angle = kFullTurn * 0.75f;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 3);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineMaxChordHeightGreaterThanDiameter) {
  Circle circle({-10, -16}, 2);
  std::vector<Point> polyline;
  float max_chord_height = 5;
  Angle starting_angle = Angle::Radians(1);
  Angle arc_angle = kFullTurn * 0.75f;
  circle.AppendArcToPolyline(starting_angle, arc_angle, max_chord_height,
                             polyline);

  EXPECT_EQ(polyline.size(), 2);
  EXPECT_THAT(polyline.front(),
              PointNear(circle.GetPoint(starting_angle), 0.001));
  EXPECT_THAT(polyline.back(),
              PointNear(circle.GetPoint(starting_angle + arc_angle), 0.001));
  EXPECT_THAT(polyline, PointsLieOnCircle(circle));
  EXPECT_THAT(polyline, PointsAreEquallySpaced());
  EXPECT_THAT(polyline, ChordHeightsAreLessThan(circle, max_chord_height));
}

TEST(CircleTest, AppendArcToPolylineDegenerateCircle) {
  Circle circle({5, 6}, 0);
  std::vector<Point> polyline;
  circle.AppendArcToPolyline(-kQuarterTurn, kHalfTurn, 0.01, polyline);

  EXPECT_THAT(polyline, ElementsAre(PointEq({5, 6}), PointEq({5, 6})));
}

TEST(CircleTest, AppendArcToPolylineKeepsPriorContents) {
  Circle circle({5, 6}, 0);
  std::vector<Point> polyline = {{-1, -1}, {0, 3}};
  circle.AppendArcToPolyline(-kQuarterTurn, kHalfTurn, 0.01, polyline);
  EXPECT_THAT(polyline, ElementsAre(PointEq({-1, -1}), PointEq({0, 3}),
                                    PointEq({5, 6}), PointEq({5, 6})));
}

TEST(CircleTest, GetArcAngleForChordHeightZeroRadius) {
  Circle circle({1, 2}, 0);

  EXPECT_THAT(circle.GetArcAngleForChordHeight(-1), AngleEq(Angle()));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(0), AngleEq(Angle()));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(1), AngleEq(Angle()));
}

TEST(CircleTest, GetArcAngleForChordHeightNonZeroRadius) {
  Circle circle({1, 2}, 10);

  EXPECT_THAT(circle.GetArcAngleForChordHeight(-1).ValueInRadians(),
              FloatNear(0, 0.001));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(0).ValueInRadians(),
              FloatNear(0, 0.001));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(0.05).ValueInRadians(),
              FloatNear(0.2, 0.001));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(10).ValueInRadians(),
              FloatNear(kHalfTurn.ValueInRadians(), 0.001));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(20).ValueInRadians(),
              FloatNear(kFullTurn.ValueInRadians(), 0.001));
  EXPECT_THAT(circle.GetArcAngleForChordHeight(21).ValueInRadians(),
              FloatNear(kFullTurn.ValueInRadians(), 0.001));
}

TEST(CircleTest, Contains) {
  EXPECT_TRUE(Circle({0, -2}, 0).Contains(Circle({0, -2}, 0)));
  EXPECT_TRUE(Circle({3, 5}, 7).Contains(Circle({3, 5}, 7)));

  EXPECT_TRUE(Circle({10, 10}, 8).Contains(Circle({10, 5}, 3)));
  EXPECT_FALSE(Circle({10, 5}, 3).Contains(Circle({10, 10}, 8)));

  EXPECT_FALSE(Circle({1, 1}, 4).Contains(Circle({1, -1}, 5)));
  EXPECT_FALSE(Circle({1, -1}, 5).Contains(Circle({1, 1}, 4)));

  EXPECT_FALSE(Circle({0, 0}, 2).Contains(Circle({4, 0}, 1)));
  EXPECT_FALSE(Circle({4, 0}, 1).Contains(Circle({0, 0}, 2)));
}

TEST(CircleDeathTest, ConstructWithNegativeRadius) {
  EXPECT_DEATH_IF_SUPPORTED(Circle circle({1, 2}, -1), "");
}

TEST(CircleDeathTest, AppendArcToPolylineZeroChordHeight) {
  Circle circle;
  std::vector<Point> polyline;
  EXPECT_DEATH_IF_SUPPORTED(
      circle.AppendArcToPolyline(kHalfTurn, kFullTurn, 0, polyline), "");
}

TEST(CircleDeathTest, AppendArcToPolylineNegativeChordHeight) {
  Circle circle;
  std::vector<Point> polyline;
  EXPECT_DEATH_IF_SUPPORTED(
      circle.AppendArcToPolyline(kHalfTurn, kFullTurn, -1, polyline), "");
}

}  // namespace
}  // namespace ink::geometry_internal
