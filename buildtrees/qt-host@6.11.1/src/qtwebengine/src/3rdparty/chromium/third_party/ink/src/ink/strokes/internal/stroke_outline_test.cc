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

#include "ink/strokes/internal/stroke_outline.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace ink::strokes_internal {
namespace {

using ::testing::ElementsAreArray;

TEST(StrokeOutlineTest, DefaultConstructedIsEmpty) {
  StrokeOutline outline;
  EXPECT_EQ(outline.GetIndexCounts().left, 0);
  EXPECT_EQ(outline.GetIndexCounts().right, 0);
  EXPECT_TRUE(outline.GetIndices().empty());
}

TEST(StrokeOutlineTest, AppendNewIndices) {
  StrokeOutline outline;

  outline.AppendNewIndices({}, {});
  EXPECT_EQ(outline.GetIndexCounts().left, 0);
  EXPECT_EQ(outline.GetIndexCounts().right, 0);
  EXPECT_TRUE(outline.GetIndices().empty());

  outline.AppendNewIndices({0, 1}, {2, 3, 4});
  EXPECT_EQ(outline.GetIndexCounts().left, 2);
  EXPECT_EQ(outline.GetIndexCounts().right, 3);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({4, 3, 2, 0, 1}));

  outline.AppendNewIndices({5}, {});
  EXPECT_EQ(outline.GetIndexCounts().left, 3);
  EXPECT_EQ(outline.GetIndexCounts().right, 3);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({4, 3, 2, 0, 1, 5}));

  outline.AppendNewIndices({}, {6, 7});
  EXPECT_EQ(outline.GetIndexCounts().left, 3);
  EXPECT_EQ(outline.GetIndexCounts().right, 5);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({7, 6, 4, 3, 2, 0, 1, 5}));
}

TEST(StrokeOutlineTest, TruncateIndicesWhenEmpty) {
  StrokeOutline outline;
  EXPECT_EQ(outline.GetIndexCounts().left, 0);
  EXPECT_EQ(outline.GetIndexCounts().right, 0);
  EXPECT_TRUE(outline.GetIndices().empty());

  outline.TruncateIndices({.left = 0, .right = 0});
  EXPECT_EQ(outline.GetIndexCounts().left, 0);
  EXPECT_EQ(outline.GetIndexCounts().right, 0);
  EXPECT_TRUE(outline.GetIndices().empty());

  outline.TruncateIndices({.left = 10, .right = 5});
  EXPECT_EQ(outline.GetIndexCounts().left, 0);
  EXPECT_EQ(outline.GetIndexCounts().right, 0);
  EXPECT_TRUE(outline.GetIndices().empty());
}

TEST(StrokeOutlineTest, TruncateIndicesWhenNonEmpty) {
  StrokeOutline outline;
  outline.AppendNewIndices({0, 1, 2, 3}, {4, 5, 6, 7, 8});
  ASSERT_EQ(outline.GetIndexCounts().left, 4);
  ASSERT_EQ(outline.GetIndexCounts().right, 5);
  ASSERT_THAT(outline.GetIndices(),
              ElementsAreArray({8, 7, 6, 5, 4, 0, 1, 2, 3}));

  // Truncate no-op:
  outline.TruncateIndices({.left = 5, .right = 6});
  EXPECT_EQ(outline.GetIndexCounts().left, 4);
  EXPECT_EQ(outline.GetIndexCounts().right, 5);
  EXPECT_THAT(outline.GetIndices(),
              ElementsAreArray({8, 7, 6, 5, 4, 0, 1, 2, 3}));

  // Truncate making only the left side smaller:
  outline.TruncateIndices({.left = 3, .right = 5});
  EXPECT_EQ(outline.GetIndexCounts().left, 3);
  EXPECT_EQ(outline.GetIndexCounts().right, 5);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({8, 7, 6, 5, 4, 0, 1, 2}));

  // Truncate making only the right side smaller:
  outline.TruncateIndices({.left = 7, .right = 3});
  EXPECT_EQ(outline.GetIndexCounts().left, 3);
  EXPECT_EQ(outline.GetIndexCounts().right, 3);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({6, 5, 4, 0, 1, 2}));

  // Truncate both sides:
  outline.TruncateIndices({.left = 1, .right = 2});
  EXPECT_EQ(outline.GetIndexCounts().left, 1);
  EXPECT_EQ(outline.GetIndexCounts().right, 2);
  EXPECT_THAT(outline.GetIndices(), ElementsAreArray({5, 4, 0}));
}

}  // namespace
}  // namespace ink::strokes_internal
