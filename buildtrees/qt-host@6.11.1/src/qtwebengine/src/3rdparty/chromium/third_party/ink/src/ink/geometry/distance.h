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

#ifndef INK_GEOMETRY_DISTANCE_H_
#define INK_GEOMETRY_DISTANCE_H_

#include <algorithm>
#include <optional>

#include "ink/geometry/intersects.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {

// These functions return the minimum distance between the given pair of
// objects. For objects that represent a shape this will be the minimum distance
// to the interior of that shape, meaning distance will be zero if that shape
// contains the other object. The return value will always be greater than or
// equal to zero, and Distance(a, b) == Distance(b, a).

float Distance(Point a, Point b);
float Distance(Point point, const Segment& segment);
float Distance(Point point, const Triangle& triangle);
float Distance(Point point, const Rect& rect);
float Distance(Point point, const Quad& quad);

float Distance(const Segment& segment, Point point);
float Distance(const Segment& a, const Segment& b);
float Distance(const Segment& segment, const Triangle& triangle);
float Distance(const Segment& segment, const Rect& rect);
float Distance(const Segment& segment, const Quad& quad);

float Distance(const Triangle& triangle, Point point);
float Distance(const Triangle& triangle, const Segment& segment);
float Distance(const Triangle& a, const Triangle& b);
float Distance(const Triangle& triangle, const Rect& rect);
float Distance(const Triangle& triangle, const Quad& quad);

float Distance(const Rect& rect, Point point);
float Distance(const Rect& rect, const Segment& segment);
float Distance(const Rect& rect, const Triangle& triangle);
float Distance(const Rect& a, const Rect& b);
float Distance(const Rect& rect, const Quad& quad);

float Distance(const Quad& quad, Point point);
float Distance(const Quad& quad, const Segment& segment);
float Distance(const Quad& quad, const Triangle& triangle);
float Distance(const Quad& quad, const Rect& rect);
float Distance(const Quad& a, const Quad& b);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline float Distance(Point a, Point b) { return (a - b).Magnitude(); }
inline float Distance(Point point, const Segment& segment) {
  // Reroute to a simpler function when the segment is point-like.
  if (segment.start == segment.end) {
    return Distance(segment.start, point);
  }

  std::optional<float> point_projection = segment.Project(point);
  if (!point_projection.has_value()) {
    // If we can't determine the projection (which can happen for very small
    // segments where the endpoints are not equal but computation of magnitude
    // squared underflows), just take the min distance from the endpoints.
    return std::min(Distance(segment.start, point),
                    Distance(segment.end, point));
  }
  if (*point_projection >= 1.0f) {
    return Distance(point, segment.end);
  }
  if (*point_projection <= 0.0f) {
    return Distance(point, segment.start);
  }

  // Closest point on the segment to the point: segment.Lerp(point_projection)
  return Distance(point, segment.Lerp(*point_projection));
}

inline float Distance(const Segment& a, const Segment& b) {
  // Reroute to a simpler function when the segment is point-like.
  if (a.start == a.end) return Distance(a.start, b);
  // Reroute to a simpler function when the segment is point-like.
  if (b.start == b.end) return Distance(b.start, a);

  // If the segments intersect then the distance between them is 0.
  if (Intersects(a, b)) {
    return 0.0f;
  }

  // If the segments do not intersect then the minimum distance will be the
  // shortest distance from one of the endpoints to the other segment.
  return std::min({Distance(a.start, b), Distance(a.end, b),
                   Distance(b.start, a), Distance(b.end, a)});
}

// Convenience overloads for order-independent function calls.
inline float Distance(const Segment& segment, Point point) {
  return Distance(point, segment);
}
inline float Distance(const Triangle& triangle, Point point) {
  return Distance(point, triangle);
}
inline float Distance(const Triangle& triangle, const Segment& segment) {
  return Distance(segment, triangle);
}
inline float Distance(const Rect& rect, Point point) {
  return Distance(point, rect);
}
inline float Distance(const Rect& rect, const Segment& segment) {
  return Distance(segment, rect);
}
inline float Distance(const Rect& rect, const Triangle& triangle) {
  return Distance(triangle, rect);
}
inline float Distance(const Quad& quad, Point point) {
  return Distance(point, quad);
}
inline float Distance(const Quad& quad, const Segment& segment) {
  return Distance(segment, quad);
}
inline float Distance(const Quad& quad, const Triangle& triangle) {
  return Distance(triangle, quad);
}
inline float Distance(const Quad& quad, const Rect& rect) {
  return Distance(rect, quad);
}

}  // namespace ink

#endif  // INK_GEOMETRY_DISTANCE_H_
