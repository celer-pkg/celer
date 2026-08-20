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

#include "ink/strokes/internal/brush_tip_extruder/directed_partial_outline.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/strokes/internal/brush_tip_extruder/mutable_mesh_view.h"
#include "ink/strokes/internal/brush_tip_extruder/side.h"
#include "ink/strokes/internal/legacy_vertex.h"

namespace ink {
namespace brush_tip_extruder_internal {
namespace {

using ::testing::FloatNear;

TEST(DirectedPartialOutlineTest, Size) {
  DirectedPartialOutline default_constructed;
  EXPECT_EQ(default_constructed.Size(), 0);
  EXPECT_EQ(default_constructed.StartingSideSize(), 0);

  std::vector<IndexType> left(10, 5);
  std::vector<IndexType> right(3, 7);
  DirectedPartialOutline outline(&left, 0, left.size(), &right, 0,
                                 right.size());
  EXPECT_EQ(outline.Size(), left.size() + right.size());
  EXPECT_EQ(outline.StartingSideSize(), left.size());
}

TEST(DirectedPartialOutlineTest, IterationOneSideEmpty) {
  std::vector<IndexType> nonempty({0, 1, 2, 3, 4});

  {
    DirectedPartialOutline outline(nullptr, 0, 0, &nonempty, 0,
                                   nonempty.size());
    for (uint32_t i = 0; i < outline.Size(); ++i) EXPECT_EQ(outline[i], i);
  }
  {
    DirectedPartialOutline outline(&nonempty, 0, nonempty.size(), nullptr, 0,
                                   0);
    for (uint32_t i = 0; i < outline.Size(); ++i) EXPECT_EQ(outline[i], 4 - i);
  }
}

TEST(DirectedPartialOutlineTest, IterationNonEmptySides) {
  std::vector<IndexType> left({3, 2, 1, 0});
  std::vector<IndexType> right({4, 5, 6, 7, 8, 9, 10});

  DirectedPartialOutline start_left_outline(&left, 0, left.size(), &right, 0,
                                            right.size());
  for (uint32_t i = 0; i < start_left_outline.Size(); ++i) {
    EXPECT_EQ(start_left_outline[i], i);
  }
}

struct MeshData {
  std::vector<strokes_internal::LegacyVertex> vertices;
  std::vector<IndexType> triangle_indices;
};

MutableMeshView MakeView(MeshData& data) {
  return MutableMeshView(data.vertices, data.triangle_indices);
}

TEST(FindOutlineIntersectionTest, EmptyOutline) {
  DirectedPartialOutline empty_outline;
  MutableMeshView empty_mesh;
  float search_budget = 10;
  OutlineIntersectionResult result = FindOutlineIntersection(
      empty_outline, Segment{.start{-2, 1}, .end = {0, 1}}, empty_mesh,
      search_budget);
  EXPECT_FALSE(result.segment_intersection.has_value());
  EXPECT_EQ(result.remaining_search_budget, search_budget);
}

TEST(FindOutlineIntersectionTest, ZeroInitialSearchBudget) {
  MeshData data = {.vertices = {{.position = {-1, 0}}, {.position = {1, 0}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0, 1};
  DirectedPartialOutline outline(&indices, 0, indices.size(), nullptr, 0, 0);
  float search_budget = 0;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, Segment{.start = {0, -1}, .end = {0, 1}}, mesh, search_budget);
  EXPECT_FALSE(result.segment_intersection.has_value());
  EXPECT_EQ(result.remaining_search_budget, 0);
}

TEST(FindOutlineIntersectionTest, SingleNondegenerateSegment) {
  MeshData data = {.vertices = {{.position = {-1, 0}}, {.position = {1, 0}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0, 1};
  // Note that starting side indices are traversed backwards, so the outline
  // segment will travel from x = 1 to x = -1.
  DirectedPartialOutline outline(&indices, 0, indices.size(), nullptr, 0, 0);
  float search_budget = 10;

  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0.5, -1}, .end = {0.5, 1}}, mesh,
        search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    EXPECT_EQ(result.segment_intersection->starting_index, 0);
    EXPECT_EQ(result.segment_intersection->ending_index, 1);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(0.25, 0.001));
    EXPECT_THAT(result.segment_intersection->segment_interpolation_value,
                FloatNear(0.5, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(9.5, 0.001));
  }
  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0, -0.5}, .end = {0, 1.5}}, mesh,
        search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    EXPECT_EQ(result.segment_intersection->starting_index, 0);
    EXPECT_EQ(result.segment_intersection->ending_index, 1);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(0.5, 0.001));
    EXPECT_THAT(result.segment_intersection->segment_interpolation_value,
                FloatNear(0.25, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(9, 0.001));
  }
  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {-0.6, -1}, .end = {-0.6, 0}}, mesh,
        search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    EXPECT_EQ(result.segment_intersection->starting_index, 0);
    EXPECT_EQ(result.segment_intersection->ending_index, 1);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(0.8, 0.001));
    EXPECT_THAT(result.segment_intersection->segment_interpolation_value,
                FloatNear(1, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(8.4, 0.001));
  }
}

