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

#include "ink/geometry/point.h"

#include <cmath>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

TEST(PointTest, Stringify) {
  EXPECT_EQ(absl::StrCat(kOrigin), "(0, 0)");
  EXPECT_EQ(absl::StrCat(Point{-3, 7}), "(-3, 7)");
  EXPECT_EQ(absl::StrCat(Point{1.125, -3.75}), "(1.125, -3.75)");
  EXPECT_EQ(absl::StrCat(
                Point{std::nanf(""), std::numeric_limits<float>::infinity()}),
            "(nan, inf)");
}

TEST(PointTest, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      kOrigin,
      Point{0, 1},
      Point{1, 0},
      Point{1, 2},
      Point{2, 1},
      Point{-2, 1},
      Point{std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()},
      Point{std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity()},
  }));
}

TEST(PointTest, Equality) {
  EXPECT_EQ((Point{1, 2}), (Point{1, 2}));
  EXPECT_EQ((Point{-.4, 17}), (Point{-.4, 17}));

  EXPECT_NE((Point{1, 2}), (Point{1, 5}));
  EXPECT_NE((Point{3, 2}), (Point{.7, 2}));
  EXPECT_NE((Point{-4, .3}), (Point{.5, 12}));
}

TEST(PointTest, NearMatcher) {
  EXPECT_THAT((Point{1, 2}), PointNear({1.05, 1.95}, .1));
  EXPECT_THAT((Point{3, 4}), Not(PointNear({3, 5}, .5)));
  EXPECT_THAT((Point{5, 6}), Not(PointNear({4, 6}, .5)));
}

TEST(PointTest, Offset) {
  EXPECT_THAT((Point{0, 0}).Offset(), VecEq({0, 0}));
  EXPECT_THAT((Point{1, 2}).Offset(), VecEq({1, 2}));
  EXPECT_THAT((Point{.1f, .2f}).Offset(), VecEq({.1f, .2f}));
  EXPECT_THAT((Point{-1, 2}).Offset(), VecEq({-1, 2}));
  EXPECT_THAT((Point{1.5f, -200}).Offset(), VecEq({1.5f, -200}));
}

TEST(PointTest, PointVecAddition) {
  Point a{3, 0};
  Point b{-1, .3};
  Point c{2.7, 4};

  EXPECT_THAT(a + a.Offset(), PointEq({6, 0}));
  EXPECT_THAT(a + b.Offset(), PointEq({2, .3}));
  EXPECT_THAT(a + c.Offset(), PointEq({5.7, 4}));
  EXPECT_THAT(b + b.Offset(), PointEq({-2, .6}));
  EXPECT_THAT(b + c.Offset(), PointEq({1.7, 4.3}));
  EXPECT_THAT(c + c.Offset(), PointEq({5.4, 8}));
}

TEST(PointTest, VecPointAddition) {
  Point a{3, 0};
  Point b{-1, .3};
  Point c{2.7, 4};

  EXPECT_THAT(a.Offset() + a, PointEq({6, 0}));
  EXPECT_THAT(b.Offset() + a, PointEq({2, .3}));
  EXPECT_THAT(c.Offset() + a, PointEq({5.7, 4}));
  EXPECT_THAT(b.Offset() + b, PointEq({-2, .6}));
  EXPECT_THAT(c.Offset() + b, PointEq({1.7, 4.3}));
  EXPECT_THAT(c.Offset() + c, PointEq({5.4, 8}));
}

TEST(PointTest, PointPointSubtraction) {
  Point a{0, -2};
  Point b{.5, 19};
  Point c{1.1, -3.4};

  EXPECT_THAT(a - a, VecEq({0, 0}));
  EXPECT_THAT(a - b, VecEq({-.5, -21}));
  EXPECT_THAT(a - c, VecEq({-1.1, 1.4}));
  EXPECT_THAT(b - a, VecEq({.5, 21}));
  EXPECT_THAT(b - b, VecEq({0, 0}));
  EXPECT_THAT(b - c, VecEq({-.6, 22.4}));
  EXPECT_THAT(c - a, VecEq({1.1, -1.4}));
  EXPECT_THAT(c - b, VecEq({.6, -22.4}));
  EXPECT_THAT(c - c, VecEq({0, 0}));
}

TEST(PointTest, PointVecSubtraction) {
  Point a{0, -2};
  Point b{.5, 19};
  Point c{1.1, -3.4};

  EXPECT_THAT(a - a.Offset(), PointEq(kOrigin));
  EXPECT_THAT(a - b.Offset(), PointEq({-.5, -21}));
  EXPECT_THAT(a - c.Offset(), PointEq({-1.1, 1.4}));
  EXPECT_THAT(b - a.Offset(), PointEq({.5, 21}));
  EXPECT_THAT(b - b.Offset(), PointEq(kOrigin));
  EXPECT_THAT(b - c.Offset(), PointEq({-.6, 22.4}));
  EXPECT_THAT(c - a.Offset(), PointEq({1.1, -1.4}));
  EXPECT_THAT(c - b.Offset(), PointEq({.6, -22.4}));
  EXPECT_THAT(c - c.Offset(), PointEq(kOrigin));
}

TEST(PointTest, AddAssign) {
  Point a{1, 2};
  a += Vec{.x = 3, .y = -1};
  EXPECT_THAT(a, PointEq({4, 1}));
  Point b{4, 1};
  b += Vec{.x = -.5, .y = 2};
  EXPECT_THAT(b, PointEq({3.5, 3}));
}

TEST(PointTest, SubractAssign) {
  Point a{1, 2};
  a -= Vec{.x = 3, .y = -1};
  EXPECT_THAT(a, PointEq({-2, 3}));
  Point b{-2, 3};
  b -= Vec{.x = -.5, .y = 2};
  EXPECT_THAT(b, PointEq({-1.5, 1}));
}

}  // namespace
}  // namespace ink
