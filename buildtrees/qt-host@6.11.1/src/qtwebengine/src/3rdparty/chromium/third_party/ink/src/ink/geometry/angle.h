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

#ifndef INK_GEOMETRY_ANGLE_H_
#define INK_GEOMETRY_ANGLE_H_

#include <cmath>
#include <cstdlib>
#include <string>

#include "ink/geometry/internal/modulo.h"
#include "ink/types/numbers.h"

namespace ink {

// This class represents a signed angle.
// Convenience methods are provided for working in degrees.
// A positive value represents rotation from the positive x-axis to
// the positive y-axis.
class Angle {
 public:
  // Constructs an angle with a value of zero radians (which is also
  // zero degrees).
  Angle() = default;

  // Constructs an angle with the given value in radians.
  static constexpr Angle Radians(float radians) { return Angle(radians); }
  // Constructs an angle with the given value in degrees.
  static constexpr Angle Degrees(float degrees) {
    return Angle(degrees * static_cast<float>(numbers::kPi / 180.0));
  }

  constexpr Angle(const Angle&) = default;
  constexpr Angle& operator=(const Angle&) = default;

  // Returns the value of the angle in radians.
  float ValueInRadians() const { return radians_; }
  // Returns the value of the angle in degrees.
  float ValueInDegrees() const {
    return radians_ * static_cast<float>(180.0 / numbers::kPi);
  }

  // Returns the angle equivalent to this in the interval [0, 2π).
  Angle Normalized() const;
  // Returns the angle equivalent to this in the interval (-π, π].
  Angle NormalizedAboutZero() const;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, Angle angle) {
    sink.Append(angle.ToFormattedString());
  }

  template <typename H>
  friend H AbslHashValue(H h, Angle angle) {
    return H::combine(std::move(h), angle.radians_);
  }

 private:
  // Constructs an angle, in radians. The value is not normalized.
  explicit constexpr Angle(float radians) : radians_(radians) {}

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  float radians_ = 0;
};

inline constexpr Angle kQuarterTurn = Angle::Radians(1.5707963268f);
inline constexpr Angle kHalfTurn = Angle::Radians(3.1415926536f);
inline constexpr Angle kFullTurn = Angle::Radians(6.2831853072f);

// Convenience functions for trigonometric functions in std.
float Sin(Angle angle);
float Cos(Angle angle);
float Tan(Angle angle);

Angle Asin(float value);
Angle Acos(float value);
Angle Atan(float value);

Angle Abs(Angle angle);
Angle Mod(Angle value, Angle divisor);
Angle Min(Angle a, Angle b);
Angle Max(Angle a, Angle b);

bool operator==(Angle lhs, Angle rhs);
bool operator!=(Angle lhs, Angle rhs);
bool operator<(Angle lhs, Angle rhs);
bool operator>(Angle lhs, Angle rhs);
bool operator<=(Angle lhs, Angle rhs);
bool operator>=(Angle lhs, Angle rhs);

Angle operator-(Angle angle);
Angle operator+(Angle lhs, Angle rhs);
Angle operator-(Angle lhs, Angle rhs);
Angle operator*(float scalar, Angle angle);
Angle operator*(Angle angle, float scalar);
Angle operator/(Angle angle, float scalar);
float operator/(Angle numerator, Angle denominator);

Angle& operator+=(Angle& lhs, Angle rhs);
Angle& operator-=(Angle& lhs, Angle rhs);
Angle& operator*=(Angle& lhs, float rhs);
Angle& operator/=(Angle& lhs, float rhs);

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline float Sin(Angle angle) { return std::sin(angle.ValueInRadians()); }
inline float Cos(Angle angle) { return std::cos(angle.ValueInRadians()); }
inline float Tan(Angle angle) { return std::tan(angle.ValueInRadians()); }

inline Angle Asin(float value) { return Angle::Radians(std::asin(value)); }
inline Angle Acos(float value) { return Angle::Radians(std::acos(value)); }
inline Angle Atan(float value) { return Angle::Radians(std::atan(value)); }

inline Angle Abs(Angle angle) {
  return Angle::Radians(std::abs(angle.ValueInRadians()));
}
inline Angle Mod(Angle value, Angle divisor) {
  return Angle::Radians(
      std::fmod(value.ValueInRadians(), divisor.ValueInRadians()));
}
inline Angle Min(Angle a, Angle b) {
  return (a.ValueInRadians() > b.ValueInRadians()) ? b : a;
}
inline Angle Max(Angle a, Angle b) {
  return (a.ValueInRadians() > b.ValueInRadians()) ? a : b;
}

inline bool operator==(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() == rhs.ValueInRadians();
}
inline bool operator!=(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() != rhs.ValueInRadians();
}
inline bool operator<(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() < rhs.ValueInRadians();
}
inline bool operator>(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() > rhs.ValueInRadians();
}
inline bool operator<=(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() <= rhs.ValueInRadians();
}
inline bool operator>=(Angle lhs, Angle rhs) {
  return lhs.ValueInRadians() >= rhs.ValueInRadians();
}

inline Angle operator-(Angle angle) {
  return Angle::Radians(-angle.ValueInRadians());
}
inline Angle operator+(Angle lhs, Angle rhs) {
  return Angle::Radians(lhs.ValueInRadians() + rhs.ValueInRadians());
}
inline Angle operator-(Angle lhs, Angle rhs) {
  return Angle::Radians(lhs.ValueInRadians() - rhs.ValueInRadians());
}
inline Angle operator*(float scalar, Angle angle) {
  return Angle::Radians(scalar * angle.ValueInRadians());
}
inline Angle operator*(Angle angle, float scalar) {
  return Angle::Radians(angle.ValueInRadians() * scalar);
}
inline Angle operator/(Angle angle, float scalar) {
  return Angle::Radians(angle.ValueInRadians() / scalar);
}
inline float operator/(Angle numerator, Angle denominator) {
  return numerator.ValueInRadians() / denominator.ValueInRadians();
}

inline Angle& operator+=(Angle& lhs, Angle rhs) {
  lhs = lhs + rhs;
  return lhs;
}
inline Angle& operator-=(Angle& lhs, Angle rhs) {
  lhs = lhs - rhs;
  return lhs;
}
inline Angle& operator*=(Angle& lhs, float rhs) {
  lhs = lhs * rhs;
  return lhs;
}
inline Angle& operator/=(Angle& lhs, float rhs) {
  lhs = lhs / rhs;
  return lhs;
}

inline Angle Angle::Normalized() const {
  return Angle::Radians(geometry_internal::FloatModulo(
      ValueInRadians(), static_cast<float>(2.0 * numbers::kPi)));
}

}  // namespace ink

#endif  // INK_GEOMETRY_ANGLE_H_
