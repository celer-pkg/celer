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

#include "ink/geometry/internal/polyline_processing.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/distance.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/internal/static_rtree.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"

namespace ink::geometry_internal {

namespace {
const float kTotalWalkDistanceToMinWalkDistanceRatio = 0.3f;
const float kMaxStraightLineDistanceToMaxConnectionDistanceRatio = 0.1f;
const float kMinConnectionRatio = 2.0f;
const float kMinTrimmingRatio = 1.8f;
}  // namespace

float WalkDistance(PolylineData& polyline, int index, float fractional_index,
                   bool walk_backwards) {
  int start_idx;
  int end_idx;
  if (walk_backwards) {
    fractional_index = 1.0 - fractional_index;
    start_idx = index + 1;
    end_idx = polyline.segments.size();
  } else {
    start_idx = 0;
    end_idx = index;
  }

  float total_distance = polyline.segments[index].length * fractional_index;
  for (int i = start_idx; i < end_idx; ++i) {
    total_distance += polyline.segments[i].length;
  }
  return total_distance;
}

float IntermediateWalkDistance(PolylineData& polyline, int start_index,
                               int end_index) {
  float total_distance = 0.0f;
  for (int i = start_index + 1; i < end_index; ++i) {
    total_distance += polyline.segments[i].length;
  }
  return total_distance;
}

PolylineData CreateNewPolylineData(absl::Span<const Point> points) {
  PolylineData polyline;

  Point last_point = points[0];
  int segment_count = 0;
  polyline.segments.reserve(points.size() - 1);
  for (int i = 1; i < static_cast<int>(points.size()); ++i) {
    if (points[i] != last_point) {
      polyline.segments.push_back(
          SegmentBundle{Segment{last_point, points[i]}, segment_count,
                        Distance(last_point, points[i])});
      ++segment_count;
      last_point = points[i];
    }
  }

  return polyline;
}

// function for use in rtree traversals.
auto segment_bounds = [](SegmentBundle segment_data) {
  return Rect::FromTwoPoints(segment_data.segment.start,
                             segment_data.segment.end);
};

void FindFirstAndLastIntersections(
    ink::geometry_internal::StaticRTree<SegmentBundle> rtree,
    PolylineData& polyline) {
  SegmentBundle earliest_intersected_segment;
  std::pair<float, float> earliest_intersection_ratios =
      std::make_pair(std::numeric_limits<float>::infinity(), 0.0f);
  for (int i = 0; i < static_cast<int>(polyline.segments.size()) - 2; ++i) {
    // -2 because if we reach the last 2 segments without finding an
    // Intersection then none are present as the last two segments cant
    // intersect each other.
    SegmentBundle current_segment_bundle = polyline.segments[i];
    rtree.VisitIntersectedElements(
        Rect::FromTwoPoints(current_segment_bundle.segment.start,
                            current_segment_bundle.segment.end),
        [&earliest_intersected_segment, &earliest_intersection_ratios,
         &current_segment_bundle, &polyline](SegmentBundle other_segment) {
          // Only check segments that are after the next segment.
          if (other_segment.index > current_segment_bundle.index + 1) {
            if (std::optional<std::pair<float, float>> intersection_ratios =
                    SegmentIntersectionRatio(current_segment_bundle.segment,
                                             other_segment.segment);
                intersection_ratios.has_value()) {
              if (IntermediateWalkDistance(polyline,
                                           current_segment_bundle.index,
                                           other_segment.index) >
                  polyline.min_walk_distance / 2.0f) {
                if (intersection_ratios->first <
                    earliest_intersection_ratios.first) {
                  earliest_intersected_segment = other_segment;
                  earliest_intersection_ratios = *intersection_ratios;
                }
              }
            }
          }
          return true;
        });
    if (earliest_intersection_ratios.first <= 1.0f) {
      // We have found an Intersection at the current segment, so we can stop
      // looking and fill in polyline.first_intersection.
      polyline.first_intersection.index_int = i;
      polyline.first_intersection.index_fraction =
          earliest_intersection_ratios.first;
      polyline.first_intersection.walk_distance =
          WalkDistance(polyline, polyline.first_intersection.index_int,
                       polyline.first_intersection.index_fraction, false);
      polyline.new_first_point = current_segment_bundle.segment.Lerp(
          earliest_intersection_ratios.first);

      // This is also the last intersection we have found so far, so fill in
      // polyline.last_intersection.
      polyline.last_intersection.index_int = earliest_intersected_segment.index;
      polyline.last_intersection.index_fraction =
          earliest_intersection_ratios.second;
      polyline.last_intersection.walk_distance =
          WalkDistance(polyline, polyline.last_intersection.index_int,
                       polyline.last_intersection.index_fraction, true);
      polyline.new_last_point = polyline.new_first_point;
      polyline.has_intersection = true;
      break;
    }
  }

  // If we didn't find an Intersection, then we can return early since we don't
  // need to check for intersections again in the other direction.
  if (!polyline.has_intersection) return;

  // This is very similar to the loop above, with some modifications made as
  // we are now starting at the back and looking for a later Intersection.
  float largest_intersection_ratio = -std::numeric_limits<float>::infinity();
  for (int i = polyline.segments.size() - 1;
       i >= polyline.last_intersection.index_int; --i) {
    // Start from the back of the polyline and only check as long as the segment
    // you are checking has an index higher than the current last intersection
    // index -1.
    SegmentBundle current_segment_bundle = polyline.segments[i];
    rtree.VisitIntersectedElements(
        Rect::FromTwoPoints(current_segment_bundle.segment.start,
                            current_segment_bundle.segment.end),
        [&polyline, &largest_intersection_ratio,
         &current_segment_bundle](SegmentBundle other_segment) {
          // Only check segments that are before the preceding segment.
          // Only check segments with an index that is >= the first
          // intersection index, since we know there are no intersections before
          // the first intersection.
          if (other_segment.index < current_segment_bundle.index - 1 &&
              other_segment.index > polyline.first_intersection.index_int - 1) {
            if (std::optional<std::pair<float, float>> intersection_ratios =
                    SegmentIntersectionRatio(current_segment_bundle.segment,
                                             other_segment.segment);
                intersection_ratios.has_value()) {
              if (IntermediateWalkDistance(polyline, other_segment.index,
                                           current_segment_bundle.index) >
                  polyline.min_walk_distance / 2.0f) {
                if (intersection_ratios->first > largest_intersection_ratio) {
                  largest_intersection_ratio = intersection_ratios->first;
                }
              }
            }
          }
          return true;
        });
    if (largest_intersection_ratio >= 0.0f) {
      polyline.last_intersection.index_int = i;
      polyline.last_intersection.index_fraction = largest_intersection_ratio;
      polyline.new_last_point =
          current_segment_bundle.segment.Lerp(largest_intersection_ratio);
      polyline.last_intersection.walk_distance =
          WalkDistance(polyline, polyline.last_intersection.index_int,
                       polyline.last_intersection.index_fraction, true);
      break;
    }
  }
}

bool EndpointIsConnectable(PolylineData& polyline, float index,
                           float fractional_index, float straight_line_distance,
                           bool walk_backwards) {
  // this will fail when the straight line and walk distance to the point is
  // very similar. If the line isn't sufficiently curvy then we don't want to
  // connect.
  float walk_distance =
      WalkDistance(polyline, index, fractional_index, walk_backwards);
  if ((walk_distance < polyline.min_walk_distance) ||
      (walk_distance / straight_line_distance <
       polyline.min_connection_ratio)) {
    return false;
  };
  if (polyline.has_intersection) {
    float walk_distance_to_intersection =
        walk_backwards ? polyline.last_intersection.walk_distance
                       : polyline.first_intersection.walk_distance;
    if (walk_distance_to_intersection / straight_line_distance <
        polyline.min_trimming_ratio) {
      return false;
    }
  }
  return true;
}

// Helper function to simplify the logic in FindBestEndpointConnections.
// Returns true if the new intersection formed by the given connection is closer
// to the end of the polyline than the current last intersection.
inline bool BetterLastIntersection(PolylineData& polyline,
                                   Intersection best_first_point_connection) {
  if (!polyline.connect_first || polyline.connect_last) {
    // The last intersection cannot be updated if we are not connecting the
    // front because there isn't a new intersection being made to check against
    // the current last intersection.
    // The last intersection shouldn't be updated if we are connecting the last
    // point because the last intersection is only needed when trimming the end
    // of the polyline, and connecting the last point means the polyline will
    // not be trimmed at the end.
    return false;
  }
  if (!polyline.has_intersection) {
    // If no Intersection was previously found, we just take the new connection.
    return true;
  }
  // If an Intersection was previously found, we only update if the connection
  // is closer to the end of the polyline (either a later index than the last
  // intersection, or farther along on the same index).
  if (best_first_point_connection.index_int >
      polyline.last_intersection.index_int) {
    return true;
  }
  return best_first_point_connection.index_int ==
             polyline.last_intersection.index_int &&
         best_first_point_connection.index_fraction >
             polyline.last_intersection.index_fraction;
}

// Helper function to simplify the logic in FindBestEndpointConnections.
// Returns true if the new intersection formed by the given connection is closer
// to the beginning of the polyline than the current first intersection.
inline bool BetterFirstIntersection(PolylineData& polyline,
                                    Intersection best_last_point_connection) {
  if (polyline.connect_first || !polyline.connect_last) {
    // The first intersection cannot be updated if we are not connecting the
    // end because there isn't a new intersection being made to check against
    // the current first intersection.
    // The first intersection shouldn't be updated if we are connecting the
    // first point because the first intersection is only needed when trimming
    // the front of the polyline, and connecting the first point means the
    // polyline will not be trimmed at the front.
    return false;
  }
  if (!polyline.has_intersection) {
    // If no Intersection was previously found, we just take the new connection.
    return true;
  }
  // If an Intersection was previously found, we only update if the connection
  // is closer to the front of the polyline (either a lower index than the first
  // intersection, or earlier along on the same index).
  if (best_last_point_connection.index_int <
      polyline.first_intersection.index_int) {
    return true;
  }
  return best_last_point_connection.index_int ==
             polyline.first_intersection.index_int &&
         best_last_point_connection.index_fraction <
             polyline.first_intersection.index_fraction;
}

void FindBestEndpointConnections(
    ink::geometry_internal::StaticRTree<SegmentBundle> rtree,
    PolylineData& polyline) {
  Intersection best_first_point_connection;
  float best_first_point_connection_length =
      std::numeric_limits<float>::infinity();
  // find the first endpoint connections first
  Point first_point = polyline.segments.front().segment.start;
  rtree.VisitIntersectedElements(
      Rect::FromCenterAndDimensions(first_point,
                                    polyline.max_connection_distance * 2,
                                    polyline.max_connection_distance * 2),
      [&first_point, &polyline, &best_first_point_connection,
       &best_first_point_connection_length](SegmentBundle current_segment) {
        float distance = ink::Distance(first_point, current_segment.segment);
        if (distance < polyline.max_connection_distance &&
            distance < best_first_point_connection_length &&
            current_segment.index > 1) {
          std::optional<float> connection_projection =
              current_segment.segment.Project(first_point);
          ABSL_CHECK(connection_projection.has_value());

          float clamped_projection =
              std::clamp(*connection_projection, 0.0f, 1.0f);

          if (EndpointIsConnectable(polyline, current_segment.index,
                                    clamped_projection, distance, false)) {
            best_first_point_connection.index_int = current_segment.index;
            best_first_point_connection.index_fraction = clamped_projection;
            best_first_point_connection_length = distance;
          }
        }
        return true;
      });

  if (best_first_point_connection_length <= polyline.max_connection_distance &&
      !std::isinf(best_first_point_connection_length)) {
    polyline.connect_first = true;
    polyline.new_first_point =
        polyline.segments[best_first_point_connection.index_int].segment.Lerp(
            best_first_point_connection.index_fraction);
  }

  Intersection best_last_point_connection;
  float best_last_point_connection_length =
      std::numeric_limits<float>::infinity();
  // find the last endpoint connections first
  Point last_point = polyline.segments.back().segment.end;
  rtree.VisitIntersectedElements(
      Rect::FromCenterAndDimensions(last_point,
                                    polyline.max_connection_distance * 2,
                                    polyline.max_connection_distance * 2),
      [&last_point, &polyline, &best_last_point_connection,
       &best_last_point_connection_length](SegmentBundle current_segment) {
        float distance = ink::Distance(last_point, current_segment.segment);
        if (distance < polyline.max_connection_distance &&
            distance < best_last_point_connection_length &&
            current_segment.index <
                static_cast<int>(polyline.segments.size()) - 2) {
          std::optional<float> connection_projection =
              current_segment.segment.Project(last_point);
          ABSL_CHECK(connection_projection.has_value());
          float clamped_projection =
              std::clamp(*connection_projection, 0.0f, 1.0f);

          if (EndpointIsConnectable(polyline, current_segment.index,
                                    clamped_projection, distance, true)) {
            best_last_point_connection.index_int = current_segment.index;
            best_last_point_connection.index_fraction = clamped_projection;
            best_last_point_connection_length = distance;
          }
        }
        return true;
      });

  if (best_last_point_connection_length <= polyline.max_connection_distance &&
      !std::isinf(best_last_point_connection_length)) {
    polyline.connect_last = true;
    polyline.new_last_point =
        polyline.segments[best_last_point_connection.index_int].segment.Lerp(
            best_last_point_connection.index_fraction);
  }

  // We check if we need to update the first or last Intersection point based on
  // any newly found connections. If we connect both or neither endpoint we
  // don't need to trim anything so we don't need to update these values.
  if (BetterLastIntersection(polyline, best_first_point_connection)) {
    polyline.last_intersection.index_int =
        best_first_point_connection.index_int;
    polyline.last_intersection.index_fraction =
        best_first_point_connection.index_fraction;
    polyline.new_last_point = polyline.new_first_point;
  } else if (BetterFirstIntersection(polyline, best_last_point_connection)) {
    polyline.first_intersection.index_int =
        best_last_point_connection.index_int;
    polyline.first_intersection.index_fraction =
        best_last_point_connection.index_fraction;
    polyline.new_first_point = polyline.new_last_point;
  }
  if (polyline.connect_first || polyline.connect_last) {
    polyline.has_intersection = true;
  }
}

std::vector<Point> CreateNewPolylineFromPolylineData(PolylineData& polyline) {
  std::vector<Point> new_polyline;

  if (polyline.has_intersection) {
    int front_trim_index =
        polyline.connect_first ? 0 : polyline.first_intersection.index_int + 1;
    int back_trim_index = polyline.connect_last
                              ? polyline.segments.size()
                              : polyline.last_intersection.index_int + 1;
    new_polyline.reserve(back_trim_index - front_trim_index + 3);
    if (polyline.new_first_point !=
        polyline.segments[front_trim_index].segment.start) {
      new_polyline.push_back(polyline.new_first_point);
    }
    for (int i = front_trim_index; i < back_trim_index; ++i) {
      new_polyline.push_back(polyline.segments[i].segment.start);
    }
    if (polyline.connect_last) {
      new_polyline.push_back(polyline.segments.back().segment.end);
    }
    if (polyline.new_last_point != new_polyline.back()) {
      new_polyline.push_back(polyline.new_last_point);
    }
  } else {
    new_polyline.reserve(polyline.segments.size() + 1);
    new_polyline.push_back(polyline.segments.front().segment.start);
    for (size_t i = 0; i < polyline.segments.size(); ++i) {
      new_polyline.push_back(polyline.segments[i].segment.end);
    }
  }
  return new_polyline;
}

std::vector<Point> ProcessPolyline(PolylineData& polyline) {
  ink::geometry_internal::StaticRTree<SegmentBundle> rtree(polyline.segments,
                                                           segment_bounds);
  FindFirstAndLastIntersections(rtree, polyline);
  FindBestEndpointConnections(rtree, polyline);

  return CreateNewPolylineFromPolylineData(polyline);
}

std::vector<Point> ProcessPolylineForMeshCreation(
    absl::Span<const Point> points, float min_walk_distance,
    float max_connection_distance, float min_connection_ratio,
    float min_trimming_ratio) {
  PolylineData polyline = CreateNewPolylineData(points);

  polyline.min_walk_distance = min_walk_distance;
  polyline.max_connection_distance = max_connection_distance;
  polyline.min_connection_ratio = min_connection_ratio;
  polyline.min_trimming_ratio = min_trimming_ratio;

  return ProcessPolyline(polyline);
}

std::vector<Point> CreateClosedShape(absl::Span<const Point> points) {
  if (points.size() < 3) {
    return std::vector<Point>(points.begin(), points.end());
  }
  PolylineData polyline = CreateNewPolylineData(points);
  if (polyline.segments.size() < 2) {
    if (polyline.segments.size() == 1) {
      return {polyline.segments.front().segment.start,
              polyline.segments.front().segment.end};
    }
    return {points.front()};
  }
  // Calculate the total walk distance of the polyline.
  for (size_t i = 0; i < polyline.segments.size(); ++i) {
    polyline.total_walk_distance += polyline.segments[i].length;
  }

  // Calculate the max straight line distance of the polyline. Computing the
  // bounding box of the points and then taking the hypotenuse of that box
  // guarantees that no two points are more than that distance apart.
  Envelope envelope;
  envelope.Add(points);
  polyline.max_straight_line_distance =
      std::hypot(envelope.AsRect()->Width(), envelope.AsRect()->Height());

  polyline.min_walk_distance =
      polyline.total_walk_distance * kTotalWalkDistanceToMinWalkDistanceRatio;
  polyline.max_connection_distance =
      polyline.max_straight_line_distance *
      kMaxStraightLineDistanceToMaxConnectionDistanceRatio;
  polyline.min_connection_ratio = kMinConnectionRatio;
  polyline.min_trimming_ratio = kMinTrimmingRatio;

  return ProcessPolyline(polyline);
}

}  // namespace ink::geometry_internal
