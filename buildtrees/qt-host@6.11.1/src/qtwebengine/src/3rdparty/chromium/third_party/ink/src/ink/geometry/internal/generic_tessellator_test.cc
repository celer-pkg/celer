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

#include "ink/geometry/internal/generic_tessellator.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/internal/point_tessellation_helper.h"
#include "ink/geometry/point.h"
#include "libtess2/tesselator.h"

namespace ink::geometry_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::SizeIs;

TEST(GenericTessellatorTest, ReturnsEmptyForZeroPoints) {
  {
    const auto& result = Tessellate<PointTessellationHelper>({});
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, IsEmpty());
  }
  {
    const auto& result =
        Tessellate<PointTessellationHelper>({{}}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, IsEmpty());
  }
}

TEST(GenericTessellatorTest, ReturnsEmptyMeshForSinglePoint) {
  {
    const auto& result = Tessellate<PointTessellationHelper>({Point{0, 0}});
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, ElementsAre(Point{0, 0}));
  }
  {
    const auto& result = Tessellate<PointTessellationHelper>(
        {{Point{0, 0}}}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, ElementsAre(Point{0, 0}));
  }
}

TEST(GenericTessellatorTest, ReturnsEmptyMeshForStraightLine) {
  {
    const auto& result =
        Tessellate<PointTessellationHelper>({Point{0, 0}, Point{10, 10}});
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, ElementsAre(Point{0, 0}, Point{10, 10}));
  }
  {
    const auto& result = Tessellate<PointTessellationHelper>(
        {{Point{0, 0}, Point{10, 10}}}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.indices, IsEmpty());
    EXPECT_THAT(result.vertices, ElementsAre(Point{0, 0}, Point{10, 10}));
  }
}

TEST(GenericTessellatorTest, ReturnsSingleTriangleForThreePoints) {
  {
    const auto& result = Tessellate<PointTessellationHelper>(
        {Point{0, 0}, Point{10, 10}, Point{10, 0}});
    EXPECT_THAT(result.indices, ElementsAre(2, 0, 1));
    EXPECT_THAT(result.vertices,
                ElementsAre(Point{0, 0}, Point{10, 10}, Point{10, 0}));
  }
  {
    const auto& result = Tessellate<PointTessellationHelper>(
        {{Point{0, 0}, Point{10, 10}, Point{10, 0}}}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.indices, ElementsAre(2, 0, 1));
    EXPECT_THAT(result.vertices,
                ElementsAre(Point{0, 0}, Point{10, 10}, Point{10, 0}));
  }
}

// Verifies that a closed loop with an internal vertex like this:
//   3---2
//  /     \
// 4   0---1,7
//  \     /
//   5---6
// is tessellated by 6 triangles, e.g.:
//   3---2
//  / \ / \
// 4---0---1, 7
//  \ / \ /
//   5---6
// with six triangles (i.e. 6 * 3 = 18 indices)
TEST(GenericTessellatorTest, ReturnsMeshForHexagon) {
  const std::vector<Point> hexagon = {Point{0, 0},  Point{2, 0},  Point{1, 2},
                                      Point{-1, 2}, Point{-2, 0}, Point{-1, -2},
                                      Point{1, -2}, Point{2, 0}};
  {
    const auto& result = Tessellate<PointTessellationHelper>(hexagon);
    EXPECT_THAT(result.vertices, SizeIs(8));
    EXPECT_THAT(result.indices, SizeIs(18));
  }
  {
    const auto& result =
        Tessellate<PointTessellationHelper>({hexagon}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.vertices, SizeIs(8));
    EXPECT_THAT(result.indices, SizeIs(18));
  }
}

// Verifies that a closed loop without an internal vertex like this:
//   2---1
//  /     \
// 3      0,6
//  \     /
//   4---5
// is tessellated by 4 triangles, e.g.:
//   2---1
//  /|\  |\
// 3 | \ | 0,6
//  \|  \|/
//   4---5
// with four triangles (i.e. 4 * 3 = 12 indices)
TEST(GenericTessellatorTest, ReturnsMeshForOpenHexagon) {
  const std::vector<Point> hexagon = {Point{2, 0},  Point{1, 2},   Point{-1, 2},
                                      Point{-2, 0}, Point{-1, -2}, Point{1, -2},
                                      Point{2, 0}};
  {
    const auto& result = Tessellate<PointTessellationHelper>(hexagon);
    EXPECT_THAT(result.vertices, SizeIs(7));
    EXPECT_THAT(result.indices, SizeIs(12));
  }
  {
    const auto& result =
        Tessellate<PointTessellationHelper>({hexagon}, TESS_WINDING_NONZERO);
    EXPECT_THAT(result.vertices, SizeIs(7));
    EXPECT_THAT(result.indices, SizeIs(12));
  }
}

}  // namespace
}  // namespace ink::geometry_internal
