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

#include "ink/geometry/triangle.h"

#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"

namespace ink {
namespace {

TEST(TriangleTest, Stringify) {
  EXPECT_EQ(
      absl::StrCat(Triangle{.p0 = {0, 1}, .p1 = {2, 3.5}, .p2 = {-5, 4.2}}),
      "{p0 = (0, 1), p1 = (2, 3.5), p2 = (-5, 4.2)}");
}

TEST(TriangleTest, Equality) {
  EXPECT_EQ((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2}, {0, 0}, {-4, 3}}));
  EXPECT_EQ((Triangle{{-.4, 17}, {-9, 6}, {12, 3.86}}),
            (Triangle{{-.4, 17}, {-9, 6}, {12, 3.86}}));

  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{10, 2}, {0, 0}, {-4, 3}}));
  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2.4}, {0, 0}, {-4, 3}}));
  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2}, {-4, 0}, {-4, 3}}));
  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2}, {0, -2.8}, {-4, 3}}));
  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2}, {0, 0}, {0, 3}}));
  EXPECT_NE((Triangle{{1, 2}, {0, 0}, {-4, 3}}),
            (Triangle{{1, 2}, {0, 0}, {-4, -3}}));
}

TEST(TriangleTest, SignedArea) {
  // Typical cases.
  EXPECT_FLOAT_EQ((Triangle{{-1, -3}, {3, -3}, {-3, -1}}).SignedArea(), 4);
  EXPECT_FLOAT_EQ((Triangle{{1, 1}, {-5, 4}, {-1, -2}}).SignedArea(), 12);
  EXPECT_FLOAT_EQ((Triangle{{-5, 5}, {2, 4}, {1, -5}}).SignedArea(), -32);
  EXPECT_FLOAT_EQ((Triangle{{1, -4}, {3, 1}, {4, 2}}).SignedArea(), -1.5);

  // Degenerate cases.
  EXPECT_FLOAT_EQ((Triangle{{3, 2}, {5, 2}, {2, 2}}.SignedArea()), 0);
  EXPECT_FLOAT_EQ((Triangle{{-1, 2}, {0, 0}, {1, -2}}.SignedArea()), 0);
  EXPECT_FLOAT_EQ((Triangle{{0, 1}, {-2, 3}, {-2, 3}}.SignedArea()), 0);
  EXPECT_FLOAT_EQ((Triangle{{5, 2}, {5, 2}, {5, 2}}.SignedArea()), 0);
}

TEST(TriangleTest, Contains) {
  Triangle right_winding_triangle{{0, 0}, {0, 10}, {10, 0}};
  // The Point is inside.
  EXPECT_TRUE(right_winding_triangle.Contains({2, 2}));

  // The Point is outside.
  EXPECT_FALSE(right_winding_triangle.Contains({12, 2}));
  EXPECT_FALSE(right_winding_triangle.Contains({-5, 2}));
  EXPECT_FALSE(right_winding_triangle.Contains({2, -10}));
  EXPECT_FALSE(right_winding_triangle.Contains({-1, -1}));
  EXPECT_FALSE(right_winding_triangle.Contains({12, -1}));
  EXPECT_FALSE(right_winding_triangle.Contains({-1, 12}));

  // The Point is aligned with an edge but outside the Triangle.
  EXPECT_FALSE(right_winding_triangle.Contains({-1, 0}));
  EXPECT_FALSE(right_winding_triangle.Contains({11, 0}));
  EXPECT_FALSE(right_winding_triangle.Contains({-1, 11}));
  EXPECT_FALSE(right_winding_triangle.Contains({11, -1}));
  EXPECT_FALSE(right_winding_triangle.Contains({0, 11}));
  EXPECT_FALSE(right_winding_triangle.Contains({0, -1}));

  // The Point is on an edge.
  EXPECT_TRUE(right_winding_triangle.Contains({0, 5}));
  EXPECT_TRUE(right_winding_triangle.Contains({8, 2}));
  EXPECT_TRUE(right_winding_triangle.Contains({7, 0}));

  // The Point is on a Triangle vertex.
  EXPECT_TRUE(right_winding_triangle.Contains({0, 0}));
  EXPECT_TRUE(right_winding_triangle.Contains({10, 0}));
  EXPECT_TRUE(right_winding_triangle.Contains({0, 10}));

  Triangle left_winding_triangle{{0, 0}, {10, 0}, {0, 10}};
  // The Point is inside.
  EXPECT_TRUE(left_winding_triangle.Contains({2, 2}));

  // The Point is outside.
  EXPECT_FALSE(left_winding_triangle.Contains({12, 2}));
  EXPECT_FALSE(left_winding_triangle.Contains({-5, 2}));
  EXPECT_FALSE(left_winding_triangle.Contains({2, -10}));
  EXPECT_FALSE(left_winding_triangle.Contains({-1, -1}));
  EXPECT_FALSE(left_winding_triangle.Contains({12, -1}));
  EXPECT_FALSE(left_winding_triangle.Contains({-1, 12}));

  // The Point is aligned with an edge but outside the Triangle.
  EXPECT_FALSE(left_winding_triangle.Contains({-1, 0}));
  EXPECT_FALSE(left_winding_triangle.Contains({11, 0}));
  EXPECT_FALSE(left_winding_triangle.Contains({-1, 11}));
  EXPECT_FALSE(left_winding_triangle.Contains({11, -1}));
  EXPECT_FALSE(left_winding_triangle.Contains({0, 11}));
  EXPECT_FALSE(left_winding_triangle.Contains({0, -1}));

  // The Point is on an edge.
  EXPECT_TRUE(left_winding_triangle.Contains({0, 5}));
  EXPECT_TRUE(left_winding_triangle.Contains({8, 2}));
  EXPECT_TRUE(left_winding_triangle.Contains({7, 0}));

  // The Point is on a Triangle vertex.
  EXPECT_TRUE(left_winding_triangle.Contains({0, 0}));
  EXPECT_TRUE(left_winding_triangle.Contains({10, 0}));
  EXPECT_TRUE(left_winding_triangle.Contains({0, 10}));
}

