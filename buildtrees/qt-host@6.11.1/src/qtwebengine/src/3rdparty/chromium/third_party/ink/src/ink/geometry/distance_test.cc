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

#include "ink/geometry/distance.h"

#include <cmath>

#include "gtest/gtest.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace {

TEST(DistanceTest, PointToPoint) {
  // Distance in the positive and negative x and y directions
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{5.0f, 1.0f}), 4.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{1.0f, 5.0f}), 4.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{-3.0f, 1.0f}), 4.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{1.0f, -3.0f}), 4.0f);

  EXPECT_FLOAT_EQ(Distance(Point{30.0f, -50.0f}, Point{-15.0f, -50.0f}), 45.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-15.0f, 20.0f}, Point{-15.0f, -50.0f}), 70.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-20015.0f, -50.0f}, Point{-15.0f, -50.0f}),
                  20000.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-15.0f, -50050.0f}, Point{-15.0f, -50.0f}),
                  50000.0f);

  // Distance where the points are diagonal to one another.
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{4.0f, 5.0f}), 5.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{-29.0f, -39.0f}), 50.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{31.0f, -39.0f}), 50.0f);
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{-29.0f, 41.0f}), 50.0f);

  // Zero distance
  EXPECT_FLOAT_EQ(Distance(Point{1.0f, 1.0f}, Point{1.0f, 1.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-15.0f, 11.0f}, Point{-15.0f, 11.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{23.0f, -66.0f}, Point{23.0f, -66.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-0.02f, -4.9f}, Point{-0.02f, -4.9f}), 0.0f);
}

TEST(DistanceTest, PointToSegmentEndpointClosest) {
  EXPECT_FLOAT_EQ(
      Distance(Point{-10.0f, 1.0f}, Segment{{8.0f, 1.0f}, {18.0f, 1.0f}}),
      18.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{18.0f, 11.0f}, {8.0f, 1.0f}}, Point{-10.0f, 1.0f}),
      18.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-5.0f, 16.0f}, {-5.0f, 60.0f}}, Point{-5.0f, -9.0f}),
      25.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, -9.0f}, Segment{{36.6f, 16.0f}, {-5.0f, 16.0f}}),
      25.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-0.01f, -50.0f}, {-5.0f, -50.0f}}, Point{0.3f, -50.0f}),
      0.31f);
  EXPECT_FLOAT_EQ(
      Distance(Point{0.3f, -50.0f}, Segment{{-0.01f, 80.0f}, {-0.01f, -50.0f}}),
      0.31f);

  // Zero distance
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, -9.0f}, Segment{{-5.0f, 60.0f}, {-5.0f, -9.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{0.3f, -50.0f}, {-5.0f, -50.0f}}, Point{0.3f, -50.0f}),
      0.0f);
}

TEST(DistanceTest, PointToSegmentInteriorPointClosest) {
  EXPECT_FLOAT_EQ(
      Distance(Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}, Point{-1.0f, 1.0f}),
      9.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-1.0f, 1.0f}, Segment{{-20.0f, -11.0f}, {2.0f, -11.0f}}),
      12.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, -9.0f}, Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
      11.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{12.0f, 9.0f}, {12.0f, -16.0f}}, Point{-5.0f, -9.0f}),
      17.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{0.3f, -50.0f}, Segment{{-40.0f, -10.0f}, {2.0f, -10.0f}}),
      40.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}}, Point{0.3f, -50.0f}),
      30.3f);

  // Zero distance
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, -8.0f}, Segment{{-5.0f, 9.0f}, {-5.0f, -16.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{0.0f, -50.0f}, {9999.0f, -50.0f}}, Point{0.3f, -50.0f}),
      0.0f);
}

TEST(DistanceTest, PointToSegmentDegenerateSegment) {
  EXPECT_FLOAT_EQ(Distance(Segment{{-0.01f, -50.0f}, {-0.01f, -50.0f}},
                           Point{0.3f, -50.0f}),
                  0.31f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-10.0f, 1.0f}, Segment{{8.0f, 1.0f}, {8.0f, 1.0f}}),
      18.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-5.0f, 16.0f}, {-5.0f, 16.0f}}, Point{-5.0f, -9.0f}),
      25.0f);

  // Zero distance
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, -9.0f}, Segment{{-5.0f, -9.0f}, {-5.0f, -9.0f}}),
      0.0f);

  // Segment endpoints not equal, but small enough distance that projection
  // can't be computed. Gets min distance from endpoints in those cases.
  ASSERT_NE((Point{0, 0}), (Point{0, 1e-23}));
  EXPECT_FLOAT_EQ(Distance(Point{-1, 0}, Segment{{0, 0}, {0, 1e-23}}), 1.0f);
  EXPECT_FLOAT_EQ(Distance(Point{0, 0}, Segment{{0, 0}, {0, 1e-23}}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{0, 1e-23}, Segment{{0, 0}, {0, 1e-23}}), 0.0f);
}

TEST(DistanceTest, SegmentToSegmentEndpointClosestToEndpoint) {
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{8.0f, 1.0f}, {18.0f, 1.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 999.0f}},
                           Segment{{18.0f, 11.0f}, {8.0f, 1.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-5.0f, -19.0f}},
                           Segment{{-5.0f, 16.0f}, {-5.0f, 60.0f}}),
                  25.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-20.0f, -9.0f}},
                           Segment{{36.0f, 16.0f}, {-5.0f, 16.0f}}),
                  25.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {30.3f, -50.0f}},
                           Segment{{-0.01f, -50.0f}, {-5.0f, -50.0f}}),
                  0.31f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {0.4f, 80.0f}},
                           Segment{{-0.01f, 80.0f}, {-0.01f, -50.0f}}),
                  0.31f);

  // Zero distance
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-20.0f, -9.0f}},
                           Segment{{36.0f, 16.0f}, {-5.0f, -9.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {30.3f, -50.0f}},
                           Segment{{0.3f, -50.0f}, {-5.0f, -50.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 999.0f}},
                           Segment{{18.0f, 11.0f}, {-10.0f, 1.0f}}),
                  0.0f);
}

