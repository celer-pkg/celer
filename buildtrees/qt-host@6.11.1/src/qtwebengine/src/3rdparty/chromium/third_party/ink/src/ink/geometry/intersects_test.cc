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

#include "ink/geometry/intersects.h"

#include "gtest/gtest.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace {

TEST(IntersectsTest, PointToPoint) {
  EXPECT_TRUE(Intersects(Point{15, 20}, Point{15, 20}));
  EXPECT_TRUE(Intersects(Point{9568, -.008}, Point{9568, -.008}));

  EXPECT_FALSE(Intersects(Point{5, 2}, Point{5, 2.1}));
  EXPECT_FALSE(Intersects(Point{-5.001, 2}, Point{-5, 2}));
  EXPECT_FALSE(Intersects(Point{-35, -123456}, Point{-123456, -35}));
}

TEST(IntersectsTest, PointToSegment) {
  Segment test_segment_1 = Segment{{1, 1}, {11, 11}};
  EXPECT_TRUE(Intersects(Point{1, 1}, test_segment_1));
  EXPECT_TRUE(Intersects(Point{11, 11}, test_segment_1));
  EXPECT_TRUE(Intersects(Point{6, 6}, test_segment_1));
  EXPECT_TRUE(Intersects(Point{3, 3}, test_segment_1));
  EXPECT_TRUE(Intersects(test_segment_1, Point{9, 9}));
  EXPECT_TRUE(Intersects(test_segment_1, Point{3.5, 3.5}));

  EXPECT_FALSE(Intersects(Point{0, 0}, test_segment_1));
  EXPECT_FALSE(Intersects(Point{20, 20}, test_segment_1));
  EXPECT_FALSE(Intersects(test_segment_1, Point{-6, -6}));
  EXPECT_FALSE(Intersects(test_segment_1, Point{5, 5.0001}));
  EXPECT_FALSE(Intersects(test_segment_1, Point{11.0001, 11.0001}));
  EXPECT_FALSE(Intersects(test_segment_1, Point{.99999, .99999}));

  Segment test_segment_2 = Segment{{-10, 2}, {10, -8}};
  EXPECT_TRUE(Intersects(Point{-10, 2}, test_segment_2));
  EXPECT_TRUE(Intersects(Point{10, -8}, test_segment_2));
  EXPECT_TRUE(Intersects(Point{-8, 1}, test_segment_2));
  EXPECT_TRUE(Intersects(Point{-6, 0}, test_segment_2));
  EXPECT_TRUE(Intersects(test_segment_2, Point{-2, -2}));
  EXPECT_TRUE(Intersects(test_segment_2, Point{0, -3}));
  EXPECT_TRUE(Intersects(test_segment_2, Point{6, -6}));

  EXPECT_FALSE(Intersects(Point{-10.0001, 2.0001}, test_segment_2));
  EXPECT_FALSE(Intersects(Point{10.0001, -8.0001}, test_segment_2));
  EXPECT_FALSE(Intersects(Point{0, 0}, test_segment_2));
  EXPECT_FALSE(Intersects(Point{-12, 3}, test_segment_2));
  EXPECT_FALSE(Intersects(test_segment_2, Point{12, -9}));
  EXPECT_FALSE(Intersects(test_segment_2, Point{-2, -2.0001}));
}

TEST(IntersectsTest, PointToTriangle) {
  Triangle right_winding_triangle{Point{0, 0}, Point{0, 10}, Point{10, 0}};
  // The Point is inside.
  EXPECT_TRUE(Intersects(right_winding_triangle, Point{2, 2}));

  // The Point is outside.
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{12, 2}));
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{-5, 2}));
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{2, -10}));
  EXPECT_FALSE(Intersects(Point{-1, -1}, right_winding_triangle));
  EXPECT_FALSE(Intersects(Point{12, -1}, right_winding_triangle));
  EXPECT_FALSE(Intersects(Point{-1, 12}, right_winding_triangle));

  // The Point is aligned with an edge but outside the triangle.
  EXPECT_FALSE(Intersects(Point{-1, 0}, right_winding_triangle));
  EXPECT_FALSE(Intersects(Point{11, 0}, right_winding_triangle));
  EXPECT_FALSE(Intersects(Point{-1, 11}, right_winding_triangle));
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{11, -1}));
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{0, 11}));
  EXPECT_FALSE(Intersects(right_winding_triangle, Point{0, -1}));

  // The Point is on an edge.
  EXPECT_TRUE(Intersects(right_winding_triangle, Point{0, 5}));
  EXPECT_TRUE(Intersects(Point{8, 2}, right_winding_triangle));
  EXPECT_TRUE(Intersects(right_winding_triangle, Point{7, 0}));

  // The Point is on a Triangle vertex.
  EXPECT_TRUE(Intersects(right_winding_triangle, Point{0, 0}));
  EXPECT_TRUE(Intersects(Point{10, 0}, right_winding_triangle));
  EXPECT_TRUE(Intersects(right_winding_triangle, Point{0, 10}));

  Triangle left_winding_triangle{Point{-15, -8}, Point{10, -8}, Point{-10, 2}};
  // The Point is inside.
  EXPECT_TRUE(Intersects(left_winding_triangle, Point{-5, -5}));

  // The Point is outside.
  EXPECT_FALSE(Intersects(Point{-15, 0}, left_winding_triangle));
  EXPECT_FALSE(Intersects(Point{-10, 5}, left_winding_triangle));
  EXPECT_FALSE(Intersects(Point{-2, -1}, left_winding_triangle));
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{20, -10}));
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{0, -10}));
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{-20, -10}));

  // The Point is aligned with an edge but outside the Triangle.
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{-16, -10}));
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{-17, -8}));
  EXPECT_FALSE(Intersects(left_winding_triangle, Point{-12, 3}));
  EXPECT_FALSE(Intersects(Point{-9, 3}, left_winding_triangle));
  EXPECT_FALSE(Intersects(Point{12, -8}, left_winding_triangle));
  EXPECT_FALSE(Intersects(Point{14, -10}, left_winding_triangle));

  // The Point is on an edge.
  EXPECT_TRUE(Intersects(left_winding_triangle, Point{-2, -2}));
  EXPECT_TRUE(Intersects(Point{-13, -4}, left_winding_triangle));
  EXPECT_TRUE(Intersects(left_winding_triangle, Point{-6, -8}));

  // The Point is on a Triangle vertex.
  EXPECT_TRUE(Intersects(left_winding_triangle, Point{-10, 2}));
  EXPECT_TRUE(Intersects(Point{-15, -8}, left_winding_triangle));
  EXPECT_TRUE(Intersects(left_winding_triangle, Point{10, -8}));
}

TEST(IntersectsTest, PointToRect) {
  Rect test_rect = Rect::FromCenterAndDimensions(Point{5, 5}, 2, 2);
  // Centered Points are contained
  EXPECT_TRUE(Intersects(test_rect, Point{5, 5}));
  EXPECT_TRUE(Intersects(test_rect, Point{5.5, 5.5}));
  EXPECT_TRUE(Intersects(test_rect, Point{4.5, 4.5}));

  // Sides are contained
  EXPECT_TRUE(Intersects(test_rect, Point{4, 5}));
  EXPECT_TRUE(Intersects(test_rect, Point{6, 5}));
  EXPECT_TRUE(Intersects(test_rect, Point{5, 4}));
  EXPECT_TRUE(Intersects(test_rect, Point{5, 6}));

  // Corners are contained
  EXPECT_TRUE(Intersects(test_rect, Point{4, 4}));
  EXPECT_TRUE(Intersects(test_rect, Point{6, 4}));
  EXPECT_TRUE(Intersects(test_rect, Point{4, 6}));
  EXPECT_TRUE(Intersects(test_rect, Point{6, 6}));

  // 8 outer areas are excluded
  EXPECT_FALSE(Intersects(test_rect, Point{3, 3}));
  EXPECT_FALSE(Intersects(test_rect, Point{3, 5}));
  EXPECT_FALSE(Intersects(test_rect, Point{3, 7}));
  EXPECT_FALSE(Intersects(test_rect, Point{5, 3}));
  EXPECT_FALSE(Intersects(test_rect, Point{5, 7}));
  EXPECT_FALSE(Intersects(test_rect, Point{7, 3}));
  EXPECT_FALSE(Intersects(test_rect, Point{7, 5}));
  EXPECT_FALSE(Intersects(test_rect, Point{7, 7}));
}

TEST(IntersectsTest, PointToQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);

  // Inside the Quad.
  EXPECT_TRUE(Intersects(test_quad, Point{-46.0f, -16.0f}));

  // On the side of the Quad.
  EXPECT_TRUE(Intersects(Point{-36.01f, -33.98f}, test_quad));

  // At the corner of the Quad.
  EXPECT_TRUE(Intersects(test_quad, Point{-48.0f, -22.0f}));

  // Below relative to the center and rotation of the Quad.
  EXPECT_FALSE(Intersects(test_quad, Point{-28.0f, -38.0f}));

  // To the Left relative to the center and rotation of the Quad.
  EXPECT_FALSE(Intersects(Point{-44.0f, -36.0f}, test_quad));

  // Above & to the right relative to the center and rotation of the Quad.
  EXPECT_FALSE(Intersects(test_quad, Point{-50.0f, -6.0f}));
}

