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

#include "ink/geometry/internal/intersects_internal.h"

#include <optional>

#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace geometry_internal {

bool IntersectsInternal(Point a, Point b) { return a == b; }
bool IntersectsInternal(Point a, const Segment& b) {
  // NOMUTANTS -- Directly compare the Points when the Segment is Point-like.
  if (b.start == b.end) return a == b.start;
  // If the point isn't on the line of the segment, they don't intersect.
  if (Vec::Determinant((a - b.start), (b.end - b.start)) != 0) return false;
  std::optional<float> projection = b.Project(a);
  // The segment is degenerate, and the point is not equivalent to either
  // endpoint.
  if (!projection.has_value()) return false;
  return *projection >= 0 && *projection <= 1;
}
bool IntersectsInternal(Point a, const Triangle& b) { return b.Contains(a); }
bool IntersectsInternal(Point a, const Rect& b) { return b.Contains(a); }
bool IntersectsInternal(Point a, const Quad& b) { return b.Contains(a); }

bool IntersectsInternal(const Segment& a, Point b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Segment& a, const Segment& b) {
  // NOMUTANTS -- Exit early when intersection is at endpoints.
  if (a.start == b.start || a.start == b.end || a.end == b.start ||
      a.end == b.end) {
    return true;
  }
  // NOMUTANTS -- Reroute to a simpler function when the Segment is Point-like.
  if (a.start == a.end) return IntersectsInternal(a.start, b);
  // NOMUTANTS -- Reroute to a simpler function when the Segment is Point-like.
  if (b.start == b.end) return IntersectsInternal(b.start, a);

  Vec vec_a = a.Vector();
  Vec vec_b = b.Vector();

  // If Parallel
  if (Vec::Determinant(vec_a, vec_b) == 0) {
    // If collinear
    if (Vec::Determinant(vec_a, b.start - a.start) == 0) {
      if (b.Length() == 0) return IntersectsInternal(a, b.start);
      // Project endpoint to check that at least one is in [0,1]. If the segment
      // is degenerate (too small to compute projection without underflowing
      // and the point equals neither endpoint), they don't interesect.
      std::optional<float> projection_1 = b.Project(a.start);
      if (!projection_1.has_value()) return false;
      std::optional<float> projection_2 = b.Project(a.end);
      if (!projection_2.has_value()) return false;
      // a is neither wholly before nor wholly after b. That is, one of the two
      // projections is greater than or equal to 0 and one is less than or equal
      // to 1, which can only occur if at least one projection is in the range
      // [0, 1] or the two projections span [0, 1].
      return (*projection_1 >= 0 || *projection_2 >= 0) &&
             (*projection_1 <= 1 || *projection_2 <= 1);
    }
    return false;
  }
  // Use cross products to verify that the endpoints of a Segment are either on
  // opposite sides of the other Segment, or that one of the endpoints lies on
  // the other Segment.
  float v1 = Vec::Determinant(vec_a, b.start - a.start);
  float v2 = Vec::Determinant(vec_a, b.end - a.start);
  float v3 = Vec::Determinant(vec_b, a.start - b.start);
  float v4 = Vec::Determinant(vec_b, a.end - b.start);
  return v1 * v2 <= 0 && v3 * v4 <= 0;
}
bool IntersectsInternal(const Segment& a, const Triangle& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Segment is Point-like.
  if (a.start == a.end) return IntersectsInternal(a.start, b);
  // NOMUTANTS -- Reroute to a simpler function when the Triangle is Point-like.
  if (b.p0 == b.p1 && b.p0 == b.p2) return IntersectsInternal(b.p0, a);
  // No special casing for Segment-like Triangles because this is faster for the
  // majority of expected inputs.

  // Check if the Triangle contains a Point from the Segment to verify that the
  // Segment is not fully contained.
  if (b.Contains(a.start)) return true;
  // Check if any lines of the Triangle intersect the Segment.
  for (int i = 0; i < 3; ++i) {
    if (IntersectsInternal(a, b.GetEdge(i))) return true;
  }
  // If none of the Triangle's lines intersect the Segment and the Triangle does
  // not fully contain the Segment then there is no intersection.
  return false;
}
bool IntersectsInternal(const Segment& a, const Rect& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Segment is Point-like.
  if (a.start == a.end) return IntersectsInternal(a.start, b);
  // NOMUTANTS -- Reroute to a simpler function when the rect is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like rects because this is faster for the
  // majority of expected inputs.

  // Check if the rect contains a Point from the Segment to verify that the
  // Segment is not fully contained.
  if (b.Contains(a.start)) return true;
  // Check if any lines of the rect intersect the Segment.
  for (int i = 0; i < 4; ++i) {
    if (IntersectsInternal(a, b.GetEdge(i))) return true;
  }
  // If none of the rect's lines intersect the Segment and the rect does not
  // fully contain the Segment then there is no intersection.
  return false;
}
bool IntersectsInternal(const Segment& a, const Quad& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Segment is Point-like.
  if (a.start == a.end) return IntersectsInternal(a.start, b);
  // NOMUTANTS -- Reroute to a simpler function when the Quad is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like Quads because this is faster for the
  // majority of expected inputs.

  // Check if the Quad contains a Point from the Segment to verify that the
  // Segment is not fully contained.
  if (b.Contains(a.start)) return true;
  // Check if any lines of the Quad intersect the Segment.
  for (int i = 0; i < 4; ++i) {
    if (IntersectsInternal(a, b.GetEdge(i))) return true;
  }
  // If none of the Quad's lines intersect the Segment and the Quad does not
  // fully contain the Segment then there is no intersection.
  return false;
}

