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

#include "ink/geometry/quad.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <utility>

#include "absl/log/absl_log.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {

std::pair<Vec, Vec> Quad::SemiAxes() const {
  if (!semi_axes_.has_value()) {
    const float cos_rotation = Cos(rotation_);
    const float sin_rotation = Sin(rotation_);
    Vec axis_1 = Vec{.5f * width_ * cos_rotation, .5f * width_ * sin_rotation};
    Vec axis_2 =
        Vec{.5f * height_ * (shear_factor_ * cos_rotation - sin_rotation),
            .5f * height_ * (shear_factor_ * sin_rotation + cos_rotation)};
    semi_axes_ = std::make_pair(axis_1, axis_2);
  }
  return semi_axes_.value();
}

std::array<Point, 4> Quad::Corners() const {
  const std::pair<Vec, Vec> semi_axes = Quad::SemiAxes();
  return {Center() - semi_axes.first - semi_axes.second,
          Center() + semi_axes.first - semi_axes.second,
          Center() + semi_axes.first + semi_axes.second,
          Center() - semi_axes.first + semi_axes.second};
}

Segment Quad::GetEdge(int index) const {
  const std::array<Point, 4> corners = Corners();
  switch (index) {
    case 0:
      return Segment{corners[0], corners[1]};
    case 1:
      return Segment{corners[1], corners[2]};
    case 2:
      return Segment{corners[2], corners[3]};
    case 3:
      return Segment{corners[3], corners[0]};
    default:
      ABSL_LOG(FATAL) << "Index " << index << " out of bounds";
  }
}

namespace {
std::pair<Vec, Vec> UnitAxes(Quad q) {
  const float cos_rotation = Cos(q.Rotation());
  const float sin_rotation = Sin(q.Rotation());
  Vec axis_1 = Vec{cos_rotation, sin_rotation};
  Vec axis_2 = Vec{q.ShearFactor() * cos_rotation - sin_rotation,
                   q.ShearFactor() * sin_rotation + cos_rotation};
  return std::make_pair(axis_1, axis_2);
}
}  // namespace

void Quad::Join(Point point) {
  std::pair<Vec, Vec> unit_axes = UnitAxes(*this);
  Vec q = point - center_;
  float u_dot_q = Vec::DotProduct(unit_axes.first, q);
  float u_cross_q = Vec::Determinant(unit_axes.first, q);
  float v_cross_q = Vec::Determinant(unit_axes.second, q);
  float new_width =
      std::max(width_, .5f * width_ + abs(u_dot_q - shear_factor_ * u_cross_q));
  float new_height = std::copysign(1.f, height_) *
                     std::max(std::abs(height_),
                              .5f * std::abs(height_) + std::abs(u_cross_q));
  float u_offset = -.5f * std::copysign(1.f, v_cross_q) * (new_width - width_);
  float v_offset = .5f * std::copysign(1.f, u_cross_q) *
                   std::copysign(1.f, height_) * (new_height - height_);
  center_ = center_ + u_offset * unit_axes.first + v_offset * unit_axes.second;
  width_ = new_width;
  height_ = new_height;
  semi_axes_.reset();
}

bool Quad::Contains(Point point) const {
  auto [u, v] = UnitAxes(*this);
  Vec q = point - center_;
  float u_cross_q = Vec::Determinant(u, q);
  if (std::abs(u_cross_q) > .5f * std::abs(height_)) return false;
  float u_dot_q = Vec::DotProduct(u, q);
  return std::abs(u_dot_q - shear_factor_ * u_cross_q) <= .5f * width_;
}

}  // namespace ink