TEST(IntersectsTest, SegmentToSegmentOverlappingSegments) {
  // Segments are fully Overlapping.
  EXPECT_TRUE(Intersects(Segment{{1, 1}, {11, 11}}, Segment{{1, 1}, {11, 11}}));
  EXPECT_TRUE(Intersects(Segment{{1, 1}, {11, 11}}, Segment{{11, 11}, {1, 1}}));
  EXPECT_TRUE(Intersects(Segment{{1, 1}, {11, 11}}, Segment{{6, 6}, {3, 3}}));
  EXPECT_TRUE(Intersects(Segment{{3, 3}, {6, 6}}, Segment{{1, 1}, {11, 11}}));

  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{10, -8}, {-10, 2}}));
  EXPECT_TRUE(
      Intersects(Segment{{-10, 2}, {10, -8}}, Segment{{10, -8}, {-10, 2}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-2, -2}, {8, -7}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-4, -1}, {-8, 1}}));

  // Segments are partially Overlapping.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{10, 10}, {18, 18}}));
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{5, 5}, {-10, -10}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-12, 3}, {-8, 1}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{8, -7}, {22, -14}}));

  // Segments are parallel but not collinear.
  EXPECT_FALSE(Intersects(Segment{{1, 1}, {11, 11}}, Segment{{3, 4}, {8, 9}}));
  EXPECT_FALSE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-4, 0}, {0, -2}}));

  // Segments are parallel and collinear but not overlapping anywhere.
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{-1, -1}, {-11, -11}}));
  EXPECT_FALSE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-22, 8}, {-12, 3}}));
}

TEST(IntersectsTest, SegmentToSegmentCrossingSegments) {
  // Segments are perpendicular.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{11, -1}, {-1, 11}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-6, -10}, {2, 6}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{1.9, -1}, {-1, 1.9}}));

  // Segments are almost Parallel.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{1.1, 1}, {10.9, 11}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{10, -7.9}, {-10, 1.9}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{1.1, 1}, {11.1, 11}}));

  // Segments intersect Near Endpoints.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{10.9, -5}, {10.9, 11}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-10, 1.9}, {90, 1.9}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{11.1, -5}, {11.1, 11}}));
}

TEST(IntersectsTest, SegmentToSegmentConnectedAtEndpoint) {
  // One Segment is perpendicular to, and connected to, the endpoint of the
  // other Segment.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{23, -1}, {-1, 23}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-12, -2}, {-8, 6}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{23.1, -1}, {-1, 23.1}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{100, 2}, {2, 100}}));

  // The endpoint of one Segment connects to a Point somewhere along the other
  // Segment.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{7, 7}, {55, -5.5}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{112, 300}, {6, -6}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{7, 6.9}, {55, -5.5}}));

  // Both Segments meet at their shared endpoint.
  EXPECT_TRUE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{11, -5}, {11, 11}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-10, 2}, {90, 2}}));
  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{11, -5}, {11, 10.9}}));
}

TEST(IntersectsTest, SegmentToSegmentDegenerateSegments) {
  EXPECT_TRUE(Intersects(Segment{{1, 1}, {11, 11}}, Segment{{8, 8}, {8, 8}}));
  EXPECT_TRUE(Intersects(Segment{{3, 3}, {3, 3}}, Segment{{1, 1}, {11, 11}}));
  EXPECT_TRUE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-6, 0}, {-6, 0}}));
  EXPECT_TRUE(
      Intersects(Segment{{4, -5}, {4, -5}}, Segment{{10, -8}, {-10, 2}}));

  EXPECT_FALSE(
      Intersects(Segment{{1, 1}, {11, 11}}, Segment{{8, 7.9}, {8, 7.9}}));
  EXPECT_FALSE(
      Intersects(Segment{{.9, .9}, {.9, .9}}, Segment{{1, 1}, {11, 11}}));
  EXPECT_FALSE(
      Intersects(Segment{{10, -8}, {-10, 2}}, Segment{{-6, 0.1}, {-6, 0.1}}));
  EXPECT_FALSE(
      Intersects(Segment{{4, -5}, {4, -5}}, Segment{{10, -8.1}, {10, -8.1}}));
}

TEST(IntersectsTest, SegmentToTriangleOverlap) {
  // Segment is fully overlapping an edge of the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{-1, 11}, {11, -1}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{-15, -5}, {5, -5}}));
  EXPECT_TRUE(Intersects(Segment{{-1, 11}, {-1, -1}},
                         Triangle{{-1, -1}, {-1, 11}, {11, -1}}));
  EXPECT_TRUE(Intersects(Segment{{-5, 10}, {5, -5}},
                         Triangle{{-5, 10}, {-15, -5}, {5, -5}}));
  EXPECT_FALSE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                          Segment{{-1, 11.1}, {11.1, -1}}));
  EXPECT_FALSE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                          Segment{{-15, -5.1}, {5, -5.1}}));

  // Segment is partially overlapping an edge of the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{-5, -1}, {5, -1}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{0, 2.5}, {10, -12.5}}));
  EXPECT_TRUE(Intersects(Segment{{5, 5}, {1, 9}},
                         Triangle{{-1, -1}, {-1, 11}, {11, -1}}));
  EXPECT_TRUE(Intersects(Segment{{-10, 0}, {5, 25}},
                         Triangle{{-5, 10}, {-15, -5}, {5, -5}}));
  EXPECT_FALSE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                          Segment{{-1, -1.1}, {-1, -200}}));
  EXPECT_FALSE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                          Segment{{-15.1, -5}, {-50, -5}}));
}

TEST(IntersectsTest, SegmentToTriangleCrossing) {
  // Segment crosses through the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{5, -20}, {2, 20}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{-20, 8}, {10, -4}}));
  EXPECT_FALSE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                          Segment{{-1, 11.1}, {-5, 84}}));
  EXPECT_FALSE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                          Segment{{-15, -5.1}, {90, -51}}));

  // Segment crosses one side of the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{5, -20}, {5, 2}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{-20, 8}, {-5, 5}}));
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{5, -1}, {5, 2}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{-7, 7}, {-5, 5}}));
}

TEST(IntersectsTest, SegmentToTriangleInside) {
  // Segment is fully inside the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{1, 1}, {2, 7}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{-5, 7}, {-10, -4}}));
  EXPECT_TRUE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                         Segment{{8, 1}, {1, 8}}));
  EXPECT_TRUE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                         Segment{{1, -2}, {-13, -4}}));
}

TEST(IntersectsTest, SegmentToTriangleOutside) {
  // Segment is fully outside the Triangle.
  EXPECT_FALSE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                          Segment{{-1000, -3}, {1000, -3}}));
  EXPECT_FALSE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                          Segment{{-1000, -6}, {1000, -6}}));
  EXPECT_FALSE(Intersects(Triangle{{-1, -1}, {-1, 11}, {11, -1}},
                          Segment{{-100, -50}, {50, 100}}));
  EXPECT_FALSE(Intersects(Triangle{{-5, 10}, {-15, -5}, {5, -5}},
                          Segment{{-100, -50}, {50, 100}}));
}

TEST(IntersectsTest, SegmentToRectOverlap) {
  // Segment is fully overlapping an edge of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{-1, -1}, {11, -1}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-3, -19}, {-3, 3}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{-1, -1}, {-1, 7}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-3, -19}, {11, -19}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                          Segment{{-1.1, -1}, {-1.1, 7}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                          Segment{{-3, -19.1}, {11, -19.1}}));

  // Segment is partially overlapping an edge of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{6, -1}, {61, -1}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-3, -5}, {-3, 25}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{-1, 9}, {-1, 5}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-1, -19}, {-15, -19}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                          Segment{{-1.1, -1}, {-9, -1}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                          Segment{{-3, -19.1}, {-3, -22}}));
}

TEST(IntersectsTest, SegmentToRectCrossing) {
  // Segment crosses through the Rect.
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{-4, -3}, {14, 9}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-10, 14}, {18, -30}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{-5, 6}, {14, 1}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-12, -15}, {18, 0}}));

  // Segment crosses one side of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{2, 1}, {14, 9}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-10, 14}, {4, -8}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{8, 5}, {14, 1}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-12, -15}, {5, 2}}));
}

TEST(IntersectsTest, SegmentToRectInside) {
  // Segment is fully inside the Rect.
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{0, 0}, {2, 2}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{-2, 2}, {9, -2}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                         Segment{{4, 6}, {10, 1}}));
  EXPECT_TRUE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                         Segment{{5, -17}, {6, -17}}));
}

TEST(IntersectsTest, SegmentToRectOutside) {
  // Segment is fully outside the Rect.
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                          Segment{{-1000, -3}, {1000, -3}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                          Segment{{-1000, 5}, {1000, 5}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({5, 3}, 12, 8),
                          Segment{{-100, -50}, {50, 100}}));
  EXPECT_FALSE(Intersects(Rect::FromCenterAndDimensions({4, -8}, 14, 22),
                          Segment{{-100, -50}, {50, 100}}));
}