TEST(DistanceTest, SegmentToSegmentEndpointClosestToInteriorPoint) {
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 100.0f}},
                           Segment{{-20.0f, -11.0f}, {2.0f, -11.0f}}),
                  12.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-5.0f, -88.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  11.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-95.0f, -88.0f}},
                           Segment{{12.0f, 9.0f}, {12.0f, -16.0f}}),
                  17.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {61.0f, -90.0f}},
                           Segment{{-40.0f, -10.0f}, {2.0f, -10.0f}}),
                  40.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {0.4f, -999.0f}},
                           Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}}),
                  30.3f);

  // Zero distance
  EXPECT_FLOAT_EQ(Distance(Segment{{8.0f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, 2.0f}, {-5.0f, -88.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {-30.0f, -800.0f}},
                           Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}}),
                  0.0f);
}

TEST(DistanceTest, SegmentToSegmentParallel) {
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-10.0f, -40.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{-20.0f, -11.0f}, {2.0f, -11.0f}}),
                  12.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-7.0f, -9.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  11.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-5.0f, -88.0f}},
                           Segment{{12.0f, 9.0f}, {12.0f, -16.0f}}),
                  17.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {61.0f, -50.0f}},
                           Segment{{-40.0f, -10.0f}, {2.0f, -10.0f}}),
                  40.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {0.3f, -999.0f}},
                           Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}}),
                  30.3f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.0f, 0.0f}, {5.0f, 5.0f}},
                           Segment{{10.0f, 0.0f}, {15.0f, 5.0f}}),
                  std::hypot(5.0f, 5.0f));

  // Zero distance
  EXPECT_FLOAT_EQ(Distance(Segment{{8.0f, 1.0f}, {8.0f, 2.0f}},
                           Segment{{8.0f, 300.0f}, {8.0f, -222.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, 2.0f}, {-21.0f, 2.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-30.0f, -0.001f}, {-30.0f, 999.0f}},
                           Segment{{-30.0f, 0.001f}, {-30.0f, -999.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.0f, 0.0f}, {5.0f, 5.0f}},
                           Segment{{-10.0f, -10.0f}, {15.0f, 15.0f}}),
                  0.0f);
}

TEST(DistanceTest, SegmentToSegmentIntersecting) {
  EXPECT_FLOAT_EQ(Distance(Segment{{8.001f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, -12.0f}, {-100.0f, 100.0f}},
                           Segment{{-20.0f, -11.0f}, {2.0f, -11.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, 15.0f}, {-5.0f, -88.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{15.0f, -5.0f}, {-95.0f, -8.0f}},
                           Segment{{12.0f, 9.0f}, {12.0f, -16.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, 0.0f}, {-61.0f, -90.0f}},
                           Segment{{-40.0f, -10.0f}, {2.0f, -10.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-40.3f, -50.0f}, {0.4f, -999.0f}},
                           Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}}),
                  0.0f);
}

TEST(DistanceTest, SegmentToSegmentDegenerateSegments) {
  // Degenerate segment is closest to an endpoint of the other segment.
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-100.0f, 1.0f}},
                           Segment{{8.0f, 1.0f}, {8.0f, 1.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-5.0f, -9.0f}},
                           Segment{{-5.0f, 16.0f}, {-5.0f, 60.0f}}),
                  25.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {30.3f, -50.0f}},
                           Segment{{-0.01f, -50.0f}, {-0.01f, -50.0f}}),
                  0.31f);

  // Degenerate segment is closest to an interior point of the other segment.
  EXPECT_FLOAT_EQ(Distance(Segment{{-10.0f, 1.0f}, {-10.0f, 1.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  18.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, -9.0f}, {-5.0f, -9.0f}},
                           Segment{{-2.0f, 2.0f}, {-22.0f, 2.0f}}),
                  11.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{-30.0f, 0.0f}, {-30.0f, -999.0f}},
                           Segment{{0.3f, -50.0f}, {0.3f, -999.0f}}),
                  30.3f);

  // Zero distance
  EXPECT_FLOAT_EQ(Distance(Segment{{-5.0f, 16.0f}, {-5.0f, 16.0f}},
                           Segment{{-5.0f, 16.0f}, {-5.0f, 60.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{0.3f, -50.0f}, {30.3f, -50.0f}},
                           Segment{{30.3f, -50.0f}, {30.3f, -50.0f}}),
                  0.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{8.0f, -1.0f}, {8.0f, -1.0f}},
                           Segment{{8.0f, 3.0f}, {8.0f, -10.0f}}),
                  0.0f);
}

TEST(DistanceTest, PointToTriangleEndpointClosest) {
  // The Point is closest to an endpoint of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}},
               Point{10.0f, 40.0f}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{25.0f, -20.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      15.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-50.0f, -20.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      10.0f);

  // Zero distance - the Point overlaps an endpoint of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Point{10.0f, -20.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{10.0f, 30.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}},
               Point{-40.0f, -20.0f}),
      0.0f);
}

TEST(DistanceTest, PointToTriangleSideClosest) {
  // The Point is closest to a Point on the side of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Point{30.0f, 10.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      20.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-15.0f, -25.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}},
               Point{-20.0f, 20.0f}),
      std::hypot(10.0f, 10.0f));

  // Zero distance - the Point lies along a side of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}},
               Point{10.0f, -10.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{4.0f, -20.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-30.0f, -10.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
}

TEST(DistanceTest, PointToTriangleContained) {
  // The Point is contained within the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Point{0.0f, 10.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}},
               Point{-20.0f, -10.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{9.99f, -19.99f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}}),
      0.0f);
}

