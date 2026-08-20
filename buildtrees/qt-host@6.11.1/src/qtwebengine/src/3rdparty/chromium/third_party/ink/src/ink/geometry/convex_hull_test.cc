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

#include "ink/geometry/convex_hull.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/point.h"

namespace ink {
namespace {

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

TEST(ConvexHullTest, ReturnsEmptyForEmptyPoints) {
  EXPECT_THAT(ConvexHull({}), IsEmpty());
}

TEST(ConvexHullTest, ReturnsTriviallyForSinglePoint) {
  EXPECT_THAT(ConvexHull({{-3, 5}}), ElementsAreArray({Point{-3, 5}}));
  EXPECT_THAT(ConvexHull({{2, -1}}), ElementsAreArray({Point{2, -1}}));
}

TEST(ConvexHullTest, ReturnsForTwoSeparatePoints) {
  EXPECT_THAT(ConvexHull({{2, -3}, {-5, -6}}),
              ElementsAreArray({Point{-5, -6}, Point{2, -3}}));
  EXPECT_THAT(ConvexHull({{-4, -6}, {4, -4}}),
              ElementsAreArray({Point{-4, -6}, Point{4, -4}}));
}

TEST(ConvexHullTest, ReturnsSinglePointForCoincidentPoints) {
  EXPECT_THAT(ConvexHull({{0, 1}, {0, 1}}), ElementsAreArray({Point{0, 1}}));
}

TEST(ConvexHullTest, ReturnsForThreePoints) {
  EXPECT_THAT(ConvexHull({{3, 4}, {4, -1}, {-2, 1}}),
              ElementsAreArray({Point{4, -1}, Point{3, 4}, Point{-2, 1}}));
  EXPECT_THAT(ConvexHull({{-3, 2}, {-1, 5}, {-3, -1}}),
              ElementsAreArray({Point{-3, -1}, Point{-1, 5}, Point{-3, 2}}));
}

TEST(ConvexHullTest, ReturnsTerminalPointsForCollinearPoints) {
  EXPECT_THAT(ConvexHull({{5, 2}, {3, -3}, {1, -8}}),
              ElementsAreArray({Point{1, -8}, Point{5, 2}}));
  EXPECT_THAT(ConvexHull({{-1, 1}, {5, 1}, {-2, 1}}),
              ElementsAreArray({Point{-2, 1}, Point{5, 1}}));
  EXPECT_THAT(ConvexHull({{3, 2}, {3, 5}, {3, 5}}),
              ElementsAreArray({Point{3, 2}, Point{3, 5}}));
}

TEST(ConvexHullTest, IgnoresInteriorPoints) {
  EXPECT_THAT(
      ConvexHull({{4, 3},
                  {2, 1},
                  {0, 4},
                  {-1, 2},
                  {-2, 0},
                  {-2, -1},
                  {1, 0},
                  {-3, 1},
                  {5, 4},
                  {2, -2},
                  {3, 5},
                  {5, 3}}),
      ElementsAreArray({Point{2, -2}, Point{5, 3}, Point{5, 4}, Point{3, 5},
                        Point{0, 4}, Point{-3, 1}, Point{-2, -1}}));
}

TEST(ConvexHullTest, PreservesAllPointsOnConvexHull) {
  EXPECT_THAT(
      ConvexHull({{1, 0}, {-2, -2}, {-3, -1}, {0, -2}, {-1, 1}, {-3, 0}}),
      ElementsAreArray({Point{-2, -2}, Point{0, -2}, Point{1, 0}, Point{-1, 1},
                        Point{-3, 0}, Point{-3, -1}}));
}

}  // namespace
}  // namespace ink
