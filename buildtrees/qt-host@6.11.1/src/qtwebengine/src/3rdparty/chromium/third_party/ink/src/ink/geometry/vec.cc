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

#include "ink/geometry/vec.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include "absl/strings/str_format.h"
#include "ink/geometry/angle.h"
#include "ink/types/numbers.h"

namespace ink {

Vec Vec::AsUnitVec() const {
  // If either component is NaN, then the direction and magnitude are both NaN,
  // and no meaningful unit vector exists.
  if (std::isnan(x) || std::isnan(y)) {
    return Vec{std::numeric_limits<float>::quiet_NaN(),
               std::numeric_limits<float>::quiet_NaN()};
  }

  // If either component is ±inf, then return a unit vector with the same
  // direction that std::atan2 would return.
  if (std::isinf(x)) {
    if (std::isinf(y)) {
      constexpr float half_sqrt2 = 0.5 * numbers::kSqrt2;
      return Vec{std::copysign(half_sqrt2, x), std::copysign(half_sqrt2, y)};
    }
    return Vec{std::copysign(1.f, x), std::copysign(0.f, y)};
  }
  if (std::isinf(y)) {
    return Vec{std::copysign(0.f, x), std::copysign(1.f, y)};
  }

  // If both components are zero, then the unit vector is mathematically
  // undefined.  However, perhaps surprisingly, std::atan2 defines a direction
  // for such a vector, so return a unit vector with that direction.
  if (x == 0 && y == 0) {
    return Vec{std::copysign(1.f, x), std::copysign(0.f, y)};
  }

  // Finally, we have the finite, nonzero case.  In theory, we can just divide
  // the vector by its magnitude, but the magnitude can overflow to infinity if
  // e.g. x and y are both very large finite floats.  We can avoid this by
  // pre-scaling the vector before normalizing.
  Vec scaled = 0.5f * *this;
  return scaled / scaled.Magnitude();
}

Angle Vec::SignedAngleBetween(const Vec &a, const Vec &b) {
  Vec a_unit = a.AsUnitVec();
  Vec b_unit = b.AsUnitVec();
  // Calculate the absolute angle between the two vectors. Note that we don't
  // use AbsoluteAngleBetween() here, so that we can reuse the unit vectors
  // again in the determinant calculation below. Since we only care about the
  // sign of the determinant, mathematically it would be just as good to use
  // Determinant(a, b); however, using the unit vectors avoids problems such as
  // Determinant returning NaN if both multiplications overflow to infinity.
  Angle angle = Acos(std::clamp(DotProduct(a_unit, b_unit), -1.0f, 1.0f));
  // Negate the angle if the determinant is negative, with one weird exception
  // for `angle == kHalfTurn` that's needed due to floating point rounding.
  //
  // Mathematically, the absolute angle will be strictly less than π radians if
  // the determinant is nonzero, but due to float rounding it's possible for the
  // determinant to be a small (but nonzero) negative number while the absolute
  // angle is exactly π radians. Since this method promises to return a value
  // strictly greater than -π radians, we always return the positive angle when
  // the angle is exactly π radians (i.e. `kHalfTurn`).
  return angle == kHalfTurn || Determinant(a_unit, b_unit) >= 0.f ? angle
                                                                  : -angle;
}

namespace vec_internal {

std::string ToFormattedString(Vec v) {
  return absl::StrFormat("<%v, %v>", v.x, v.y);
}

}  // namespace vec_internal
}  // namespace ink
