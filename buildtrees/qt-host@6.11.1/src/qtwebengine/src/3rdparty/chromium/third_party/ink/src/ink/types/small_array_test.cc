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

#include "ink/types/small_array.h"

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"

namespace ink {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(SmallArrayTest, DefaultCtor) {
  SmallArray<float, 4> array;
  EXPECT_EQ(array.Size(), 0);
  EXPECT_THAT(array.Values(), IsEmpty());
}

TEST(SmallArrayTest, MaxSize) {
  EXPECT_EQ((SmallArray<float, 2>()).MaxSize(), 2);
  EXPECT_EQ((SmallArray<float, 10>()).MaxSize(), 10);
}

TEST(SmallArrayTest, ConstructWithSize) {
  SmallArray<float, 4> array(3);
  EXPECT_EQ(array.Size(), 3);
  EXPECT_THAT(array.Values(), ElementsAre(0, 0, 0));
}

TEST(SmallArrayTest, ConstructWithSizeAndValue) {
  SmallArray<float, 4> array(2, 5);
  EXPECT_EQ(array.Size(), 2);
  EXPECT_THAT(array.Values(), ElementsAre(5, 5));
}

TEST(SmallArrayTest, ConstructWithInitializerList) {
  SmallArray<float, 4> array({2, 3, 4});
  EXPECT_EQ(array.Size(), 3);
  EXPECT_THAT(array.Values(), ElementsAre(2, 3, 4));
}

TEST(SmallArrayTest, ConstructWithSpan) {
  std::vector<float> values{2, 4, 6};
  SmallArray<float, 4> array(absl::MakeSpan(values));
  EXPECT_EQ(array.Size(), 3);
  EXPECT_THAT(array.Values(), ElementsAre(2, 4, 6));
}

TEST(SmallArrayTest, EqualsOperatorTrueSameValue) {
  std::vector<float> values1{2, 4, 6};
  SmallArray<float, 4> array1(absl::MakeSpan(values1));
  SmallArray<float, 4> array2(absl::MakeSpan(values1));
  EXPECT_TRUE(array1 == array2);
  // Empty array.
  std::vector<float> values2{};
  SmallArray<float, 4> array3(absl::MakeSpan(values2));
  SmallArray<float, 4> array4(absl::MakeSpan(values2));
  EXPECT_TRUE(array3 == array4);
}

TEST(SmallArrayTest, EqualsOperatorFalseDifferentValues) {
  std::vector<float> values1{2, 4, 6};
  SmallArray<float, 4> array1(absl::MakeSpan(values1));
  // Different ordering of values.
  std::vector<float> values2{6, 4, 2};
  SmallArray<float, 4> array2(absl::MakeSpan(values2));
  EXPECT_FALSE(array1 == array2);
  // Extra value.
  std::vector<float> values3{2, 4, 6, 0};
  SmallArray<float, 4> array3(absl::MakeSpan(values3));
  EXPECT_FALSE(array1 == array3);
  // Empty array.
  std::vector<float> values4{};
  SmallArray<float, 4> array4(absl::MakeSpan(values4));
  EXPECT_FALSE(array1 == array4);
}

TEST(SmallArrayTest, ValuesConst) {
  const SmallArray<float, 4> array({5, 6, 7, 8});
  EXPECT_THAT(array.Values(), ElementsAre(5, 6, 7, 8));
}

TEST(SmallArrayTest, SubscriptOperator) {
  SmallArray<float, 4> array({2, 4, 6});
  EXPECT_EQ(array[0], 2);
  EXPECT_EQ(array[1], 4);
  EXPECT_EQ(array[2], 6);

  array[1] = 9;
  EXPECT_EQ(array[1], 9);
}

TEST(SmallArrayTest, SubscriptOperatorConst) {
  const SmallArray<float, 4> array({2, 4, 6});
  EXPECT_EQ(array[0], 2);
  EXPECT_EQ(array[1], 4);
  EXPECT_EQ(array[2], 6);
}

TEST(SmallArrayTest, ResizeSmaller) {
  SmallArray<float, 6> array({1, 2, 3, 4, 5, 6});
  array.Resize(3);
  EXPECT_EQ(array.Size(), 3);
  EXPECT_EQ(array[0], 1);
  EXPECT_EQ(array[1], 2);
  EXPECT_EQ(array[2], 3);
}

TEST(SmallArrayTest, ResizeLargerDefaultValue) {
  SmallArray<float, 6> array({1, 2});
  array.Resize(5);
  EXPECT_EQ(array.Size(), 5);
  EXPECT_EQ(array[0], 1);
  EXPECT_EQ(array[1], 2);
  EXPECT_EQ(array[2], 0);
  EXPECT_EQ(array[3], 0);
  EXPECT_EQ(array[4], 0);
}

TEST(SmallArrayTest, ResizeLargerCustomValue) {
  SmallArray<float, 6> array({1, 2});
  array.Resize(5, 19);
  EXPECT_EQ(array.Size(), 5);
  EXPECT_EQ(array[0], 1);
  EXPECT_EQ(array[1], 2);
  EXPECT_EQ(array[2], 19);
  EXPECT_EQ(array[3], 19);
  EXPECT_EQ(array[4], 19);
}

TEST(SmallArrayTest, ResizeNonTrivialTypeSmaller) {
  auto p = std::make_shared<int>(42);
  SmallArray<std::shared_ptr<int>, 8> array(4, p);
  ASSERT_EQ(p.use_count(), 5);

  array.Resize(2);
  EXPECT_EQ(p.use_count(), 3);
}

TEST(SmallArrayDeathTest, ConstructWithTooManyElements) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED((SmallArray<float, 4>(5)), "");
  EXPECT_DEATH_IF_SUPPORTED((SmallArray<float, 4>(5, 1.5)), "");
  EXPECT_DEATH_IF_SUPPORTED((SmallArray<float, 4>({1, 2, 3, 4, 5})), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(SmallArrayDeathTest, ResizeWithTooManyElements) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  SmallArray<float, 4> array;
  EXPECT_DEATH_IF_SUPPORTED(array.Resize(5), "");
  EXPECT_DEATH_IF_SUPPORTED(array.Resize(5, 6.5), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(SmallArrayDeathTest, GetOutOfBoundsElement) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  SmallArray<float, 4> array({1, 2, 3});
  EXPECT_DEATH_IF_SUPPORTED(array[3], "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(SmallArrayDeathTest, GetOutOfBoundsElementAfterResizeSmaller) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  SmallArray<float, 10> array(8, 1.234);
  array.Resize(3);
  EXPECT_DEATH_IF_SUPPORTED(array[3], "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(SmallArrayDeathTest, GetOutOfBoundsElementAfterResizeLarger) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  SmallArray<float, 10> array(3, 1.234);
  array.Resize(7);
  EXPECT_DEATH_IF_SUPPORTED(array[7], "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

}  // namespace
}  // namespace ink
