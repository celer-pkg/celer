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

#include "ink/strokes/internal/brush_tip_extruder/find_clockwise_winding_segment.h"

#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/legacy_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::ink::strokes_internal::LegacyVertex;
using ::testing::Eq;
using ::testing::Optional;

class FindLastClockwiseWindingTriangleFanSegmentTest : public testing::Test {
 protected:
  void SetUp() override {
    mesh_ = MutableMeshView(vertices_, triangle_indices_);
  }

  // A square lightbulb, with some repeated vertices:
  //
  // y = 3    X------------X
  //          |            |
  //          |            |
  //          |            |
  // y = 1    X---X    X---X
  //              |    |
  // y = 0        X    X
  //
  // x =     -2  -1    1   2
  std::vector<LegacyVertex> vertices_ = {
      {.position = {1, 0}},  {.position = {1, 0}},  {.position = {1, 1}},
      {.position = {2, 1}},  {.position = {2, 3}},  {.position = {-2, 3}},
      {.position = {-2, 3}}, {.position = {-2, 1}}, {.position = {-1, 1}},
      {.position = {-1, 0}},
  };
  std::vector<IndexType> left_indices_ = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  std::vector<IndexType> right_indices_ = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<IndexType> triangle_indices_;
  MutableMeshView mesh_;
};

TEST_F(FindLastClockwiseWindingTriangleFanSegmentTest,
       CounterClockwiseTrianglesOnly) {
  // Test position: "O"
  //
  // y = 3    X-------------X
  //          |             |
  //          |      O      |
  //          |             |
  // y = 1    X---X     X---X
  //              |     |
  // y = 0        X     X
  //
  // x =     -2  -1     1   2

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(mesh_, left_indices_,
                                                         SideId::kLeft, {0, 2}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {0, 2}),
              Eq(std::nullopt));
}

TEST_F(FindLastClockwiseWindingTriangleFanSegmentTest,
       IncludeDegenerateTriangles) {
  // Test positions: "O"
  //
  // y = 3    X------O------X
  //          |             |
  //          |   O     O   |
  //          |             |
  // y = 1    X---X  O  X---X
  //              |     |
  // y = 0        X     X
  //
  // x =     -2  -1     1   2

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, left_indices_, SideId::kLeft, {-1, 2}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {-1, 2}),
              Eq(std::nullopt));

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(mesh_, left_indices_,
                                                         SideId::kLeft, {1, 2}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {1, 2}),
              Eq(std::nullopt));

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(mesh_, left_indices_,
                                                         SideId::kLeft, {0, 3}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {0, 3}),
              Eq(std::nullopt));

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(mesh_, left_indices_,
                                                         SideId::kLeft, {0, 1}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {0, 1}),
              Eq(std::nullopt));
}

TEST_F(FindLastClockwiseWindingTriangleFanSegmentTest, ClockwiseTriangles) {
  // Test positions: "O"
  //
  // y = 3    X-------------X
  //          |             |
  //          | O         O |
  //          |             |
  // y = 1    X---X     X---X
  //              |     |
  // y = 0        X  O  X
  //
  // x =     -2  -1     1   2

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, left_indices_, SideId::kLeft, {-1.5, 2}),
              Optional(SegmentEq({.start = {-1, 1}, .end = {-1, 0}})));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {-1.5, 2}),
              Optional(SegmentEq({.start = {-1, 1}, .end = {-1, 0}})));

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, left_indices_, SideId::kLeft, {1.5, 2}),
              Optional(SegmentEq({.start = {1, 0}, .end = {1, 1}})));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {1.5, 2}),
              Optional(SegmentEq({.start = {1, 0}, .end = {1, 1}})));

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(mesh_, left_indices_,
                                                         SideId::kLeft, {0, 0}),
              Optional(SegmentEq({.start = {1, 1}, .end = {2, 1}})));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, right_indices_, SideId::kRight, {0, 0}),
              Optional(SegmentEq({.start = {-2, 1}, .end = {-1, 1}})));
}

TEST_F(FindLastClockwiseWindingTriangleFanSegmentTest, EmptyOutline) {
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, {}, SideId::kLeft, {10, 10}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, {}, SideId::kRight, {10, 10}),
              Eq(std::nullopt));
}