TEST(DistanceTest, PointToTriangleDegenerateTriangle) {
  // The Triangle is Point-like.
  EXPECT_FLOAT_EQ(
      Distance(Point{20.0f, 30.0f},
               Triangle{{10.0f, 30.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 30.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}},
               Point{10.0f, -30.0f}),
      60.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{10.0f, 30.0f},
               Triangle{{10.0f, 30.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}}),
      0.0f);

  // The Triangle is Segment-like.
  EXPECT_FLOAT_EQ(
      Distance(Point{20.0f, 30.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}},
               Point{10.0f, -30.0f}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}},
               Point{10.0f, -20.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{10.0f, -10.0f},
               Triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {10.0f, 30.0f}}),
      0.0f);
}

TEST(DistanceTest, PointToRectEndpointClosest) {
  // The Point is closest to an endpoint of the Rect.
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{50.0f, 20.0f}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-10.0f, 40.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      20.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-55.0f, -10.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      45.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{45.0f, -15.0f}),
      std::hypot(5.0f, 5.0f));

  // Zero distance - the Point overlaps an endpoint of the Rect.
  EXPECT_FLOAT_EQ(
      Distance(Point{40.0f, 20.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-10.0f, 20.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{40.0f, -10.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{-10.0f, -10.0f}),
      0.0f);
}

TEST(DistanceTest, PointToRectSideClosest) {
  // The Point is closest to a Point along the side of the Rect.
  EXPECT_FLOAT_EQ(
      Distance(Point{50.0f, 10.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{10.0f, 40.0f}),
      20.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-55.0f, 5.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      45.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{25.0f, -15.0f}),
      5.0f);

  // Zero distance - the Point lies along a side of the Rect.
  EXPECT_FLOAT_EQ(
      Distance(Point{40.0f, 5.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{-1.0f, 20.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{15.0f, -10.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{-10.0f, -1.0f}),
      0.0f);
}

TEST(DistanceTest, PointToRectContained) {
  // The Point is contained within the Rect.
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{-5.0f, 15.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{15.0f, 5.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{0.0f, -5.0f},
               Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f})),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, 20.0f}),
               Point{39.99f, 19.99f}),
      0.0f);
}

TEST(DistanceTest, PointToRectDegenerateRect) {
  // The Rect is Point-like.
  EXPECT_FLOAT_EQ(Distance(Point{-10.0f, -50.0f},
                           Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{-10.0f, -10.0f})),
                  40.0f);
  EXPECT_FLOAT_EQ(Distance(Point{60.0f, -10.0f},
                           Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{-10.0f, -10.0f})),
                  70.0f);
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{-10.0f, -10.0f}),
                           Point{10.0f, -10.0f}),
                  20.0f);
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{-10.0f, -10.0f}),
                           Point{-10.0f, 20.0f}),
                  30.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-10.0f, -10.0f},
                           Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{-10.0f, -10.0f})),
                  0.0f);

  // The Rect is Segment-like.
  EXPECT_FLOAT_EQ(
      Distance(Point{80.0f, -10.0f}, Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                                         Point{40.0f, -10.0f})),
      40.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-60.0f, -10.0f},
                           Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                               Point{40.0f, -10.0f})),
                  50.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, -10.0f}),
               Point{30.0f, -20.0f}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{-5.0f, 30.0f}, Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                                        Point{40.0f, -10.0f})),
      40.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints(Point{-10.0f, -10.0f}, Point{40.0f, -10.0f}),
               Point{40.0f, -10.0f}),
      0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Point{25.0f, -10.0f}, Rect::FromTwoPoints(Point{-10.0f, -10.0f},
                                                         Point{40.0f, -10.0f})),
      0.0f);
}

TEST(DistanceTest, PointToQuadEndpointClosest) {
  // Test Quad Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);

  // The Point is closest to an endpoint of the Quad.
  EXPECT_FLOAT_EQ(Distance(Point{-20.0f, -17.0f}, test_quad), 7.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-37.0f, 0.0f}, test_quad), 17.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-65.0f, -33.0f}), 12.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-43.0f, -60.0f}), 27.0f);

  // Zero distance - the Point overlaps an endpoint of the Quad.
  EXPECT_FLOAT_EQ(Distance(Point{-53.0f, -33.0f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-43.0f, -33.0f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-27.0f, -17.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-37.0f, -17.0f}), 0.0f);
}

TEST(DistanceTest, PointToQuadSideClosest) {
  // Test Quad Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);

  // The Point is closest to a Point along the side of the Quad.
  EXPECT_FLOAT_EQ(Distance(Point{-30.0f, -25.0f}, test_quad),
                  std::hypot(2.5f, 2.5f));
  EXPECT_FLOAT_EQ(Distance(Point{-55.0f, -15.0f}, test_quad),
                  std::hypot(10.0f, 10.0f));
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-35.0f, -15.0f}), 2.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-50.0f, -40.0f}), 7.0f);

  // Zero distance - the Point lies along a side of the Quad.
  EXPECT_FLOAT_EQ(Distance(Point{-40.0f, -30.0f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-50.0f, -33.0f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-45.0f, -25.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-35.0f, -17.0f}), 0.0f);
}