TEST(IntersectsTest, SegmentToQuadOverlap) {
  // Segment is fully overlapping an edge of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-43, -33}, {-27, -17}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{2, -14}, {2, 6}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-37, -17}, {-27, -17}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-2, 14}, {-2, -6}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40, -25}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Segment{{-42.9, -33}, {-26.9, -17}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Segment{{2, -14.1}, {-2, -6.1}}));

  // Segment is partially overlapping an edge of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-40, -30}, {-49, -39}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{2, -4}, {2, 22}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-40, -17}, {-30, -17}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-2, 10}, {-2, 33}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Segment{{-2, 14.1}, {-2, 200}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40, -25}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Segment{{-42, -33}, {1, -33}}));
}

TEST(IntersectsTest, SegmentToQuadCrossing) {
  // Segment crosses through Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-46, -41}, {-34, -9}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-3, -9}, {3, 9}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-50, -28}, {-30, -22}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-2, -9}, {1, 9}}));

  // Segment crosses one side of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-46, -41}, {-50, -25}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-3, -9}, {3, 9}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-50, -28}, {-35, -18}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-2, -9}, {0, 0}}));
}

TEST(IntersectsTest, SegmentToQuadInside) {
  // Segment is fully inside the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-50, -32}, {-42, -30}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{.5, .5}, {-.5, -.5}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Segment{{-32, -20}, {-33, -19}}));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Segment{{-1, 10}, {0, 6}}));
}

TEST(IntersectsTest, SegmentToQuadOutside) {
  // Segment is fully outside the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Segment{{-1000, 5}, {1000, 5}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Segment{{-3, -1000}, {-3, 1000}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Segment{{100, -50}, {-50, 100}}));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Segment{{-100, -20}, {50, 100}}));
}

TEST(IntersectsTest, TriangleToTriangleEdgeIntersecting) {
  // Intersections where a Point of one Triangle is inside the other Triangle
  // Intersects the "first" side of the Triangle (p0 -> p1)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {40, -10}, {-5, 25}},
                         Triangle{{-5, -30}, {1, 10}, {10, -30}}));
  EXPECT_TRUE(Intersects(Triangle{{1, 10}, {10, -30}, {-5, -30}},
                         Triangle{{-30, -20}, {40, -10}, {-5, 25}}));
  // Intersects the "second" side of the Triangle (p1 -> p2)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {40, -10}, {-5, 25}},
                         Triangle{{40, 20}, {1, 10}, {20, 40}}));
  EXPECT_TRUE(Intersects(Triangle{{20, 40}, {40, 20}, {1, 10}},
                         Triangle{{-30, -20}, {40, -10}, {-5, 25}}));
  // Intersects the "third" side of the Triangle (p2 -> p0)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {40, -10}, {-5, 25}},
                         Triangle{{-30, -10}, {1, 10}, {-15, 20}}));
  EXPECT_TRUE(Intersects(Triangle{{1, 10}, {-30, -10}, {-15, 20}},
                         Triangle{{-30, -20}, {40, -10}, {-5, 25}}));

  // Intersections where one Triangle crosses through the other Triangle but
  // does not have a Point within it.
  // Intersects the "first" and "second" sides of the Triangle
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-60, -20}, {-10, 50}, {40, 20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, 20}, {-60, -20}, {-10, 50}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Intersects the "second" and "third" sides of the Triangle
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{5, 50}, {40, 40}, {20, -60}}));
  EXPECT_TRUE(Intersects(Triangle{{20, -60}, {5, 50}, {40, 40}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Intersects the "third" and "first" sides of the Triangle
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-50, -40}, {-30, 30}, {20, -60}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, 30}, {20, -60}, {-50, -40}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
}

TEST(IntersectsTest, TriangleToTriangleFullyContained) {
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-25, -15}, {-5, 20}, {30, -10}}));
  EXPECT_TRUE(Intersects(Triangle{{40, 20}, {-60, -20}, {-10, 50}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-35, -10}, {34, -8}, {37, -9}}));
  EXPECT_TRUE(Intersects(Triangle{{37, -9}, {34, -8}, {-35, -10}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
}

TEST(IntersectsTest, TriangleToTriangleOverlappingEdges) {
  // Overlaps the "first" edge of the Triangle (p0 -> p1)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-35, -29}, {0, 34}, {-40, 20}}));
  EXPECT_TRUE(Intersects(Triangle{{-40, 20}, {-35, -29}, {0, 34}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Overlaps the "second" edge of the Triangle (p1 -> p2)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{20, 20}, {4, 18}, {31, -3}}));
  EXPECT_TRUE(Intersects(Triangle{{4, 18}, {31, -3}, {20, 20}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Overlaps the "third" edge of the Triangle (p2 -> p0)
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-65, -25}, {10, -25}, {75, -5}}));
  EXPECT_TRUE(Intersects(Triangle{{75, -5}, {-65, -25}, {10, -25}},
                         Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Flat Triangle
  EXPECT_TRUE(Intersects(Triangle{{-20, -40}, {40, 20}, {80, 60}},
                         Triangle{{-40, -60}, {-35, -55}, {10, -10}}));
  EXPECT_TRUE(Intersects(Triangle{{10, -10}, {-40, -60}, {-35, -55}},
                         Triangle{{-20, -40}, {40, 20}, {80, 60}}));
}

TEST(IntersectsTest, TriangleToTriangleFullyOutside) {
  // Slightly too far in the negative x direction to overlap the "first" edge of
  // the Triangle (p0 -> p1)
  EXPECT_FALSE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                          Triangle{{-35.01f, -29}, {-0.01f, 34}, {-40, 20}}));
  EXPECT_FALSE(Intersects(Triangle{{-40, 20}, {-35.01f, -29}, {-0.01f, 34}},
                          Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Slightly too far in the positive x direction to overlap the "first" edge of
  // the Triangle (p1 -> p2)
  EXPECT_FALSE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                          Triangle{{20, 20}, {4.01f, 18}, {31.01f, -3}}));
  EXPECT_FALSE(Intersects(Triangle{{4.01f, 18}, {31.01f, -3}, {20, 20}},
                          Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
  // Slightly too far in the negative y direction to overlap the "third" edge of
  // the Triangle (p2 -> p0)
  EXPECT_FALSE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                          Triangle{{-65, -25.01f}, {10, -25}, {75, -5.01f}}));
  EXPECT_FALSE(Intersects(Triangle{{75, -5.01f}, {-65, -25.01f}, {10, -25}},
                          Triangle{{-30, -20}, {-5, 25}, {40, -10}}));
}

TEST(IntersectsTest, TriangleToTriangleTouchingCorners) {
  // One Triangle's p0 is the same as the other Triangle's p0
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-30, -20}, {-40, 5}, {-50, -90}}));
  // One Triangle's p0 is the same as the other Triangle's p1
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-40, 5}, {-30, -20}, {-50, -90}}));
  // One Triangle's p0 is the same as the other Triangle's p2
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-40, 5}, {-50, -90}, {-30, -20}}));
  // One Triangle's p1 is the same as the other Triangle's p1
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-35, 35}, {-5, 25}, {20, 60}}));
  // One Triangle's p1 is the same as the other Triangle's p2
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-35, 35}, {20, 60}, {-5, 25}}));
  // One Triangle's p2 is the same as the other Triangle's p2
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{40, -11}, {80, -20}, {40, -10}}));
  // Flat Triangle
  EXPECT_TRUE(Intersects(Triangle{{-20, -40}, {40, 20}, {80, 60}},
                         Triangle{{-20, -40}, {25, -15}, {15, -25}}));
  EXPECT_TRUE(Intersects(Triangle{{15, -25}, {-20, -40}, {25, -15}},
                         Triangle{{-20, -40}, {40, 20}, {80, 60}}));
}

TEST(IntersectsTest, TriangleToTriangleCornerTouchingEdge) {
  // One Triangle's "first" side is touched by the other Triangle's p0, p1, or
  // p2 respectively
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-25, -11}, {-40, 5}, {-50, -5}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-40, 5}, {-25, -11}, {-50, -5}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{-40, 5}, {-50, -5}, {-25, -11}}));
  // One Triangle's "second" side is touched by the other Triangle's p0, p1, or
  // p2 respectively
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{13, 11}, {20, 60}, {40, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{20, 60}, {13, 11}, {40, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{20, 60}, {40, 10}, {13, 11}}));
  // One Triangle's "third" side is touched by the other Triangle's p0, p1, or
  // p2 respectively
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{19, -13}, {80, -20}, {40, -40}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{80, -20}, {19, -13}, {40, -40}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, -20}, {-5, 25}, {40, -10}},
                         Triangle{{80, -20}, {40, -40}, {19, -13}}));
  // Malformed Triangles
  // Flat Triangle
  EXPECT_TRUE(Intersects(Triangle{{-20, -40}, {40, 20}, {80, 60}},
                         Triangle{{10, -10}, {25, -15}, {15, -25}}));
  EXPECT_TRUE(Intersects(Triangle{{25, -15}, {15, -25}, {10, -10}},
                         Triangle{{-20, -40}, {40, 20}, {80, 60}}));
  // Point-like Triangle
  EXPECT_TRUE(Intersects(Triangle{{10, -10}, {10, -10}, {10, -10}},
                         Triangle{{10, -10}, {25, -15}, {15, -25}}));
  EXPECT_TRUE(Intersects(Triangle{{25, -15}, {15, -25}, {10, -10}},
                         Triangle{{10, -10}, {10, -10}, {10, -10}}));
}

