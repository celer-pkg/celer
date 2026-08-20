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

#ifndef INK_GEOMETRY_AFFINE_TRANSFORM_H_
#define INK_GEOMETRY_AFFINE_TRANSFORM_H_

#include <optional>
#include <string>

#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink {

// An affine transformation in the plane. The transformation can be
// thought of as a 3x3 matrix:
//   ⎡a  b  c⎤
//   ⎢d  e  f⎥
//   ⎣0  0  1⎦
// Applying the transformation can be thought of as a matrix
// multiplication, with the to-be-transformed point represented as a
// column vector with an extra 1:
//   ⎡a  b  c⎤   ⎡x⎤   ⎡a*x + b*y + c⎤
//   ⎢d  e  f⎥ * ⎢y⎥ = ⎢d*x + e*y + f⎥
//   ⎣0  0  1⎦   ⎣1⎦   ⎣      1      ⎦
// Transformations are composed via multiplication. Multiplication is
// not commutative (i.e. A*B != B*A), and the left-hand transformation
// is composed "after" the right hand transformation. E.g., if you have:
//   auto rotate = AffineTransform::Rotate(Angle::Degrees(45));
//   auto translate = AffineTransform::Translate(Vec{10, 0});
// then the `rotate * translate` first translates 10 units in the
// positive x-direction, then rotates 90° about the origin.
class AffineTransform {
 public:
  // Constructs an identity transformation, which maps a point to
  // itself, i.e. it leaves it unchanged.
  AffineTransform() = default;

  // Constructs a transformation with the given coefficients.
  // This is provided for completeness; however, you may find it easier
  // to create transformations by composing the transformations created
  // via the static factory methods.
  AffineTransform(float a, float b, float c, float d, float e, float f) {
    a_ = a;
    b_ = b;
    c_ = c;
    d_ = d;
    e_ = e;
    f_ = f;
  }

  // AffineTransforms are copyable and movable.
  AffineTransform(const AffineTransform&) = default;
  AffineTransform(AffineTransform&&) = default;
  AffineTransform& operator=(const AffineTransform&) = default;
  AffineTransform& operator=(AffineTransform&&) = default;

  // Returns the identity transformation. This is equivalent to the
  // default constructor, but is provided for readability.
  static AffineTransform Identity() { return AffineTransform(); }

  // Returns a transformation that translates by the given vector.
  static AffineTransform Translate(Vec offset) {
    return AffineTransform(1, 0, offset.x, 0, 1, offset.y);
  }

  // Returns a transformation that scales in both the x- and y-direction
  // by the given factor, centered about the origin.
  static AffineTransform Scale(float scale_factor) {
    return Scale(scale_factor, scale_factor);
  }

  // Returns a transformation that scales in both the x- and y-direction
  // by the given pair of factors, centered about the origin.
  static AffineTransform Scale(float x_scale_factor, float y_scale_factor) {
    return AffineTransform(x_scale_factor, 0, 0, 0, y_scale_factor, 0);
  }

  // Returns a transformation that scales in the x-direction by the
  // given factor, centered about the origin.
  static AffineTransform ScaleX(float scale_factor) {
    return Scale(scale_factor, 1);
  }

  // Returns a transformation that scales in the y-direction by the
  // given factor, centered about the origin.
  static AffineTransform ScaleY(float scale_factor) {
    return Scale(1, scale_factor);
  }

  // Returns a transformation that rotates by the given angle, centered
  // about the origin.
  static AffineTransform Rotate(Angle angle) {
    float sin = Sin(angle);
    float cos = Cos(angle);
    return AffineTransform(cos, -sin, 0, sin, cos, 0);
  }

  // Returns a transformation that shears in the x-direction by the
  // given factor.
  static AffineTransform ShearX(float shear_factor) {
    return AffineTransform(1, shear_factor, 0, 0, 1, 0);
  }

  // Returns a transformation that shears in the y-direction by the
  // given factor.
  static AffineTransform ShearY(float shear_factor) {
    return AffineTransform(1, 0, 0, shear_factor, 1, 0);
  }

  // Returns a transformation that scales in both the x- and y-directions by the
  // given factor, centered about the given point.
  //
  // For finite inputs, this is equivalent to (but faster than):
  //
  //   AffineTransform::Translate(center - kOrigin) *
  //       AffineTransform::Scale(scale_factor) *
  //       AffineTransform::Translate(kOrigin - center);
  static AffineTransform ScaleAboutPoint(float scale_factor, Point center);

