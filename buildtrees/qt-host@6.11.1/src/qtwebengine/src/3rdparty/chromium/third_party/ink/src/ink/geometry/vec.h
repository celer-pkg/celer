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

#ifndef INK_GEOMETRY_VEC_H_
#define INK_GEOMETRY_VEC_H_

#include <algorithm>
#include <cmath>
#include <string>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"

namespace ink {

// A 2-dimensional vector, representing an offset in space. See `Point` for a
// location in space.
struct Vec {
  float x = 0;
  float y = 0;

  // Constructs a Vec with the given direction and magnitude.
  static Vec FromDirectionAndMagnitude(Angle direction, float magnitude) {
    return Vec{magnitude * Cos(direction), magnitude * Sin(direction)};
  }

  // Returns the length of the vector.
  float Magnitude() const { return std::hypot(x, y); }

  // Returns the squared length of the vector.
  float MagnitudeSquared() const { return x * x + y * y; }

  // Returns the direction of the vector, represented as the angle between the
  // positive x-axis and this vector. If either component of the vector is NaN,
  // this returns a NaN angle; otherwise, the returned angle will lie in the
  // interval [-π, π] radians, and will have the same sign as the vector's
  // y-component.
  //
  // Following the behavior of std::atan2, this will return either ±0 or ±π
  // radians for the zero vector, depending on the signs of the zeros.
  Angle Direction() const { return Angle::Radians(std::atan2(y, x)); }

  // Returns a vector with the same magnitude as this one, but rotated by
  // (positive) 90 degrees.
  Vec Orthogonal() const { return Vec{-y, x}; }

  // Returns a vector with the same direction as this one, but with a magnitude
  // of 1.  This is equivalent to (but faster than):
  //   Vec2::WithDirectionAndMagnitude(v.Direction(), 1);
  //
  // In keeping with the above equivalence, this will return <±1, ±0> for the
  // zero vector, depending on the signs of the zeros.
  Vec AsUnitVec() const;

  // Returns the dot product (⋅) of the two vectors. The dot product
  // has the property that, for vectors a and b:
  //   a ⋅ b = ‖a‖ * ‖b‖ * cos(θ)
  // where ‖v‖ is the magnitude of the vector, and θ is the angle
  // from a to b.
  static float DotProduct(const Vec &lhs, const Vec &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
  }

  // Returns the determinant (×) of the two vectors. The determinant
  // can be thought of as the z-component of the 3D cross product of
  // the two vectors, if they were placed on the xy-plane in 3D
  // space.
  // The determinant has the property that, for vectors a and b:
  //   a × b = ‖a‖ * ‖b‖ * sin(θ)
  // where ‖v‖ is the magnitude of the vector, and θ is the signed
  // angle from a to b.
  static float Determinant(const Vec &a, const Vec &b) {
    return a.x * b.y - a.y * b.x;
  }

  // Returns the absolute angle between the given vectors. If either component
  // of either vector is NaN, this returns a NaN angle; otherwise, the return
  // value will lie in the interval [0, π] radians. This method is equivalent to
  // (but faster than):
  //   Abs((b.Direction() - a.Direction()).NormalizedAboutZero())
  // or:
  //   Abs(Vec::SignedAngleBetween(a, b))
  static Angle AbsoluteAngleBetween(const Vec &a, const Vec &b) {
    return Acos(
        std::clamp(Vec::DotProduct(a.AsUnitVec(), b.AsUnitVec()), -1.0f, 1.0f));
  }

  // Returns the signed angle between the given vectors. If either component of
  // either vector is NaN, this returns a NaN angle; otherwise, the return value
  // will lie in the interval (-π, π] radians. This method is equivalent to (but
  // faster than):
  //   (b.Direction() - a.Direction()).NormalizedAboutZero();
  static Angle SignedAngleBetween(const Vec &a, const Vec &b);
};

bool operator==(Vec lhs, Vec rhs);
bool operator!=(Vec lhs, Vec rhs);

Vec operator+(Vec lhs, Vec rhs);
Vec operator-(Vec lhs, Vec rhs);
Vec operator-(Vec v);
Vec operator*(float scalar, Vec v);
Vec operator*(Vec v, float scalar);
Vec operator/(Vec v, float scalar);

Vec &operator+=(Vec &lhs, Vec rhs);
Vec &operator-=(Vec &lhs, Vec rhs);
Vec &operator*=(Vec &lhs, float scalar);
Vec &operator/=(Vec &lhs, float scalar);

namespace vec_internal {
std::string ToFormattedString(Vec v);
}  // namespace vec_internal

template <typename Sink>
void AbslStringify(Sink &sink, Vec v) {
  sink.Append(vec_internal::ToFormattedString(v));
}

template <typename H>
H AbslHashValue(H h, Vec v) {
  return H::combine(std::move(h), v.x, v.y);
}

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(Vec lhs, Vec rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline bool operator!=(Vec lhs, Vec rhs) { return !(lhs == rhs); }

inline Vec operator+(Vec lhs, Vec rhs) {
  return {.x = lhs.x + rhs.x, .y = lhs.y + rhs.y};
}
inline Vec operator-(Vec lhs, Vec rhs) {
  return {.x = lhs.x - rhs.x, .y = lhs.y - rhs.y};
}
inline Vec operator-(Vec v) { return {.x = -v.x, .y = -v.y}; }
inline Vec operator*(float scalar, Vec v) {
  return {.x = scalar * v.x, .y = scalar * v.y};
}
inline Vec operator*(Vec v, float scalar) { return scalar * v; }
inline Vec operator/(Vec v, float scalar) {
  ABSL_CHECK_NE(scalar, 0);
  return {.x = v.x / scalar, .y = v.y / scalar};
}

inline Vec &operator+=(Vec &lhs, Vec rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}
inline Vec &operator-=(Vec &lhs, Vec rhs) {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}
inline Vec &operator*=(Vec &lhs, float scalar) {
  lhs.x *= scalar;
  lhs.y *= scalar;
  return lhs;
}
inline Vec &operator/=(Vec &lhs, float scalar) {
  lhs.x /= scalar;
  lhs.y /= scalar;
  return lhs;
}

}  // namespace ink

#endif  // INK_GEOMETRY_VEC_H_