TEST(IntersectsTest, TriangleToRectEdgeIntersecting) {
  // Intersections where a Point of the Triangle is inside the Rect.
  // Intersects the "right" side of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{28, 2}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {28, 2}, {50, -20}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "bottom" of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-16, -10}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {50, -20}, {-16, -10}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top" of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-19, -5}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, 10}, {-26, 20}, {-19, -5}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "left" side of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-10, 4}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-26, 20}, {-10, 4}, {-30, 10}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));

  // Intersections where the Triangle crosses through the Rect but does not have
  // a Point within it.
  // Intersects the "left" and "top" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-35, 30}, {55, 40}, {-25, -10}}));
  EXPECT_TRUE(Intersects(Triangle{{55, 40}, {-35, 30}, {-25, -10}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "left" and "bottom" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-40, -40}, {-24, 2}, {5, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{-24, 2}, {5, -20}, {-40, -40}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "left" and "right" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{65, -15}, {60, 60}, {-50, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-50, 10}, {65, -15}, {60, 60}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top" and "right" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{65, -15}, {60, 60}, {-10, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{60, 60}, {-10, 10}, {65, -15}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "bottom" and "right" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{25, -15}, {60, 60}, {65, -15}}));
  EXPECT_TRUE(Intersects(Triangle{{65, -15}, {60, 60}, {25, -15}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top" and "bottom" sides of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{20, 40}, {100, 0}, {0, -60}}));
  EXPECT_TRUE(Intersects(Triangle{{0, -60}, {20, 40}, {100, 0}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
}

TEST(IntersectsTest, TriangleToRectFullyContained) {
  // The Triangle is contained within the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 50}, {-45, 95}),
                         Triangle{{-99, 80}, {-60, 94}, {-46, 52}}));
  EXPECT_TRUE(Intersects(Triangle{{-60, 94}, {-46, 52}, {-99, 80}},
                         Rect::FromTwoPoints({-100, 50}, {-45, 95})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-1000, -500}, {450, 950}),
                         Triangle{{-99, 80}, {-99, 81}, {-98, 80}}));
  EXPECT_TRUE(Intersects(Triangle{{-98, 80}, {-99, 80}, {-99, 81}},
                         Rect::FromTwoPoints({-1000, -500}, {450, 950})));

  // The Rect is contained within the Triangle.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 50}, {-45, 95}),
                         Triangle{{-50, 200}, {-200, 40}, {100, 30}}));
  EXPECT_TRUE(Intersects(Triangle{{100, 30}, {-50, 200}, {-200, 40}},
                         Rect::FromTwoPoints({-100, 50}, {-45, 95})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-80, -50}, {-10, -8}),
                         Triangle{{0, 0}, {-200, 0}, {0, -200}}));
  EXPECT_TRUE(Intersects(Triangle{{-200, 0}, {0, -200}, {0, 0}},
                         Rect::FromTwoPoints({-80, -50}, {-10, -8})));
}

TEST(IntersectsTest, TriangleToRectFullyOutside) {
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                 Triangle{{-100.01f, -500}, {-100.01f, 500}, {-101, 0}}));
  EXPECT_FALSE(
      Intersects(Triangle{{-101, 0}, {-100.01f, -500}, {-100.01f, 500}},
                 Rect::FromTwoPoints({-100, -50}, {50, 100})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                          Triangle{{50.01f, -500}, {50.01f, 500}, {501, 0}}));
  EXPECT_FALSE(Intersects(Triangle{{501, 0}, {50.01f, -500}, {50.01f, 500}},
                          Rect::FromTwoPoints({-100, -50}, {50, 100})));
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                 Triangle{{-200, -50.01f}, {200, -50.01f}, {0, -100}}));
  EXPECT_FALSE(Intersects(Triangle{{0, -100}, {-200, -50.01f}, {200, -50.01f}},
                          Rect::FromTwoPoints({-100, -50}, {50, 100})));
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                 Triangle{{-200, 100.01f}, {200, 100.01f}, {0, 1000}}));
  EXPECT_FALSE(Intersects(Triangle{{200, 100.01f}, {0, 1000}, {-200, 100.01f}},
                          Rect::FromTwoPoints({-100, -50}, {50, 100})));
}

TEST(IntersectsTest, TriangleToRectOverlappingEdges) {
  // A side of the Triangle overlaps the "bottom" of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                         Triangle{{-100, -500}, {-100, 500}, {-101, 0}}));
  EXPECT_TRUE(Intersects(Triangle{{-101, 0}, {-100, -500}, {-100, 500}},
                         Rect::FromTwoPoints({-100, -50}, {50, 100})));
  // A side of the Triangle overlaps the "right" side of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                         Triangle{{50, -500}, {50, 500}, {501, 0}}));
  EXPECT_TRUE(Intersects(Triangle{{501, 0}, {50, -500}, {50, 500}},
                         Rect::FromTwoPoints({-100, -50}, {50, 100})));
  // A side of the Triangle overlaps the "left" side of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                         Triangle{{-200, -50}, {200, -50}, {0, -100}}));
  EXPECT_TRUE(Intersects(Triangle{{0, -100}, {-200, -50}, {200, -50}},
                         Rect::FromTwoPoints({-100, -50}, {50, 100})));
  // A side of the Triangle overlaps the "top" of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -50}, {50, 100}),
                         Triangle{{-200, 100}, {200, 100}, {0, 1000}}));
  EXPECT_TRUE(Intersects(Triangle{{200, 100}, {0, 1000}, {-200, 100}},
                         Rect::FromTwoPoints({-100, -50}, {50, 100})));
}

TEST(IntersectsTest, TriangleToRectTouchingCorners) {
  // Intersects the "bottom-right" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{30, -13}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {30, -13}, {50, -20}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top-right" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{30, 5}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {50, -20}, {30, 5}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top-left" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-20, 5}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, 10}, {-26, 20}, {-20, 5}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "bottom-left" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-20, -13}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-26, 20}, {-20, -13}, {-30, 10}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
}

TEST(IntersectsTest, TriangleToRectCornerTouchingEdge) {
  // Touches the "right" edge of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{30, -2}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {30, -2}, {50, -20}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Touches the "bottom" edge of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-10, -13}, {40, -30}, {50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{40, -30}, {50, -20}, {-10, -13}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "top-left" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{5, 5}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-30, 10}, {-26, 20}, {5, 5}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
  // Intersects the "bottom-left" corner of the Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -13}, {30, 5}),
                         Triangle{{-10, -5}, {-26, 20}, {-30, 10}}));
  EXPECT_TRUE(Intersects(Triangle{{-26, 20}, {-10, -5}, {-30, 10}},
                         Rect::FromTwoPoints({-20, -13}, {30, 5})));
}

TEST(IntersectsTest, TriangleToQuadEdgeIntersecting) {
  // Quad 1 Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);
  // Quad 2 Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);

  // Intersects the "right" side of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1, Triangle{{-37, -25}, {-40, -50}, {-30, -40}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-40, -50}, {-37, -25}, {-30, -40}}, test_quad_1));
  // Intersects the "bottom" of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1, Triangle{{-50, -32}, {-40, -50}, {-30, -40}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-40, -50}, {-30, -40}, {-50, -32}}, test_quad_1));
  // Intersects the "top" of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1, Triangle{{-30, -18}, {-50, 0}, {-40, 10}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-50, 0}, {-30, -18}, {-40, 10}}, test_quad_1));
  // Intersects the "left" side of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1, Triangle{{-46, -28}, {-50, 0}, {-40, 10}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-50, 0}, {-40, 10}, {-46, -28}}, test_quad_1));

  // Intersects the "right" side of test_quad_2
  EXPECT_TRUE(Intersects(test_quad_2, Triangle{{1, -8}, {20, 14}, {30, 6}}));
  EXPECT_TRUE(Intersects(Triangle{{20, 14}, {1, -8}, {30, 6}}, test_quad_2));
  // Intersects the "top" of test_quad_2
  EXPECT_TRUE(Intersects(test_quad_2, Triangle{{-1, 9}, {20, 14}, {30, 6}}));
  EXPECT_TRUE(Intersects(Triangle{{20, 14}, {30, 6}, {-1, 9}}, test_quad_2));
  // Intersects the "left" side of test_quad_2
  EXPECT_TRUE(
      Intersects(test_quad_2, Triangle{{-1, 4}, {-10, -20}, {-20, -10}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-10, -20}, {-20, -10}, {-1, 4}}, test_quad_2));
  // Intersects the "bottom" of test_quad_2
  EXPECT_TRUE(
      Intersects(test_quad_2, Triangle{{1, -10}, {-10, -20}, {-20, -10}}));
  EXPECT_TRUE(
      Intersects(Triangle{{-20, -10}, {1, -10}, {-10, -20}}, test_quad_2));
}

