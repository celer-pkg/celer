#include "ink/geometry/internal/polyline_processing.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/internal/static_rtree.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"

namespace ink::geometry_internal {

namespace {

const float kMinWalkDistance = 2.0f;
const float kMaxConnectionDistance = 1.1f;
const float kMinConnectionRatio = 2.0f;
const float kMinTrimmingRatio = 1.8f;

MATCHER(PointsEq, "") {
  const Point& p1 = std::get<0>(arg);
  const Point& p2 = std::get<1>(arg);
  return testing::ExplainMatchResult(PointEq(p1), p2, result_listener);
}

PolylineData CreateWalkingPolylineData() {
  std::vector<Point> walking_points = {
      Point{3, 3},   Point{3, 10}, Point{3, 20}, Point{10, 20},
      Point{10, 15}, Point{5, 15}, Point{5, 0},  Point{2, 0}};
  // Individual Segment lengths: 7, 10, 7, 5, 5, 15, 3
  return CreateNewPolylineData(walking_points);
}

// function for use in rtree traversals.
auto segment_bounds = [](SegmentBundle segment_data) {
  return Rect::FromTwoPoints(segment_data.segment.start,
                             segment_data.segment.end);
};

PolylineData CreatePolylineAndFindIntersections(
    std::vector<Point> points, float min_walk_distance = 0.0f) {
  PolylineData output_polyline = CreateNewPolylineData(points);
  output_polyline.min_walk_distance = min_walk_distance;
  ink::geometry_internal::StaticRTree<SegmentBundle> rtree(
      output_polyline.segments, segment_bounds);
  FindFirstAndLastIntersections(rtree, output_polyline);
  return output_polyline;
}

PolylineData CreatePolylineAndFindBestConnections(
    std::vector<Point> points, float min_walk_distance = kMinWalkDistance,
    float max_connection_distance = kMaxConnectionDistance,
    float min_connection_ratio = kMinConnectionRatio,
    float min_trimming_ratio = kMinTrimmingRatio) {
  PolylineData output_polyline = CreateNewPolylineData(points);

  output_polyline.min_walk_distance = min_walk_distance;
  output_polyline.max_connection_distance = max_connection_distance;
  output_polyline.min_connection_ratio = min_connection_ratio;
  output_polyline.min_trimming_ratio = min_trimming_ratio;

  ink::geometry_internal::StaticRTree<SegmentBundle> rtree(
      output_polyline.segments, segment_bounds);
  FindFirstAndLastIntersections(rtree, output_polyline);
  FindBestEndpointConnections(rtree, output_polyline);
  return output_polyline;
}

TEST(PolylineProcessingTest, WalkDistanceReturnsCorrectValuesAtEndPoints) {
  PolylineData walking_polyline = CreateWalkingPolylineData();
  EXPECT_EQ(WalkDistance(walking_polyline, 0, 0.0f, false), 0.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 6, 1.0f, true), 0.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 0, 0.0f, true), 52.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 6, 1.0f, false), 52.0f);
}

TEST(PolylineProcessingTest, WalkDistanceReturnsCorrectValuesAtMidPoints) {
  PolylineData walking_polyline = CreateWalkingPolylineData();
  EXPECT_EQ(WalkDistance(walking_polyline, 0, 0.5f, false), 3.5f);
  EXPECT_EQ(WalkDistance(walking_polyline, 0, 0.5f, true), 48.5f);

  EXPECT_EQ(WalkDistance(walking_polyline, 1, 0.4f, false), 11.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 1, 0.4f, true), 41.0f);

  EXPECT_EQ(WalkDistance(walking_polyline, 5, 0.6f, false), 43.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 5, 0.6f, true), 9.0f);

  EXPECT_EQ(WalkDistance(walking_polyline, 6, 0.5f, false), 50.5f);
  EXPECT_EQ(WalkDistance(walking_polyline, 6, 0.5f, true), 1.5f);
}

TEST(PolylineProcessingTest,
     WalkDistanceReturnsEqualValuesForEquivalentIndices) {
  PolylineData walking_polyline = CreateWalkingPolylineData();
  EXPECT_EQ(WalkDistance(walking_polyline, 2, 1.0f, false), 24.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 2, 1.0f, true), 28.0f);

  EXPECT_EQ(WalkDistance(walking_polyline, 3, 0.0f, false), 24.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 3, 0.0f, true), 28.0f);
}

TEST(PolylineProcessingTest, WalkDistanceReturnsEqualValuesAtMidpoint) {
  PolylineData walking_polyline = CreateWalkingPolylineData();
  EXPECT_EQ(WalkDistance(walking_polyline, 3, 0.4f, false), 26.0f);
  EXPECT_EQ(WalkDistance(walking_polyline, 3, 0.4f, true), 26.0f);
}

TEST(PolylineProcessingTest,
     CreateNewPolylineDataCreatesCorrectSegmentBundles) {
  PolylineData walking_polyline = CreateWalkingPolylineData();
  EXPECT_EQ(walking_polyline.segments.size(), 7);

  EXPECT_EQ(walking_polyline.segments[0].index, 0);
  EXPECT_EQ(walking_polyline.segments[0].length, 7);
  EXPECT_THAT(walking_polyline.segments[0].segment,
              SegmentEq(Segment{{3, 3}, {3, 10}}));

  EXPECT_EQ(walking_polyline.segments[3].index, 3);
  EXPECT_EQ(walking_polyline.segments[3].length, 5);
  EXPECT_THAT(walking_polyline.segments[3].segment,
              SegmentEq(Segment{{10, 20}, {10, 15}}));

  EXPECT_EQ(walking_polyline.segments[5].index, 5);
  EXPECT_EQ(walking_polyline.segments[5].length, 15);
  EXPECT_THAT(walking_polyline.segments[5].segment,
              SegmentEq(Segment{{5, 15}, {5, 0}}));

  EXPECT_EQ(walking_polyline.segments[6].index, 6);
  EXPECT_EQ(walking_polyline.segments[6].length, 3);
  EXPECT_THAT(walking_polyline.segments[6].segment,
              SegmentEq(Segment{{5, 0}, {2, 0}}));
}

