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

#ifndef INK_GEOMETRY_INTERNAL_ALGORITHMS_H_
#define INK_GEOMETRY_INTERNAL_ALGORITHMS_H_

#include <array>
#include <optional>
#include <utility>

#include "absl/types/span.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {

// Returns the barycentric coordinates of the given `position` with respect to
// the given `triangle` or nullopt if the `triangle` is degenerate (i.e. it's
// three points are collinear).
//
// See https://en.wikipedia.org/wiki/Barycentric_coordinate_system.
std::optional<std::array<float, 3>> GetBarycentricCoordinates(
    const Triangle& triangle, Point position);

// Interpolates from `a` to `b` by separately interpolating or extrapolating
// the direction and magnitude based on the parameter `t`.
//
//   * When `a` and `b` have different magnitudes and directions, this produces
//     points on an Archimedean spiral containing `a`, `b`, and the origin.
//   * When `a` and `b` have the same direction, this is equivalent to a regular
//     linear interpolation or extrapolation. This is a degenerate spiral with
//     infinite radial separation between loops.
//   * When `a` and `b` have the same magnitude, this is equivalent to the 2D
//     case of spherical interpolation (https://en.wikipedia.org/wiki/Slerp).
//     This is a degenerate spiral with zero radial separation between loops.
Vec SpiralLerp(Vec a, Vec b, float t);

// Returns the `Envelope` of `mesh` positions.
//
// Vertex positions contribute to the envelope by just being in the `mesh` and
// do not need to be a part of any triangles.
//
// There is currently no equivalent public API as a member function of
// `MutableMesh`. This is done on purpose to deter inefficient repeated calls,
// because there is no efficient way to track envelope changes while performing
// arbitrary mutations.
Envelope CalculateEnvelope(const MutableMesh& mesh);

// Returns the delta from the given `point` to its projection on infinite line
// coinciding with the `segment`, if it can be determined.
//
// When `segment` is non-degenerate, this is equivalent to and somewhat more
// efficient than:
//     segment.Lerp(segment.Project(point)) - point
std::optional<Vec> VectorFromPointToSegmentProjection(Point point,
                                                      const Segment& segment);

// Linearly interpolates between `a` and `b`. Extrapolates when `t` is not in
// [0, 1].
//
// In the case where `a` == `b` the function will return `a` for any value of
// `t`.
//
// Note that the `Angle` overload simply interpolates the value of the `Angle`;
// it does not have any special case logic for congruent angles. I.e., for
// `Angle`s that differ by more than 2π, this will interpolate through one (or
// more) full rotations, and for `Angle`s that differ by less than 2π, this
// may interpolate the "long way" around the unit circle. If you require that
// behavior, you can achieve it by normalizing the `Angle`s w.r.t. a reference
// `Angle` (see also `Angle::Normalized` and `Angle::NormalizedAboutZero`).
float Lerp(float a, float b, float t);
Point Lerp(Point a, Point b, float t);
Color::RgbaFloat Lerp(const Color::RgbaFloat& a, const Color::RgbaFloat& b,
                      float t);
Angle Lerp(Angle a, Angle b, float t);
Vec Lerp(Vec a, Vec b, float t);

// Linearly interpolates between `a` and `b` in the shorter direction between
// the two angles and returns a value in range [0, 2pi).
Angle NormalizedAngleLerp(Angle a, Angle b, float t);

// Linearly rescales `t` relative to `a` and `b`, such that `a` maps to 0, and
// `b` maps to 1. If `value` is between `a` and `b`, the result will lie in the
// interval [0, 1].
//
// If `a` == `b` this function will return 0, for any `value`.
float InverseLerp(float a, float b, float value);

// Linearly maps an `input_value` from an `input_range` to an`output_range` such
// that `input_range.first` maps to `output_range.first` and
// `input_range.second` maps to `output_range.second`.
float LinearMap(float input_value, std::pair<float, float> input_range,
                std::pair<float, float> output_range);

// Returns the ratio along `a` (per `Segment::Lerp`) at which it intersects `b`,
// and along `b' (per `Segment::Lerp`) at which it intersects `a` or
// `std::nullopt` if they do not intersect. If `a` and `b` are overlapping
// (which only occurs if they are also parallel), this will return the ratio
// along each segment at which they first overlap (i.e. the smallest ratios).
// If a segment is degenerate (i.e. zero-length) and the segments intersect,
// this will return 0 for any degenerate segment.
std::optional<std::pair<float, float>> SegmentIntersectionRatio(
    const Segment& a, const Segment& b);

// Returns the lerp-ratios for the single point of intersection of two lines, if
// it exists and the ratios for that point are unique on both lines.
//
// The lines are defined in parametric form as:
//   * Line A = line_a_segment.start + t_a * line_a_segment.Vector()
//   * Line B = line_b_segment.start + t_b * line_b_segment.Vector()
//
// If the intersection exists, the returned value will be a pair of {t_a, t_b}
// for that point. This is similar to `SegmentIntersectionRatio()`, except:
//   * The returned ratios are not constrained to the range [0, 1].
//   * The function never returns a value if either segment is degenerate or if
//     the lines are parallel.
std::optional<std::pair<float, float>> UniqueLineIntersectionRatio(
    const Segment& line_a_segment, const Segment& line_b_segment);

// Return the first point along the first segment at which two segments
// intersect, if they do.
std::optional<Point> SegmentIntersection(const Segment& a, const Segment& b);

// Returns the `Segment` that results from "collapsing" a set of `Mesh`es by
// applying a non-invertible transform. The return value may be degenerate.
//
// `meshes` should contain the `Mesh`es to collapse, and `bounds` should be the
// minimum bounding rectangle for those `Mesh`es. `non_invertible_transform` is
// the transform to apply.
//
// `non_invertible_transform` is expected to be non-invertible, and `bounds` is
// expected to contain each element of `meshes`; these are `DCHECK`ed, and
// should be guaranteed by higher-level logic.
Segment CalculateCollapsedSegment(
    absl::Span<const Mesh> meshes, const Rect& bounds,
    const AffineTransform& non_invertible_transform);

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_ALGORITHMS_H_