bool IntersectsInternal(const Triangle& a, Point b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Triangle& a, const Segment& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Triangle& a, const Triangle& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Triangle is Point-like.
  if (a.p0 == a.p1 && a.p0 == a.p2) return IntersectsInternal(a.p0, b);
  // NOMUTANTS -- Reroute to a simpler function when the Triangle is Point-like.
  if (b.p0 == b.p1 && b.p0 == b.p2) return IntersectsInternal(b.p0, a);
  // No special casing for Segment-like Triangles because this is faster for the
  // majority of expected inputs.

  // Check if either Triangle contains a Point from the other to verify that
  // neither fully contains the other.
  if (b.Contains(a.p0) || a.Contains(b.p0)) return true;
  // Check if any lines of one Triangle intersect any lines of the other.
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (IntersectsInternal(a.GetEdge(i), b.GetEdge(j))) return true;
    }
  }
  // If none of the lines intersect and neither Triangle fully contains the
  // other then there is no intersection.
  return false;
}
bool IntersectsInternal(const Triangle& a, const Rect& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Triangle is Point-like.
  if (a.p0 == a.p1 && a.p0 == a.p2) return IntersectsInternal(a.p0, b);
  // NOMUTANTS -- Reroute to a simpler function when the rect is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like Triangles and rects because this is
  // faster for the majority of expected inputs.

  // Check if either shape contains a Point from the other to verify that
  // neither shape fully contains the other.
  if (b.Contains(a.p0) || a.Contains(b.Center())) return true;
  // Check if any lines of one shape intersect any lines of the other shape.
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (IntersectsInternal(a.GetEdge(i), b.GetEdge(j))) return true;
    }
  }
  // If none of the lines intersect and neither shape fully contains the other
  // then there is no intersection.
  return false;
}

bool IntersectsInternal(const Triangle& a, const Quad& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Triangle is Point-like.
  if (a.p0 == a.p1 && a.p0 == a.p2) return IntersectsInternal(a.p0, b);
  // NOMUTANTS -- Reroute to a simpler function when the Quad is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like Triangles and Quads because this is
  // faster for the majority of expected inputs.

  // Check if either shape contains a Point from the other to verify that
  // neither shape fully contains the other.
  if (b.Contains(a.p0) || a.Contains(b.Center())) return true;
  // Check if any lines of one shape intersect any lines of the other shape.
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (IntersectsInternal(a.GetEdge(i), b.GetEdge(j))) return true;
    }
  }
  // If none of the lines intersect and neither shape fully contains the other
  // then there is no intersection.
  return false;
}

bool IntersectsInternal(const Rect& a, Point b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Rect& a, const Segment& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Rect& a, const Triangle& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Rect& a, const Rect& b) {
  // No special casing for Point-like rects because this is just as fast.
  // No special casing for Segment-like rects because this is just as fast.
  // Check if either rect is completely to the left or right of the other.
  if (a.XMin() > b.XMax() || b.XMin() > a.XMax()) return false;
  // Check if either rect is completely above or below the other.
  if (a.YMin() > b.YMax() || b.YMin() > a.YMax()) return false;
  // If neither is true they must intersect.
  return true;
}
bool IntersectsInternal(const Rect& a, const Quad& b) {
  // NOMUTANTS -- Reroute to a simpler function when the rect is Point-like.
  if (a.Width() == 0 && a.Height() == 0)
    return IntersectsInternal(a.Center(), b);
  // NOMUTANTS -- Reroute to a simpler function when the Quad is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like rects and Quads because this is faster
  // for the majority of expected inputs.

  // Check if either shape contains a Point from the other to verify that
  // neither shape fully contains the other.
  if (b.Contains(a.Center()) || a.Contains(b.Center())) return true;
  // Check if any lines of one shape intersect any lines of the other shape.
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (IntersectsInternal(a.GetEdge(i), b.GetEdge(j))) return true;
    }
  }
  // If none of the lines intersect and neither shape fully contains the other
  // then there is no intersection.
  return false;
}

bool IntersectsInternal(const Quad& a, Point b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Quad& a, const Segment& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Quad& a, const Triangle& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Quad& a, const Rect& b) {
  return IntersectsInternal(b, a);
}
bool IntersectsInternal(const Quad& a, const Quad& b) {
  // NOMUTANTS -- Reroute to a simpler function when the Quad is Point-like.
  if (a.Width() == 0 && a.Height() == 0)
    return IntersectsInternal(a.Center(), b);
  // NOMUTANTS -- Reroute to a simpler function when the Quad is Point-like.
  if (b.Width() == 0 && b.Height() == 0)
    return IntersectsInternal(b.Center(), a);
  // No special casing for Segment-like Quads because this is faster for the
  // majority of expected inputs.

  // Check if either shape contains a Point from the other to verify that
  // neither shape fully contains the other.
  if (b.Contains(a.Center()) || a.Contains(b.Center())) return true;
  // Check if any lines of one shape intersect any lines of the other shape.
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (IntersectsInternal(a.GetEdge(i), b.GetEdge(j))) return true;
    }
  }
  // If none of the lines intersect and neither shape fully contains the other
  // then there is no intersection.
  return false;
}

}  // namespace geometry_internal
}  // namespace ink
