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

#include "ink/geometry/internal/legacy_vector_utils.h"

#include "gtest/gtest.h"
#include "ink/geometry/point.h"

namespace ink {
namespace geometry_internal {
namespace {

TEST(VectorUtilsTest, PositionRelativeToLine) {
  Point point0{4, 1};
  Point point1{3, -2};
  EXPECT_EQ(RelativePos::kLeft,
            PositionRelativeToLine(point0, point1, {4, -4}));
  EXPECT_EQ(RelativePos::kLeft, PositionRelativeToLine(point0, point1, {5, 0}));
  EXPECT_EQ(RelativePos::kRight,
            PositionRelativeToLine(point0, point1, {-2, -1}));
  EXPECT_EQ(RelativePos::kRight,
            PositionRelativeToLine(point0, point1, {3, 2}));
  EXPECT_EQ(RelativePos::kCollinear,
            PositionRelativeToLine(point0, point1, {3.5, -.5}));
  EXPECT_EQ(RelativePos::kCollinear,
            PositionRelativeToLine(point0, point1, {5, 4}));
}

TEST(VectorUtilsTest, PositionRelativeToLineDegenerate) {
  Point point{-3, 1};
  EXPECT_EQ(RelativePos::kIndeterminate,
            PositionRelativeToLine(point, point, {0, 0}));
  EXPECT_EQ(RelativePos::kIndeterminate,
            PositionRelativeToLine(point, point, {4, 4}));
  EXPECT_EQ(RelativePos::kIndeterminate,
            PositionRelativeToLine(point, point, {-6, 2}));
  EXPECT_EQ(RelativePos::kIndeterminate,
            PositionRelativeToLine(point, point, {-5, -1}));
  EXPECT_EQ(RelativePos::kIndeterminate,
            PositionRelativeToLine(point, point, {-3, 1}));
}

}  // namespace
}  // namespace geometry_internal
}  // namespace ink