TEST(PolylineProcessingTest,
     CreateNewPolylineDataDiscardsDuplicatePointsAtPolylineStart) {
  std::vector<Point> points = {
      Point{5, 3},   Point{5, 3},   Point{5, 3},   Point{5, 3},   Point{5, 3},
      Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25}, Point{20, 30},
      Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{5, 3}};
  PolylineData polyline = CreateNewPolylineData(points);

  EXPECT_EQ(polyline.segments.size(), 9);
  EXPECT_THAT(polyline.segments.front().segment,
              SegmentEq(Segment{{5, 3}, {5, 8}}));
  EXPECT_THAT(polyline.segments.back().segment,
              SegmentEq(Segment{{11, 3}, {5, 3}}));
}

TEST(PolylineProcessingTest,
     CreateNewPolylineDataDiscardsDuplicatePointsAtPolylineEnd) {
  std::vector<Point> points = {
      Point{1, 9},  Point{-1, 13}, Point{1, 17},  Point{6, 19},  Point{14, 15},
      Point{18, 9}, Point{16, 3},  Point{10, 0},  Point{4, 3},   Point{2, 9},
      Point{6, 15}, Point{14, 19}, Point{19, 17}, Point{21, 13}, Point{19, 9},
      Point{19, 9}, Point{19, 9},  Point{19, 9},  Point{19, 9},  Point{19, 9}};
  PolylineData polyline = CreateNewPolylineData(points);

  EXPECT_EQ(polyline.segments.size(), 14);
  EXPECT_THAT(polyline.segments.front().segment,
              SegmentEq(Segment{{1, 9}, {-1, 13}}));
  EXPECT_THAT(polyline.segments.back().segment,
              SegmentEq(Segment{{21, 13}, {19, 9}}));
}

TEST(PolylineProcessingTest,
     CreateNewPolylineDataDiscardsDuplicatePointsInPolylineMiddle) {
  std::vector<Point> points = {
      Point{1, 9},   Point{-1, 13}, Point{1, 17},  Point{1, 17},  Point{1, 17},
      Point{1, 17},  Point{6, 19},  Point{14, 15}, Point{18, 9},  Point{16, 3},
      Point{10, 0},  Point{10, 0},  Point{10, 0},  Point{4, 3},   Point{2, 9},
      Point{6, 15},  Point{6, 15},  Point{14, 19}, Point{19, 17}, Point{21, 13},
      Point{21, 13}, Point{21, 13}, Point{21, 13}, Point{21, 13}, Point{21, 13},
      Point{21, 13}, Point{19, 9}};
  PolylineData polyline = CreateNewPolylineData(points);

  EXPECT_EQ(polyline.segments.size(), 14);
  EXPECT_THAT(polyline.segments[2].segment,
              SegmentEq(Segment{{1, 17}, {6, 19}}));
  EXPECT_THAT(polyline.segments[7].segment,
              SegmentEq(Segment{{10, 0}, {4, 3}}));
  EXPECT_THAT(polyline.segments[10].segment,
              SegmentEq(Segment{{6, 15}, {14, 19}}));
  EXPECT_THAT(polyline.segments[12].segment,
              SegmentEq(Segment{{19, 17}, {21, 13}}));
  EXPECT_THAT(polyline.segments[13].segment,
              SegmentEq(Segment{{21, 13}, {19, 9}}));
}

TEST(PolylineProcessingTest, IntersectionsForPerfectlyClosedLoop) {
  std::vector<Point> points = {
      Point{5, 3},   Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{5, 3}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 0);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 0.0f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.last_intersection.index_int, 8);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 1.0f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest, IntersectionsWithExplicitIntersectionPoint) {
  std::vector<Point> points = {
      Point{-8, 25}, Point{-3, 23}, Point{2, 21},  Point{6, 19},  Point{10, 17},
      Point{14, 15}, Point{18, 9},  Point{16, 3},  Point{10, 0},  Point{4, 3},
      Point{2, 9},   Point{6, 15},  Point{10, 17}, Point{14, 19}, Point{19, 22},
      Point{24, 24}, Point{30, 26}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 3);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 1.0f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{10, 17}));

  EXPECT_EQ(polyline.last_intersection.index_int, 12);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 0.0f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{10, 17}));
}

TEST(PolylineProcessingTest, IntersectionsWithImplicitIntersectionPoint) {
  std::vector<Point> points = {Point{-8, 25}, Point{-3, 23}, Point{2, 21},
                               Point{6, 19},  Point{14, 15}, Point{18, 9},
                               Point{16, 3},  Point{10, 0},  Point{4, 3},
                               Point{2, 9},   Point{6, 15},  Point{14, 19},
                               Point{19, 22}, Point{24, 24}, Point{30, 26}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 3);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{10, 17}));

  EXPECT_EQ(polyline.last_intersection.index_int, 10);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{10, 17}));
}

TEST(PolylineProcessingTest, IntersectionsWithUnconnectedLineReturnsFalse) {
  std::vector<Point> points = {Point{-1, 0}, Point{5, 1},  Point{10, 2},
                               Point{15, 4}, Point{20, 6}, Point{25, 9}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, false);
}

TEST(PolylineProcessingTest,
     IntersectionsWithCloseButUnconnectedLineReturnsFalse) {
  std::vector<Point> points = {
      Point{19, 22}, Point{14, 19}, Point{10.05f, 17}, Point{14, 15},
      Point{18, 9},  Point{16, 3},  Point{10, 0},      Point{4, 3},
      Point{2, 9},   Point{6, 15},  Point{9.95f, 17},  Point{6, 19},
      Point{2, 21}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, false);
}

TEST(PolylineProcessingTest,
     IntersectionsDiscardsIntersectionsWithTooShortWalkDistance) {
  std::vector<Point> points = {
      Point{19, 22}, Point{14, 19},     Point{10.05f, 17},
      Point{14, 15}, Point{18, 9},      Point{16, 3},
      Point{10, 0},  Point{9.9f, -.1f}, Point{10.1f, -.1f},
      Point{10, 0},  Point{4, 3},       Point{2, 9},
      Point{6, 15},  Point{9.95f, 17},  Point{6, 19},
      Point{2, 21}};

  PolylineData polyline_with_walk_distance =
      CreatePolylineAndFindIntersections(points, 2.0f);

  EXPECT_EQ(polyline_with_walk_distance.has_intersection, false);
}

