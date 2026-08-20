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

#include "ink/geometry/internal/legacy_triangle_contains.h"

#include "gtest/gtest.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"

namespace ink {
namespace geometry_internal {
namespace {

TEST(TriangleTest, Contains) {
  Triangle triangle = {{0, 0}, {0, 10}, {10, 0}};
  // Point is inside.
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{2, 2}));

  // Point is outside.
  EXPECT_FALSE(LegacyTriangleContains(triangle, Point{12, 2}));
  EXPECT_FALSE(LegacyTriangleContains(triangle, Point{-5, 2}));

  // Point is on an edge.
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{0, 5}));
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{8, 2}));
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{7, 0}));

  // Point is on a triangle vertex.
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{0, 0}));
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{10, 0}));
  EXPECT_TRUE(LegacyTriangleContains(triangle, Point{0, 10}));
}

}  // namespace
}  // namespace geometry_internal
}  // namespace ink