TEST(DistanceTest, PointToQuadContained) {
  // Test Quad Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);

  // The Point is contained within the Quad.
  EXPECT_FLOAT_EQ(Distance(Point{-40.0f, -25.0f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-52.99f, -32.99f}, test_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(test_quad, Point{-35.0f, -20.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-47.0f, -28.0f}, test_quad), 0.0f);
}

TEST(DistanceTest, PointToQuadDegenerateQuad) {
  Quad point_like_quad = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 0.0f, 0.0f, kFullTurn, 1.0f);

  // The Quad is Point-like.
  EXPECT_FLOAT_EQ(Distance(Point{-40.0f, -25.0f}, point_like_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(point_like_quad, Point{-50.0f, -25.0f}), 10.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-20.0f, -25.0f}, point_like_quad), 20.0f);
  EXPECT_FLOAT_EQ(Distance(point_like_quad, Point{-40.0f, 5.0f}), 30.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-40.0f, -65.0f}, point_like_quad), 40.0f);

  // Test Quad Corners: (-35,-25), (-35,-25), (-45,-25), (-45,-25)
  Quad segment_like_quad = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 0.0f, kFullTurn, 1.0f);
  // The Quad is Segment-like.
  EXPECT_FLOAT_EQ(Distance(Point{-37.0f, -35.0f}, segment_like_quad), 10.0f);
  EXPECT_FLOAT_EQ(Distance(segment_like_quad, Point{-42.0f, -5.0f}), 20.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-50.0f, -25.0f}, segment_like_quad), 5.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-20.0f, -25.0f}, segment_like_quad), 15.0f);
  EXPECT_FLOAT_EQ(Distance(segment_like_quad, Point{-40.0f, -25.0f}), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-44.0f, -25.0f}, segment_like_quad), 0.0f);
  EXPECT_FLOAT_EQ(Distance(Point{-35.0f, -25.0f}, segment_like_quad), 0.0f);
}

TEST(DistanceTest, SegmentToTriangleEndpointClosestToEndpoint) {
  Triangle test_triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}};

  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{15.0f, -20.0f}, {100.0f, -100.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, 45.0f}, {100.0f, 100.0f}}, test_triangle),
      15.0f);

  // The endpoints are overlapping.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-40.0f, -20.0f}, {-100.0f, -100.0f}}),
      0.0f);
}

TEST(DistanceTest, SegmentToTriangleEndpointClosestToInteriorPoint) {
  Triangle test_triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}};

  // One of the Triangle's endpoints is closest to an interior point of the
  // Segment.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-45.0f, 20.0f}, {-45.0f, -100.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, -60.0f}, {50.0f, -20.0f}}, test_triangle),
      std::sqrt(800.0f));

  // One of the Segment's endpoints is closest to a point along an edge
  // of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-15.0f, -25.0f}, {-35.0f, -80.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-80.0f, 90.0f}, {-30.0f, 30.0f}}, test_triangle),
      std::sqrt(800.0f));

  // One object's endpoint overlaps the other object's edge.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-40.0f, 20.0f}, {-30.0f, -10.0f}}),
      0.0f);
}

TEST(DistanceTest, SegmentToTriangleParallel) {
  Triangle test_triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}};

  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{15.0f, 200.0f}, {15.0f, -500.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-60.0f, 0.0f}, {-10.0f, 50.0f}}, test_triangle),
      std::sqrt(800.0f));

  // The Segment overlaps an edge of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{10.0f, 30.0f}, {-40.0f, -20.0f}}), 0.0f);
}

TEST(DistanceTest, SegmentToTriangleIntersecting) {
  Triangle test_triangle{{10.0f, -20.0f}, {10.0f, 30.0f}, {-40.0f, -20.0f}};

  // The Segment intersects 1 edge of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-40.0f, -30.0f}, {-4.0f, 12.0f}}), 0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-19.99f, 0.0f}, {-40.0f, 20.0f}}, test_triangle), 0.0f);

  // The Segment intersects 2 edges of the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-40.0f, -30.0f}, {6.0f, 30.0f}}, test_triangle), 0.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, -30.0f}, {-40.0f, 20.0f}}, test_triangle), 0.0f);

  // The Segment is fully contained by the Triangle.
  EXPECT_FLOAT_EQ(
      Distance(test_triangle, Segment{{-14.0f, 2.0f}, {4.0f, -14.0f}}), 0.0f);
}

TEST(DistanceTest, SegmentToTriangleDegenerateObjects) {
  // Segment to degenerate Triangle (flat or point-like).
  EXPECT_FLOAT_EQ(
      Distance(Segment{{0.0f, 5.0f}, {10.0f, 5.0f}},
               Triangle{{0.0f, 15.0f}, {0.0f, 15.0f}, {0.0f, 15.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{0.0f, 15.0f}, {5.0f, 15.0f}, {10.0f, 15.0f}},
               Segment{{0.0f, 5.0f}, {10.0f, 5.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{0.0f, 5.0f}, {10.0f, 5.0f}},
               Triangle{{15.0f, 5.0f}, {25.0f, 5.0f}, {20.0f, 5.0f}}),
      5.0f);
  EXPECT_FLOAT_EQ(Distance(Triangle{{5.0f, 5.0f}, {25.0f, 5.0f}, {20.0f, 5.0f}},
                           Segment{{0.0f, 5.0f}, {10.0f, 5.0f}}),
                  0.0f);

  // Triangle to degenerate Segment (point-like).
  EXPECT_FLOAT_EQ(Distance(Segment{{20.0f, 0.0f}, {20.0f, 0.0f}},
                           Triangle{{0, 0}, {0, 10}, {10, 0}}),
                  10.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{10.0f, 10.0f}, {10.0f, 10.0f}},
                           Triangle{{0, 0}, {0, 10}, {10, 0}}),
                  sqrt(50.0f));

  // Both the Triangle and the Segment are Degenerate.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{0.0f, 15.0f}, {0.0f, 15.0f}, {0.0f, 15.0f}},
               Segment{{0.0f, 5.0f}, {0.0f, 5.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, 5.0f}, {10.0f, 5.0f}},
               Triangle{{0.0f, 15.0f}, {5.0f, 15.0f}, {10.0f, 15.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{5.0f, 5.0f}, {5.0f, 5.0f}},
               Triangle{{5.0f, 30.0f}, {5.0f, 25.0f}, {5.0f, 35.0f}}),
      20.0f);
}

