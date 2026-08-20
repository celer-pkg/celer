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

#include "ink/types/internal/copy_on_write.h"

#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace ink_internal {
namespace {

using ::testing::ElementsAreArray;

TEST(CopyOnWriteTest, DefaultConstructed) {
  CopyOnWrite<int> x;

  EXPECT_FALSE(x.HasValue());
  EXPECT_FALSE(x.IsShared());
}

TEST(CopyOnWriteTest, ConstructWithValue) {
  CopyOnWrite<int> x(4);

  EXPECT_TRUE(x.HasValue());
  EXPECT_FALSE(x.IsShared());
  EXPECT_EQ(x.Value(), 4);
  EXPECT_EQ(x.MutableValue(), 4);
  EXPECT_EQ(*x, 4);
  EXPECT_EQ(*(x.operator->()), 4);
}

TEST(CopyOnWriteTest, EmplaceValue) {
  CopyOnWrite<int> x;
  ASSERT_FALSE(x.HasValue());

  x.Emplace(7);
  EXPECT_TRUE(x.HasValue());
  EXPECT_FALSE(x.IsShared());
  EXPECT_EQ(x.Value(), 7);
  EXPECT_EQ(x.MutableValue(), 7);
  EXPECT_EQ(*x, 7);
  EXPECT_EQ(*(x.operator->()), 7);
}

TEST(CopyOnWriteTest, ResetClearsValue) {
  CopyOnWrite<int> x(4);
  ASSERT_TRUE(x.HasValue());

  x.Reset();
  EXPECT_FALSE(x.HasValue());
  EXPECT_FALSE(x.IsShared());

  x.Emplace(8);
  ASSERT_TRUE(x.HasValue());

  x.Reset();
  EXPECT_FALSE(x.HasValue());
  EXPECT_FALSE(x.IsShared());
}

TEST(CopyOnWriteTest, MutateValue) {
  CopyOnWrite<int> x(4);
  ASSERT_EQ(*x, 4);

  x.MutableValue() = 13;
  EXPECT_EQ(x.Value(), 13);
  EXPECT_EQ(x.MutableValue(), 13);
  EXPECT_EQ(*x, 13);
  EXPECT_EQ(*(x.operator->()), 13);
}

TEST(CopyOnWriteTest, ValueAfterCopyConstruction) {
  CopyOnWrite<int> x(3);
  CopyOnWrite<int> y(x);

  EXPECT_TRUE(x.HasValue());
  EXPECT_TRUE(x.IsShared());
  EXPECT_EQ(*x, 3);

  EXPECT_TRUE(y.HasValue());
  EXPECT_TRUE(y.IsShared());
  EXPECT_EQ(*y, 3);
}

TEST(CopyOnWriteTest, ValueAfterCopyAssignment) {
  CopyOnWrite<int> x(3);
  CopyOnWrite<int> y;
  y = x;

  EXPECT_TRUE(x.HasValue());
  EXPECT_TRUE(x.IsShared());
  EXPECT_EQ(*x, 3);

  EXPECT_TRUE(y.HasValue());
  EXPECT_TRUE(y.IsShared());
  EXPECT_EQ(*y, 3);
}

TEST(CopyOnWriteTest, ValueAfterMoveConstruction) {
  CopyOnWrite<int> x(3);
  CopyOnWrite<int> y(std::move(x));

  // (Suppress ClangTidy bugprone-use-after-move)
  EXPECT_FALSE(x.HasValue());  // NOLINT
  EXPECT_FALSE(x.IsShared());  // NOLINT

  EXPECT_TRUE(y.HasValue());
  EXPECT_FALSE(y.IsShared());
  EXPECT_EQ(*y, 3);
}

TEST(CopyOnWriteTest, ValueAfterMoveAssignment) {
  CopyOnWrite<int> x(3);
  CopyOnWrite<int> y;
  y = std::move(x);

  // (Suppress ClangTidy bugprone-use-after-move)
  EXPECT_FALSE(x.HasValue());  // NOLINT
  EXPECT_FALSE(x.IsShared());  // NOLINT

  EXPECT_TRUE(y.HasValue());
  EXPECT_FALSE(y.IsShared());
  EXPECT_EQ(*y, 3);
}

TEST(CopyOnWriteTest, CopyIsShallow) {
  CopyOnWrite<std::vector<int>> x;
  x.Emplace() = {1, 2, 3};
  const int* address_before_copy = x->data();

  CopyOnWrite<std::vector<int>> y = x;
  EXPECT_EQ(x->data(), address_before_copy);
  EXPECT_EQ(y->data(), address_before_copy);
}

TEST(CopyOnWriteTest, GetMutatableValueAfterCopy) {
  CopyOnWrite<std::vector<int>> x;
  x.Emplace() = {1, 2, 3};
  const int* address_before_copy = x->data();

  CopyOnWrite<std::vector<int>> y = x;
  ASSERT_EQ(x->data(), address_before_copy);
  ASSERT_EQ(y->data(), address_before_copy);

  // Mutate through the copy:
  (void)y.MutableValue();
  EXPECT_FALSE(x.IsShared());
  EXPECT_FALSE(y.IsShared());
  EXPECT_EQ(x->data(), address_before_copy);
  EXPECT_NE(y->data(), address_before_copy);
  EXPECT_THAT(*y, ElementsAreArray(*x));

  y = x;
  ASSERT_EQ(x->data(), address_before_copy);
  ASSERT_EQ(y->data(), address_before_copy);

  // Mutate through the copied from object:
  (void)x.MutableValue();
  EXPECT_FALSE(x.IsShared());
  EXPECT_FALSE(y.IsShared());
  EXPECT_NE(x->data(), address_before_copy);
  EXPECT_EQ(y->data(), address_before_copy);
  EXPECT_THAT(*y, ElementsAreArray(*x));
}

TEST(CopyOnWriteTest, MutableValueOnEmpty) {
  CopyOnWrite<int> x;
  ASSERT_FALSE(x.HasValue());
  EXPECT_DEATH_IF_SUPPORTED(x.MutableValue(), "");
}

TEST(CopyOnWriteTest, ValueOnEmpty) {
  CopyOnWrite<int> x;
  ASSERT_FALSE(x.HasValue());
  EXPECT_DEATH_IF_SUPPORTED(x.Value(), "");
}

}  // namespace
}  // namespace ink_internal
