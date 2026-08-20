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

#include "ink/geometry/internal/legacy_segment_intersection.h"

#include <cmath>
#include <limits>
#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/substitute.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace geometry_internal {
namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Field;
using ::testing::FloatEq;
using ::testing::Matcher;
using ::testing::Optional;

Matcher<LegacySegmentIntersection> OverlappingSegmentIntx(
    float segment1_start_param, float segment1_end_param,
    float segment2_start_param, float segment2_end_param, Point start_position,
    Point end_position) {
  return AllOf(Field(&LegacySegmentIntersection::segment1_interval,
                     ElementsAre(FloatEq(segment1_start_param),
                                 FloatEq(segment1_end_param))),
               Field(&LegacySegmentIntersection::segment2_interval,
                     ElementsAre(FloatEq(segment2_start_param),
                                 FloatEq(segment2_end_param))),
               Field(&LegacySegmentIntersection::intx,
                     SegmentEq({start_position, end_position})));
}

Matcher<LegacySegmentIntersection> SinglePointSegmentIntx(float segment1_param,
                                                          float segment2_param,
                                                          Point position) {
  return OverlappingSegmentIntx(segment1_param, segment1_param, segment2_param,
                                segment2_param, position, position);
}

TEST(LegacySegmentIntersectionsTest, NormalCases) {
  EXPECT_THAT(LegacyIntersection(Segment{{10, 10}, {20, 20}},
                                 Segment{{10, 20}, {20, 10}}),
              Optional(SinglePointSegmentIntx(.5, .5, {15, 15})));

  EXPECT_THAT(LegacyIntersection(Segment{{10, 10}, {20, 20}},
                                 Segment{{10, 15}, {20, 15}}),
              Optional(SinglePointSegmentIntx(.5, .5, {15, 15})));

  EXPECT_THAT(LegacyIntersection(Segment{{15, 10}, {15, 20}},
                                 Segment{{10, 15}, {20, 15}}),
              Optional(SinglePointSegmentIntx(.5, .5, {15, 15})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{10, 12.5}, {20, 20}},
                         Segment{{10, 17.5}, {20, 10}}),
      Optional(SinglePointSegmentIntx(1.0 / 3, 1.0 / 3, {40.0 / 3, 15})));
}

TEST(LegacySegmentIntersectionsTest, ParallelSegments) {
  // These segments are parallel, but not collinear.
  EXPECT_EQ(
      LegacyIntersection(Segment{{0, 0}, {0, 4}}, Segment{{1, 1}, {3, 1}}),
      std::nullopt);
  EXPECT_EQ(
      LegacyIntersection(Segment{{1, 1}, {2, 2}}, Segment{{3, 0}, {1, -2}}),
      std::nullopt);

  // These segments are collinear, but do not overlap.
  EXPECT_EQ(
      LegacyIntersection(Segment{{2, 1}, {3, 1}}, Segment{{4, 1}, {5, 1}}),
      std::nullopt);
  EXPECT_EQ(
      LegacyIntersection(Segment{{1, 1}, {0, -1}}, Segment{{2, 3}, {3, 5}}),
      std::nullopt);

  // These segments only touch at an endpoint.
  EXPECT_THAT(
      LegacyIntersection(Segment{{0, 0}, {1, 1}}, Segment{{1, 1}, {2, 2}}),
      Optional(SinglePointSegmentIntx(1, 0, {1, 1})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{-1, 4}, {-5, 4}}, Segment{{-1, 4}, {2, 4}}),
      Optional(SinglePointSegmentIntx(0, 0, {-1, 4})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{-3, 2}, {-3, 5}}, Segment{{-3, 7}, {-3, 5}}),
      Optional(SinglePointSegmentIntx(1, 1, {-3, 5})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{0, 1}, {1, 0}}, Segment{{-1, 2}, {0, 1}}),
      Optional(SinglePointSegmentIntx(0, 1, {0, 1})));

  // These segments overlap.
  EXPECT_THAT(
      LegacyIntersection(Segment{{0, 0}, {2, 2}}, Segment{{1, 1}, {3, 3}}),
      Optional(OverlappingSegmentIntx(.5, 1, 0, .5, {1, 1}, {2, 2})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{-6, 1}, {-1, 1}}, Segment{{-2, 1}, {-3, 1}}),
      Optional(OverlappingSegmentIntx(.6, .8, 1, 0, {-3, 1}, {-2, 1})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{2, 2}, {-2, 0}}, Segment{{1, 1.5}, {5, 3.5}}),
      Optional(OverlappingSegmentIntx(0, .25, .25, 0, {2, 2}, {1, 1.5})));

  EXPECT_THAT(
      LegacyIntersection(Segment{{2, 0}, {2, -2}}, Segment{{2, 1}, {2, -3}}),
      Optional(OverlappingSegmentIntx(0, 1, .25, .75, {2, 0}, {2, -2})));
}