TEST(IntersectsTest, TriangleToQuadFullyContained) {
  // The Triangle is contained within the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Triangle{{-50, -32}, {-42, -30}, {-30, -18}}));
  EXPECT_TRUE(Intersects(Triangle{{-50, -32}, {-42, -30}, {-30, -18}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Triangle{{-1, 9}, {-1, -6}, {1, -10}}));
  EXPECT_TRUE(Intersects(Triangle{{-1, 9}, {-1, -6}, {1, -10}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));

  // The Quad is contained within the Triangle.
  EXPECT_TRUE(Intersects(Triangle{{-70, -35}, {-20, -35}, {-20, 20}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Triangle{{-10, 30}, {-10, -50}, {40, 10}}));
}

TEST(IntersectsTest, TriangleToQuadFullyOutside) {
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Triangle{{-53, -32}, {-38, -17}, {-50, -20}}));
  EXPECT_FALSE(
      Intersects(Triangle{{-53, -32}, {-38, -17}, {-50, -20}},
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Triangle{{3, 5}, {5, -5}, {3, -14}}));
  EXPECT_FALSE(Intersects(Triangle{{3, 5}, {5, -5}, {3, -14}},
                          Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, TriangleToQuadOverlappingEdges) {
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Triangle{{2, 6}, {5, -5}, {2, -14}}));
  EXPECT_TRUE(Intersects(Triangle{{2, 6}, {5, -5}, {2, -14}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Triangle{{-53, -33}, {-37, -17}, {-50, -20}}));
  EXPECT_TRUE(Intersects(Triangle{{-53, -33}, {-37, -17}, {-50, -20}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The Triangle is slightly too far in the positive x direction to make
  // contact with the "right" side of the Quad.
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Triangle{{-26.99f, -16.99f}, {-42.99f, -33.01f}, {-25, -30}}));
  EXPECT_FALSE(
      Intersects(Triangle{{-26.99f, -16.99f}, {-42.99f, -33.01f}, {-25, -30}},
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, TriangleToQuadTouchingCorners) {
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Triangle{{1.99f, 6}, {10, 3}, {30, 0}}));
  EXPECT_TRUE(Intersects(Triangle{{2, -14}, {5, -5}, {30, 0}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Triangle{{-27, -17}, {-35, -35}, {-50, -50}}));
  EXPECT_TRUE(Intersects(Triangle{{-53, -33}, {-37, -17}, {-50, -20}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The corner of the Triangle is slightly too far in the positive x direction
  // to make contact with the "top-right" corner of the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Triangle{{-26.99, -17}, {-35, -35}, {-50, -50}}));
  // The corner of the Triangle is slightly too far in the negative y direction
  // to make contact with the "bottom-left" corner of the Quad.
  EXPECT_FALSE(
      Intersects(Triangle{{-53, -33.01f}, {-35, -35}, {-50, -50}},
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, TriangleToQuadCornerTouchingEdge) {
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Triangle{{1.99f, -5}, {10, 3}, {30, 0}}));
  EXPECT_TRUE(Intersects(Triangle{{0, 10}, {5, -5}, {30, 0}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Triangle{{-40, -30}, {-35, -35}, {-50, -50}}));
  EXPECT_TRUE(Intersects(Triangle{{-48, -33}, {-35, -35}, {-50, -50}},
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The Triangle is slightly too far in the positive x direction to make
  // contact with the "right" side of the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Triangle{{-39.99, -30}, {-35, -35}, {-50, -50}}));
  // The Triangle is slightly too far in the negative y direction to make
  // contact with the "bottom" of the Quad.
  EXPECT_FALSE(
      Intersects(Triangle{{-48, -33.01f}, {-35, -35}, {-50, -50}},
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, RectToRectEdgeIntersecting) {
  // Intersections where one Rect crosses only one side of the other Rect.
  // One Rect Intersects the "right" side of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({20, 10}, {200, 20})));
  // One Rect Intersects the "left" side of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, 10}, {-200, 20}),
                         Rect::FromTwoPoints({-100, -100}, {100, 100})));
  // One Rect Intersects the "top" of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({-10, 10}, {10, 200})));
  // One Rect Intersects the "bottom" of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-10, 10}, {10, -200}),
                         Rect::FromTwoPoints({-100, -100}, {100, 100})));

  // Intersections where one Rect contains a full side of the other and
  // intersects the 2 adjacent sides.
  // One Rect contains the "right" side and intersects the "top" and "bottom" of
  // the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({20, -200}, {200, 200})));
  // One Rect Contains the "left" side and intersects the "top" and "bottom" of
  // the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-20, -200}, {-200, 200}),
                         Rect::FromTwoPoints({-100, -100}, {100, 100})));
  // One Rect contains the "top" side and intersects the "right" and "left"
  // sides of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({-200, 20}, {200, 200})));
  // One Rect contains the "bottom" side and intersects the "right" and "left"
  // sides of the other Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-200, -20}, {200, -200}),
                         Rect::FromTwoPoints({-100, -100}, {100, 100})));
}

TEST(IntersectsTest, RectToRectFullyContained) {
  // The first Rect contains the second Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 50}, {100, -50}),
                         Rect::FromTwoPoints({-99, 49}, {99, -49})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-99999, -99999}, {99999, 99999}),
                         Rect::FromTwoPoints({-.001, -.001}, {.001, .001})));
  // The second Rect contains the first Rect.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-99, 49}, {99, -49}),
                         Rect::FromTwoPoints({-100, 50}, {100, -50})));
  EXPECT_TRUE(
      Intersects(Rect::FromTwoPoints({-.001, -.001}, {.001, .001}),
                 Rect::FromTwoPoints({-99999, -99999}, {99999, 99999})));

  // Point-like Rects.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 50}, {100, -50}),
                         Rect::FromTwoPoints({-1, 10}, {-1, 10})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-1, 10}, {-1, 10}),
                         Rect::FromTwoPoints({-100, 50}, {100, -50})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-1, 10}, {-1, 10}),
                         Rect::FromTwoPoints({-1, 10}, {-1, 10})));
}

TEST(IntersectsTest, RectToRectFullyOutside) {
  // The Rects cover the same range in the x direction but do not overlap y
  // direction.
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-1000, -35}, {1000, -1000}),
                          Rect::FromTwoPoints({-1000, -34}, {1000, 1000})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-1000, 72}, {1000, 1000}),
                          Rect::FromTwoPoints({-1000, 71}, {1000, -1000})));
  // The Rects cover the same range in the y direction but do not overlap x
  // direction.
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({525.555, -1000}, {1000, 1000}),
                 Rect::FromTwoPoints({525.554, -1000}, {-1000, 1000})));
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-123.456f, -1000}, {-1000, 1000}),
                 Rect::FromTwoPoints({-123.455f, -1000}, {1000, 1000})));
}

TEST(IntersectsTest, RectToRectOverlappingEdges) {
  // The right side of one Rect is overlapping the left side of the other.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({100, -100}, {200, 100})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-2.211f, -100}, {200, 100}),
                         Rect::FromTwoPoints({-100, -100}, {-2.211f, 100})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                          Rect::FromTwoPoints({100.001f, -100}, {200, 100})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-2.211f, -100}, {200, 100}),
                          Rect::FromTwoPoints({-100, -100}, {-2.2111f, 100})));

  // The top of one Rect is overlapping the bottom of the other.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({-100, 100}, {100, 200})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 32.222f}, {100, 200}),
                         Rect::FromTwoPoints({-100, -100}, {100, 32.222f})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                          Rect::FromTwoPoints({-100, 100.001f}, {100, 200})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, 32.222f}, {100, 200}),
                          Rect::FromTwoPoints({-100, -100}, {100, 32.2219f})));
}

TEST(IntersectsTest, RectToRectTouchingCorners) {
  // top-right/bottom-left side is overlapping.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                         Rect::FromTwoPoints({100, 100}, {200, 200})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({100, 100}, {101, 101}),
                         Rect::FromTwoPoints({-1, -1}, {100, 100})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, -100}, {100, 100}),
                          Rect::FromTwoPoints({100.001f, 100}, {200, 200})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({100, 100.001f}, {200, 200}),
                          Rect::FromTwoPoints({-100, -100}, {100, 100})));

  // top-left/bottom-right side is overlapping.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({100, 100}, {-100, -100}),
                         Rect::FromTwoPoints({-100, 100}, {-200, 200})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-100, 100}, {-100.01f, 100.01f}),
                         Rect::FromTwoPoints({100, 100}, {-100, -100})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, 100}, {100, -100}),
                          Rect::FromTwoPoints({-100.001f, 100}, {-200, 200})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-100, 100.001f}, {-200, 200}),
                          Rect::FromTwoPoints({-100, 100}, {100, -100})));
}

