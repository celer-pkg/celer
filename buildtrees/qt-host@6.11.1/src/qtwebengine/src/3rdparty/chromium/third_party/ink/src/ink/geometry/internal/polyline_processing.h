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

#ifndef INK_GEOMETRY_INTERNAL_POLYLINE_PROCESSING_H_
#define INK_GEOMETRY_INTERNAL_POLYLINE_PROCESSING_H_

#include <stdbool.h>

#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/internal/static_rtree.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"

namespace ink::geometry_internal {

struct SegmentBundle {
  Segment segment;
  int index;
  float length;
};

struct Intersection {
  int index_int;
  float index_fraction;
  float walk_distance;
};

struct PolylineData {
  std::vector<SegmentBundle> segments;
  Intersection first_intersection;
  Intersection last_intersection;
  Point new_first_point;
  Point new_last_point;
  bool connect_first = false;
  bool connect_last = false;
  bool has_intersection = false;
  float max_straight_line_distance;
  float total_walk_distance = 0.0f;
  float min_walk_distance = 0.0f;
  float max_connection_distance;
  float min_connection_ratio;
  float min_trimming_ratio;
};

// Finds the first and last intersections in the polyline and updates the input
// PolylineData with the results.
void FindFirstAndLastIntersections(
    ink::geometry_internal::StaticRTree<SegmentBundle> rtree,
    PolylineData& polyline);

// Finds the best connections for the first and last points of the polyline and
// updates the input PolylineData with the results.
void FindBestEndpointConnections(
    ink::geometry_internal::StaticRTree<SegmentBundle> rtree,
    PolylineData& polyline);

PolylineData CreateNewPolylineData(absl::Span<const Point> points);

float WalkDistance(PolylineData& polyline, int index, float fractional_index,
                   bool walk_backwards);

bool EndpointIsConnectable(PolylineData& polyline, float index,
                           float fractional_index, float straight_line_distance,
                           bool walk_backwards);

// For a given polyline this algorithm aims to (1) identify and create any
// connections that the user may have intended to make but did not fully connect
// and (2) trim any extra end points that the user did not intend to be part of
// the selected area. To do this, the algorithm completes 3 main tasks:
//
// 1.  Find the first and last intersections in the polyline.
// 2.  Find the best connections for the first and last points of the polyline.
// 3.  Construct a new polyline based on the intersections and connections
//     found in step 1 and 2.
//     3.1. If there are no intersections found or connections to be made, then
//     the new polyline will be the same as the input polyline.
//     3.2. For any endpoint that is not connectable, any points past the
//     nearest intersection point will be trimmed off that end of the polyline.
//     3.3. For any endpoint that is connectable, it will not be trimmed and an
//     additional point will be added for the new connection.
//
// A key component to the algorithm is how it determines whether an endpoint is
// connectable to another point along the polyline.
// This determination is configurable with the following parameters:
//
// min_walk_distance: The minimum walking distance that a point must be from the
//                    endpoint to be considered valid for connection.
// max_connection_distance: The maximum straight line distance that can be
//                          between the endpoint and any valid connection point.
// min_connection_ratio: The “walking distance from endpoint to point” / “the
//                       straight-line distance from endpoint to point” must be
//                       greater than this value for any valid connection point.
//                       This value must be >1 to have any effect.
// min_trimming_ratio: The “walking distance from endpoint to the nearest
//                     intersection point” / “the straight-line distance from
//                     endpoint to point” must be greater than this value for
//                     any valid connection point.
std::vector<Point> ProcessPolylineForMeshCreation(
    absl::Span<const Point> points, float min_walk_distance,
    float max_connection_distance, float min_connection_ratio,
    float min_trimming_ratio);

// A version of ProcessPolylineForMeshCreation thats uses default parameters
// which have been tested to work well for most shapes. If there are fewer than
// 3 input points, or if there are fewer than 3 points remaining after removing
// points with the same (x,y) coordinates as the previous point, this function
// will return the remaining points.
std::vector<Point> CreateClosedShape(absl::Span<const Point> points);
}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_POLYLINE_PROCESSING_H_