TEST(PolylineProcessingTest,
     IntersectionsWithExplictlyOverlappingLineSegments) {
  std::vector<Point> points = {
      Point{18, 21}, Point{14, 19}, Point{12, 17}, Point{8, 17}, Point{6, 15},
      Point{2, 9},   Point{4, 3},   Point{10, 0},  Point{16, 3}, Point{18, 9},
      Point{14, 15}, Point{12, 17}, Point{8, 17},  Point{6, 19}, Point{2, 21}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 1);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 1.0f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{12, 17}));

  EXPECT_EQ(polyline.last_intersection.index_int, 12);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 0.0f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{8, 17}));
}

TEST(PolylineProcessingTest,
     IntersectionsWithImplicitlyOverlappingLineSegments) {
  std::vector<Point> points = {
      Point{18, 21}, Point{14, 19}, Point{12, 17}, Point{8, 17}, Point{6, 15},
      Point{2, 9},   Point{4, 3},   Point{10, 0},  Point{16, 3}, Point{18, 9},
      Point{14, 15}, Point{11, 17}, Point{9, 17},  Point{6, 19}, Point{2, 21}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 2);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 0.25f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{11, 17}));

  EXPECT_EQ(polyline.last_intersection.index_int, 12);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 0.0f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{9, 17}));
}

TEST(PolylineProcessingTest, IntersectionsWithMultipleIntersections) {
  std::vector<Point> points = {Point{4, 9},    Point{2.5, 9},   Point{1.5, 9},
                               Point{5.5, 15}, Point{10, 17},   Point{14, 15},
                               Point{18, 9},   Point{16, 3},    Point{10, 0},
                               Point{4, 3},    Point{2, 9},     Point{6, 15},
                               Point{10, 17},  Point{14.5, 15}, Point{18.5, 9},
                               Point{17.5, 9}, Point{16, 9}};
  PolylineData polyline = CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.first_intersection.index_int, 1);
  EXPECT_EQ(polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{2, 9}));

  EXPECT_EQ(polyline.last_intersection.index_int, 14);
  EXPECT_EQ(polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{18, 9}));
}

TEST(PolylineProcessingTest, BestConnectionsWithTinyMaxConnectionDistance) {
  std::vector<Point> points = {Point{5, 3.1f}, Point{5, 8},   Point{5, 15},
                               Point{10, 20},  Point{15, 25}, Point{20, 30},
                               Point{30, 20},  Point{20, 10}, Point{11, 3},
                               Point{5.1f, 3}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 0.1f, kMinConnectionRatio, kMinTrimmingRatio);

  EXPECT_EQ(polyline.has_intersection, false);

  EXPECT_EQ(polyline.connect_first, false);
  EXPECT_EQ(polyline.connect_last, false);
}

TEST(PolylineProcessingTest, BestConnectionsWithLargeMaxConnectionDistance) {
  std::vector<Point> points = {
      Point{5, 7},   Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{1, 3}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 4.5f, kMinConnectionRatio, kMinTrimmingRatio);

  EXPECT_EQ(polyline.has_intersection, true);

  // The connection distance is large enough that the first point is able to
  // connect with a connection length of 4.
  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.connect_last, false);
}

TEST(PolylineProcessingTest,
     BestConnectionsWithVeryLargeMaxConnectionDistance) {
  std::vector<Point> points = {
      Point{5, 7},   Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{1, 3}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 400.0f, kMinConnectionRatio, kMinTrimmingRatio);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  // The connection distance is large enough that the last point is able to
  // connect with the first point.
  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 7}));
}

TEST(PolylineProcessingTest, BestConnectionsNormalTrimmingRatio) {
  std::vector<Point> points = {
      Point{6, 23}, Point{8, 21},  Point{11, 18}, Point{14, 15}, Point{18, 9},
      Point{16, 3}, Point{10, 0},  Point{4, 3},   Point{2, 9},   Point{6, 15},
      Point{9, 18}, Point{12, 21}, Point{14, 23}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 1000.0f, kMinConnectionRatio,
      kMinTrimmingRatio);
  // Even with a large max connection distance, a polyline endpoint will not
  // connect if it is part of a straight extension, which is defined by the
  // trimming ratio, and should be trimmed.
  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, false);
  EXPECT_EQ(polyline.connect_last, false);
}

TEST(PolylineProcessingTest, BestConnectionsWithSmallTrimmingRatio) {
  std::vector<Point> points = {
      Point{6, 23}, Point{8, 21},  Point{11, 18}, Point{14, 15}, Point{18, 9},
      Point{16, 3}, Point{10, 0},  Point{4, 3},   Point{2, 9},   Point{6, 15},
      Point{9, 18}, Point{12, 21}, Point{14, 23}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 1000.0f, kMinConnectionRatio, .1f);
  // With a large max connection distance and a small trimming ratio, the
  // polyline will connect even if it is part of a straight extension which
  // would otherwise be trimmed. Both connections are to the nearest point, even
  // if it is the intersection point as it is here.
  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{10, 19}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{10, 19}));
}

TEST(PolylineProcessingTest, BestConnectionsMinWalkingDistance) {
  std::vector<Point> points = {
      Point{2, 2}, Point{3, 2}, Point{4, 2}, Point{5, 2},
      Point{6, 2}, Point{7, 2}, Point{8, 2}, Point{9, 2},
  };
  PolylineData polyline = CreatePolylineAndFindBestConnections(
      points, 5.0f, 1000.0f, .9f, kMinTrimmingRatio);
  // With a large max connection distance and a connection ratio less than 1,
  // the endpoints will connect to the nearest point on the nearest segment that
  // is at least min_walking_distance away from them.
  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{7, 2}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{4, 2}));
}