TEST(FindOutlineIntersectionTest, IncludingDegenerateSegment) {
  // Tests that we correctly handle the situation of duplicate adjacent vertex
  // positions, causing a degenerate segment in the outline.
  MeshData data = {.vertices = {{.position = {0, 1}},
                                {.position = {0, 2}},
                                {.position = {0, 2}},
                                {.position = {0, 3}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0, 1, 2, 3};
  DirectedPartialOutline outline(nullptr, 0, 0, &indices, 0, indices.size());
  float search_budget = 10;

  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {-1, 1.9}, .end = {1, 1.9}}, mesh,
        search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    // Intersection should stop on the first non-degenerate segment.
    EXPECT_EQ(result.segment_intersection->starting_index, 0);
    EXPECT_EQ(result.segment_intersection->ending_index, 1);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(0.9, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(9.1, 0.001));
  }
  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {-1, 2}, .end = {1, 2}}, mesh, search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    // Intersection should still stop on the first non-degenerate segment.
    EXPECT_EQ(result.segment_intersection->starting_index, 0);
    EXPECT_EQ(result.segment_intersection->ending_index, 1);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(1, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(9, 0.001));
  }
  {
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {-1, 2.1}, .end = {1, 2.1}}, mesh,
        search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    // Intersection should stop on the second non-degenerate segment.
    EXPECT_EQ(result.segment_intersection->starting_index, 2);
    EXPECT_EQ(result.segment_intersection->ending_index, 3);
    EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
                FloatNear(0.1, 0.001));
    EXPECT_THAT(result.remaining_search_budget, FloatNear(8.9, 0.001));
  }
}