TEST(IntersectsTest, RectToQuadEdgeIntersecting) {
  // Quad 1 Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);

  // The Rect intersects just the "right" side of test_quad_1.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-41, -30}, {-30, -25})));
  // test_quad_1 intersects just the "right" side of the Rect.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-60, -35}, {-50, -25})));
  // The Rect intersects just the "left" side of test_quad_1.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-50, -28}, {-40, -25})));
  // test_quad_1 intersects just the "left" side of the Rect.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-30, -25}, {-20, -15})));

  // The Rect intersects just the "top" of test_quad_1.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-35, -20}, {-32, -15})));
  // test_quad_1 intersects just the "top" of the Rect.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-60, -40}, {-30, -30})));
  // The Rect intersects just the "bottom" of test_quad_1.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-48, -40}, {-45, -30})));
  // test_quad_1 intersects just the "bottom" of the Rect.
  EXPECT_TRUE(
      Intersects(test_quad_1, Rect::FromTwoPoints({-45, -20}, {-25, -15})));

  // Quad 2 Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);

  // The Rect intersects just the "right" side of test_quad_2.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-1, -5}, {5, 5})));
  // test_quad_2 intersects just the "right" side of the Rect.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-5, -20}, {1, 20})));
  // The Rect intersects just the "left" side of test_quad_2.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-5, -5}, {1, 5})));
  // test_quad_2 intersects just the "left" side of the Rect.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-1, -20}, {5, 20})));

  // The Rect intersects just the "top" of test_quad_2.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-1, -5}, {1, 20})));
  // test_quad_2 intersects just the "top" of the Rect.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-5, -20}, {5, 5})));
  // The Rect intersects just the "bottom" of test_quad_2.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-1, -20}, {1, 5})));
  // test_quad_2 intersects just the "bottom" of the Rect.
  EXPECT_TRUE(Intersects(test_quad_2, Rect::FromTwoPoints({-5, -5}, {5, 20})));
}