TEST(PolylineProcessingTest, BestConnectionsMinConnectionRatio) {
  std::vector<Point> points = {
      Point{20, 0}, Point{15, 0}, Point{10, 0}, Point{5, 0}, Point{0, 0},
      Point{0, 1},  Point{0, 2},  Point{0, 3},  Point{0, 4}, Point{0, 5},
      Point{0, 6},  Point{0, 7},  Point{0, 8},  Point{0, 9}, Point{0, 10},
      Point{0, 11}, Point{0, 12}, Point{0, 13}, Point{0, 14}};
  PolylineData polyline_1 = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 1000.0f, 1.1f, kMinTrimmingRatio);

  EXPECT_EQ(polyline_1.has_intersection, true);

  EXPECT_EQ(polyline_1.connect_first, true);
  EXPECT_THAT(polyline_1.new_first_point, PointEq(Point{0, 3}));

  EXPECT_EQ(polyline_1.connect_last, true);
  EXPECT_THAT(polyline_1.new_last_point, PointEq(Point{5, 0}));

  PolylineData polyline_2 = CreatePolylineAndFindBestConnections(
      points, kMinWalkDistance, 1000.0f, 1.3f, kMinTrimmingRatio);

  EXPECT_EQ(polyline_2.has_intersection, true);

  EXPECT_EQ(polyline_2.connect_first, true);
  EXPECT_THAT(polyline_2.new_first_point, PointEq(Point{0, 9}));

  EXPECT_EQ(polyline_2.connect_last, true);
  EXPECT_THAT(polyline_2.new_last_point, PointEq(Point{10, 0}));
}

TEST(PolylineProcessingTest, BestConnectionsForPerfectlyClosedLoop) {
  std::vector<Point> points = {
      Point{5, 3},   Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{5, 3}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsUpdatesFirstIntersectionWithSmallerIndexBetterConnection) {
  std::vector<Point> points = {Point{-1, 3},  Point{11, 3},  Point{20, 10},
                               Point{20, 20}, Point{10, 30}, Point{20, 30},
                               Point{10, 20}, Point{5, 15},  Point{5, 8},
                               Point{5, 3.2}};
  PolylineData intersection_polyline =
      CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(intersection_polyline.has_intersection, true);

  EXPECT_EQ(intersection_polyline.first_intersection.index_int, 3);
  EXPECT_EQ(intersection_polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_first_point, PointEq(Point{15, 25}));

  EXPECT_EQ(intersection_polyline.last_intersection.index_int, 5);
  EXPECT_EQ(intersection_polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_last_point, PointEq(Point{15, 25}));

  PolylineData connection_polyline =
      CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(connection_polyline.has_intersection, true);

  EXPECT_EQ(connection_polyline.connect_first, false);
  EXPECT_EQ(connection_polyline.first_intersection.index_int, 0);
  EXPECT_EQ(connection_polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(connection_polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(connection_polyline.connect_last, true);
  EXPECT_THAT(connection_polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsUpdatesLastIntersectionWithLargerIndexBetterConnection) {
  std::vector<Point> points = {
      Point{5.2, 3}, Point{11, 3},  Point{20, 10}, Point{20, 20}, Point{10, 30},
      Point{20, 30}, Point{10, 20}, Point{5, 15},  Point{5, 8},   Point{5, -2}};
  PolylineData intersection_polyline =
      CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(intersection_polyline.has_intersection, true);

  EXPECT_EQ(intersection_polyline.first_intersection.index_int, 3);
  EXPECT_EQ(intersection_polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_first_point, PointEq(Point{15, 25}));

  EXPECT_EQ(intersection_polyline.last_intersection.index_int, 5);
  EXPECT_EQ(intersection_polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_last_point, PointEq(Point{15, 25}));

  PolylineData connection_polyline =
      CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(connection_polyline.has_intersection, true);

  EXPECT_EQ(connection_polyline.connect_first, true);
  EXPECT_THAT(connection_polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(connection_polyline.connect_last, false);
  EXPECT_EQ(connection_polyline.last_intersection.index_int, 8);
  EXPECT_EQ(connection_polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(connection_polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsUpdatesFirstIntersectionWithSameIndexBetterConnection) {
  std::vector<Point> points = {
      Point{40, 0},  Point{35, 5},  Point{30, 10},
      Point{10, 30}, Point{20, 30}, Point{10, 20},
      Point{5, 15},  Point{10, 10}, Point{19.5f, 19.5f}};
  PolylineData intersection_polyline =
      CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(intersection_polyline.has_intersection, true);

  EXPECT_EQ(intersection_polyline.first_intersection.index_int, 2);
  EXPECT_EQ(intersection_polyline.first_intersection.index_fraction, 0.75f);
  EXPECT_THAT(intersection_polyline.new_first_point, PointEq(Point{15, 25}));

  EXPECT_EQ(intersection_polyline.last_intersection.index_int, 4);
  EXPECT_EQ(intersection_polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_last_point, PointEq(Point{15, 25}));

  PolylineData connection_polyline =
      CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(connection_polyline.has_intersection, true);

  EXPECT_EQ(connection_polyline.connect_first, false);
  EXPECT_EQ(connection_polyline.first_intersection.index_int, 2);
  EXPECT_EQ(connection_polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(connection_polyline.new_first_point, PointEq(Point{20, 20}));

  EXPECT_EQ(connection_polyline.connect_last, true);
  EXPECT_THAT(connection_polyline.new_last_point, PointEq(Point{20, 20}));
}

TEST(PolylineProcessingTest,
     BestConnectionsUpdatesLastIntersectionWithSameIndexBetterConnection) {
  std::vector<Point> points = {
      Point{19.5f, 19.5f}, Point{10, 10}, Point{5, 15},
      Point{10, 20},       Point{20, 30}, Point{10, 30},
      Point{30, 10},       Point{35, 5},  Point{40, 0}};
  PolylineData intersection_polyline =
      CreatePolylineAndFindIntersections(points);

  EXPECT_EQ(intersection_polyline.has_intersection, true);

  EXPECT_EQ(intersection_polyline.first_intersection.index_int, 3);
  EXPECT_EQ(intersection_polyline.first_intersection.index_fraction, 0.5f);
  EXPECT_THAT(intersection_polyline.new_first_point, PointEq(Point{15, 25}));

  EXPECT_EQ(intersection_polyline.last_intersection.index_int, 5);
  EXPECT_EQ(intersection_polyline.last_intersection.index_fraction, 0.25f);
  EXPECT_THAT(intersection_polyline.new_last_point, PointEq(Point{15, 25}));

  PolylineData connection_polyline =
      CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(connection_polyline.has_intersection, true);

  EXPECT_EQ(connection_polyline.connect_first, true);
  EXPECT_THAT(connection_polyline.new_first_point, PointEq(Point{20, 20}));

  EXPECT_EQ(connection_polyline.connect_last, false);
  EXPECT_EQ(connection_polyline.last_intersection.index_int, 5);
  EXPECT_EQ(connection_polyline.last_intersection.index_fraction, 0.5f);
  EXPECT_THAT(connection_polyline.new_last_point, PointEq(Point{20, 20}));
}

TEST(PolylineProcessingTest, BestConnectionsForNearlyClosedLoop) {
  std::vector<Point> points = {Point{5, 3.1f}, Point{5, 8},   Point{5, 15},
                               Point{10, 20},  Point{15, 25}, Point{20, 30},
                               Point{30, 20},  Point{20, 10}, Point{11, 3},
                               Point{5.1f, 3}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5.1f, 3}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3.1f}));
}

TEST(PolylineProcessingTest,
     BestConnectionsWithNoIntersectionsAndNoConnections) {
  std::vector<Point> points = {Point{-1, 0}, Point{5, 1},  Point{10, 2},
                               Point{15, 4}, Point{20, 6}, Point{25, 9}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, false);

  EXPECT_EQ(polyline.connect_first, false);
  EXPECT_EQ(polyline.connect_last, false);
}

TEST(PolylineProcessingTest,
     BestConnectionsWithOneIntersectionAndNoConnections) {
  std::vector<Point> points = {
      Point{6, 23},  Point{8, 21},  Point{10, 19}, Point{14, 15}, Point{18, 9},
      Point{16, 3},  Point{10, 0},  Point{4, 3},   Point{2, 9},   Point{6, 15},
      Point{10, 19}, Point{12, 21}, Point{14, 23}};
  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, false);
  EXPECT_EQ(polyline.connect_last, false);
}

TEST(PolylineProcessingTest,
     BestConnectionsWithOneIntersectionConnectsBothPoints) {
  std::vector<Point> points = {
      Point{1, 9},  Point{-1, 13}, Point{1, 17},  Point{6, 19},  Point{14, 15},
      Point{18, 9}, Point{16, 3},  Point{10, 0},  Point{4, 3},   Point{2, 9},
      Point{6, 15}, Point{14, 19}, Point{19, 17}, Point{21, 13}, Point{19, 9}};

  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{2, 9}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{18, 9}));
}

TEST(PolylineProcessingTest,
     BestConnectionsConnectsFirstPointAndUpdatesLastPoint) {
  std::vector<Point> points = {
      Point{5, 3.2}, Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{1, 3}};

  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.connect_last, false);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsConnectsLastPointAndUpdatesFirstPoint) {
  std::vector<Point> points = {Point{1, 3},   Point{11, 3},  Point{20, 10},
                               Point{30, 20}, Point{20, 30}, Point{15, 25},
                               Point{10, 20}, Point{5, 15},  Point{5, 8},
                               Point{5, 3.2}};

  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, false);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsConnectsFrontToBackAndBackToClosestPoint) {
  std::vector<Point> points = {Point{4.95, 3}, Point{11, 3},  Point{20, 10},
                               Point{30, 20},  Point{20, 30}, Point{15, 25},
                               Point{10, 20},  Point{5, 15},  Point{5, 8},
                               Point{5, 3.2}};

  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3.2}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5, 3}));
}