TEST(FindOutlineIntersectionTest, SingleVertexOutline) {
  MeshData data = {.vertices = {{.position = {0, 0}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0};
  DirectedPartialOutline outline(&indices, 0, indices.size(), nullptr, 0, 0);
  float search_budget = 10;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, Segment{.start = {-1, 0}, .end = {1, 0}}, mesh, search_budget);
  EXPECT_TRUE(result.segment_intersection.has_value());
  EXPECT_EQ(result.segment_intersection->starting_index, 0);
  EXPECT_EQ(result.segment_intersection->ending_index, 0);
  EXPECT_EQ(result.remaining_search_budget, search_budget);
}

TEST(FindOutlineIntersectionTest, OnlyDegenerateSegments) {
  MeshData data = {.vertices = {{.position = {0, 0}},
                                {.position = {0, 0}},
                                {.position = {0, 0}},
                                {.position = {0, 0}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0, 1, 2, 3};
  DirectedPartialOutline outline(&indices, 0, indices.size(), nullptr, 0, 0);
  float search_budget = 10;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, Segment{.start = {-1, 0}, .end = {1, 0}}, mesh, search_budget);
  EXPECT_TRUE(result.segment_intersection.has_value());
  EXPECT_EQ(result.segment_intersection->starting_index, 3);
  EXPECT_EQ(result.segment_intersection->ending_index, 3);
  EXPECT_EQ(result.remaining_search_budget, search_budget);
}

TEST(FindOutlineIntersectionTest, NoIntersectionExcessSearchBudget) {
  // Total outline length: 6
  //
  // (-1,2) X      X (1, 2)
  //        |      |
  // (-1,0) X------X (1, 0)
  //
  MeshData data = {.vertices = {{.position = {-1, 0}},
                                {.position = {-1, 2}},
                                {.position = {1, 0}},
                                {.position = {1, 2}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> left_indices = {0, 1};
  std::vector<IndexType> right_indices = {2, 3};
  DirectedPartialOutline outline(&left_indices, 0, left_indices.size(),
                                 &right_indices, 0, right_indices.size());
  float search_budget = 7;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, Segment{.start = {0, 1}, .end = {0, 2}}, mesh, search_budget);
  EXPECT_FALSE(result.segment_intersection.has_value());
  EXPECT_THAT(result.remaining_search_budget,
              FloatNear(search_budget - 6, 0.001));
}

TEST(FindOutlineIntersectionTest, PartiallyCoincidentSegments) {
  MeshData data = {.vertices = {{.position = {-1, 0}}, {.position = {1, 0}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> indices = {0, 1};
  DirectedPartialOutline outline(nullptr, 0, 0, &indices, 0, indices.size());
  float search_budget = 2;
  OutlineIntersectionResult result = FindOutlineIntersection(
      outline, Segment{.start = {0, 0}, .end = {0.5, 0}}, mesh, search_budget);
  EXPECT_TRUE(result.segment_intersection.has_value());
  EXPECT_THAT(result.segment_intersection->outline_interpolation_value,
              FloatNear(0.5, 0.001));
  EXPECT_THAT(result.segment_intersection->segment_interpolation_value,
              FloatNear(0, 0.001));
}

TEST(FindOutlineIntersectionTest, IntersectionPastBudget) {
  //
  // (-1,2) X      X (1, 2)
  //        |      |
  // (-1,0) X------X (1, 0)
  //
  MeshData data = {.vertices = {{.position = {-1, 0}},
                                {.position = {-1, 2}},
                                {.position = {1, 0}},
                                {.position = {1, 2}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> left_indices = {0, 1};
  std::vector<IndexType> right_indices = {2, 3};
  DirectedPartialOutline outline(&left_indices, 0, left_indices.size(),
                                 &right_indices, 0, right_indices.size());

  {
    // We run out of search budget after looking at the first segment.
    float search_budget = 1.9;
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0, -1}, .end = {0, 1}}, mesh, search_budget);
    EXPECT_FALSE(result.segment_intersection.has_value());
    EXPECT_EQ(result.remaining_search_budget, 0);
  }
  {
    // The start of the intersecting outline segment is at a distance traveled
    // of 2 along the outline. This is less than the search budget of 2.1, so we
    // detect the intersection even though the intersection itself lies a
    // distance traveled of 3 along the outline.
    float search_budget = 2.1;
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0, -1}, .end = {0, 1}}, mesh, search_budget);
    EXPECT_TRUE(result.segment_intersection.has_value());
    EXPECT_EQ(result.segment_intersection->starting_index, 1);
    EXPECT_EQ(result.segment_intersection->ending_index, 2);
    EXPECT_EQ(result.remaining_search_budget, 0);
  }
}

TEST(FindOutlineIntersectionTest, WithContainingTriangle) {
  //
  // (-1,2) X      X (1, 2)
  //        |      |
  // (-1,0) X------X (1, 0)
  //
  MeshData data = {.vertices = {{.position = {-1, 0}},
                                {.position = {-1, 2}},
                                {.position = {1, 0}},
                                {.position = {1, 2}}}};
  MutableMeshView mesh = MakeView(data);
  std::vector<IndexType> left_indices = {0, 1};
  std::vector<IndexType> right_indices = {2, 3};
  DirectedPartialOutline outline(&left_indices, 0, left_indices.size(),
                                 &right_indices, 0, right_indices.size());

  {
    // Fail because (-1, 0) is not contained in the passed-in triangle.
    float search_budget = 6;
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0, -1}, .end = {0, 1}}, mesh, search_budget,
        Triangle{.p0 = {-1, 2}, .p1 = {0, -1}, .p2 = {0, 1}});
    EXPECT_FALSE(result.segment_intersection.has_value());
    EXPECT_EQ(result.remaining_search_budget, 0);
  }
  {
    // This time (-1, 0) is inside the passed-in triangle.
    float search_budget = 6;
    OutlineIntersectionResult result = FindOutlineIntersection(
        outline, Segment{.start = {0, -1}, .end = {0, 1}}, mesh, search_budget,
        Triangle{.p0 = {-1, 2}, .p1 = {-1.5, -1}, .p2 = {0.5, 0.5}});
    EXPECT_TRUE(result.segment_intersection.has_value());
    EXPECT_EQ(result.segment_intersection->starting_index, 1);
    EXPECT_EQ(result.segment_intersection->ending_index, 2);
    EXPECT_GT(result.remaining_search_budget, 0);
  }
}

}  // namespace
}  // namespace brush_tip_extruder_internal
}  // namespace ink
