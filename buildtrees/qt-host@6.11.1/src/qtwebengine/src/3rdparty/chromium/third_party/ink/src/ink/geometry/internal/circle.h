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

#ifndef INK_GEOMETRY_INTERNAL_CIRCLE_H_
#define INK_GEOMETRY_INTERNAL_CIRCLE_H_

#include <optional>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink::geometry_internal {

// Type representing a circle, permitting the degenerate case where the radius
// equals zero.
class Circle {
 public:
  // Constructs a circle of radius 1 centered at the origin.
  Circle() = default;

  // Constructs with the given `center` and `radius`. CHECK-fails if `radius` is
  // not greater than or equal to zero.
  Circle(Point center, float radius);

  Circle(const Circle&) = default;
  Circle& operator=(const Circle&) = default;
  ~Circle() = default;

  Point Center() const;
  float Radius() const;

  // Returns the point on the circle at the given `angle`.
  Point GetPoint(Angle angle) const;

  // Computes the pair of angles at which the exterior tangents meet `this` and
  // the `other` circle. (see get_tangent_angles.svg).
  //
  // In the event that there do not exist two distinct exterior tangents, this
  // function returns `std::nullopt`. This occurs if one of the circles is
  // entirely inside the other, the circles coincide, or there is only one
  // tangent. Returned angles will be normalized to the range (-π, π].
  //
  // The angles are labeled by splitting the xy-plane into a "left" and "right"
  // side when the plane is viewed from the positive z-axis in the travel
  // direction from `this->Center()` toward `other.Center()`.
  struct TangentAngles {
    Angle left;
    Angle right;
  };
  std::optional<TangentAngles> GetTangentAngles(const Circle& other) const;

  // Returns the angle for the exterior tangent that meets `this` and the
  // `other` circle on their "right" sides relative to the direction of travel
  // when viewed from the positive z-axis.
  //
  // The returned angle will be normalized to the range (-π, π].
  //
  // This function should only be called if `*this` and `other` are known to not
  // contain one another. This is equivalent to calling
  // `GetTangentAngles(other)->right`, but is more efficient.
  Angle GuaranteedRightTangentAngle(const Circle& other) const;

  // Appends evenly spaced points on the circle to `polyline` starting at
  // `starting_angle` and ending at `starting_angle + arc_angle`.
  //
  // The sign of the `arc_angle` determines the direction in which the arc is
  // traversed.
  //
  // The function always generates at least two points; one at the start and one
  // at the end of the arc. This is the case even when `arc_angle` is zero. The
  // function will generate the smallest number of points needed so that the
  // chord height (i.e. sagitta) of each polyline segment does not exceed
  // `max_chord_height`.  See https://en.wikipedia.org/wiki/Sagitta_(geometry)."
  //
  // The value of `max_chord_height` must be greater than 0 (CHECK-enforced),
  // with higher values resulting in a coarser approximation made of fewer
  // points.
  //
  // This function has a hard cap of generating 2^16 = 32768 points at most;
  // if it would take more points than that to stay within `max_chord_height`,
  // then this function will instead use a larger chord height so that the
  // points can cover the whole arc.
  void AppendArcToPolyline(Angle starting_angle, Angle arc_angle,
                           float max_chord_height,
                           std::vector<Point>& polyline) const;

  // Calculates the angle of the arc required to generate a chord with the given
  // `chord_height`  (i.e. sagitta).
  //
  // The return value is always in the range [0, 2π), and a chord height greater
  // than the radius results in an arc angle greater than π. A non-positive
  // value of `chord_height` returns zero.
  Angle GetArcAngleForChordHeight(float chord_height) const;

  // Returns true if this circle contains `other`, which includes the case where
  // the two circles coincide.
  bool Contains(const Circle& other) const;

 private:
  Point center_ = {0, 0};
  float radius_ = 1;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline Circle::Circle(Point center, float radius)
    : center_(center), radius_(radius) {
  ABSL_CHECK_GE(radius, 0);
}

inline Point Circle::Center() const { return center_; }

inline float Circle::Radius() const { return radius_; }

inline Point Circle::GetPoint(Angle angle) const {
  return center_ + Vec::FromDirectionAndMagnitude(angle, radius_);
}

inline Angle Circle::GuaranteedRightTangentAngle(const Circle& other) const {
  ABSL_DCHECK(!Contains(other));
  ABSL_DCHECK(!other.Contains(*this));
  Vec center_offset = other.center_ - center_;
  return (center_offset.Direction() -
          Acos((radius_ - other.radius_) / center_offset.Magnitude()))
      .NormalizedAboutZero();
}

inline bool Circle::Contains(const Circle& other) const {
  return (other.center_ - center_).Magnitude() + other.radius_ <= radius_;
}

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_CIRCLE_H_
