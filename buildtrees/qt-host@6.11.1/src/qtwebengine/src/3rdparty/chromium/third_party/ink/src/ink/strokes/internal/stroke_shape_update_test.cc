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

#include "ink/strokes/internal/stroke_shape_update.h"

#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ::testing::Eq;
using ::testing::Optional;

TEST(StrokeShapeUpdateTest, AddEmptyUpdateToEmpty) {
  StrokeShapeUpdate update;
  StrokeShapeUpdate other;

  update.Add(other);

  EXPECT_THAT(update.region.AsRect(), Eq(std::nullopt));
  EXPECT_THAT(update.first_index_offset, Eq(std::nullopt));
  EXPECT_THAT(update.first_vertex_offset, Eq(std::nullopt));
}

TEST(StrokeShapeUpdateTest, AddNonEmptyUpdateToEmpty) {
  StrokeShapeUpdate update;
  StrokeShapeUpdate other = {
      .region = Envelope(Rect::FromCenterAndDimensions({1, 2}, 0, 0)),
      .first_index_offset = 5,
      .first_vertex_offset = 10};

  update.Add(other);

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectEq(Rect::FromCenterAndDimensions({1, 2}, 0, 0))));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(5)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(10)));
}

TEST(StrokeShapeUpdateTest, AddEmptyUpdateToNonEmpty) {
  StrokeShapeUpdate update = {
      .region = Envelope(Rect::FromCenterAndDimensions({1, 2}, 0, 0)),
      .first_index_offset = 5,
      .first_vertex_offset = 10};
  StrokeShapeUpdate other;

  update.Add(other);

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectEq(Rect::FromCenterAndDimensions({1, 2}, 0, 0))));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(5)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(10)));
}

TEST(StrokeShapeUpdateTest, AddNonEmptyUpdateToNonEmpty) {
  StrokeShapeUpdate update = {
      .region = Envelope(Rect::FromCenterAndDimensions({1, 2}, 0, 0)),
      .first_index_offset = 5,
      .first_vertex_offset = 45};
  StrokeShapeUpdate other = {
      .region = Envelope(Rect::FromCenterAndDimensions({4, 5}, 0, 0)),
      .first_index_offset = 17,
      .first_vertex_offset = 10};

  update.Add(other);

  EXPECT_THAT(update.region.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints({1, 2}, {4, 5}))));
  EXPECT_THAT(update.first_index_offset, Optional(Eq(5)));
  EXPECT_THAT(update.first_vertex_offset, Optional(Eq(10)));
}

}  // namespace
}  // namespace ink::strokes_internal
