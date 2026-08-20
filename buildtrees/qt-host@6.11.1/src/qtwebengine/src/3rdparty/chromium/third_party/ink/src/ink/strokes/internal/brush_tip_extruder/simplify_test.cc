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

#include "ink/strokes/internal/brush_tip_extruder/simplify.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"

namespace ink::brush_tip_extruder_internal {

using ::testing::ExplainMatchResult;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::Pointwise;

std::vector<Point> VerticesToPositions(
    const std::vector<ExtrudedVertex>& vertices) {
  std::vector<Point> positions;
  positions.reserve(vertices.size());
  for (const ExtrudedVertex& vertex : vertices) {
    positions.push_back(vertex.position);
  }
  return positions;
}

MATCHER(PointsPairwiseEq, "") {
  return ExplainMatchResult(PointEq(std::get<1>(arg)), std::get<0>(arg),
                            result_listener);
}

Matcher<std::vector<Point>> PolylineEq(
    const std::vector<ExtrudedVertex>& expected) {
  return Pointwise(PointsPairwiseEq(), VerticesToPositions(expected));
}

TEST(SimplifyTest, EmptyList) {
  std::vector<ExtrudedVertex> empty;
  std::vector<ExtrudedVertex> output;
  SimplifyPolyline(empty.cbegin(), empty.cend(), 2, output);
  EXPECT_THAT(VerticesToPositions(output), IsEmpty());
}

TEST(SimplifyTest, SinglePoint) {
  std::vector<ExtrudedVertex> single_point{ExtrudedVertex{.position = {1, 3}}};
  std::vector<ExtrudedVertex> output;
  SimplifyPolyline(single_point.cbegin(), single_point.cend(), 1, output);

  EXPECT_THAT(VerticesToPositions(output), PolylineEq(single_point));
}

TEST(SimplifyTest, SingleSegment) {
  std::vector<ExtrudedVertex> single_segment{
      ExtrudedVertex{.position = {-1, 4}}, ExtrudedVertex{.position = {3, 7}}};
  std::vector<ExtrudedVertex> output;
  SimplifyPolyline(single_segment.begin(), single_segment.end(), 1, output);

  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({single_segment[0], single_segment[1]}));
}

TEST(SimplifyTest, TwoSegments) {
  std::vector<ExtrudedVertex> polyline{ExtrudedVertex{.position = {1, 1}},
                                       ExtrudedVertex{.position = {2, 2}},
                                       ExtrudedVertex{.position = {3, 1}}};

  std::vector<ExtrudedVertex> output;
  SimplifyPolyline(polyline.begin(), polyline.end(), .5, output);
  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({polyline[0], polyline[1], polyline[2]}));

  output.clear();
  SimplifyPolyline(polyline.begin(), polyline.end(), 1.5, output);
  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({polyline[0],
                          // polyline[1] was removed.
                          polyline[2]}));
}

TEST(SimplifyTest, MultipleSegments) {
  std::vector<ExtrudedVertex> polyline{ExtrudedVertex{.position = {0, 0}},
                                       ExtrudedVertex{.position = {3, -1}},
                                       ExtrudedVertex{.position = {5, -3}},
                                       ExtrudedVertex{.position = {6, -5}},
                                       ExtrudedVertex{.position = {8, -4}},
                                       ExtrudedVertex{.position = {7, -5}},
                                       ExtrudedVertex{.position = {10, -7}},
                                       ExtrudedVertex{.position = {9, -8}},
                                       ExtrudedVertex{.position = {10, -9}},
                                       ExtrudedVertex{.position = {13, -8}},
                                       ExtrudedVertex{.position = {15, -6}},
                                       ExtrudedVertex{.position = {14, -5}},
                                       ExtrudedVertex{.position = {15, -3}},
                                       ExtrudedVertex{.position = {14, -3}}};

  std::vector<ExtrudedVertex> output;
  SimplifyPolyline(polyline.begin(), polyline.end(), .5, output);
  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({polyline[0], polyline[1],
                          // polyline[2] was removed.
                          polyline[3], polyline[4], polyline[5], polyline[6],
                          polyline[7], polyline[8], polyline[9], polyline[10],
                          polyline[11], polyline[12], polyline[13]}));

  output.clear();
  SimplifyPolyline(polyline.begin(), polyline.end(), 1.25, output);
  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({polyline[0],
                          // polyline[1] and polyline[2] were removed.
                          polyline[3], polyline[4], polyline[5],
                          // polyline[6] and polyline[7] were removed.
                          polyline[8],
                          // polyline[9] was removed.
                          polyline[10],
                          // polyline[11] and polyline[12] were removed.
                          polyline[13]}));

  output.clear();
  SimplifyPolyline(polyline.begin(), polyline.end(), 3, output);
  EXPECT_THAT(VerticesToPositions(output),
              PolylineEq({polyline[0],
                          // polyline[1] through polyline[7] were removed.
                          polyline[8],
                          // polyline[9] through polyline[12] were removed.
                          polyline[13]}));
}

}  // namespace ink::brush_tip_extruder_internal