TEST(DistanceTest,
     SegmentToRectReturnsCorrectValueWhenSegmentEndpointIsClosestToRectEdge) {
  Rect test_rect = Rect::FromTwoPoints({-10.0f, -20.0f}, {40.0f, 80.0f});

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, -25.0f}, {25.0f, -45.0f}}, test_rect), 5.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(Distance(test_rect, Segment{{50.0f, 5.0f}, {65.0f, 65.0f}}),
                  10.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(test_rect, Segment{{-5.0f, 100.0f}, {45.0f, 150.0f}}), 20.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-35.0f, -5.0f}, {-50.0f, -95.0f}}, test_rect), 25.0f);
}

TEST(DistanceTest, SegmentToRectReturnsZeroWhenSegmentIsIntersectingRect) {
  Rect test_rect = Rect::FromTwoPoints({-10.0f, -20.0f}, {40.0f, 80.0f});

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Segment{{10.0f, -15.0f}, {25.0f, -45.0f}}, test_rect), 0.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Segment{{35.0f, -15.0f}, {-50.0f, 15.0f}}, test_rect), 0.0f);
  // Contained
  EXPECT_FLOAT_EQ(Distance(test_rect, Segment{{20.0f, -15.0f}, {-0.5f, 75.0f}}),
                  0.0f);
}

TEST(DistanceTest,
     SegmentToRectReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  Rect test_rect = Rect::FromTwoPoints({-10.0f, -20.0f}, {40.0f, 80.0f});

  // Segment is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {55.0f, -15.0f}}, test_rect), 15.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{35.0f, -15.0f}, {35.0f, -15.0f}}, test_rect), 0.0f);

  // Rect is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {-25.0f, 40.0f}},
               Rect::FromTwoPoints({65.0f, -15.0f}, {65.0f, -15.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{55.0f, -15.0f}, {-25.0f, 65.0f}},
                           Rect::FromTwoPoints({15.0f, 25.0f}, {15.0f, 25.0f})),
                  0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {55.0f, -15.0f}},
               Rect::FromTwoPoints({65.0f, -15.0f}, {65.0f, -15.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {55.0f, -15.0f}},
               Rect::FromTwoPoints({55.0f, -15.0f}, {55.0f, -15.0f})),
      0.0f);
}

TEST(DistanceTest,
     SegmentToQuadReturnsCorrectValueWhenSegmentEndpointIsClosestToQuadEdge) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);

  // Edge 0
  EXPECT_FLOAT_EQ(Distance(Segment{{10.0f, 2.0f}, {15.0f, 5.0f}}, test_quad),
                  8.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(Distance(test_quad, Segment{{8.0f, 14.0f}, {20.0f, 35.0f}}),
                  sqrt(80));
  // Edge 2
  EXPECT_FLOAT_EQ(Distance(test_quad, Segment{{-20.0f, 2.0f}, {-25.0f, 5.0f}}),
                  18.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Segment{{-4.0f, -12.0f}, {-8.0f, -14.0f}}, test_quad), sqrt(20));
}

TEST(DistanceTest, SegmentToQuadReturnsZeroWhenSegmentIsIntersectingQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);

  // Edge 0
  EXPECT_FLOAT_EQ(Distance(Segment{{-1.0f, 2.0f}, {15.0f, 5.0f}}, test_quad),
                  0.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(Distance(test_quad, Segment{{1.0f, -9.0f}, {-8.0f, -14.0f}}),
                  0.0f);
  // Contained
  EXPECT_FLOAT_EQ(Distance(Segment{{-1.0f, 10.0f}, {0.5f, -9.5f}}, test_quad),
                  0.0f);
}