TEST(PolylineProcessingTest,
     BestConnectionsConnectsBacktoFrontAndFrontToClosestPoint) {
  std::vector<Point> points = {Point{5.2, 3}, Point{11, 3},  Point{20, 10},
                               Point{30, 20}, Point{20, 30}, Point{15, 25},
                               Point{10, 20}, Point{5, 15},  Point{5, 8},
                               Point{5, 2.95}};

  PolylineData polyline = CreatePolylineAndFindBestConnections(points);

  EXPECT_EQ(polyline.has_intersection, true);

  EXPECT_EQ(polyline.connect_first, true);
  EXPECT_THAT(polyline.new_first_point, PointEq(Point{5, 3}));

  EXPECT_EQ(polyline.connect_last, true);
  EXPECT_THAT(polyline.new_last_point, PointEq(Point{5.2, 3}));
}

TEST(PolylineProcessingTest, ProcessPolylineForPerfectlyClosedLoop) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{5, 3}, Point{5, 8}, Point{5, 15}, Point{10, 20}, Point{15, 25},
           Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},
           Point{5, 3}},
          kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
          kMinTrimmingRatio),
      testing::Pointwise(PointsEq(),
                         {Point{5, 3}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                          Point{15, 25}, Point{20, 30}, Point{30, 20},
                          Point{20, 10}, Point{11, 3}, Point{5, 3}}));
}

TEST(PolylineProcessingTest, ProcessPolylineWithTinyMaxConnectionDistance) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{5, 3.1f}, Point{5, 8}, Point{5, 15}, Point{10, 20},
           Point{15, 25}, Point{20, 30}, Point{30, 20}, Point{20, 10},
           Point{11, 3}, Point{5.1f, 3}},
          kMinWalkDistance, 0.1f, kMinConnectionRatio, kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(), {Point{5, 3.1f}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                       Point{15, 25}, Point{20, 30}, Point{30, 20},
                       Point{20, 10}, Point{11, 3}, Point{5.1f, 3}}));
}

TEST(PolylineProcessingTest, ProcessPolylineWithLargeMaxConnectionDistances) {
  std::vector<Point> points = {
      Point{5, 7},   Point{5, 8},   Point{5, 15},  Point{10, 20}, Point{15, 25},
      Point{20, 30}, Point{30, 20}, Point{20, 10}, Point{11, 3},  Point{1, 3}};

  // This max connection distance is large enough that the first point is able
  // to connect but small enough that the last point is not able to connect and
  // is trimmed.
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 4.5f,
                                     kMinConnectionRatio, kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{5, 3}, Point{5, 7}, Point{5, 8}, Point{5, 15}, Point{10, 20},
           Point{15, 25}, Point{20, 30}, Point{30, 20}, Point{20, 10},
           Point{11, 3}, Point{5, 3}}));
  // A very large max connection distance allows both ends of the polyline to
  // connect with each other.
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 400.0f,
                                     kMinConnectionRatio, kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{5, 3}, Point{5, 7}, Point{5, 8}, Point{5, 15}, Point{10, 20},
           Point{15, 25}, Point{20, 30}, Point{30, 20}, Point{20, 10},
           Point{11, 3}, Point{1, 3}, Point{5, 7}}));
}

