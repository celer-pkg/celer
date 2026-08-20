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

#include "ink/geometry/internal/algorithms.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/intersects_internal.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {

std::optional<std::array<float, 3>> GetBarycentricCoordinates(
    const Triangle& triangle, Point position) {
  Vec a = triangle.p2 - triangle.p0;
  Vec b = triangle.p1 - triangle.p2;
  float triangle_determinant = Vec::Determinant(a, b);
  if (triangle_determinant == 0) return std::nullopt;

  Vec c = position - triangle.p2;
  float b0 = Vec::Determinant(b, c) / triangle_determinant;
  float b1 = Vec::Determinant(a, c) / triangle_determinant;
  return std::array<float, 3>({b0, b1, 1 - b0 - b1});
}

Vec SpiralLerp(Vec a, Vec b, float t) {
  if (t == 0) return a;
  if (t == 1) return b;

  Angle direction = a.Direction() + t * Vec::SignedAngleBetween(a, b);
  // Note that magnitude can lerp to a negative value, which evaluates to the
  // desired reflection through the origin.
  float magnitude = (1 - t) * a.Magnitude() + t * b.Magnitude();
  return Vec::FromDirectionAndMagnitude(direction, magnitude);
}

Envelope CalculateEnvelope(const MutableMesh& mesh) {
  Envelope envelope;
  for (uint32_t i = 0; i < mesh.VertexCount(); ++i) {
    envelope.Add(mesh.VertexPosition(i));
  }
  return envelope;
}

std::optional<Vec> VectorFromPointToSegmentProjection(Point point,
                                                      const Segment& segment) {
  // The sought after value is orthogonal to `segment` and has length equal to
  // the shortest distance between `point` and the line on which the segment
  // lies.
  //
  // This distance is also equal to the height of the triangle made from
  // `segment` and `point` with the segment as its base. See
  // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points

  Vec base_vector = segment.Vector();
  float base_length_squared = base_vector.MagnitudeSquared();
  if (base_length_squared == 0) return std::nullopt;

  float negative_twice_triangle_signed_area =
      Vec::Determinant(base_vector, segment.start - point);

  // The triangle height is given by:
  //
  //     abs(negative_twice_triangle_signed_area) / sqrt(base_length_squared)
  //
  // and the unit vector from `point` toward the extended base line is given by:
  //
  //     sgn(negative_twice_triangle_signed_area) *
  //        base_vector.Orthogonal() / sqrt(base_length_squared);
  //
  // where sgn() is +/-1.
  return (negative_twice_triangle_signed_area / base_length_squared) *
         base_vector.Orthogonal();
}

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

Point Lerp(Point a, Point b, float t) {
  return Segment{.start = a, .end = b}.Lerp(t);
}

Color::RgbaFloat Lerp(const Color::RgbaFloat& a, const Color::RgbaFloat& b,
                      float t) {
  return {.r = a.r * (1 - t) + b.r * t,
          .g = a.g * (1 - t) + b.g * t,
          .b = a.b * (1 - t) + b.b * t,
          .a = a.a * (1 - t) + b.a * t};
}

Angle Lerp(Angle a, Angle b, float t) { return (1 - t) * a + t * b; }
Angle NormalizedAngleLerp(Angle a, Angle b, float t) {
  return (a + Lerp(Angle(), (b - a).NormalizedAboutZero(), t)).Normalized();
}

Vec Lerp(Vec a, Vec b, float t) {
  return Vec{Lerp(a.x, b.x, t), Lerp(a.y, b.y, t)};
}

float InverseLerp(float a, float b, float value) {
  // If the interval between `a` and `b` is 0, there is no way to get to `t`
  // because in the other direction the value of `t` won't impact the result.
  if (b - a == 0.f) {
    return 0.f;
  }
  return (value - a) / (b - a);
}

float LinearMap(float input_value, std::pair<float, float> input_range,
                std::pair<float, float> output_range) {
  return Lerp(output_range.first, output_range.second,
              InverseLerp(input_range.first, input_range.second, input_value));
}

