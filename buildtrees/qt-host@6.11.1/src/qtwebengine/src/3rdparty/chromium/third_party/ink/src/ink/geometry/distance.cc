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

#include "ink/geometry/distance.h"

#include <algorithm>

#include "ink/geometry/intersects.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"

namespace ink {

float Distance(Point point, const Triangle& triangle) {
  // NOMUTANTS -- Reroute to a simpler function when the triangle is point-like.
  if (triangle.p0 == triangle.p1 && triangle.p1 == triangle.p2) {
    return Distance(triangle.p0, point);
  }

  // If the point and the triangle intersect, the distance between them is 0.
  if (Intersects(point, triangle)) {
    return 0.0f;
  }

  // If the point and triangle do not intersect then the minimum distance will
  // be the shortest distance from the point to one of the triangle's edges.
  return std::min({Distance(point, triangle.GetEdge(0)),
                   Distance(point, triangle.GetEdge(1)),
                   Distance(point, triangle.GetEdge(2))});
}

float Distance(Point point, const Rect& rect) {
  // NOMUTANTS -- Reroute to a simpler function when the rect is point-like.
  if (rect.Width() == 0 && rect.Height() == 0) {
    return Distance(rect.Center(), point);
  }

  // If the point and the rect intersect then the distance between them is 0.
  if (Intersects(point, rect)) {
    return 0.0f;
  }

  // If the point and rect do not intersect then the minimum distance will be
  // the shortest distance from the point to one of the rect's edges.
  return std::min(
      {Distance(point, rect.GetEdge(0)), Distance(point, rect.GetEdge(1)),
       Distance(point, rect.GetEdge(2)), Distance(point, rect.GetEdge(3))});
}

float Distance(Point point, const Quad& quad) {
  // NOMUTANTS -- Reroute to a simpler function when the quad is point-like.
  if (quad.Width() == 0 && quad.Height() == 0)
    return Distance(quad.Center(), point);

  // If the point and the quad intersect then the distance between them is 0.
  if (Intersects(point, quad)) {
    return 0.0f;
  }

  // If the point and quad do not intersect then the minimum distance will be
  // the shortest distance from the point to one of the quad's edges.
  return std::min(
      {Distance(point, quad.GetEdge(0)), Distance(point, quad.GetEdge(1)),
       Distance(point, quad.GetEdge(2)), Distance(point, quad.GetEdge(3))});
}

float Distance(const Segment& segment, const Triangle& triangle) {
  // If the Segment is point-like, defer to Point-to-Triangle distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (segment.start == segment.end) return Distance(segment.start, triangle);

  // If the Triangle is point-like, defer to Point-to-Segment distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (triangle.p0 == triangle.p1 && triangle.p1 == triangle.p2) {
    return Distance(triangle.p0, segment);
  }

  // If the Segment and Triangle intersect then the distance between them is 0.
  if (Intersects(segment, triangle)) {
    return 0.0f;
  }

  // If the Segment and Triangle do not intersect then the distance will be the
  // minimum distance from the Segment to any of the Triangle's edges.
  return std::min({Distance(segment, triangle.GetEdge(0)),
                   Distance(segment, triangle.GetEdge(1)),
                   Distance(segment, triangle.GetEdge(2))});
}

float Distance(const Segment& segment, const Rect& rect) {
  // If the Segment is point-like, defer to Point-to-Rect distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (segment.start == segment.end) return Distance(segment.start, rect);

  // If the Rect is point-like, defer to Point-to-Segment distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (rect.Width() == 0 && rect.Height() == 0) {
    return Distance(rect.Center(), segment);
  }

  // If the Segment and Rect intersect then the distance between them is 0.
  if (Intersects(segment, rect)) {
    return 0.0f;
  }

  // If the Segment and Rect do not intersect then the distance will be the
  // minimum distance from the Segment to any of the Rect's edges.
  return std::min(
      {Distance(segment, rect.GetEdge(0)), Distance(segment, rect.GetEdge(1)),
       Distance(segment, rect.GetEdge(2)), Distance(segment, rect.GetEdge(3))});
}

float Distance(const Segment& segment, const Quad& quad) {
  // If the Segment is point-like, defer to Point-to-Quad distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (segment.start == segment.end) return Distance(segment.start, quad);

  // If the Quad is point-like, defer to Point-to-Segment distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (quad.Width() == 0 && quad.Height() == 0) {
    return Distance(quad.Center(), segment);
  }

  // If the Segment and Quad intersect then the distance between them is 0.
  if (Intersects(segment, quad)) {
    return 0.0f;
  }

  // If the Segment and Quad do not intersect then the distance will be the
  // minimum distance from the Segment to any of the Quad's edges.
  return std::min(
      {Distance(segment, quad.GetEdge(0)), Distance(segment, quad.GetEdge(1)),
       Distance(segment, quad.GetEdge(2)), Distance(segment, quad.GetEdge(3))});
}