  // Returns a transformation that scales in the x- and y-directions by the
  // given pair of factors, centered about the given point.
  //
  // For finite inputs, this is equivalent to (but faster than):
  //
  //   AffineTransform::Translate(center - kOrigin) *
  //       AffineTransform::ScaleX(x_scale_factor) *
  //       AffineTransform::ScaleY(y_scale_factor) *
  //       AffineTransform::Translate(kOrigin - center);
  static AffineTransform ScaleAboutPoint(float x_scale_factor,
                                         float y_scale_factor, Point center);

  // Returns a transformation that rotates by the given angle, centered
  // about the given point.
  // This is equivalent to:
  //   AffineTransform::Translate(center - kOrigin) *
  //       AffineTransform::Rotate(angle) *
  //       AffineTransform::Translate(kOrigin - center);
  // but this is faster.
  static AffineTransform RotateAboutPoint(Angle angle, Point center);

  // Returns the inverse of this transformation, if it exists, or
  // std::nullopt if it cannot be inverted.
  // Transformation T and its inverse T⁻¹ have the property that:
  //   T * T⁻¹ == T⁻¹ * T == AffineTransform::Identity()
  //
  // For a transformation of the form:
  //       ⎡a  b  c⎤
  //   T = ⎢d  e  f⎥
  //       ⎣0  0  1⎦
  // its inverse will be:
  //   ⎡ e / |T|     -b / |T|     (b*f - c*e) / |T|⎤
  //   ⎢-d / |T|      a / |T|     (c*d - a*f) / |T|⎥
  //   ⎣   0            0                 1        ⎦
  // where |T| = a*e - b*d is the determinant of the matrix.
  std::optional<AffineTransform> Inverse() const;

  // Returns a copy of the given object with the transformation applied.
  Point Apply(Point p) const;
  Segment Apply(const Segment& s) const;
  Triangle Apply(const Triangle& t) const;
  Quad Apply(const Quad& q) const;

  // Returns a copy of the Rect with the transformation applied. Note
  // that, since AffineTransform allows for rotation and shear, the
  // result is not necessarily an axis-aligned rectangle.
  // If you want an axis-aligned rect, you must get the envelope of the
  // returned Quad.
  Quad Apply(const Rect& r) const;

  // These convenience methods return an isotropic transformation that, if
  // applied to `from`, results in `to`. Returns std::nullopt if a transform
  // cannot be found.

  // A Transform cannot be found from a segment with zero length to a segment
  // with non-zero length.
  static std::optional<AffineTransform> Find(const Segment& from,
                                             const Segment& to);
  // A Transform cannot be found when the "from" triangle is degenerate, meaning
  // it has an area of zero.
  static std::optional<AffineTransform> Find(const Triangle& from,
                                             const Triangle& to);
  // A Transform cannot be found when the "from" rect is degenerate, meaning it
  // has an area of zero.
  static std::optional<AffineTransform> Find(const Rect& from, const Rect& to);
  // A Transform cannot be found when the "from" Quad is degenerate, meaning it
  // has an area of zero.
  static std::optional<AffineTransform> Find(const Quad& from, const Quad& to);

  // Accessors for the transformation coefficients in the form:
  //   ⎡a  b  c⎤
  //   ⎢d  e  f⎥
  //   ⎣0  0  1⎦
  float A() const { return a_; }
  float B() const { return b_; }
  float C() const { return c_; }
  float D() const { return d_; }
  float E() const { return e_; }
  float F() const { return f_; }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, AffineTransform transform) {
    sink.Append(transform.ToFormattedString());
  }

 private:
  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  float a_ = 1;
  float b_ = 0;
  float c_ = 0;
  float d_ = 0;
  float e_ = 1;
  float f_ = 0;
};

AffineTransform operator*(const AffineTransform& lhs,
                          const AffineTransform& rhs);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline std::optional<AffineTransform> AffineTransform::Inverse() const {
  // We calculate the determinant in two parts here in order to avoid a
  // fused-multiply-add, because loss of precision on certain Mac arm64 CPUs can
  // result in a non-zero determinant even when `a_` == `b_` and `d_` == `e_`.
  float determinant_lhs = a_ * e_;
  float determinant_rhs = b_ * d_;
  if (determinant_lhs == determinant_rhs) return std::nullopt;

  float determinant = determinant_lhs - determinant_rhs;
  return AffineTransform{
      e_ / determinant,  -b_ / determinant, (b_ * f_ - c_ * e_) / determinant,
      -d_ / determinant, a_ / determinant,  (c_ * d_ - a_ * f_) / determinant};
}

}  // namespace ink

#endif  // INK_GEOMETRY_AFFINE_TRANSFORM_H_