TEST(PolylineProcessingTest, ProcessPolylineWithDifferentTrimmingRatios) {
  std::vector<Point> points = {
      Point{6, 23}, Point{8, 21},  Point{11, 18}, Point{14, 15}, Point{18, 9},
      Point{16, 3}, Point{10, 0},  Point{4, 3},   Point{2, 9},   Point{6, 15},
      Point{9, 18}, Point{12, 21}, Point{14, 23}};
  // A normal trimming ratio causes both ends of the polyline to be trimmed,
  // even with a large max connection distance.
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 1000.0f,
                                     kMinConnectionRatio, kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{10, 19}, Point{11, 18}, Point{14, 15}, Point{18, 9},
           Point{16, 3}, Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
           Point{9, 18}, Point{10, 19}}));
  // a small trimming ratio allows both ends to connect with a large max
  // connection distance.
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 1000.0f,
                                     kMinConnectionRatio, 0.1f),
      testing::Pointwise(
          PointsEq(), {Point{10, 19}, Point{6, 23}, Point{8, 21}, Point{11, 18},
                       Point{14, 15}, Point{18, 9}, Point{16, 3}, Point{10, 0},
                       Point{4, 3}, Point{2, 9}, Point{6, 15}, Point{9, 18},
                       Point{12, 21}, Point{14, 23}, Point{10, 19}}));
}

TEST(PolylineProcessingTest, ProcessPolylineMinWalkingDistance) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{2, 2}, Point{3, 2}, Point{4, 2}, Point{5, 2}, Point{6, 2},
           Point{7, 2}, Point{8, 2}, Point{9, 2}},
          5.0f, 1000.0f, .9f, kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{7, 2}, Point{2, 2}, Point{3, 2}, Point{4, 2}, Point{5, 2},
           Point{6, 2}, Point{7, 2}, Point{8, 2}, Point{9, 2}, Point{4, 2}}));
}

TEST(PolylineProcessingTest, ProcessPolylineMinConnectionRatio) {
  std::vector<Point> points = {
      Point{20, 0}, Point{15, 0}, Point{10, 0}, Point{5, 0}, Point{0, 0},
      Point{0, 1},  Point{0, 2},  Point{0, 3},  Point{0, 4}, Point{0, 5},
      Point{0, 6},  Point{0, 7},  Point{0, 8},  Point{0, 9}, Point{0, 10},
      Point{0, 11}, Point{0, 12}, Point{0, 13}, Point{0, 14}};
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 1000.0f, 1.1f,
                                     kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{0, 3},  Point{20, 0}, Point{15, 0}, Point{10, 0}, Point{5, 0},
           Point{0, 0},  Point{0, 1},  Point{0, 2},  Point{0, 3},  Point{0, 4},
           Point{0, 5},  Point{0, 6},  Point{0, 7},  Point{0, 8},  Point{0, 9},
           Point{0, 10}, Point{0, 11}, Point{0, 12}, Point{0, 13}, Point{0, 14},
           Point{5, 0}}));
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(points, kMinWalkDistance, 1000.0f, 1.3f,
                                     kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{0, 9},  Point{20, 0}, Point{15, 0}, Point{10, 0}, Point{5, 0},
           Point{0, 0},  Point{0, 1},  Point{0, 2},  Point{0, 3},  Point{0, 4},
           Point{0, 5},  Point{0, 6},  Point{0, 7},  Point{0, 8},  Point{0, 9},
           Point{0, 10}, Point{0, 11}, Point{0, 12}, Point{0, 13}, Point{0, 14},
           Point{10, 0}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineUpdatesFirstIntersectionWithSameIndexBetterConnection) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{40, 0}, Point{35, 5}, Point{30, 10}, Point{10, 30},
                   Point{20, 30}, Point{10, 20}, Point{5, 15}, Point{10, 10},
                   Point{19.5f, 19.5f}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(PointsEq(),
                                 {Point{20, 20}, Point{10, 30}, Point{20, 30},
                                  Point{10, 20}, Point{5, 15}, Point{10, 10},
                                  Point{19.5f, 19.5f}, Point{20.0f, 20.0f}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineUpdatesLastIntersectionWithSameIndexBetterConnection) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{19.5f, 19.5f}, Point{10, 10}, Point{5, 15},
                   Point{10, 20}, Point{20, 30}, Point{10, 30}, Point{30, 10},
                   Point{35, 5}, Point{40, 0}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(
                  PointsEq(), {Point{20.0f, 20.0f}, Point{19.5f, 19.5f},
                               Point{10, 10}, Point{5, 15}, Point{10, 20},
                               Point{20, 30}, Point{10, 30}, Point{20, 20}}));
}

TEST(PolylineProcessingTest, ProcessPolylineForNearlyClosedLoop) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{5, 3.1f}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                   Point{15, 25}, Point{20, 30}, Point{30, 20}, Point{20, 10},
                   Point{11, 3}, Point{5.1f, 3}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(
                  PointsEq(), {Point{5.1f, 3}, Point{5, 3.1f}, Point{5, 8},
                               Point{5, 15}, Point{10, 20}, Point{15, 25},
                               Point{20, 30}, Point{30, 20}, Point{20, 10},
                               Point{11, 3}, Point{5.1f, 3}, Point{5, 3.1f}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineWithNoIntersectionsAndNoConnections) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation({Point{-1, 0}, Point{5, 1}, Point{10, 2},
                                      Point{15, 4}, Point{20, 6}, Point{25, 9}},
                                     kMinWalkDistance, kMaxConnectionDistance,
                                     kMinConnectionRatio, kMinTrimmingRatio),
      testing::Pointwise(PointsEq(),
                         {Point{-1, 0}, Point{5, 1}, Point{10, 2}, Point{15, 4},
                          Point{20, 6}, Point{25, 9}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineWithOneIntersectionAndNoConnections) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{6, 23}, Point{8, 21}, Point{10, 19}, Point{14, 15},
           Point{18, 9}, Point{16, 3}, Point{10, 0}, Point{4, 3}, Point{2, 9},
           Point{6, 15}, Point{10, 19}, Point{12, 21}, Point{14, 23}},
          kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
          kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(), {Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
                       Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
                       Point{10, 19}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineWithOneValidIntersectionTrimsTwoInvalidIntersections) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{6, 23}, Point{6.2f, 23.1f}, Point{6.1f, 23.1f}, Point{8, 21},
           Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
           Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15}, Point{10, 19},
           Point{11.9f, 21.2f}, Point{11.9f, 21.1f}, Point{14, 23}},
          kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
          kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(), {Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
                       Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
                       Point{10, 19}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineWithOneIntersectionConnectsBothPoints) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(

          {Point{1, 9}, Point{-1, 13}, Point{1, 17}, Point{6, 19},
           Point{14, 15}, Point{18, 9}, Point{16, 3}, Point{10, 0}, Point{4, 3},
           Point{2, 9}, Point{6, 15}, Point{14, 19}, Point{19, 17},
           Point{21, 13}, Point{19, 9}},
          kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
          kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(),
          {Point{2, 9}, Point{1, 9}, Point{-1, 13}, Point{1, 17}, Point{6, 19},
           Point{14, 15}, Point{18, 9}, Point{16, 3}, Point{10, 0}, Point{4, 3},
           Point{2, 9}, Point{6, 15}, Point{14, 19}, Point{19, 17},
           Point{21, 13}, Point{19, 9}, Point{18, 9}}));
}

