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

#ifndef INK_GEOMETRY_QUAD_H_
#define INK_GEOMETRY_QUAD_H_

#include <array>
#include <optional>
#include <utility>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {

// This class represents a quadrilateral with parallel sides (i.e. a
// parallelogram), defined by its center, width, height, rotation, and shear
// factor.
//
// A Quad's parameters are used to define a pair of vector semi-axes:
//   u = {.5 * w * cos(θ), .5 * w * sin(θ)}
//   v = {.5 * h * (s * cos(θ) - sin(θ)),
//        .5 * h * (s * sin(θ) + cos(θ))}
// where `w` is the width, `h` is the height, `s` is the shear factor and `θ` is
// the angle of rotation. From the semi-axes, we define the shape of the Quad as
// the set of all points c + 𝛼 * u + 𝛽 * v, where `c` is the center, and `𝛼` and
// `𝛽` are real numbers in the interval [-1, 1].
//
// A Quad may have a positive or negative height; a positive height indicates
// that the angle from the first semi-axis to the second will also be positive.
//
// A Quad may have a positive or negative shear factor; a positive shear factor
// indicates a smaller absolute angle between the semi-axes (the shear factor
// is, in fact, the cotangent of that angle). A Quad may *not* have a negative
// width. If an operation on a Quad would result in a negative width, it is
// instead normalized, by negating both the width and the height, adding π to
// the angle of rotation, and normalizing rotation to the range [0, 2π).
//
// A Quad may also be degenerate; that is, its width or height, or both, may be
// zero. Degenerate `Quad`s may still have a non-zero rotation and/or shear
// factor. A `Quad` that has both width and height of zero is effectively a
// point, and so rotation and shear factor do not affect the values of the axes
// or corners. A `Quad` that has either width or height of zero (but not both)
// is effectively a line segment, and so is similarly unaffected by shear
// factor.
//
// More intuitively, you can think of the shape of the Quad, before taking the
// center and rotation into account, like this:
//        s*h
//      |------|__________
//     ⎡       /         /
//     ⎢      /         /
//     ⎢     /         /
//   h ⎢    /         /
//     ⎢   /         /
//     ⎢  /         /
//     ⎣ /_________/
//       |---------|
//            w
// Where `w` is the width, `h` is the height, and `s` is the shear factor. You
// then rotate, and translate such that the center is in the correct position.
//
// Note that rectangles and axis-aligned boundings boxes may be represented as
// special cases of Quads; a rectangle is a Quad with a shear factor of zero,
// and an axis-aligned bounding box is a Quad with both a shear factor and a
// rotation of zero.
class Quad {
 public:
  // Constructs a Quad centered on the origin, with zero width, height,
  // rotation, and shear factor.
  Quad();

  // Constructs a Quad with the specified parameters, setting unspecified
  // parameters to zero. If the given width is less than zero, the Quad will be
  // normalized. Rotation is normalized to the range [0, 2π).
  static Quad FromCenterAndDimensions(Point center, float width, float height) {
    return Quad(center, width, height, Angle::Radians(0), 0);
  }
  static Quad FromCenterDimensionsAndRotation(Point center, float width,
                                              float height, Angle rotation) {
    return Quad(center, width, height, rotation, 0);
  }
  static Quad FromCenterDimensionsRotationAndShear(Point center, float width,
                                                   float height, Angle rotation,
                                                   float shear_factor) {
    return Quad(center, width, height, rotation, shear_factor);
  }

  // Constructs a Quad that is equivalent to the given Rect.
  static Quad FromRect(const Rect& r) {
    return Quad::FromCenterAndDimensions(r.Center(), r.Width(), r.Height());
  }

  Quad(const Quad&) = default;
  Quad(Quad&&) = default;
  Quad& operator=(const Quad&) = default;
  Quad& operator=(Quad&&) = default;

  // Accessors for the center of the Quad.
  Point Center() const;
  void SetCenter(Point center);

  // Accessors for the width of the base of the Quad.
  // If the width is set to a value less than zero, the Quad will be normalized.
  float Width() const;
  void SetWidth(float width);

  // Accessors for the height of the Quad.
  float Height() const;
  void SetHeight(float height);

  // Accessors for the rotation of the Quad.
  // Rotation is normalized to the range [0, 2π).
  Angle Rotation() const;
  void SetRotation(Angle rotation);

  // Accessors for the shear factor of the Quad.
  float ShearFactor() const;
  void SetShearFactor(float shear_factor);

