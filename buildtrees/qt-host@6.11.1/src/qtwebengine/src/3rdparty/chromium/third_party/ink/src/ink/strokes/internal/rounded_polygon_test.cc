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

#include "ink/strokes/internal/rounded_polygon.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/internal/circle.h"
#include "ink/geometry/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ::ink::geometry_internal::Circle;

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::PrintToString;
using ::testing::Property;

MATCHER_P2(ArcNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't equal" : "equals",
                        " Arc (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(
          Field(
              "circle", &RoundedPolygon::Arc::circle,
              AllOf(Property("Center", &Circle::Center,
                             PointNear(expected.circle.Center(), tolerance)),
                    Property("Radius", &Circle::Radius,
                             FloatNear(expected.circle.Radius(), tolerance)))),
          Field("start_unit_vector", &RoundedPolygon::Arc::start_unit_vector,
                VecNear(expected.start_unit_vector, tolerance)),
          Field("end_unit_vector", &RoundedPolygon::Arc::end_unit_vector,
                VecNear(expected.end_unit_vector, tolerance))),
      arg, result_listener);
}
Matcher<RoundedPolygon::Arc> ArcNear(const RoundedPolygon::Arc& arc,
                                     float tolerance) {
  return ArcNearMatcher(arc, tolerance);
}

