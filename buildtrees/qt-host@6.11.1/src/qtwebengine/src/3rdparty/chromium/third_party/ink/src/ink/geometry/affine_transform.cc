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

#include "ink/geometry/affine_transform.h"

#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink {

AffineTransform AffineTransform::ScaleAboutPoint(float scale_factor,
                                                 Point center) {
  return AffineTransform(scale_factor, 0, center.x - (scale_factor * center.x),
                         0, scale_factor, center.y - (scale_factor * center.y));
}

AffineTransform AffineTransform::ScaleAboutPoint(float x_scale_factor,
                                                 float y_scale_factor,
                                                 Point center) {
  return AffineTransform(
      x_scale_factor, 0, center.x - (x_scale_factor * center.x), 0,
      y_scale_factor, center.y - (y_scale_factor * center.y));
}

AffineTransform AffineTransform::RotateAboutPoint(Angle angle, Point center) {
  float sin = Sin(angle);
  float cos = Cos(angle);
  return AffineTransform(cos, -sin, center.x - center.x * cos + center.y * sin,
                         sin, cos, center.y - center.x * sin - center.y * cos);
}

Point AffineTransform::Apply(Point p) const {
  return Point{A() * p.x + B() * p.y + C(), D() * p.x + E() * p.y + F()};
}

Segment AffineTransform::Apply(const Segment& s) const {
  return Segment{Apply(s.start), Apply(s.end)};
}

Triangle AffineTransform::Apply(const Triangle& t) const {
  return Triangle{Apply(t.p0), Apply(t.p1), Apply(t.p2)};
}

namespace {

Vec ApplyAffineTransformToVec(const AffineTransform& t, Vec v) {
  return Vec{t.A() * v.x + t.B() * v.y, t.D() * v.x + t.E() * v.y};
}

}  // namespace

Quad AffineTransform::Apply(const Quad& q) const {
  Point new_center = Apply(q.Center());
  std::pair<Vec, Vec> semi_axes = q.SemiAxes();
  Vec u = q.Width() == 0 ? Vec::FromDirectionAndMagnitude(q.Rotation(), 1)
                         : semi_axes.first;
  Vec v =
      q.Height() == 0 ? q.ShearFactor() * u + u.Orthogonal() : semi_axes.second;
  u = ApplyAffineTransformToVec(*this, u);
  v = ApplyAffineTransformToVec(*this, v);

  float u_magnitude = u.Magnitude();
  float u_dot_v = Vec::DotProduct(u, v);
  float u_cross_v = Vec::Determinant(u, v);

  float new_width = q.Width() == 0 ? 0 : 2 * u_magnitude;
  float new_height =
      q.Height() == 0 || u_cross_v == 0 ? 0 : 2 * u_cross_v / u_magnitude;

  Angle new_rotation = u.Direction();
  float new_shear = u_cross_v == 0 ? 0 : u_dot_v / u_cross_v;

  return Quad::FromCenterDimensionsRotationAndShear(
      new_center, new_width, new_height, new_rotation, new_shear);
}

Quad AffineTransform::Apply(const Rect& r) const {
  return Apply(Quad::FromRect(r));
}

std::optional<AffineTransform> AffineTransform::Find(const Segment& from,
                                                     const Segment& to) {
  float from_length = from.Length();
  float to_length = to.Length();
  if (from_length == 0) {
    if (to_length == 0) {
      return Translate(to.start - from.start);
    }
    return std::nullopt;
  }

  float scale = to_length / from_length;
  Angle rotation = Vec::SignedAngleBetween(from.Vector(), to.Vector());
  float scaled_sin = scale * Sin(rotation);
  float scaled_cos = scale * Cos(rotation);
  Vec v1 = -from.start.Offset();
  Vec v2 = to.start.Offset();

  // Equivalent to:
  //   Translate(p2) * Scale(scale) * Rotate(rotation) * Translate(p1)
  return AffineTransform(
      scaled_cos, -scaled_sin, scaled_cos * v1.x - scaled_sin * v1.y + v2.x,
      scaled_sin, scaled_cos, scaled_sin * v1.x + scaled_cos * v1.y + v2.y);
}