TEST(PolylineProcessingTest, ProcessPolylineConnectsFirstPointAndTrimsEnd) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{5, 3.2}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                   Point{15, 25}, Point{20, 30}, Point{30, 20}, Point{20, 10},
                   Point{11, 3}, Point{1, 3}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3}, Point{5, 3.2}, Point{5, 8}, Point{5, 15},
                   Point{10, 20}, Point{15, 25}, Point{20, 30}, Point{30, 20},
                   Point{20, 10}, Point{11, 3}, Point{5, 3}}));
}

TEST(PolylineProcessingTest, ProcessPolylineConnectsLastPointAndTrimsFront) {
  EXPECT_THAT(
      ProcessPolylineForMeshCreation(
          {Point{1, 3}, Point{11, 3}, Point{20, 10}, Point{30, 20},
           Point{20, 30}, Point{15, 25}, Point{10, 20}, Point{5, 15},
           Point{5, 8}, Point{5, 3.2}},
          kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
          kMinTrimmingRatio),
      testing::Pointwise(
          PointsEq(), {Point{5, 3}, Point{11, 3}, Point{20, 10}, Point{30, 20},
                       Point{20, 30}, Point{15, 25}, Point{10, 20},
                       Point{5, 15}, Point{5, 8}, Point{5, 3.2}, Point{5, 3}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineConnectsFrontToBackAndBackToClosestPoint) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{4.95, 3}, Point{11, 3}, Point{20, 10}, Point{30, 20},
                   Point{20, 30}, Point{15, 25}, Point{10, 20}, Point{5, 15},
                   Point{5, 8}, Point{5, 3.2}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3.2}, Point{4.95, 3}, Point{11, 3}, Point{20, 10},
                   Point{30, 20}, Point{20, 30}, Point{15, 25}, Point{10, 20},
                   Point{5, 15}, Point{5, 8}, Point{5, 3.2}, Point{5, 3}}));
}

TEST(PolylineProcessingTest,
     ProcessPolylineConnectsBacktoFrontAndFrontToClosestPoint) {
  EXPECT_THAT(ProcessPolylineForMeshCreation(
                  {Point{5.2, 3}, Point{11, 3}, Point{20, 10}, Point{30, 20},
                   Point{20, 30}, Point{15, 25}, Point{10, 20}, Point{5, 15},
                   Point{5, 8}, Point{5, 2.95}},
                  kMinWalkDistance, kMaxConnectionDistance, kMinConnectionRatio,
                  kMinTrimmingRatio),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3}, Point{5.2, 3}, Point{11, 3}, Point{20, 10},
                   Point{30, 20}, Point{20, 30}, Point{15, 25}, Point{10, 20},
                   Point{5, 15}, Point{5, 8}, Point{5, 2.95}, Point{5.2, 3}}));
}