TEST(RoundedPolygonTest, ConstructWithTwoCircles) {
  RoundedPolygon poly({Circle({0, 0}, 0.5), Circle({2, 0.5}, 1)});

  EXPECT_THAT(poly.Arcs(),
              ElementsAre(ArcNear({.circle = Circle({0, 0}, 0.5),
                                   .start_unit_vector = {-0.4706, 0.8824},
                                   .end_unit_vector = {0, -1}},
                                  1e-4),
                          ArcNear({.circle = Circle({2, 0.5}, 1),
                                   .start_unit_vector = {0, -1},
                                   .end_unit_vector = {-0.4706, 0.8824}},
                                  1e-4)));
  EXPECT_THAT(poly.GetSegment(0), SegmentNear({{0, -0.5}, {2, -0.5}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(1),
              SegmentNear({{1.5294, 1.3824}, {-0.2353, 0.4412}}, 1e-4));
}

TEST(RoundedPolygonTest, ConstructWithThreeCircles) {
  RoundedPolygon poly({Circle({0, 0}, 0.5), Circle({3, 0.75}, 1.25),
                       Circle({0.25, 2.5}, 0.75)});

  EXPECT_THAT(poly.Arcs(),
              ElementsAre(ArcNear({.circle = Circle({0, 0}, 0.5),
                                   .start_unit_vector = {-1, 0},
                                   .end_unit_vector = {0, -1}},
                                  1e-4),
                          ArcNear({.circle = Circle({3, 0.75}, 1.25),
                                   .start_unit_vector = {0, -1},
                                   .end_unit_vector = {0.4011, 0.9160}},
                                  1e-4),
                          ArcNear({.circle = Circle({0.25, 2.5}, 0.75),
                                   .start_unit_vector = {0.4011, 0.9160},
                                   .end_unit_vector = {-1, 0}},
                                  1e-4)));
  EXPECT_THAT(poly.GetSegment(0), SegmentNear({{0, -0.5}, {3, -0.5}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(1),
              SegmentNear({{3.5014, 1.8950}, {0.5508, 3.1870}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(2), SegmentNear({{-0.5, 2.5}, {-0.5, 0}}, 1e-4));
}

TEST(RoundedPolygonTest, ConstructWithFourCircles) {
  RoundedPolygon poly({Circle({1, 1}, 1), Circle({9, 3}, 3), Circle({10, 9}, 2),
                       Circle({1.5, 9.5}, 1.5)});

  EXPECT_THAT(poly.Arcs(),
              ElementsAre(ArcNear({.circle = Circle({1, 1}, 1),
                                   .start_unit_vector = {-1, 0},
                                   .end_unit_vector = {0, -1}},
                                  1e-4),
                          ArcNear({.circle = Circle({9, 3}, 3),
                                   .start_unit_vector = {0, -1},
                                   .end_unit_vector = {1, 0}},
                                  1e-4),
                          ArcNear({.circle = Circle({10, 9}, 2),
                                   .start_unit_vector = {1, 0},
                                   .end_unit_vector = {0, 1}},
                                  1e-4),
                          ArcNear({.circle = Circle({1.5, 9.5}, 1.5),
                                   .start_unit_vector = {0, 1},
                                   .end_unit_vector = {-1, 0}},
                                  1e-4)));
  EXPECT_THAT(poly.GetSegment(0), SegmentNear({{1, 0}, {9, 0}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(1), SegmentNear({{12, 3}, {12, 9}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(2), SegmentNear({{10, 11}, {1.5, 11}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(3), SegmentNear({{0, 9.5}, {0, 1}}, 1e-4));
}

TEST(RoundedPolygonTest, ConstructWithZeroRadiusCircle) {
  RoundedPolygon poly({Circle({0, 0}, 1), Circle({1, 1}, 0)});

  EXPECT_THAT(poly.Arcs(), ElementsAre(ArcNear({.circle = Circle({0, 0}, 1),
                                                .start_unit_vector = {0, 1},
                                                .end_unit_vector = {1, 0}},
                                               1e-4),
                                       ArcNear({.circle = Circle({1, 1}, 0),
                                                .start_unit_vector = {1, 0},
                                                .end_unit_vector = {0, 1}},
                                               1e-4)));
  EXPECT_THAT(poly.GetSegment(0), SegmentNear({{1, 0}, {1, 1}}, 1e-4));
  EXPECT_THAT(poly.GetSegment(1), SegmentNear({{1, 1}, {0, 1}}, 1e-4));
}

TEST(RoundedPolygonTest, DoesNotContainCircleExteriorToSegment) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{10, 2}, 1}));
  EXPECT_FALSE(poly.ContainsCircle({{2, 15}, 5}));
  EXPECT_FALSE(poly.ContainsCircle({{-5, 5}, 2}));
  EXPECT_FALSE(poly.ContainsCircle({{4, -8}, 3}));
}

TEST(RoundedPolygonTest, DoesNotContainCircleExteriorToArc) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{10, 10}, 2}));
  EXPECT_FALSE(poly.ContainsCircle({{-5, 12}, 1}));
  EXPECT_FALSE(poly.ContainsCircle({{-4, -12}, 3}));
  EXPECT_FALSE(poly.ContainsCircle({{15, -10}, 4}));
}

TEST(RoundedPolygonTest, DoesNotContainCircleExteriorToMajorArc) {
  RoundedPolygon poly({Circle({0, 0}, 5), Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{-5, -5}, 1}));
}

TEST(RoundedPolygonTest, DoesNotContainCircleStraddlingSegment) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{9, 2}, 2}));
  EXPECT_FALSE(poly.ContainsCircle({{2, 6.5}, 1}));
  EXPECT_FALSE(poly.ContainsCircle({{-3, 4}, 3}));
  EXPECT_FALSE(poly.ContainsCircle({{5, 0}, 1.5}));
}

TEST(RoundedPolygonTest, DoesNotContainCircleStraddlingArc) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{6, 6}, 3}));
  EXPECT_FALSE(poly.ContainsCircle({{-1.5, 7.5}, 1}));
  EXPECT_FALSE(poly.ContainsCircle({{0, 0}, 2}));
  EXPECT_FALSE(poly.ContainsCircle({{8, -1}, 0.5}));
}

TEST(RoundedPolygonTest, DoesNotContainCircleStraddlingMajorArc) {
  RoundedPolygon poly({Circle({0, 0}, 5), Circle({7, 0}, 1)});

  EXPECT_FALSE(poly.ContainsCircle({{4, 3}, 3}));
}

TEST(RoundedPolygonTest, ContainsCircleInteriorToSegment) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_TRUE(poly.ContainsCircle({{6, 1}, 0.5}));
  EXPECT_TRUE(poly.ContainsCircle({{3, 5}, 1.5}));
  EXPECT_TRUE(poly.ContainsCircle({{-0.5, 4}, 0.2}));
  EXPECT_TRUE(poly.ContainsCircle({{4, 0}, 0.3}));
}