  // Returns the semi-axes of the Quad.
  std::pair<Vec, Vec> SemiAxes() const;

  // Returns true if the Quad is a rectangle, i.e. if its corners form right
  // angles. This is equivalent to:
  //   p.ShearFactor() == 0;
  // Note that only the shear factor determines the return value, even for
  // degenerate Quads.
  bool IsRectangular() const { return shear_factor_ == 0; }

  // Returns true if the Quad is an axis-aligned rectangle, i.e. if its corners
  // form right angles and its sides are parallel to the x- and y-axes. Due to
  // floating point precision loss, this takes a tolerance value for checking
  // the rotation of Quad. Note that only the shear factor and rotation angle
  // determine the return value, even for degenerate Quads.
  bool IsAxisAligned(Angle tolerance = Angle::Radians(1e-5)) const {
    return IsRectangular() &&
           (Abs(Mod(rotation_, kQuarterTurn)) <= tolerance ||
            Abs(Mod(rotation_, kQuarterTurn) - kQuarterTurn) <= tolerance);
  }

  // Returns the signed area of the Quad. The area will be negative if and only
  // if the height is negative and the width is non-zero.
  float SignedArea() const { return width_ * height_; }

  // Returns the aspect ratio of the Quad, i.e. the width divided by the height.
  // This CHECK-fails if the height is zero.
  float AspectRatio() const {
    ABSL_CHECK(Height() != 0)
        << "Cannot determine the aspect ratio when the height is 0";
    return Width() / Height();
  }

  // Returns the corners of the Quad. The order of the corners is:
  //   C - u - v, C + u - v, C + u + v, C - u + v
  // where C is the center, u is the first semi-axis, and v is the second
  // semi-axis.
  std::array<Point, 4> Corners() const;

  // Returns the Segment of the Quad between the corner at `index` and the Point
  // at `index` + 1 modulo 4. This CHECK-fails if `index` is not 0, 1, 2, or 3.
  Segment GetEdge(int index) const;

  // Expands the Quad such that it contains the given Point, without altering
  // the rotation or shear factor.
  void Join(Point point);

  // Returns whether the given Point is contained within the Quad.
  bool Contains(Point point) const;

 private:
  Quad(Point center, float width, float height, Angle rotation,
       float shear_factor) {
    center_ = center;
    shear_factor_ = shear_factor;
    width_ = width;
    height_ = height;
    rotation_ = rotation.Normalized();
    Normalize();
  }

  void Normalize() {
    if (width_ < 0) {
      width_ = -width_;
      height_ = -height_;
      rotation_ = (rotation_ + kHalfTurn).Normalized();
    }
  }

  Point center_ = Point{0, 0};
  float width_ = 0;
  float height_ = 0;
  Angle rotation_ = Angle::Radians(0);
  float shear_factor_ = 0;
  mutable std::optional<std::pair<Vec, Vec>> semi_axes_;
};

bool operator==(const Quad& lhs, const Quad& rhs);
bool operator!=(const Quad& lhs, const Quad& rhs);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline Point Quad::Center() const { return center_; }
inline float Quad::Width() const { return width_; }
inline float Quad::Height() const { return height_; }
inline Angle Quad::Rotation() const { return rotation_; }
inline float Quad::ShearFactor() const { return shear_factor_; }

inline void Quad::SetCenter(Point center) { center_ = center; }
inline void Quad::SetHeight(float height) {
  if (height == height_) {
    return;
  }
  semi_axes_.reset();
  height_ = height;
}
inline void Quad::SetRotation(Angle rotation) {
  if (rotation == rotation_) {
    return;
  }
  semi_axes_.reset();
  rotation_ = rotation.Normalized();
}
inline void Quad::SetShearFactor(float shear_factor) {
  if (shear_factor == shear_factor_) {
    return;
  }
  semi_axes_.reset();
  shear_factor_ = shear_factor;
}
inline void Quad::SetWidth(float width) {
  if (width == width_) {
    return;
  }
  semi_axes_.reset();
  width_ = width;
  Normalize();
}

inline bool operator==(const Quad& lhs, const Quad& rhs) {
  return lhs.Center() == rhs.Center() && lhs.Width() == rhs.Width() &&
         lhs.Height() == rhs.Height() && lhs.Rotation() == rhs.Rotation() &&
         lhs.ShearFactor() == rhs.ShearFactor();
}

inline bool operator!=(const Quad& lhs, const Quad& rhs) {
  return !(lhs == rhs);
}

}  // namespace ink

#endif  // INK_GEOMETRY_QUAD_H_