std::optional<AffineTransform> AffineTransform::Find(const Triangle& from,
                                                     const Triangle& to) {
  auto [a0, a1, a2] =
      std::tuple<Point, Point, Point>(from.p0, from.p1, from.p2);
  auto [b0, b1, b2] = std::tuple<Point, Point, Point>(to.p0, to.p1, to.p2);

  // This is the denominator (either positive or negative) of each term in the
  // AffineTransform. The denominator is 0 if and only if the starting triangle
  // has an area of 0.
  float d = (a1.x * a0.y) - (a2.x * a0.y) - (a0.x * a1.y) + (a2.x * a1.y) +
            (a0.x * a2.y) - (a1.x * a2.y);

  // We check that d is nonzero to ensure we will not divide by 0. We
  // additionally check that from.SignedArea is nonzero because, with large
  // enough points, its possible to get a very small but technically nonzero
  // value for d due to floating point math.
  if (d == 0 || from.SignedArea() == 0) return std::nullopt;

  float n0 = (b1.x * a0.y) - (b2.x * a0.y) - (b0.x * a1.y) + (b2.x * a1.y) +
             (b0.x * a2.y) - (b1.x * a2.y);
  float n1 = (b1.x * a0.x) - (b2.x * a0.x) - (b0.x * a1.x) + (b2.x * a1.x) +
             (b0.x * a2.x) - (b1.x * a2.x);
  float n2 = (b2.x * a1.x * a0.y) - (b1.x * a2.x * a0.y) -
             (b2.x * a0.x * a1.y) + (b0.x * a2.x * a1.y) +
             (b1.x * a0.x * a2.y) - (b0.x * a1.x * a2.y);
  float n3 = (b1.y * a0.y) - (b2.y * a0.y) - (b0.y * a1.y) + (b2.y * a1.y) +
             (b0.y * a2.y) - (b1.y * a2.y);
  float n4 = (b1.y * a0.x) - (b2.y * a0.x) - (b0.y * a1.x) + (b2.y * a1.x) +
             (b0.y * a2.x) - (b1.y * a2.x);
  float n5 = (b2.y * a1.x * a0.y) - (b1.y * a2.x * a0.y) -
             (b2.y * a0.x * a1.y) + (b0.y * a2.x * a1.y) +
             (b1.y * a0.x * a2.y) - (b0.y * a1.x * a2.y);

  return AffineTransform(n0 / d, n1 / -d, n2 / d, n3 / d, n4 / -d, n5 / d);
}

std::optional<AffineTransform> AffineTransform::Find(const Rect& from,
                                                     const Rect& to) {
  auto [a0, a1, a2, a3] = from.Corners();
  auto [b0, b1, b2, b3] = to.Corners();
  return AffineTransform::Find(Triangle{a0, a1, a2}, Triangle{b0, b1, b2});
}

std::optional<AffineTransform> AffineTransform::Find(const Quad& from,
                                                     const Quad& to) {
  auto [a0, a1, a2, a3] = from.Corners();
  auto [b0, b1, b2, b3] = to.Corners();
  return AffineTransform::Find(Triangle{a0, a1, a2}, Triangle{b0, b1, b2});
}

AffineTransform operator*(const AffineTransform& lhs,
                          const AffineTransform& rhs) {
  return AffineTransform(lhs.A() * rhs.A() + lhs.B() * rhs.D(),
                         lhs.A() * rhs.B() + lhs.B() * rhs.E(),
                         lhs.A() * rhs.C() + lhs.B() * rhs.F() + lhs.C(),
                         lhs.D() * rhs.A() + lhs.E() * rhs.D(),
                         lhs.D() * rhs.B() + lhs.E() * rhs.E(),
                         lhs.D() * rhs.C() + lhs.E() * rhs.F() + lhs.F());
}

std::string AffineTransform::ToFormattedString() const {
  return absl::StrCat("AffineTransform(", a_, ", ", b_, ", ", c_, ", ", d_,
                      ", ", e_, ", ", f_, ")");
}

}  // namespace ink