TEST(DistanceTest,
     SegmentToQuadReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  // Segment is point-like.
  EXPECT_FLOAT_EQ(Distance(Segment{{55.0f, -5.0f}, {55.0f, -5.0f}},
                           Quad::FromCenterDimensionsRotationAndShear(
                               {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
                  53.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{1.0f, -5.0f}, {1.0f, -5.0f}},
                           Quad::FromCenterDimensionsRotationAndShear(
                               {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
                  0.0f);

  // Quad is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {-25.0f, 40.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {65.0f, -15.0f}, 0.0f, 0.0f, kQuarterTurn, 2.0f)),
      10.0f);
  EXPECT_FLOAT_EQ(Distance(Segment{{55.0f, -15.0f}, {-25.0f, 65.0f}},
                           Quad::FromCenterDimensionsRotationAndShear(
                               {15.0f, 25.0f}, 0.0f, 0.0f, kQuarterTurn, 2.0f)),
                  0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {55.0f, -15.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {45.0f, -15.0f}, 0.0f, 0.0f, kQuarterTurn, 2.0f)),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Segment{{55.0f, -15.0f}, {55.0f, -15.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {55.0f, -15.0f}, 0.0f, 0.0f, kQuarterTurn, 2.0f)),
      0.0f);
}

TEST(DistanceTest,
     TriangleToTriangleReturnsCorrectValueWhenEndpointIsClosestToEdge) {
  Triangle test_triangle_1 =
      Triangle{{5.0f, 40.0f}, {40.0f, 5.0f}, {5.0f, 5.0f}};
  Triangle test_triangle_2 =
      Triangle{{20.0f, 60.0f}, {-20.0f, 40.0f}, {-20.0f, 60.0f}};
  Triangle test_triangle_3 =
      Triangle{{-45.0f, 5.0f}, {-45.0f, 25.0f}, {-5.0f, 15.0f}};

  // Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(Distance(test_triangle_1, test_triangle_2), sqrt(125.0f));
  EXPECT_FLOAT_EQ(Distance(test_triangle_2, test_triangle_1), sqrt(125.0f));
  // Edge 1
  EXPECT_FLOAT_EQ(Distance(test_triangle_2, test_triangle_3), sqrt(425.0f));
  EXPECT_FLOAT_EQ(Distance(test_triangle_3, test_triangle_2), sqrt(425.0f));
  // Edge 2
  EXPECT_FLOAT_EQ(Distance(test_triangle_3, test_triangle_1), 10.0f);
  EXPECT_FLOAT_EQ(Distance(test_triangle_1, test_triangle_3), 10.0f);
}

TEST(DistanceTest, TriangleToTriangleReturnsZeroWhenTrianglesAreIntersecting) {
  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{5.0f, 40.0f}, {5.0f, 5.0f}, {40.0f, 5.0f}},
               Triangle{{-45.0f, 5.0f}, {-45.0f, 25.0f}, {15.0f, 15.0f}}),
      0.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-45.0f, 5.0f}, {-45.0f, 25.0f}, {15.0f, 15.0f}},
               Triangle{{5.0f, 40.0f}, {40.0f, 5.0f}, {5.0f, 5.0f}}),
      0.0f);
  // Contained
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{5.0f, 40.0f}, {5.0f, 5.0f}, {40.0f, 5.0f}},
               Triangle{{10.0f, 20.0f}, {10.0f, 10.0f}, {20.0f, 10.0f}}),
      0.0f);
}

TEST(DistanceTest,
     TriangleToTriangleReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  // First Triangle is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{5.0f, 50.0f}, {5.0f, 50.0f}, {5.0f, 50.0f}},
               Triangle{{5.0f, 40.0f}, {40.0f, 5.0f}, {5.0f, 5.0f}}),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{15.0f, 15.0f}, {15.0f, 15.0f}, {15.0f, 15.0f}},
               Triangle{{5.0f, 40.0f}, {40.0f, 5.0f}, {5.0f, 5.0f}}),
      0.0f);

  // Second Triangle is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{20.0f, 60.0f}, {-20.0f, 40.0f}, {-20.0f, 60.0f}},
               Triangle{{-10.0f, 90.0f}, {-10.0f, 90.0f}, {-10.0f, 90.0f}}),
      30.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{20.0f, 60.0f}, {-20.0f, 40.0f}, {-20.0f, 60.0f}},
               Triangle{{-10.0f, 50.0f}, {-10.0f, 50.0f}, {-10.0f, 50.0f}}),
      0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{5.0f, 50.0f}, {5.0f, 50.0f}, {5.0f, 50.0f}},
               Triangle{{5.0f, 70.0f}, {5.0f, 70.0f}, {5.0f, 70.0f}}),
      20.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{5.0f, 70.0f}, {5.0f, 70.0f}, {5.0f, 70.0f}},
               Triangle{{5.0f, 70.0f}, {5.0f, 70.0f}, {5.0f, 70.0f}}),
      0.0f);
}

TEST(DistanceTest,
     TriangleToRectReturnsCorrectValueWhenRectEndpointIsClosestToTriangleEdge) {
  Triangle test_triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}};

  // We only test the three edges of the Triangle because that is how distance
  // is being calculated. Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(Distance(test_triangle, Rect::FromTwoPoints({10.0f, -15.0f},
                                                              {20.0f, -15.0f})),
                  5.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({10.0f, -15.0f}, {20.0f, -15.0f}),
               test_triangle),
      5.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(Distance(test_triangle, Rect::FromTwoPoints({-80.0f, 10.0f},
                                                              {-20.0f, 5.0f})),
                  10.0f);
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({-80.0f, 10.0f}, {-20.0f, 5.0f}),
                           test_triangle),
                  10.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(Distance(test_triangle,
                           Rect::FromTwoPoints({30.0f, 30.0f}, {20.0f, 20.0f})),
                  sqrt(50.0f));
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({30.0f, 30.0f}, {20.0f, 20.0f}),
                           test_triangle),
                  sqrt(50.0f));
}

TEST(DistanceTest,
     TriangleToRectReturnsZeroWhenTriangleAndRectAreIntersecting) {
  Triangle test_triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}};

  // Edge 0
  EXPECT_FLOAT_EQ(Distance(test_triangle,
                           Rect::FromTwoPoints({10.0f, -5.0f}, {20.0f, -5.0f})),
                  0.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(Distance(test_triangle,
                           Rect::FromTwoPoints({30.0f, 30.0f}, {12.0f, 12.0f})),
                  0.0f);
  // Contained
  EXPECT_FLOAT_EQ(Distance(test_triangle,
                           Rect::FromTwoPoints({-5.0f, -5.0f}, {10.0f, 10.0f})),
                  0.0f);
}

TEST(DistanceTest,
     TriangleToRectReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  // Triangle is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Rect::FromTwoPoints({30.0f, 30.0f}, {12.0f, 10.0f})),
      2.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Rect::FromTwoPoints({30.0f, 30.0f}, {10.0f, 10.0f})),
      0.0f);

  // Rect is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}},
               Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}},
               Rect::FromTwoPoints({20.0f, 0.0f}, {20.0f, 0.0f})),
      0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Rect::FromTwoPoints({50.0f, 10.0f}, {50.0f, 10.0f})),
      40.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Rect::FromTwoPoints({10.0f, 10.0f}, {10.0f, 10.0f})),
      0.0f);
}

