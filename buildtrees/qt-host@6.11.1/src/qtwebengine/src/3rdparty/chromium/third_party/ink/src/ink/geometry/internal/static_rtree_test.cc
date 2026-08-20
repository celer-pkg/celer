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

#include "ink/geometry/internal/static_rtree.h"

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"
#include "ink/types/small_array.h"

namespace ink::geometry_internal {
namespace {

using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::IsEmpty;
using ::testing::Matcher;
using ::testing::Not;
using ::testing::Property;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

// Convenience alias for a `StaticRTree` of `Points`. We use a small value for
// `kBranchingFactor` so that we can test trees of different depths without
// needing thousands of elements.
using PointRTree = StaticRTree<Point, 3>;

// Bounds computation functor for a `StaticRTree` of `Point`s
auto point_bounds = [](const Point& p) {
  return Rect::FromCenterAndDimensions(p, 0, 0);
};

// Matches a `BranchNode` in a `PointRTree` by the index of its parent, its
// bounds, whether it contains leaf nodes, and the indices of its children.
Matcher<PointRTree::BranchNode> Branch(
    const Rect& bounds, bool is_leaf_parent,
    absl::Span<const uint32_t> child_indices) {
  return AllOf(Field("bounds", &PointRTree::BranchNode::bounds, RectEq(bounds)),
               Field("is_leaf_parent", &PointRTree::BranchNode::is_leaf_parent,
                     is_leaf_parent),
               Field("child_indices", &PointRTree::BranchNode::child_indices,
                     Property("Values", &SmallArray<uint32_t, 3>::Values,
                              ElementsAreArray(child_indices))));
}

TEST(StaticRTreeTest, DefaultCtor) {
  PointRTree rtree;

  EXPECT_THAT(rtree.BranchNodes(), IsEmpty());
  EXPECT_THAT(rtree.Elements(), IsEmpty());
}

TEST(StaticRTreeTest, CreateEmptyRTree) {
  PointRTree rtree(std::vector<Point>(), point_bounds);

  EXPECT_THAT(rtree.BranchNodes(), IsEmpty());
  EXPECT_THAT(rtree.Elements(), IsEmpty());
}

TEST(StaticRTreeTest, CreateEmptyRTreeWithGenerator) {
  auto generator = []() { return Point{0, 0}; };

  PointRTree rtree(0, generator, point_bounds);

  EXPECT_THAT(rtree.BranchNodes(), IsEmpty());
  EXPECT_THAT(rtree.Elements(), IsEmpty());
}

TEST(StaticRTreeTest, CreateFromOnePoint) {
  // B0 -- L0
  std::vector<Point> elements{{1, 2}};

  PointRTree rtree(elements, point_bounds);

  EXPECT_THAT(
      rtree.BranchNodes(),
      ElementsAre(Branch(Rect::FromTwoPoints({1, 2}, {1, 2}), true, {0})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateFromThreePoints) {
  // B0 +-- L0
  //    |
  //    +-- L1
  //    |
  //    +-- L2
  std::vector<Point> elements{{0, 0}, {1, 2}, {2, 4}};

  PointRTree rtree(elements, point_bounds);

  EXPECT_THAT(rtree.BranchNodes(),
              ElementsAre(Branch(Rect::FromTwoPoints({0, 0}, {2, 4}), true,
                                 {0, 1, 2})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateFromFivePoints) {
  // B0 +-- B1 +-- L0
  //    |      |
  //    |      +-- L4
  //    |      |
  //    |      +-- L3
  //    |
  //    +-- B2 +-- L1
  //           |
  //           +-- L2
  std::vector<Point> elements{{-1, -1}, {0, 2}, {4, 3}, {2, 1}, {-2, 0}};

  PointRTree rtree(elements, point_bounds);

  EXPECT_THAT(
      rtree.BranchNodes(),
      ElementsAre(
          Branch(Rect::FromTwoPoints({-2, -1}, {4, 3}), false, {1, 2}),
          Branch(Rect::FromTwoPoints({-2, -1}, {2, 1}), true, {0, 4, 3}),
          Branch(Rect::FromTwoPoints({0, 2}, {4, 3}), true, {1, 2})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateWithGeneratorFromFivePoints) {
  // B0 +-- B1 +-- L0
  //    |      |
  //    |      +-- L4
  //    |      |
  //    |      +-- L3
  //    |
  //    +-- B2 +-- L1
  //           |
  //           +-- L2
  std::vector<Point> elements{{-1, -1}, {0, 2}, {4, 3}, {2, 1}, {-2, 0}};
  auto generator = [&elements, idx = 0]() mutable { return elements[idx++]; };

  PointRTree rtree(elements.size(), generator, point_bounds);

  EXPECT_THAT(
      rtree.BranchNodes(),
      ElementsAre(
          Branch(Rect::FromTwoPoints({-2, -1}, {4, 3}), false, {1, 2}),
          Branch(Rect::FromTwoPoints({-2, -1}, {2, 1}), true, {0, 4, 3}),
          Branch(Rect::FromTwoPoints({0, 2}, {4, 3}), true, {1, 2})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateFromTwelvePoints) {
  // B0 +-- B1 +-- B3 +-- L0
  //    |      |      |
  //    |      |      +-- L5
  //    |      |      |
  //    |      |      +-- L6
  //    |      |
  //    |      +-- B5 +-- L3
  //    |      |      |
  //    |      |      +-- L1
  //    |      |      |
  //    |      |      +-- L11
  //    |      |
  //    |      +-- B4 +-- L10
  //    |             |
  //    |             +-- L9
  //    |             |
  //    |             +-- L4
  //    |
  //    +-- B2 +-- B6 +-- L7
  //                  |
  //                  +-- L8
  //                  |
  //                  +-- L2
  std::vector<Point> elements{{0, 0},  {7, 5}, {11, 11}, {10, 2},
                              {2, 10}, {4, 1}, {5, 3},   {8, 8},
                              {6, 9},  {1, 7}, {3, 4},   {9, 6}};

  PointRTree rtree(elements, point_bounds);

  EXPECT_THAT(
      rtree.BranchNodes(),
      ElementsAre(
          Branch(Rect::FromTwoPoints({0, 0}, {11, 11}), false, {1, 2}),
          Branch(Rect::FromTwoPoints({0, 0}, {10, 10}), false, {3, 5, 4}),
          Branch(Rect::FromTwoPoints({6, 8}, {11, 11}), false, {6}),
          Branch(Rect::FromTwoPoints({0, 0}, {5, 3}), true, {0, 5, 6}),
          Branch(Rect::FromTwoPoints({1, 4}, {3, 10}), true, {10, 9, 4}),
          Branch(Rect::FromTwoPoints({7, 2}, {10, 6}), true, {3, 1, 11}),
          Branch(Rect::FromTwoPoints({6, 8}, {11, 11}), true, {7, 8, 2})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateFromSixteenPoints) {
  // B0 +-- B1 +-- B3 +-- L9
  //    |      |      |
  //    |      |      +-- L13
  //    |      |      |
  //    |      |      +-- L11
  //    |      |
  //    |      +-- B6 +-- L6
  //    |      |      |
  //    |      |      +-- L0
  //    |      |      |
  //    |      |      +-- L10
  //    |      |
  //    |      +-- B4 +-- L4
  //    |             |
  //    |             +-- L7
  //    |             |
  //    |             +-- L12
  //    |
  //    +-- B2 +-- B7 +-- L8
  //           |      |
  //           |      +-- L5
  //           |      |
  //           |      +-- L15
  //           |
  //           +-- B5 +-- L14
  //           |      |
  //           |      +-- L2
  //           |      |
  //           |      +-- L3
  //           |
  //           +-- B8 +-- L1
  std::vector<Point> elements{
      {12, 5}, {14, 15}, {2, 12}, {6, 14}, {4, 4}, {15, 11}, {10, 3}, {7, 7},
      {9, 9},  {5, 0},   {13, 6}, {3, 2},  {1, 8}, {0, 1},   {8, 10}, {11, 13}};

  PointRTree rtree(elements, point_bounds);

  EXPECT_THAT(
      rtree.BranchNodes(),
      ElementsAre(
          Branch(Rect::FromTwoPoints({0, 0}, {15, 15}), false, {1, 2}),
          Branch(Rect::FromTwoPoints({0, 0}, {13, 8}), false, {3, 6, 4}),
          Branch(Rect::FromTwoPoints({2, 9}, {15, 15}), false, {7, 5, 8}),
          Branch(Rect::FromTwoPoints({0, 0}, {5, 2}), true, {9, 13, 11}),
          Branch(Rect::FromTwoPoints({1, 4}, {7, 8}), true, {4, 7, 12}),
          Branch(Rect::FromTwoPoints({2, 10}, {8, 14}), true, {14, 2, 3}),
          Branch(Rect::FromTwoPoints({10, 3}, {13, 6}), true, {6, 0, 10}),
          Branch(Rect::FromTwoPoints({9, 9}, {15, 13}), true, {8, 5, 15}),
          Branch(Rect::FromTwoPoints({14, 15}, {14, 15}), true, {1})));
  EXPECT_THAT(rtree.Elements(), ElementsAreArray(elements));
}

TEST(StaticRTreeTest, CreateWithNormalBranchingFactor) {
  // This test loosely validates that the structure is correct even with the
  // default branching factor of 16, by checking the number of branch nodes and
  // the depth of the tree. Both of these can be calculated by dividing the
  // number of elements by the branching factor, rounding up, and repeating this
  // process until you have only a single root node.

  // Computes the depth of the R-Tree. Note that this only needs to explore on
  // path, as R-Trees are balanced such that all leaf nodes are at the same
  // depth.
  auto get_depth = [](const StaticRTree<Point>& rtree) {
    absl::Span<const StaticRTree<Point>::BranchNode> branch_nodes =
        rtree.BranchNodes();
    // We start at the root, which is depth 1.
    uint32_t current_depth = 1;
    uint32_t current_idx = 0;
    while (!branch_nodes[current_idx].is_leaf_parent) {
      ABSL_CHECK_NE(branch_nodes[current_idx].child_indices.Size(), 0);
      ABSL_CHECK_LT(current_idx, branch_nodes[current_idx].child_indices[0]);
      current_idx = branch_nodes[current_idx].child_indices[0];
      ++current_depth;
    }
    // Add one for the last level, which contains the leaf nodes.
    return current_depth + 1;
  };

  {
    StaticRTree<Point> rtree(std::vector<Point>(10), point_bounds);

    EXPECT_EQ(get_depth(rtree), 2);
    EXPECT_EQ(rtree.BranchNodes().size(), 1);
  }
  {
    StaticRTree<Point> rtree(std::vector<Point>(100), point_bounds);

    EXPECT_EQ(get_depth(rtree), 3);
    EXPECT_EQ(rtree.BranchNodes().size(), 8);
  }
  {
    StaticRTree<Point> rtree(std::vector<Point>(1000), point_bounds);

    EXPECT_EQ(get_depth(rtree), 4);
    EXPECT_EQ(rtree.BranchNodes().size(), 68);
  }
  {
    StaticRTree<Point> rtree(std::vector<Point>(10000), point_bounds);

    EXPECT_EQ(get_depth(rtree), 5);
    EXPECT_EQ(rtree.BranchNodes().size(), 669);
  }
}

TEST(StaticRTree, VisitIntersectedElementsPointsWithRectQuery) {
  // There are 20 points, laid out on an integer grid like so:
  //
  //  9 . . . . . . . . . .
  //    . . . . . . X X . X
  //    . . . . . . . X . .
  //  6 . . X . X . . . X .
  //    X . . . . . . . . X
  //    . . . . . . X X X .
  //  3 . . . X . . . . . .
  //    . X . . . . . . X .
  //    . . . X . X . . . X
  //  0 . . . . . . X . . X
  //    0     3     6     9
  std::vector<Point> points{{1, 2}, {7, 8}, {0, 5}, {8, 4}, {2, 6},
                            {8, 6}, {9, 1}, {6, 4}, {9, 0}, {8, 2},
                            {7, 4}, {5, 1}, {7, 7}, {6, 8}, {3, 1},
                            {3, 3}, {4, 6}, {6, 0}, {9, 5}, {9, 8}};

  PointRTree rtree(points, point_bounds);

  // This helper function visits all the points that intersect `r`, and places
  // them in a `std::vector` for easier verification.
  auto get_intersected_points = [&rtree](const Rect& r) {
    std::vector<Point> intersected_points;
    rtree.VisitIntersectedElements(r, [&intersected_points](Point p) {
      intersected_points.push_back(p);
      return true;
    });
    return intersected_points;
  };

  // This query completely misses the R-Tree, it's off to the side.
  EXPECT_THAT(get_intersected_points(Rect::FromTwoPoints({20, 20}, {25, 25})),
              IsEmpty());
  // This query intersects the bounds of the R-Tree, but falls between the
  // actually elements without touching them.
  EXPECT_THAT(get_intersected_points(Rect::FromTwoPoints({2, 4}, {3, 5})),
              UnorderedElementsAre());
  // This query covers the entire R-Tree.
  EXPECT_THAT(get_intersected_points(Rect::FromTwoPoints({-10, -10}, {30, 30})),
              UnorderedElementsAreArray(points));
  // These two queries each partially cover the R-Tree.
  EXPECT_THAT(get_intersected_points(Rect::FromTwoPoints({2, 0}, {6, 5})),
              UnorderedElementsAre(Point{3, 1}, Point{3, 3}, Point{5, 1},
                                   Point{6, 0}, Point{6, 4}));
  EXPECT_THAT(
      get_intersected_points(Rect::FromTwoPoints({5.5, 3.5}, {8.5, 7.5})),
      UnorderedElementsAre(Point{8, 4}, Point{8, 6}, Point{6, 4}, Point{7, 4},
                           Point{7, 7}));
}

TEST(StaticRTree, VisitIntersectedElementsCirclesWithCircleQuery) {
  // We populate the R-Tree with 25 circles, each with a radius of 0.5, at
  // integer coordinates from (0, 0) to (4, 4). Because each circle has the same
  // radius, we don't explicitly encode it, and we just store the centers.
  std::vector<Point> centers(25);
  for (int i = 0; i < centers.size(); ++i) {
    centers[i] = {static_cast<float>(i % 5), static_cast<float>(i / 5)};
  }
  // The bounds of a radius 0.5 circle centered at `p` is a unit square centered
  // at `p`.
  auto bounds_func = [](const Point& p) {
    return Rect::FromCenterAndDimensions(p, 1, 1);
  };
  StaticRTree<Point> rtree(centers, bounds_func);
  // This convenience function returns the range of elements intersected by the
  // circle at `center` with `radius`; it's just here to reduce boilerplate.
  auto get_intersected_elements_for_circle = [&rtree](Point center,
                                                      float radius) {
    std::vector<Point> intersected_points;
    rtree.VisitIntersectedElements(
        Rect::FromCenterAndDimensions(center, 2 * radius, 2 * radius),
        [center, radius, &intersected_points](const Point& p) {
          if (Distance(p, center) <= radius + 0.5)
            intersected_points.push_back(p);
          return true;
        });
    return intersected_points;
  };

  // This circle completely misses the R-Tree. Note that we can't use
  // testing::IsEmpty(), because it depends on having a size() method,
  // iterator_range does not define for input iterators.
  EXPECT_THAT(get_intersected_elements_for_circle({10, 3}, 4),
              UnorderedElementsAre());
  // This circle misses the R-Tree, even though its bounding box intersects it.
  EXPECT_THAT(get_intersected_elements_for_circle({7, 7}, 3),
              UnorderedElementsAre());
  // This circle intersects every element of the R-Tree, even though doesn't
  // completely cover it.
  EXPECT_THAT(get_intersected_elements_for_circle({2, 3}, 3.2),
              UnorderedElementsAreArray(centers));
  // This circle intersects just some of the elements.
  EXPECT_THAT(get_intersected_elements_for_circle({4, 0}, 1.6),
              UnorderedElementsAre(Point{2, 0}, Point{3, 0}, Point{4, 0},
                                   Point{3, 1}, Point{4, 1}, Point{4, 2}));
  // This circle is the gap between elements, and does not intersect any, even
  // though its bounding box does.
  EXPECT_THAT(get_intersected_elements_for_circle({0.5, 1.5}, 0.2),
              UnorderedElementsAre());
}

TEST(StaticRTree, VisitIntersectedElementsStopEarly) {
  std::vector<Point> points{{0, 0}, {2, 0}, {1, 1}, {4, 1},
                            {3, 2}, {1, 3}, {2, 4}};
  PointRTree rtree(points, point_bounds);

  // This visitor will populate `visited` with the first three points it finds,
  // then stop traversing.
  std::vector<Point> visited;
  auto visitor = [&visited](Point p) {
    visited.push_back(p);
    return visited.size() < 3;
  };
  rtree.VisitIntersectedElements(Rect::FromTwoPoints({1, 1}, {4, 4}), visitor);

  // The visitation order is arbitrary, and the query rect contains five points,
  // so we don't know which ones are actually in `visited`; but we know that
  // there should be three, and that {0, 0}, and {2, 0} should not be present,
  // because they're not in the query rect.
  EXPECT_EQ(visited.size(), 3);
  EXPECT_THAT(visited, Not(Contains(Point{0, 0})));
  EXPECT_THAT(visited, Not(Contains(Point{2, 0})));
}

TEST(StaticRTreeDeathTest, CannotConstructWithNullBoundsFunction) {
  EXPECT_DEATH_IF_SUPPORTED(PointRTree({{0, 0}, {1, 1}}, nullptr),
                            "must be non-null");
  EXPECT_DEATH_IF_SUPPORTED(
      PointRTree(3, []() { return Point{0, 0}; }, nullptr), "must be non-null");
}

}  // namespace
}  // namespace ink::geometry_internal