TEST(PolylineProcessingTest, CreateClosedShapeForPerfectlyClosedLoop) {
  EXPECT_THAT(
      CreateClosedShape({Point{5, 3}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                         Point{15, 25}, Point{20, 30}, Point{30, 20},
                         Point{20, 10}, Point{11, 3}, Point{5, 3}}),
      testing::Pointwise(PointsEq(),
                         {Point{5, 3}, Point{5, 8}, Point{5, 15}, Point{10, 20},
                          Point{15, 25}, Point{20, 30}, Point{30, 20},
                          Point{20, 10}, Point{11, 3}, Point{5, 3}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeUpdatesFirstIntersectionWithSameIndexBetterConnection) {
  EXPECT_THAT(
      CreateClosedShape({Point{40, 0}, Point{35, 5}, Point{30, 10},
                         Point{10, 30}, Point{20, 30}, Point{10, 20},
                         Point{5, 15}, Point{10, 10}, Point{19.5f, 19.5f}}),
      testing::Pointwise(PointsEq(),
                         {Point{20, 20}, Point{10, 30}, Point{20, 30},
                          Point{10, 20}, Point{5, 15}, Point{10, 10},
                          Point{19.5f, 19.5f}, Point{20.0f, 20.0f}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeUpdatesLastIntersectionWithSameIndexBetterConnection) {
  EXPECT_THAT(
      CreateClosedShape({Point{19.5f, 19.5f}, Point{10, 10}, Point{5, 15},
                         Point{10, 20}, Point{20, 30}, Point{10, 30},
                         Point{30, 10}, Point{35, 5}, Point{40, 0}}),
      testing::Pointwise(
          PointsEq(), {Point{20.0f, 20.0f}, Point{19.5f, 19.5f}, Point{10, 10},
                       Point{5, 15}, Point{10, 20}, Point{20, 30},
                       Point{10, 30}, Point{20, 20}}));
}

TEST(PolylineProcessingTest, CreateClosedShapeForNearlyClosedLoop) {
  EXPECT_THAT(CreateClosedShape({Point{5, 3.1f}, Point{5, 8}, Point{5, 15},
                                 Point{10, 20}, Point{15, 25}, Point{20, 30},
                                 Point{30, 20}, Point{20, 10}, Point{11, 3},
                                 Point{5.1f, 3}}),
              testing::Pointwise(
                  PointsEq(), {Point{5.1f, 3}, Point{5, 3.1f}, Point{5, 8},
                               Point{5, 15}, Point{10, 20}, Point{15, 25},
                               Point{20, 30}, Point{30, 20}, Point{20, 10},
                               Point{11, 3}, Point{5.1f, 3}, Point{5, 3.1f}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeWithNoIntersectionsAndNoConnections) {
  EXPECT_THAT(CreateClosedShape({Point{-1, 0}, Point{5, 1}, Point{10, 2},
                                 Point{15, 4}, Point{20, 6}, Point{25, 9}}),
              testing::Pointwise(PointsEq(),
                                 {Point{-1, 0}, Point{5, 1}, Point{10, 2},
                                  Point{15, 4}, Point{20, 6}, Point{25, 9}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeWithOneIntersectionAndNoConnections) {
  EXPECT_THAT(
      CreateClosedShape({Point{6, 23}, Point{8, 21}, Point{10, 19},
                         Point{14, 15}, Point{18, 9}, Point{16, 3},
                         Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
                         Point{10, 19}, Point{12, 21}, Point{14, 23}}),
      testing::Pointwise(
          PointsEq(), {Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
                       Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
                       Point{10, 19}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeWithOneValidIntersectionTrimsTwoInvalidIntersections) {
  EXPECT_THAT(
      CreateClosedShape(
          {Point{6, 23}, Point{6.2f, 23.1f}, Point{6.1f, 23.1f}, Point{8, 21},
           Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
           Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15}, Point{10, 19},
           Point{11.9f, 21.2f}, Point{11.9f, 21.1f}, Point{14, 23}}),
      testing::Pointwise(
          PointsEq(), {Point{10, 19}, Point{14, 15}, Point{18, 9}, Point{16, 3},
                       Point{10, 0}, Point{4, 3}, Point{2, 9}, Point{6, 15},
                       Point{10, 19}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeWithOneIntersectionConnectsBothPoints) {
  EXPECT_THAT(
      CreateClosedShape(

          {Point{1, 9}, Point{-1, 13}, Point{1, 17}, Point{6, 19},
           Point{14, 15}, Point{18, 9}, Point{16, 3}, Point{10, 0}, Point{4, 3},
           Point{2, 9}, Point{6, 15}, Point{14, 19}, Point{19, 17},
           Point{21, 13}, Point{19, 9}}),
      testing::Pointwise(
          PointsEq(),
          {Point{2, 9}, Point{1, 9}, Point{-1, 13}, Point{1, 17}, Point{6, 19},
           Point{14, 15}, Point{18, 9}, Point{16, 3}, Point{10, 0}, Point{4, 3},
           Point{2, 9}, Point{6, 15}, Point{14, 19}, Point{19, 17},
           Point{21, 13}, Point{19, 9}, Point{18, 9}}));
}

TEST(PolylineProcessingTest, CreateClosedShapeConnectsFirstPointAndTrimsEnd) {
  EXPECT_THAT(CreateClosedShape({Point{5, 3.2}, Point{5, 8}, Point{5, 15},
                                 Point{10, 20}, Point{15, 25}, Point{20, 30},
                                 Point{30, 20}, Point{20, 10}, Point{11, 3},
                                 Point{1, 3}}),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3}, Point{5, 3.2}, Point{5, 8}, Point{5, 15},
                   Point{10, 20}, Point{15, 25}, Point{20, 30}, Point{30, 20},
                   Point{20, 10}, Point{11, 3}, Point{5, 3}}));
}

TEST(PolylineProcessingTest, CreateClosedShapeConnectsLastPointAndTrimsFront) {
  EXPECT_THAT(
      CreateClosedShape({Point{1, 3}, Point{11, 3}, Point{20, 10},
                         Point{30, 20}, Point{20, 30}, Point{15, 25},
                         Point{10, 20}, Point{5, 15}, Point{5, 8},
                         Point{5, 3.2}}),
      testing::Pointwise(
          PointsEq(), {Point{5, 3}, Point{11, 3}, Point{20, 10}, Point{30, 20},
                       Point{20, 30}, Point{15, 25}, Point{10, 20},
                       Point{5, 15}, Point{5, 8}, Point{5, 3.2}, Point{5, 3}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeConnectsFrontToBackAndBackToClosestPoint) {
  EXPECT_THAT(CreateClosedShape({Point{4.95, 3}, Point{11, 3}, Point{20, 10},
                                 Point{30, 20}, Point{20, 30}, Point{15, 25},
                                 Point{10, 20}, Point{5, 15}, Point{5, 8},
                                 Point{5, 3.2}}),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3.2}, Point{4.95, 3}, Point{11, 3}, Point{20, 10},
                   Point{30, 20}, Point{20, 30}, Point{15, 25}, Point{10, 20},
                   Point{5, 15}, Point{5, 8}, Point{5, 3.2}, Point{5, 3}}));
}

TEST(PolylineProcessingTest,
     CreateClosedShapeConnectsBacktoFrontAndFrontToClosestPoint) {
  EXPECT_THAT(CreateClosedShape({Point{5.2, 3}, Point{11, 3}, Point{20, 10},
                                 Point{30, 20}, Point{20, 30}, Point{15, 25},
                                 Point{10, 20}, Point{5, 15}, Point{5, 8},
                                 Point{5, 2.95}}),
              testing::Pointwise(
                  PointsEq(),
                  {Point{5, 3}, Point{5.2, 3}, Point{11, 3}, Point{20, 10},
                   Point{30, 20}, Point{20, 30}, Point{15, 25}, Point{10, 20},
                   Point{5, 15}, Point{5, 8}, Point{5, 2.95}, Point{5.2, 3}}));
}

void CreateClosedShapeDoesNotCrash(const std::vector<Point>& polyline) {
  CreateClosedShape(polyline);
}
FUZZ_TEST(PolylineProcessingFuzzTest, CreateClosedShapeDoesNotCrash)
    .WithDomains(fuzztest::VectorOf(FinitePoint()));

TEST(PolylineProcessingFuzzTest,
     CreateClosedShapeDoesNotCrashOnInfiniteMaxConnectionDistance) {
  CreateClosedShapeDoesNotCrash({Point{3.40282347e+38f, 0.f}, Point{-0.f, 0.f},
                                 Point{-3.40282347e+38f, 0.379977018f}});
}

}  // namespace
}  // namespace ink::geometry_internal