TEST(DistanceTest,
     TriangleToQuadReturnsCorrectValueWhenQuadEndpointIsClosestToTriangleEdge) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // We only test the three edges of the Triangle because that is how distance
  // is being calculated. Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-5.0f, 20.0f}, {5.0f, 20.0f}, {0.0f, 40.0f}},
               test_quad),
      6.0f);
  EXPECT_FLOAT_EQ(
      Distance(test_quad,
               Triangle{{-5.0f, 20.0f}, {5.0f, 20.0f}, {0.0f, 40.0f}}),
      6.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, -20.0f}, {0.0f, -20.0f}, {10.0f, -10.0f}},
               test_quad),
      sqrt(8.0f));
  EXPECT_FLOAT_EQ(
      Distance(test_quad,
               Triangle{{10.0f, -20.0f}, {0.0f, -20.0f}, {10.0f, -10.0f}}),
      sqrt(8.0f));
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-8.0f, -16.0f}, {-12.0f, -16.0f}, {-12.0f, 0.0f}},
               test_quad),
      sqrt(68.0f));
  EXPECT_FLOAT_EQ(
      Distance(test_quad,
               Triangle{{-8.0f, -16.0f}, {-12.0f, -16.0f}, {-12.0f, 0.0f}}),
      sqrt(68.0f));
}

TEST(DistanceTest,
     TriangleToQuadReturnsZeroWhenTriangleAndQuadAreIntersecting) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, -10.0f}, {10.0f, -10.0f}, {0.0f, -30.0f}},
               test_quad),
      0.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, 10.0f}, {0.0f, 30.0f}, {10.0f, 10.0f}},
               test_quad),
      0.0f);
  // Contained
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 3.0f}},
               test_quad),
      0.0f);
}

TEST(DistanceTest,
     TriangleToQuadReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  // Triangle is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 1.0f}, {10.0f, 1.0f}, {10.0f, 1.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
      8.0f);
  EXPECT_FLOAT_EQ(Distance(Triangle{{1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}},
                           Quad::FromCenterDimensionsRotationAndShear(
                               {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
                  0.0f);

  // Quad is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {50.0f, -10.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{-10.0f, 40.0f}, {40.0f, -10.0f}, {-10.0f, -10.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {20.0f, 0.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
      0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {50.0f, 10.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
      40.0f);
  EXPECT_FLOAT_EQ(
      Distance(Triangle{{10.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 10.0f}},
               Quad::FromCenterDimensionsRotationAndShear(
                   {10.0f, 10.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
      0.0f);
}

TEST(DistanceTest,
     RectToRectReturnsCorrectValueWhenRectEdgeIsClosestToRectEdge) {
  Rect test_rect = Rect::FromTwoPoints({-10.0f, -20.0f}, {40.0f, 80.0f});

  // We only test the four edges of the first Rect because that is how distance
  // is being calculated. Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(Distance(test_rect, Rect::FromTwoPoints({-5.0f, -50.0f},
                                                          {30.0f, -90.0f})),
                  30.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, -50.0f}, {30.0f, -90.0f}),
               test_rect),
      30.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(
      Distance(test_rect, Rect::FromTwoPoints({50.0f, -5.0f}, {60.0f, 25.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({50.0f, -5.0f}, {60.0f, 25.0f}), test_rect),
      10.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(test_rect, Rect::FromTwoPoints({0.0f, 120.0f}, {1.0f, 150.0f})),
      40.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({0.0f, 120.0f}, {1.0f, 150.0f}), test_rect),
      40.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(test_rect, Rect::FromTwoPoints({-50.0f, 5.0f}, {-30.0f, 40.0f})),
      20.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-50.0f, 5.0f}, {-30.0f, 40.0f}), test_rect),
      20.0f);
}

TEST(DistanceTest, RectToRectReturnsZeroWhenTheRectsAreIntersecting) {
  Rect test_rect = Rect::FromTwoPoints({-10.0f, -20.0f}, {40.0f, 80.0f});

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, -10.0f}, {30.0f, -90.0f}),
               test_rect),
      0.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-50.0f, -5.0f}, {-10.0f, 40.0f}),
               test_rect),
      0.0f);
  // Contained
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, -15.0f}, {30.0f, -10.0f}),
               test_rect),
      0.0f);
}

TEST(DistanceTest,
     RectToRectReturnsCorrectValueWhenOneOrBothRectsAreDegenerate) {
  // One Rect is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({20.0f, -30.0f}, {40.0f, 10.0f}),
               Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({20.0f, -30.0f}, {60.0f, 10.0f}),
               Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f})),
      0.0f);
  // Arguments are flipped to verify order independence.
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f}),
               Rect::FromTwoPoints({20.0f, -30.0f}, {40.0f, 10.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f}),
               Rect::FromTwoPoints({20.0f, -30.0f}, {60.0f, 10.0f})),
      0.0f);

  // Both Rects are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({50.0f, -10.0f}, {50.0f, -10.0f}),
               Rect::FromTwoPoints({40.0f, -10.0f}, {40.0f, -10.0f})),
      10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({40.0f, -10.0f}, {40.0f, -10.0f}),
               Rect::FromTwoPoints({40.0f, -10.0f}, {40.0f, -10.0f})),
      0.0f);
}