TEST_F(FindLastClockwiseWindingTriangleFanSegmentTest, SinglePosition) {
  ASSERT_EQ(vertices_[0].position, vertices_[1].position);

  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, {1, 0}, SideId::kLeft, {10, 10}),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, {0, 1}, SideId::kRight, {10, 10}),
              Eq(std::nullopt));
}

class FindLastClockwiseWindingMultiTriangleFanSegmentTest
    : public testing::Test {
 protected:
  void SetUp() override {
    // A loop. Include some placeholder vertices so that indices do not match
    // the offsets into `Side::indices`.
    //
    // Discontinuity bounds: "*"
    //
    // y = 3        2 -- 3
    //              |    |
    // y = 2   5 ------- 4*
    //              |
    // y = 1        1*
    //              |
    // y = 0        0
    //
    // x =     -1   0   1
    const LegacyVertex kPlaceholder = {.position{-1, -1}};
    vertices_ = {
        kPlaceholder, {.position = {0, 0}}, {.position = {0, 1}},
        kPlaceholder, {.position = {0, 3}}, {.position = {1, 3}},
        kPlaceholder, {.position = {1, 2}}, {.position = {-1, 2}},
    };

    left_side_.self_id = SideId::kLeft;
    left_side_.first_triangle_vertex = 0;
    left_side_.indices = {1, 2, 4, 5, 7, 8};
    left_side_.intersection_discontinuities.push_back(
        Side::IndexOffsetRange{.first = 1, .last = 4});
    left_side_indices_ = left_side_.indices;

    mesh_ = MutableMeshView(vertices_, triangle_indices_);
  }

  std::vector<LegacyVertex> vertices_;
  Side left_side_;
  absl::Span<const IndexType> left_side_indices_;
  std::vector<IndexType> triangle_indices_;
  MutableMeshView mesh_;
};

TEST_F(FindLastClockwiseWindingMultiTriangleFanSegmentTest, EntireRange) {
  // Test position: "X"
  // Discontinuity bounds: "*"
  //
  // y = 3        2 -- 3
  //              |    |     X
  // y = 2   5 ------- 4
  //              |
  // y = 1        1
  //              |
  // y = 0        0
  //
  // x =     -1   0   1

  // Position for which checking the entire range of indices naively returns a
  // clockwise triangle, but checking with the discontinuity taken into account
  // does not.
  Point test_position = {2, 2.5};
  EXPECT_THAT(FindLastClockwiseWindingTriangleFanSegment(
                  mesh_, left_side_indices_, SideId::kLeft, test_position),
              Optional(SegmentEq({.start = {1, 2}, .end = {1, 3}})));
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 0, .last = 5}, test_position),
              Eq(std::nullopt));
}

TEST_F(FindLastClockwiseWindingMultiTriangleFanSegmentTest,
       DiscontinuityBoundaries) {
  // Test positions: "X"
  // Discontinuity bounds: "*"
  //
  // y = 3        2 -- 3
  //              | X  |
  // y = 2   5 ------- 4*
  //              |
  // y = 1   X    1*
  //              |
  // y = 0        0
  //
  // x =     -1   0   1

  // Should not test the triangle connecting index offsets 1 and 4 when the
  // offset range does not completely span the discontinuity range:

  Point test_position = {0, 2.5};
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 0, .last = 3}, test_position),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 0, .last = 4}, test_position),
              Optional(SegmentEq({.start = {1, 2}, .end = {0, 1}})));

  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 2, .last = 5}, test_position),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 1, .last = 5}, test_position),
              Optional(SegmentEq({.start = {1, 2}, .end = {0, 1}})));

  test_position = {-1, 1};
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 1, .last = 3}, test_position),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 2, .last = 4}, test_position),
              Eq(std::nullopt));
  EXPECT_THAT(FindLastClockwiseWindingMultiTriangleFanSegment(
                  mesh_, left_side_,
                  Side::IndexOffsetRange{.first = 1, .last = 4}, test_position),
              Optional(SegmentEq({.start = {1, 2}, .end = {0, 1}})));
}

}  // namespace
}  // namespace brush_tip_extruder_internal
}  // namespace ink
