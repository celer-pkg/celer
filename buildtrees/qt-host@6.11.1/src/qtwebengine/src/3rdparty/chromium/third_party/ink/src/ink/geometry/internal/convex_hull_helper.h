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

#ifndef INK_GEOMETRY_INTERNAL_CONVEX_HULL_HELPER_H_
#define INK_GEOMETRY_INTERNAL_CONVEX_HULL_HELPER_H_

#include <cstddef>
#include <cstdlib>
#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {

// Prunes interior points from the input using the Aki-Toussaint heuristic
// see:
// https://en.wikipedia.org/wiki/Convex_hull_algorithms#Akl%E2%80%93Toussaint_heuristic.
template <typename VertexTessellationHelper>
std::vector<typename VertexTessellationHelper::VertexType>
PruneUsingAkiToussaint(
    absl::Span<const typename VertexTessellationHelper::VertexType> points);

template <typename VertexTessellationHelper>
std::vector<typename VertexTessellationHelper::VertexType> ConvexHull(
    absl::Span<const typename VertexTessellationHelper::VertexType> points);

// ---------------------------------------------------------------------------
//                     Implementation details below
template <typename VertexTessellationHelper>
std::vector<typename VertexTessellationHelper::VertexType>
PruneUsingAkiToussaint(
    absl::Span<const typename VertexTessellationHelper::VertexType> points) {
  constexpr auto GetX = &VertexTessellationHelper::GetX;
  constexpr auto GetY = &VertexTessellationHelper::GetY;
  using VertexType = typename VertexTessellationHelper::VertexType;

  VertexType max_x_point = points.front();
  VertexType max_y_point = points.front();
  VertexType min_x_point = points.front();

  VertexType min_y_point = points.front();
  for (int i = 1; i < points.size(); ++i) {
    if (GetX(points[i]) > GetX(max_x_point)) {
      max_x_point = points[i];
    } else if (GetX(points[i]) < GetX(min_x_point)) {
      min_x_point = points[i];
    }

    if (GetY(points[i]) > GetY(max_y_point)) {
      max_y_point = points[i];
    } else if (GetY(points[i]) < GetY(min_y_point)) {
      min_y_point = points[i];
    }
  }

  // Points that are inside of the quadrilateral formed by the extrema will not
  // be in the convex hull, and can be eliminated.
  Triangle upper_triangle{Point{GetX(min_x_point), GetY(min_x_point)},
                          Point{GetX(max_x_point), GetY(max_x_point)},
                          Point{GetX(max_y_point), GetY(max_y_point)}};

  Triangle lower_triangle{Point{GetX(min_x_point), GetY(min_x_point)},
                          Point{GetX(min_y_point), GetY(min_y_point)},
                          Point{GetX(max_x_point), GetY(max_x_point)}};

  std::vector<VertexType> pruned_points;
  pruned_points.reserve(points.size());
  for (const auto& p : points) {
    Point test_point{GetX(p), GetY(p)};
    if (!upper_triangle.Contains(test_point) &&
        !lower_triangle.Contains(test_point))
      pruned_points.push_back(p);
  }

  return pruned_points;
}

template <typename VertexTessellationHelper>
std::vector<typename VertexTessellationHelper::VertexType> ConvexHull(
    absl::Span<const typename VertexTessellationHelper::VertexType> points) {
  constexpr auto GetX = &VertexTessellationHelper::GetX;
  constexpr auto GetY = &VertexTessellationHelper::GetY;
  using VertexType = typename VertexTessellationHelper::VertexType;

  if (points.size() < 2)
    return std::vector<VertexType>(points.begin(), points.end());

  auto pruned_points =
      points.size() > 500
          ? PruneUsingAkiToussaint<VertexTessellationHelper>(points)
          : std::vector<VertexType>(points.begin(), points.end());

  // Find the point with the lowest y-coordinate, selecting the lowest
  // x-coordinate in the case of ties.
  VertexType start_point = *std::min_element(
      points.begin(), points.end(),
      [](const VertexType& lhs, const VertexType& rhs) {
        return GetY(lhs) < GetY(rhs) ||
               (GetY(lhs) == GetY(rhs) && GetX(lhs) < GetX(rhs));
      });

  // Sort the remaining points by their angle from the start point, placing the
  // closest first in the case of ties.
  std::vector<VertexType> sorted_points;
  sorted_points.reserve(points.size());
  std::copy_if(pruned_points.begin(), pruned_points.end(),
               std::back_inserter(sorted_points),
               [start_point](const VertexType& v) { return v != start_point; });
  std::sort(sorted_points.begin(), sorted_points.end(),
            [start_point](const VertexType& lhs, const VertexType& rhs) {
              Vec lhs_vec{GetX(lhs) - GetX(start_point),
                          GetY(lhs) - GetY(start_point)};
              Vec rhs_vec{GetX(rhs) - GetX(start_point),
                          GetY(rhs) - GetY(start_point)};
              float det = Vec::Determinant(lhs_vec, rhs_vec);
              return det > 0 ||
                     (det == 0 && std::abs(lhs_vec.x) < std::abs(rhs_vec.x));
            });

  // Add the sorted points to the hull, removing any that form concavities.
  std::vector<VertexType> hull;
  constexpr size_t kHullSizeEstimate = 64;  // From real-world experimentation.
  hull.reserve(std::min(kHullSizeEstimate, sorted_points.size()));
  hull.push_back(start_point);
  for (const auto& point : sorted_points) {
    while (
        hull.size() >= 2 &&
        Vec::Determinant(Vec{GetX(hull.back()) - GetX(hull[hull.size() - 2]),
                             GetY(hull.back()) - GetY(hull[hull.size() - 2])},
                         Vec{GetX(point) - GetX(hull.back()),
                             GetY(point) - GetY(hull.back())}) <= 0)
      hull.pop_back();
    hull.push_back(point);
  }

  return hull;
}

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_CONVEX_HULL_HELPER_H_