TEST(DistanceTest,
     RectToQuadReturnsCorrectValueWhenQuadEndpointIsClosestToRectEdge) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // We only test the four edges of the Rect because that is how distance
  // is being calculated. Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, 20.0f}, {5.0f, 50.0f}), test_quad),
      6.0f);
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Rect::FromTwoPoints({-5.0f, 20.0f}, {5.0f, 50.0f})),
      6.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-20.0f, -2.0f}, {-10.0f, 2.0f}), test_quad),
      8.0f);
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Rect::FromTwoPoints({-20.0f, -2.0f}, {-10.0f, 2.0f})),
      8.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, -30.0f}, {5.0f, -50.0f}), test_quad),
      16.0f);
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Rect::FromTwoPoints({-5.0f, -30.0f}, {5.0f, -50.0f})),
      16.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({20.0f, -2.0f}, {30.0f, 2.0f}), test_quad),
      18.0f);
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Rect::FromTwoPoints({20.0f, -2.0f}, {30.0f, 2.0f})),
      18.0f);
}

TEST(DistanceTest, RectToQuadReturnsZeroWhenRectAndQuadAreIntersecting) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-5.0f, 2.0f}, {5.0f, 50.0f}), test_quad),
      0.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({1.0f, -20.0f}, {30.0f, 20.0f}), test_quad),
      0.0f);
  // Contained
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-0.8f, -3.0f}, {1.4f, 2.2f}), test_quad),
      0.0f);
}

TEST(DistanceTest,
     RectToQuadReturnsCorrectValueWhenOneOrBothObjectsAreDegenerate) {
  // Rect is point-like.
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({-10.0f, 2.0f}, {-10.0f, 2.0f}),
                           Quad::FromCenterDimensionsRotationAndShear(
                               {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
                  8.0f);
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({1.0f, 2.0f}, {1.0f, 2.0f}),
                           Quad::FromCenterDimensionsRotationAndShear(
                               {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
                  0.0f);

  // Quad is point-like.
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({-20.0f, -2.0f}, {-10.0f, 2.0f}),
                           Quad::FromCenterDimensionsRotationAndShear(
                               {-30.0f, 1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
                  10.0f);
  EXPECT_FLOAT_EQ(
      Distance(Rect::FromTwoPoints({-20.0f, -2.0f}, {-10.0f, 2.0f}),
               Quad::FromCenterDimensionsRotationAndShear(
                   {-15.0f, -1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
      0.0f);

  // Both are point-like.
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({-10.0f, 2.0f}, {-10.0f, 2.0f}),
                           Quad::FromCenterDimensionsRotationAndShear(
                               {50.0f, 2.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
                  60.0f);
  EXPECT_FLOAT_EQ(Distance(Rect::FromTwoPoints({-10.0f, 2.0f}, {-10.0f, 2.0f}),
                           Quad::FromCenterDimensionsRotationAndShear(
                               {-10.0f, 2.0f}, 0.0f, 0.0f, kQuarterTurn, 0.0f)),
                  0.0f);
}

TEST(DistanceTest,
     QuadToQuadReturnsCorrectValueWhenQuadEdgeIsClosestToQuadEdge) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // We only test the four edges of the first Quad because that is how distance
  // is being calculated. Arguments are flipped to verify order independence.
  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {23.0f, 12.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      8.0f);
  EXPECT_FLOAT_EQ(Distance(Quad::FromCenterDimensionsRotationAndShear(
                               {23.0f, 12.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                           test_quad),
                  8.0f);
  // Edge 1
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {11.0f, 28.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      6.0f);
  EXPECT_FLOAT_EQ(Distance(Quad::FromCenterDimensionsRotationAndShear(
                               {11.0f, 28.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                           test_quad),
                  6.0f);
  // Edge 2
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {-33.0f, -8.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      18.0f);
  EXPECT_FLOAT_EQ(Distance(Quad::FromCenterDimensionsRotationAndShear(
                               {-33.0f, -8.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                           test_quad),
                  18.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {-11.0f, -38.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      16.0f);
  EXPECT_FLOAT_EQ(Distance(Quad::FromCenterDimensionsRotationAndShear(
                               {-11.0f, -38.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                           test_quad),
                  16.0f);
}

TEST(DistanceTest, QuadToQuadReturnsZeroWhenTheQuadsAreIntersecting) {
  // Test Quad Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      /*center=*/{0.0f, 0.0f}, /*width=*/20.0f, /*height=*/4.0f,
      /*rotation=*/kQuarterTurn, /*shear=*/2.0f);

  // Edge 0
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {13.0f, 12.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      0.0f);
  // Edge 3
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {-13.0f, -15.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)),
      0.0f);
  // Contained
  EXPECT_FLOAT_EQ(
      Distance(test_quad, Quad::FromCenterDimensionsRotationAndShear(
                              {0.1f, -1.0f}, 0.5f, 1.0f, kFullTurn, 0.2f)),
      0.0f);
}

TEST(DistanceTest,
     QuadToQuadReturnsCorrectValueWhenOneOrBothQuadsAreDegenerate) {
  // One Quad is point-like.
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterAndDimensions({9.0f, 3.0f}, 0.0f, 0.0f),
               Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
      7.0f);
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterAndDimensions({1.0f, 3.0f}, 0.0f, 0.0f),
               Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)),
      0.0f);
  // Arguments are flipped to verify order independence.
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
               Quad::FromCenterAndDimensions({9.0f, 3.0f}, 0.0f, 0.0f)),
      7.0f);
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
               Quad::FromCenterAndDimensions({1.0f, 3.0f}, 0.0f, 0.0f)),
      0.0f);

  // Both Quads are point-like.
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterAndDimensions({11.0f, 3.0f}, 0.0f, 0.0f),
               Quad::FromCenterAndDimensions({9.0f, 3.0f}, 0.0f, 0.0f)),
      2.0f);
  EXPECT_FLOAT_EQ(
      Distance(Quad::FromCenterAndDimensions({11.0f, 3.0f}, 0.0f, 0.0f),
               Quad::FromCenterAndDimensions({11.0f, 3.0f}, 0.0f, 0.0f)),
      0.0f);
}

}  // namespace
}  // namespace ink