float Distance(const Triangle& a, const Triangle& b) {
  // If either Triangle is point-like, defer to Point-to-Triangle distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (a.p0 == a.p1 && a.p1 == a.p2) {
    return Distance(a.p0, b);
  }
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (b.p0 == b.p1 && b.p1 == b.p2) {
    return Distance(b.p0, a);
  }

  // If the Triangles intersect then the distance between them is 0.
  if (Intersects(a, b)) {
    return 0.0f;
  }

  // If the Triangles do not intersect then the distance will be the minimum
  // distance from one of the edges of one Triangle to the other Triangle.
  return std::min({Distance(a, b.GetEdge(0)), Distance(a, b.GetEdge(1)),
                   Distance(a, b.GetEdge(2))});
}

float Distance(const Triangle& triangle, const Rect& rect) {
  // If the Triangle is point-like, defer to Point-to-Rect distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (triangle.p0 == triangle.p1 && triangle.p1 == triangle.p2) {
    return Distance(triangle.p0, rect);
  }

  // If the Rect is point-like, defer to Point-to-Triangle distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (rect.Width() == 0 && rect.Height() == 0) {
    return Distance(rect.Center(), triangle);
  }

  // If the Triangle and Rect intersect then the distance between them is 0.
  if (Intersects(triangle, rect)) {
    return 0.0f;
  }

  // If the Triangle and Rect do not intersect then the distance will be the
  // minimum distance from one of the edges of the Triangle to the Rect.
  return std::min({Distance(triangle.GetEdge(0), rect),
                   Distance(triangle.GetEdge(1), rect),
                   Distance(triangle.GetEdge(2), rect)});
}

float Distance(const Triangle& triangle, const Quad& quad) {
  // If the Triangle is point-like, defer to Point-to-Quad distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (triangle.p0 == triangle.p1 && triangle.p1 == triangle.p2) {
    return Distance(triangle.p0, quad);
  }

  // If the Quad is point-like, defer to Point-to-Triangle distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (quad.Width() == 0 && quad.Height() == 0) {
    return Distance(quad.Center(), triangle);
  }

  // If the Triangle and Quad intersect then the distance between them is 0.
  if (Intersects(triangle, quad)) {
    return 0.0f;
  }

  // If the Triangle and Quad do not intersect then the distance will be the
  // minimum distance from one of the edges of the Triangle to the Quad.
  return std::min({Distance(triangle.GetEdge(0), quad),
                   Distance(triangle.GetEdge(1), quad),
                   Distance(triangle.GetEdge(2), quad)});
}

float Distance(const Rect& a, const Rect& b) {
  // If either Rect is point-like, defer to Point-to-Rect distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (a.Width() == 0 && a.Height() == 0) {
    return Distance(a.Center(), b);
  }
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (b.Width() == 0 && b.Height() == 0) {
    return Distance(b.Center(), a);
  }

  // If the Rects intersect then the distance between them is 0.
  if (Intersects(a, b)) {
    return 0.0f;
  }

  // If the Rects do not intersect then the distance will be the minimum
  // distance from one of the edges of one Rect to the other Rect.
  return std::min({Distance(a.GetEdge(0), b), Distance(a.GetEdge(1), b),
                   Distance(a.GetEdge(2), b), Distance(a.GetEdge(3), b)});
}

float Distance(const Rect& rect, const Quad& quad) {
  // If the Rect is point-like, defer to Point-to-Quad distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (rect.Width() == 0 && rect.Height() == 0) {
    return Distance(rect.Center(), quad);
  }

  // If the Quad is point-like, defer to Point-to-Rect distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (quad.Width() == 0 && quad.Height() == 0) {
    return Distance(quad.Center(), rect);
  }

  // If the Rect and Quad intersect then the distance between them is 0.
  if (Intersects(rect, quad)) {
    return 0.0f;
  }

  // If the Rect and Quad do not intersect then the distance will be the
  // minimum distance from one of the edges of the Rect to the Quad.
  return std::min(
      {Distance(rect.GetEdge(0), quad), Distance(rect.GetEdge(1), quad),
       Distance(rect.GetEdge(2), quad), Distance(rect.GetEdge(3), quad)});
}

float Distance(const Quad& a, const Quad& b) {
  // If either Quad is point-like, defer to Point-to-Quad distance.
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (a.Width() == 0 && a.Height() == 0) {
    return Distance(a.Center(), b);
  }
  // NOMUTANTS -- This produces the same result, but is slightly faster.
  if (b.Width() == 0 && b.Height() == 0) {
    return Distance(b.Center(), a);
  }

  // If the Quads intersect then the distance between them is 0.
  if (Intersects(a, b)) {
    return 0.0f;
  }

  // If the Quads do not intersect then the distance will be the minimum
  // distance from one of the edges of one Quad to the other Quad.
  return std::min({Distance(a.GetEdge(0), b), Distance(a.GetEdge(1), b),
                   Distance(a.GetEdge(2), b), Distance(a.GetEdge(3), b)});
}

}  // namespace ink