TEST(LegacySegmentIntersectionsTest, LargeSegments) {
  // In these cases, the segments are sufficiently large that naively performing
  // the intersection would result in an overflow.
  EXPECT_EQ(LegacyIntersection(Segment{{-1e20, 0}, {1e20, 0}},
                               Segment{{0, 1}, {1e20, 1}}),
            std::nullopt);

  EXPECT_THAT(
      LegacyIntersection(Segment{{5, -1e20}, {5, 1e20}},
                         Segment{{5, 0}, {5, 2e20}}),
      Optional(OverlappingSegmentIntx(.5, 1, 0, .5, {5, 0}, {5, 1e20})));

  EXPECT_THAT(LegacyIntersection(Segment{{1e20, 2e20}, {0, 1e20}},
                                 Segment{{-1e20, 1.5e20}, {5e20, 1.5e20}}),
              Optional(SinglePointSegmentIntx(.5, .25, {5e19, 1.5e20})));

  EXPECT_THAT(LegacyIntersection(Segment{{-1e20, 0}, {0, -4e20}},
                                 Segment{{0, 0}, {-2.5e19, -3e20}}),
              Optional(SinglePointSegmentIntx(.75, 1, {-2.5e19, -3e20})));
}

TEST(LegacySegmentIntersectionsTest, Combinations) {
  // This test attempts to find the intersection of crossing segments of varying
  // precision. It will iterate over all the possible exponents (-127 to 127 for
  // floats), and construct two lines: one from the top-left corner (min, min)
  // to the bottom-right corner (max, max), and another from the bottom-left
  // corner (min, max) to the top-right corner (max, min), and check for an
  // intersection between them.
  // Since these lines are connecting corners of a square, it's easy to
  // determine the expected intersection position (it should be in the middle of
  // the square).
  // Numbers are chosen by doing 1 * 2^x, where x is the exponent.
  int min_exponent = std::numeric_limits<float>::min_exponent;
  int max_exponent = std::numeric_limits<float>::max_exponent;
  for (int a = min_exponent; a < max_exponent; a++) {
    SCOPED_TRACE(absl::Substitute("a = $0", a));
    for (int b = a; b < max_exponent; b++) {
      SCOPED_TRACE(absl::Substitute("b = $0", b));
      float min = ldexp(1, a);
      float max = ldexp(1, b);
      float expected_intersection = min / 2 + max / 2;

      Segment seg1 = {{min, min}, {max, max}};
      Segment seg2 = {{min, max}, {max, min}};

      std::optional<LegacySegmentIntersection> custom_intersection =
          LegacyIntersection(seg1, seg2);

      ASSERT_TRUE(custom_intersection.has_value());
      EXPECT_THAT(custom_intersection->intx,
                  SegmentEq({{expected_intersection, expected_intersection},
                             {expected_intersection, expected_intersection}}));
    }
  }
}

TEST(LegacySegmentIntersectionsTest, PrecisionLimits) {
  // This test verifies that the intersection functions are resilient to
  // floating point precision limits.
  // While a lot of thought has gone into allowing the custom implementation to
  // properly handle large numbers, it doesn't protect against using really
  // small numbers. If you'd like to test that case, you can substitute the
  // following numbers:
  // min = 0.00000000000000000000001f,
  // max = 0.00000000000000000000002f,
  // intersectionPos = 0.000000000000000000000015f;
  // Note that these are the numbers that cause the custom implementation to
  // fail. The boost implementation actually fails before this, failing to find
  // the proper intersection for min/max pairs as large as (0.0001f, 0.0002f).
  float min_value = 1000000000000000000000.0f;
  float max_value = 2000000000000000000000.0f;
  float intersectionPos = 1500000000000000000000.0f;
  Segment seg1 = {{min_value, min_value}, {max_value, max_value}};
  Segment seg2 = {{min_value, max_value}, {max_value, min_value}};

  std::optional<LegacySegmentIntersection> intersection =
      LegacyIntersection(seg1, seg2);
  ASSERT_TRUE(intersection.has_value());
  EXPECT_THAT(intersection->intx,
              SegmentEq({{intersectionPos, intersectionPos},
                         {intersectionPos, intersectionPos}}));
}

}  // namespace
}  // namespace geometry_internal
}  // namespace ink