TEST(IntersectsTest, RectToQuadFullyContained) {
  // The Rect is contained within the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Rect::FromTwoPoints({-49, -32}, {-43, -30})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-42, -26}, {-38, -24}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Rect::FromTwoPoints({-1, -7}, {1, -2})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({1, -3}, {-1, 6}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));

  // The Quad is contained within the Rect.
  EXPECT_TRUE(
      Intersects(Rect::FromTwoPoints({-6000, -4000}, {-20, -10}),
                 Quad::FromCenterDimensionsRotationAndShear(
                     Point{-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Rect::FromTwoPoints({-1000, -2000}, {10, 20})));
}

TEST(IntersectsTest, RectToQuadFullyOutside) {
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Rect::FromTwoPoints({-36, -32}, {-29, -27})));
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-51, -23}, {-44, -18}),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Rect::FromTwoPoints({0, 11}, {5, 20})));
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({3, -20}, {5, 20}),
                          Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, RectToQuadOverlappingEdges) {
  // The "left" side of the Rect is overlapping the "right" side of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Rect::FromTwoPoints({2, -14}, {5, 6})));
  // The "right" side of the Rect is overlapping the "left" side of the Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-5, -6}, {-2, 14}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  // The "bottom" of the Rect is overlapping the "top" of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Rect::FromTwoPoints({-37, -17}, {-27, 40})));
  // The "top" of the Rect is overlapping the "bottom" of the Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-53, -33}, {-43, -50}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));

  // The "left" side of the Rect is slightly too far in the positive x direction
  // to overlap the "right" side of the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                          Rect::FromTwoPoints({2.001f, -14}, {5, 6})));
  // The "right" side of the Rect is slightly too far in the negative x
  // direction to overlap the "left" side of the Quad.
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-5, -6}, {-2.001f, 14}),
                          Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  // The "bottom" of the Rect is slightly too far in the positive y direction to
  // overlap the "top" of the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Rect::FromTwoPoints({-37, -16.999f}, {-27, 40})));
  // The "top" of the Rect is slightly too far in the negative y direction to
  // overlap the "bottom" of the Quad.
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-53, -33.001f}, {-43, -50}),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, RectToQuadTouchingCorners) {
  // The "top-left" corner of the Rect is overlapping the "bottom-right" corner
  // of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Rect::FromTwoPoints({2, -14}, {5, -20})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-43, -33}, {-20, -50}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The "top-right" corner of the Rect is overlapping the "bottom-left" corner
  // of the Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-1.99f, -6}, {-10, -14}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-53, -33}, {-80, -50}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The "bottom-left" corner of the Rect is overlapping the "top-right" corner
  // of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Rect::FromTwoPoints({-27, -17}, {-10, 40})));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Rect::FromTwoPoints({1.99f, 6}, {5, 20})));
  // The "bottom-right" corner of the Rect is overlapping the "top-left" corner
  // of the Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Rect::FromTwoPoints({-37, -17}, {-50, 40})));
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-2, 14}, {-10, 20}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, RectToQuadCornerTouchingEdge) {
  // The "left" side of the Rect is overlapped by the "top-right" corner of the
  // Quad.
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Rect::FromTwoPoints({-27, -30}, {-10, -10})));
  // The "right" side of the Rect is overlapped by the "bottom-left" corner of
  // the Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-53, -20}, {-80, -50}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The "bottom" of the Rect is overlapped by the "top-left" corner of the
  // Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-10, 14}, {10, 20}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  // The "top" of the Rect is overlapped by the "bottom-right" corner of the
  // Quad.
  EXPECT_TRUE(Intersects(Rect::FromTwoPoints({-10, -14}, {10, -20}),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));

  // The "left" side of the Rect is slightly too far in the positive x direction
  // to be overlapped by the "top-right" corner of the Quad.
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Rect::FromTwoPoints({-26.999f, -30}, {-10, -10})));
  // The "right" side of the Rect is slightly too far in the negative x
  // direction to be overlapped by the "bottom-left" corner of the Quad.
  EXPECT_FALSE(
      Intersects(Rect::FromTwoPoints({-53.001f, -20}, {-80, -50}),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The "bottom" of the Rect is slightly too far in the positive y direction to
  // be overlapped by the "top-left" corner of the Quad.
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-10, 14.001f}, {10, 20}),
                          Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  // The "top" of the Rect is slightly too far in the negative y direction to
  // be overlapped by the "bottom-right" corner of the Quad.
  EXPECT_FALSE(Intersects(Rect::FromTwoPoints({-10, -14.001f}, {10, -20}),
                          Quad::FromCenterDimensionsRotationAndShear(
                              {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, QuadToQuadEdgeIntersecting) {
  // Quad 1 Corners: (-53,-33), (-43,-33), (-27,-17), (-37,-17)
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);
  // Quad 2 Corners: (2, -14), (2, 6), (-2, 14), (-2, -6),
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);
  // Intersects the "right" side of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1,
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-30.0f, -20.0f}, 2.0f, 3.0f, Angle::Degrees(270), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-30.0f, -20.0f}, 2.0f, 3.0f, Angle::Degrees(270), 0.3f),
                 test_quad_1));
  // Intersects the "top" of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1,
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-34.0f, -17.0f}, 2.0f, 3.0f, Angle::Degrees(150), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-34.0f, -17.0f}, 2.0f, 3.0f, Angle::Degrees(150), 0.3f),
                 test_quad_1));
  // Intersects the "left" side of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1,
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-47.0f, -27.0f}, 2.0f, 3.0f, Angle::Degrees(20), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-47.0f, -27.0f}, 2.0f, 3.0f, Angle::Degrees(20), 0.3f),
                 test_quad_1));
  // Intersects the "bottom" of test_quad_1
  EXPECT_TRUE(
      Intersects(test_quad_1,
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-50.0f, -33.0f}, 2.0f, 3.0f, Angle::Degrees(300), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-50.0f, -33.0f}, 2.0f, 3.0f, Angle::Degrees(300), 0.3f),
                 test_quad_1));

  // Intersects the "right" side of test_quad_2
  EXPECT_TRUE(Intersects(
      test_quad_2, Quad::FromCenterDimensionsRotationAndShear(
                       {2.0f, -5.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {2.0f, -5.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f),
                 test_quad_2));
  // Intersects the "top" of test_quad_2
  EXPECT_TRUE(Intersects(
      test_quad_2, Quad::FromCenterDimensionsRotationAndShear(
                       {1.0f, 8.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {1.0f, 8.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f),
                 test_quad_2));
  // Intersects the "left" side of test_quad_2
  EXPECT_TRUE(Intersects(
      test_quad_2, Quad::FromCenterDimensionsRotationAndShear(
                       {-2.0f, 0.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-2.0f, 0.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f),
                 test_quad_2));
  // Intersects the "bottom" of test_quad_2
  EXPECT_TRUE(
      Intersects(test_quad_2,
                 Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, -10.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, -10.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.3f),
                 test_quad_2));
}

TEST(IntersectsTest, QuadToQuadFullyContained) {
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-43.0f, -27.0f}, 2.0f, 3.0f, Angle(), 0.1f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-43.0f, -27.0f}, 2.0f, 3.0f, Angle(), 0.1f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, -3.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.1f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, -3.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.1f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, QuadToQuadFullyOutside) {
  EXPECT_FALSE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                              {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                          Quad::FromCenterDimensionsRotationAndShear(
                              {-38.0f, -32.0f}, 2.0f, 3.0f, Angle(), 0.1f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-38.0f, -32.0f}, 2.0f, 3.0f, Angle(), 0.1f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-5.0f, -1.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.1f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-5.0f, -1.0f}, 2.0f, 3.0f, Angle::Radians(2.5f), 0.1f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
}

TEST(IntersectsTest, QuadToQuadOverlappingEdges) {
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {4.0f, -10.0f}, 20.0f, 4.0f, kQuarterTurn, 0.1f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {4.0f, -10.0f}, 20.0f, 4.0f, kQuarterTurn, 0.1f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-35.0f, -30.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-35.0f, -30.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The Quad centered at (-47, -47.01) is slightly too far in the -y direction
  // to make contact with the "bottom" edge of the Quad centered at (-40, -25).
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-35.0f, -30.01f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-35.0f, -30.01f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, QuadToQuadTouchingCorners) {
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-3.99f, -11.99f}, 20.0f, 4.0f, kQuarterTurn, 0.1f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-3.99f, -11.99f}, 20.0f, 4.0f, kQuarterTurn, 0.1f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-66.0f, -41.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-66.0f, -41.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The Quad centered at (-66.01, -41) is slightly too far in the -x direction
  // to make contact with the "top-left" corner of the Quad centered at (-40,
  // -25).
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-66.01f, -41.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-66.01f, -41.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, QuadToQuadCornerTouchingEdge) {
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-14.99f, -11.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-14.99f, -11.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-47.0f, -47.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_TRUE(Intersects(Quad::FromCenterDimensionsRotationAndShear(
                             {-47.0f, -47.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                         Quad::FromCenterDimensionsRotationAndShear(
                             {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
  // The Quad centered at (-47, -47.01) is slightly too far in the -y direction
  // to make contact with the "bottom" edge of the Quad centered at (-40, -25).
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-47.0f, -47.01f}, 20.0f, 4.0f, kQuarterTurn, 2.0f)));
  EXPECT_FALSE(
      Intersects(Quad::FromCenterDimensionsRotationAndShear(
                     {-47.0f, -47.01f}, 20.0f, 4.0f, kQuarterTurn, 2.0f),
                 Quad::FromCenterDimensionsRotationAndShear(
                     {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)));
}

TEST(IntersectsTest, PartitionedMeshToPointWithIdentityTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Point p0{1, -0.5};
  Point p1{3, 4};

  EXPECT_TRUE(Intersects(p0, shape, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(shape, AffineTransform::Identity(), p0));
  EXPECT_FALSE(Intersects(p1, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), p1));
}

TEST(IntersectsTest, PartitionedMeshToPointWithNormalTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Point p0{3, -1.5};
  Point p1{0, 0};
  AffineTransform transform0 = AffineTransform::Scale(2);
  AffineTransform transform1 = AffineTransform::Translate({2, 0});

  EXPECT_TRUE(Intersects(p0, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, p0));
  EXPECT_FALSE(Intersects(p1, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, p1));
}

TEST(IntersectsTest, PartitionedMeshToPointWithNonInvertibleTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Point p{3, 4};
  // This transform collapses the mesh to the segment (1, 4)-(5, 4).
  AffineTransform transform0{1, 0, 1, 0, 0, 4};
  // This transform collapses the mesh to the point (10, 5).
  AffineTransform transform1{0, 0, 10, 0, 0, 5};

  EXPECT_TRUE(Intersects(p, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, p));

  EXPECT_FALSE(Intersects(p, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, p));
}

TEST(IntersectsTest, PartitionedMeshToPointEmptyShape) {
  PartitionedMesh shape;
  Point p{5, 10};

  EXPECT_FALSE(Intersects(p, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), p));
  EXPECT_FALSE(
      Intersects(p, shape, AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(
      Intersects(shape, AffineTransform::Rotate(Angle::Degrees(45)), p));
  EXPECT_FALSE(Intersects(p, shape, AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(shape, AffineTransform{0, 0, 1, 1, 1, 1}, p));
}

TEST(IntersectsTest, PartitionedMeshToSegmentWithIdentityTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Segment s0{{1, 1}, {1, -1}};
  Segment s1{{3, 4}, {4, 5}};

  EXPECT_TRUE(Intersects(s0, shape, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(shape, AffineTransform::Identity(), s0));
  EXPECT_FALSE(Intersects(s1, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), s1));
}

TEST(IntersectsTest, PartitionedMeshToSegmentWithNormalTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Segment s0{{3, -1.5}, {5, -1.5}};
  Segment s1{{0, 0}, {0, 5}};
  AffineTransform transform0 = AffineTransform::Scale(2);
  AffineTransform transform1 = AffineTransform::Translate({2, 0});

  EXPECT_TRUE(Intersects(s0, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, s0));
  EXPECT_FALSE(Intersects(s1, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, s1));
}

TEST(IntersectsTest, PartitionedMeshToSegmentWithNonInvertibleTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Segment s{{3, 3}, {3, 5}};
  // This transform collapses the mesh to the segment (1, 4)-(5, 4).
  AffineTransform transform0{1, 0, 1, 0, 0, 4};
  // This transform collapses the mesh to the point (10, 5).
  AffineTransform transform1{0, 0, 10, 0, 0, 5};

  EXPECT_TRUE(Intersects(s, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, s));
  EXPECT_FALSE(Intersects(s, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, s));
}

TEST(IntersectsTest, PartitionedMeshToSegmentEmptyShape) {
  PartitionedMesh shape;
  Segment s{{5, 10}, {10, 20}};

  EXPECT_FALSE(Intersects(s, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), s));
  EXPECT_FALSE(
      Intersects(s, shape, AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(
      Intersects(shape, AffineTransform::Rotate(Angle::Degrees(45)), s));
  EXPECT_FALSE(Intersects(s, shape, AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(shape, AffineTransform{0, 0, 1, 1, 1, 1}, s));
}

TEST(IntersectsTest, PartitionedMeshToTriangleWithIdentityTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Triangle t0{{1, 1}, {1, -1}, {0.5, 0}};
  Triangle t1{{3, 4}, {4, 5}, {2, 6}};

  EXPECT_TRUE(Intersects(t0, shape, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(shape, AffineTransform::Identity(), t0));
  EXPECT_FALSE(Intersects(t1, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), t1));
}

TEST(IntersectsTest, PartitionedMeshToTriangleWithNormalTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Triangle t0{{3, -1.5}, {5, -1.5}, {4, -2}};
  Triangle t1{{0, 0}, {0, 5}, {-1, 3}};
  AffineTransform transform0 = AffineTransform::Scale(2);
  AffineTransform transform1 = AffineTransform::Translate({2, 0});

  EXPECT_TRUE(Intersects(t0, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, t0));
  EXPECT_FALSE(Intersects(t1, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, t1));
}

TEST(IntersectsTest, PartitionedMeshToTriangleWithNonInvertibleTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Triangle t{{3, 3}, {3, 5}, {5, 5}};
  // This transform collapses the mesh to the segment (1, 4)-(5, 4).
  AffineTransform transform0{1, 0, 1, 0, 0, 4};
  // This transform collapses the mesh to the point (10, 5).
  AffineTransform transform1{0, 0, 10, 0, 0, 5};

  EXPECT_TRUE(Intersects(t, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, t));
  EXPECT_FALSE(Intersects(t, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, t));
}

TEST(IntersectsTest, PartitionedMeshToTriangleEmptyShape) {
  PartitionedMesh shape;
  Triangle t{{5, 10}, {10, 20}, {50, 0}};

  EXPECT_FALSE(Intersects(t, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), t));
  EXPECT_FALSE(
      Intersects(t, shape, AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(
      Intersects(shape, AffineTransform::Rotate(Angle::Degrees(45)), t));
  EXPECT_FALSE(Intersects(t, shape, AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(shape, AffineTransform{0, 0, 1, 1, 1, 1}, t));
}

TEST(IntersectsTest, PartitionedMeshToRectWithIdentityTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Rect r0 = Rect::FromTwoPoints({1, 1}, {1, -1});
  Rect r1 = Rect::FromTwoPoints({3, 4}, {4, 5});

  EXPECT_TRUE(Intersects(r0, shape, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(shape, AffineTransform::Identity(), r0));
  EXPECT_FALSE(Intersects(r1, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), r1));
}

TEST(IntersectsTest, PartitionedMeshToRectWithNormalTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Rect r0 = Rect::FromTwoPoints({3, -1.5}, {5, -2});
  Rect r1 = Rect::FromTwoPoints({0, 0}, {-1, 5});
  AffineTransform transform0 = AffineTransform::Scale(2);
  AffineTransform transform1 = AffineTransform::Translate({2, 0});

  EXPECT_TRUE(Intersects(r0, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, r0));
  EXPECT_FALSE(Intersects(r1, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, r1));
}

TEST(IntersectsTest, PartitionedMeshToRectWithNonInvertibleTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Rect r = Rect::FromTwoPoints({3, 3}, {5, 5});
  // This transform collapses the mesh to the segment (1, 4)-(5, 4).
  AffineTransform transform0{1, 0, 1, 0, 0, 4};
  // This transform collapses the mesh to the point (10, 5).
  AffineTransform transform1{0, 0, 10, 0, 0, 5};

  EXPECT_TRUE(Intersects(r, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, r));
  EXPECT_FALSE(Intersects(r, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, r));
}

TEST(IntersectsTest, PartitionedMeshToRectEmptyShape) {
  PartitionedMesh shape;
  Rect r = Rect::FromTwoPoints({5, 10}, {10, 20});

  EXPECT_FALSE(Intersects(r, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), r));
  EXPECT_FALSE(
      Intersects(r, shape, AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(
      Intersects(shape, AffineTransform::Rotate(Angle::Degrees(45)), r));
  EXPECT_FALSE(Intersects(r, shape, AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(shape, AffineTransform{0, 0, 1, 1, 1, 1}, r));
}

TEST(IntersectsTest, PartitionedMeshToQuadWithIdentityTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Quad q0 = Quad::FromCenterDimensionsAndRotation({1, 0}, 0.5, 0.5,
                                                  Angle::Degrees(45));
  Quad q1 = Quad::FromCenterDimensionsRotationAndShear(
      {4, 4}, 2, 0.5, Angle::Degrees(-15), 0.5);

  EXPECT_TRUE(Intersects(q0, shape, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(shape, AffineTransform::Identity(), q0));
  EXPECT_FALSE(Intersects(q1, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), q1));
}

TEST(IntersectsTest, PartitionedMeshToQuadWithNormalTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Quad q0 = Quad::FromCenterAndDimensions({4, -3}, 1, 2);
  Quad q1 =
      Quad::FromCenterDimensionsAndRotation({0, 0}, 1, 1, Angle::Degrees(60));
  AffineTransform transform0 = AffineTransform::Scale(2);
  AffineTransform transform1 = AffineTransform::Translate({2, 0});

  EXPECT_TRUE(Intersects(q0, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, q0));
  EXPECT_FALSE(Intersects(q1, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, q1));
}

TEST(IntersectsTest, PartitionedMeshToQuadWithNonInvertibleTransform) {
  PartitionedMesh shape = MakeStraightLinePartitionedMesh(3);
  Quad q = Quad::FromCenterAndDimensions({4, 4}, 2, 2);
  // This transform collapses the mesh to the segment (1, 4)-(5, 4).
  AffineTransform transform0{1, 0, 1, 0, 0, 4};
  // This transform collapses the mesh to the point (10, 5).
  AffineTransform transform1{0, 0, 10, 0, 0, 5};

  EXPECT_TRUE(Intersects(q, shape, transform0));
  EXPECT_TRUE(Intersects(shape, transform0, q));
  EXPECT_FALSE(Intersects(q, shape, transform1));
  EXPECT_FALSE(Intersects(shape, transform1, q));
}

TEST(IntersectsTest, PartitionedMeshToQuadEmptyShape) {
  PartitionedMesh shape;
  Quad q = Quad::FromCenterDimensionsRotationAndShear({10, 10}, 20, 30,
                                                      Angle::Degrees(-75), -1);

  EXPECT_FALSE(Intersects(q, shape, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(shape, AffineTransform::Identity(), q));
  EXPECT_FALSE(
      Intersects(q, shape, AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(
      Intersects(shape, AffineTransform::Rotate(Angle::Degrees(45)), q));
  EXPECT_FALSE(Intersects(q, shape, AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(shape, AffineTransform{0, 0, 1, 1, 1, 1}, q));
}

TEST(IntersectsTest, PartitionedMeshToPartitionedMeshWithIdentityTransform) {
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);
  PartitionedMesh ring_at_origin = MakeCoiledRingPartitionedMesh(12, 6);
  PartitionedMesh ring_with_offset = MakeCoiledRingPartitionedMesh(
      12, 6, {}, AffineTransform::Translate({2, 2}));

  EXPECT_TRUE(Intersects(line_at_origin, AffineTransform::Identity(),
                         ring_at_origin, AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(ring_at_origin, AffineTransform::Identity(),
                         line_at_origin, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Identity(),
                          ring_with_offset, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(ring_with_offset, AffineTransform::Identity(),
                          line_at_origin, AffineTransform::Identity()));
}

TEST(IntersectsTest, PartitionedMeshToPartitionedMeshWithNormalTransform) {
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);
  PartitionedMesh ring_at_origin = MakeCoiledRingPartitionedMesh(12, 6);
  PartitionedMesh ring_with_offset = MakeCoiledRingPartitionedMesh(
      12, 6, {}, AffineTransform::Translate({2, 2}));

  EXPECT_TRUE(Intersects(line_at_origin, AffineTransform::Translate({0, 1}),
                         ring_with_offset,
                         AffineTransform::ScaleAboutPoint(1.5, {2, 2})));
  EXPECT_TRUE(Intersects(ring_with_offset,
                         AffineTransform::ScaleAboutPoint(1.5, {2, 2}),
                         line_at_origin, AffineTransform::Translate({0, 1})));
  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Scale(.1),
                          ring_at_origin,
                          AffineTransform::Translate({0.5, 0.2})));
  EXPECT_FALSE(Intersects(ring_at_origin,
                          AffineTransform::Translate({0.5, 0.2}),
                          line_at_origin, AffineTransform::Scale(.1)));
}

TEST(IntersectsTest,
     PartitionedMeshToPartitionedMeshWithOneNonInvertibleTransform) {
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);
  PartitionedMesh ring_at_origin = MakeCoiledRingPartitionedMesh(12, 6);
  PartitionedMesh ring_with_offset = MakeCoiledRingPartitionedMesh(
      12, 6, {}, AffineTransform::Translate({2, 2}));

  // This transform collapses all shapes to the line y = x / 2.
  AffineTransform transform0{1, 1, 0, 0.5, 0.5, 0};
  // This transform collapses all shapes to the point (2, 1).
  AffineTransform transform1{0, 0, 2, 0, 0, 1};

  EXPECT_TRUE(Intersects(line_at_origin, transform0, ring_with_offset,
                         AffineTransform::Identity()));
  EXPECT_TRUE(Intersects(ring_with_offset, AffineTransform::Identity(),
                         line_at_origin, transform0));
  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Identity(),
                          ring_at_origin, transform1));
  EXPECT_FALSE(Intersects(ring_at_origin, transform1, line_at_origin,
                          AffineTransform::Identity()));
}

TEST(IntersectsTest,
     PartitionedMeshToPartitionedMeshWithTwoNonInvertibleTransforms) {
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);
  PartitionedMesh ring_at_origin = MakeCoiledRingPartitionedMesh(12, 6);

  // This transform collapses all shapes to the line y = x / 5.
  AffineTransform transform0{5, 5, 0, 1, 1, 0};
  // This transform collapses all shapes to the line x = 0.
  AffineTransform transform1{0, 0, 0, 0, 10, 5};
  // This transform collapses all shapes to the line x = 2.
  AffineTransform transform2{0, 0, 2, 0, 10, 5};

  EXPECT_TRUE(
      Intersects(line_at_origin, transform0, ring_at_origin, transform1));
  EXPECT_TRUE(
      Intersects(ring_at_origin, transform1, line_at_origin, transform0));
  EXPECT_FALSE(
      Intersects(line_at_origin, transform1, ring_at_origin, transform2));
  EXPECT_FALSE(
      Intersects(ring_at_origin, transform2, line_at_origin, transform1));
}

TEST(IntersectsTest, PartitionedMeshToPartitionedMeshEmptyShape) {
  PartitionedMesh empty;
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);

  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Identity(), empty,
                          AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(empty, AffineTransform::Identity(), line_at_origin,
                          AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Identity(), empty,
                          AffineTransform::Rotate(Angle::Degrees(45))));
  EXPECT_FALSE(Intersects(empty, AffineTransform::Rotate(Angle::Degrees(45)),
                          line_at_origin, AffineTransform::Identity()));
  EXPECT_FALSE(Intersects(line_at_origin, AffineTransform::Identity(), empty,
                          AffineTransform{0, 0, 1, 1, 1, 1}));
  EXPECT_FALSE(Intersects(empty, AffineTransform{0, 0, 1, 1, 1, 1},
                          line_at_origin, AffineTransform::Identity()));
}

TEST(IntersectsTest,
     PartitionedMeshToPartitionedMeshEmptyShapeWithTwoNonInvertibleTransforms) {
  PartitionedMesh empty;
  PartitionedMesh line_at_origin = MakeStraightLinePartitionedMesh(3);

  // This transform collapses all shapes to the line y = x / 5.
  AffineTransform transform0{5, 5, 0, 1, 1, 0};
  // This transform collapses all shapes to the line x = 0.
  AffineTransform transform1{0, 0, 0, 0, 10, 5};

  EXPECT_FALSE(Intersects(empty, transform0, line_at_origin, transform1));
  EXPECT_FALSE(Intersects(line_at_origin, transform1, empty, transform0));
}

}  // namespace
}  // namespace ink