std::optional<std::pair<float, float>> SegmentIntersectionRatio(
    const Segment& a, const Segment& b) {
  if (a == b) return std::pair{0.f, 0.f};
  Vec u = a.Vector();
  Vec v = b.Vector();
  Vec w = b.start - a.start;

  float det = Vec::Determinant(u, v);
  if (det == 0) {
    // The segments are parallel.

    if (Vec::Determinant(u, w) != 0) {
      // The segments are parallel, but not on the same line, so they don't
      // intersect.
      return std::nullopt;
    }

    std::optional<float> a_project_b_start = a.Project(b.start);
    std::optional<float> b_project_a_start = b.Project(a.start);

    if (!a_project_b_start.has_value() && b_project_a_start.has_value() &&
        IntersectsInternal(a.start, b)) {
      // Just A is point-like, and intersects B.
      return std::pair{0.0f, *b_project_a_start};
    }
    if (a_project_b_start.has_value() && !b_project_a_start.has_value() &&
        IntersectsInternal(b.start, a)) {
      // Just B is point-like and intersects A.
      return std::pair{*a_project_b_start, 0.0f};
    }
    if (!a_project_b_start.has_value() || !b_project_a_start.has_value()) {
      // At least one of A and B is point-like, but they don't intersect.
      return std::nullopt;
    }

    std::optional<float> a_project_b_end = a.Project(b.end);
    std::optional<float> b_project_a_end = b.Project(a.end);
    // If projection was computable for one point, it should be for all.
    ABSL_CHECK(a_project_b_end.has_value() && b_project_a_end.has_value());

    if ((*a_project_b_start < 0 && *a_project_b_end < 0) ||
        (*a_project_b_start > 1 && *a_project_b_end > 1)) {
      // They are on the same line, but don't overlap.
      return std::nullopt;
    }

    return std::pair{
        std::max(std::min(*a_project_b_start, *a_project_b_end), 0.0f),
        std::max(std::min(*b_project_a_start, *b_project_a_end), 0.0f)};
  }

  // We can find the ratio of the intersection by solving:
  //   t * u = w + s * v
  // where `t` and `s` are scalars representing the ratio along the segments.
  // Vector determinants are distributive, i.e. for vectors a, b and scalar k:
  //   (k * a) ⨯ b = a ⨯ (k * b) = k * (a ⨯ b)
  // Additionally, the determinant of a vector and itself is always zero. A
  // little algebra magic then gives us:
  //   (t * u) ⨯ v = (w + s * v) ⨯ v
  //   t * (u ⨯ v) = w ⨯ v + s * (v ⨯ v)
  //   t = (w ⨯ v) / (u ⨯ v)
  // A similar series of steps gets us s = (w ⨯ u) / (u ⨯ v).
  //
  // Note that, if either segment were degenerate, the determinant would be
  // zero, so we know that both segments have a non-zero length.
  float t = Vec::Determinant(w, v) / det;
  float s = Vec::Determinant(w, u) / det;
  if (0 <= t && t <= 1 && 0 <= s && s <= 1) return std::pair{t, s};
  return std::nullopt;
}

std::optional<std::pair<float, float>> UniqueLineIntersectionRatio(
    const Segment& line_a_segment, const Segment& line_b_segment) {
  Vec u = line_a_segment.Vector();
  Vec v = line_b_segment.Vector();
  float det = Vec::Determinant(u, v);
  if (det == 0) return std::nullopt;

  // See comment inside `SegmentIntersectionRatio()` above for a detailed
  // description of the following arithmetic.
  Vec w = line_b_segment.start - line_a_segment.start;
  return std::make_pair(Vec::Determinant(w, v) / det,
                        Vec::Determinant(w, u) / det);
}

std::optional<Point> SegmentIntersection(const Segment& a, const Segment& b) {
  if (std::optional<std::pair<float, float>> intersection_ratio =
          SegmentIntersectionRatio(a, b);
      intersection_ratio.has_value()) {
    return a.Lerp(intersection_ratio->first);
  }
  return std::nullopt;
}

Segment CalculateCollapsedSegment(
    absl::Span<const Mesh> meshes, const Rect& bounds,
    const AffineTransform& non_invertible_transform) {
  ABSL_DCHECK(!non_invertible_transform.Inverse().has_value());

  // We first transform the diagonal of `query`'s bounds to find the line that
  // `query` will lie on after transforming it.
  std::array<Point, 4> corners = bounds.Corners();
  Segment transformed_diagonal =
      non_invertible_transform.Apply(Segment{corners[0], corners[2]});

  if (transformed_diagonal.Vector().MagnitudeSquared() == 0) {
    // The transform collapses the shape to a single point.
    return transformed_diagonal;
  }

  // Now we iterate all of the triangles in the mesh, and transform and
  // project their vertices onto that line to find the actual bounds of the
  // segment that `query` collaspes to. Note that we don't just iterate the
  // vertices, as there may be vertices that are not referenced by any
  // triangle and thus don't contribute to the mesh.
  // TODO: b/308794418 - Compute and cache the convex hull to speed up
  // repeated queries.
  float min_value = std::numeric_limits<float>::infinity();
  float max_value = -std::numeric_limits<float>::infinity();
  for (const Mesh& mesh : meshes) {
    ABSL_DCHECK(mesh.Bounds().IsEmpty() ||
                bounds.Contains(*mesh.Bounds().AsRect()));
    for (uint32_t t_idx = 0; t_idx < mesh.TriangleCount(); ++t_idx) {
      for (uint32_t v_idx : mesh.TriangleIndices(t_idx)) {
        Point transformed_vertex =
            non_invertible_transform.Apply(mesh.VertexPosition(v_idx));
        std::optional<float> projection =
            transformed_diagonal.Project(transformed_vertex);
        if (projection.has_value()) {
          min_value = std::min(min_value, *projection);
          max_value = std::max(max_value, *projection);
        }
      }
    }
  }
  return {transformed_diagonal.Lerp(min_value),
          transformed_diagonal.Lerp(max_value)};
}

}  // namespace ink::geometry_internal