TEST(TriangleTest, SegmentLikeTriangleContains) {
  Triangle t{{1, 1}, {9, 5}, {5, 3}};

  EXPECT_TRUE(t.Contains({1, 1}));
  EXPECT_TRUE(t.Contains({3, 2}));
  EXPECT_TRUE(t.Contains({5, 3}));
  EXPECT_TRUE(t.Contains({7, 4}));
  EXPECT_TRUE(t.Contains({9, 5}));

  EXPECT_FALSE(t.Contains({0, 0}));
  EXPECT_FALSE(t.Contains({1, 2}));
  EXPECT_FALSE(t.Contains({-1, 0}));
  EXPECT_FALSE(t.Contains({11, 6}));
}

TEST(TriangleTest, PointLikeTriangleContains) {
  Triangle t{{2, 2}, {2, 2}, {2, 2}};

  EXPECT_TRUE(t.Contains({2, 2}));

  EXPECT_FALSE(t.Contains({1, 2}));
  EXPECT_FALSE(t.Contains({3, 2}));
  EXPECT_FALSE(t.Contains({2, 3}));
}

void TriangleContainsItsCorners(Triangle triangle) {
  EXPECT_TRUE(triangle.Contains(triangle.p0))
      << "Where triangle is: " << testing::PrintToString(triangle);
  EXPECT_TRUE(triangle.Contains(triangle.p1))
      << "Where triangle is: " << testing::PrintToString(triangle);
  EXPECT_TRUE(triangle.Contains(triangle.p2))
      << "Where triangle is: " << testing::PrintToString(triangle);
}
FUZZ_TEST(TriangleTest, TriangleContainsItsCorners)
    .WithDomains(
        // This currently doesn't work for Triangles with extreme corner
        // positions, due to float overflow.
        TriangleInRect(Rect::FromCenterAndDimensions(kOrigin, 1e19f, 1e19f)));

TEST(TriangleTest, GetEdge) {
  Triangle triangle{{1, 2}, {3, 4}, {5, 6}};

  EXPECT_EQ(triangle.GetEdge(0), (Segment{{1, 2}, {3, 4}}));
  EXPECT_NE(triangle.GetEdge(0), (Segment{{3, 4}, {1, 2}}));

  EXPECT_EQ(triangle.GetEdge(1), (Segment{{3, 4}, {5, 6}}));
  EXPECT_NE(triangle.GetEdge(1), (Segment{{5, 6}, {3, 4}}));

  EXPECT_EQ(triangle.GetEdge(2), (Segment{{5, 6}, {1, 2}}));
  EXPECT_NE(triangle.GetEdge(2), (Segment{{1, 2}, {5, 6}}));
}

TEST(TriangleDeathTest, GetEdge) {
  Triangle triangle{{1, 2}, {3, 4}, {5, 6}};
  // Bad Index
  EXPECT_DEATH_IF_SUPPORTED((triangle.GetEdge(3)), "");
  EXPECT_DEATH_IF_SUPPORTED((triangle.GetEdge(10)), "");
  EXPECT_DEATH_IF_SUPPORTED((triangle.GetEdge(-1)), "");
}

}  // namespace
}  // namespace ink