TEST(RoundedPolygonTest, ContainsCircleInteriorToArc) {
  RoundedPolygon poly({Circle({5, 4}, 3), Circle({-1, 7}, 0), Circle({1, 1}, 2),
                       Circle({7, 0}, 1)});

  EXPECT_TRUE(poly.ContainsCircle({{6, 5}, 1}));
  EXPECT_TRUE(poly.ContainsCircle({{0.5, 0.5}, 0.8}));
  EXPECT_TRUE(poly.ContainsCircle({{7.5, -0.5}, 0.2}));
}

TEST(RoundedPolygonTest, ContainsCircleInteriorToMajorArc) {
  RoundedPolygon poly({Circle({0, 0}, 5), Circle({7, 0}, 1)});

  EXPECT_TRUE(poly.ContainsCircle({{-2, 1}, 2}));
}

TEST(RoundedPolygonTest, ContainsCircleHandlesPrecisionLossInArcVectors) {
  // The first three and last three centers of `poly`'s circles are collinear,
  // forming a rotated rectangle with extra points on the bottom-right and
  // top-left edges.
  //
  //       o
  //      / \
  //     /   \
  //    /     \
  //   o       o
  //  /       /
  // o       /
  //  \  x  /
  //   \   o
  //    \ /
  //     o
  //
  // This case results in one of the arcs (middle of the bottom-right edge)
  // starting at -45 degrees, and ending at -45.000004 degrees, meaning that it
  // nominally loops all the way around, so *any* point would be "between" the
  // two vectors and the helper function `SignedDistanceToArc` would return a
  // signed distance when the arc is not the closest point on the boundary. The
  // tolerance in `SignedDistanceToArc` ensures that it is instead recognized as
  // a degenerate arc, that contains nothing.
  const float kRadius = 6.6988391e-06;
  RoundedPolygon poly({{{-3.06082225, 2.88892508}, kRadius},
                       {{1.88892508, -2.06082225}, kRadius},
                       {{2.16330528, -1.78644204}, kRadius},
                       {{5.69883919, 1.74909163}, kRadius},
                       {{0.74909163, 6.69883919}, kRadius},
                       {{-2.78644204, 3.16330528}, kRadius}});
  EXPECT_TRUE(poly.ContainsCircle({{.42445850, 1.47471142}, 0}));
}

TEST(RoundedPolygonDeathTest, ConstructWithLessThanTwoCircles) {
  EXPECT_DEATH_IF_SUPPORTED(RoundedPolygon({}), "");
  EXPECT_DEATH_IF_SUPPORTED(RoundedPolygon({Circle({0, 0}, 1)}), "");
}

TEST(RoundedPolygonDeathTest,
     ConstructWithAdjacentCirclesThatContainOneAnother) {
  EXPECT_DEATH_IF_SUPPORTED(
      RoundedPolygon({Circle({0, 0}, 1), Circle({1, 0}, 3), Circle({7, 0}, 1)}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      RoundedPolygon(
          {Circle({0, 0}, 2), Circle({1, 0}, 0.5), Circle({7, 0}, 1)}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      RoundedPolygon(
          {Circle({0, 0}, 2), Circle({5, 0}, 0.5), Circle({0, 0}, 1)}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      RoundedPolygon(
          {Circle({0, 0}, 2), Circle({5, 0}, 0.5), Circle({0, 0}, 3)}),
      "");
}

TEST(RoundedPolygonDeathTest, GetSegmentWithOutOfBoundsIndex) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  RoundedPolygon poly(
      {Circle({0, 0}, 1), Circle({4, 0}, 2), Circle({3, 3}, 0.5)});

  EXPECT_DEATH_IF_SUPPORTED(poly.GetSegment(-1), "");
  EXPECT_DEATH_IF_SUPPORTED(poly.GetSegment(3), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

}  // namespace
}  // namespace ink::strokes_internal
