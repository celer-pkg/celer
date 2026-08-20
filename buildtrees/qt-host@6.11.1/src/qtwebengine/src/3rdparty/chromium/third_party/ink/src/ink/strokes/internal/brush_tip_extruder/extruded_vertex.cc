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

#include "ink/strokes/internal/brush_tip_extruder/extruded_vertex.h"

#include <array>
#include <optional>

#include "absl/log/absl_check.h"
#include "ink/color/color.h"
#include "ink/geometry/internal/algorithms.h"
#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {

using ::ink::geometry_internal::Lerp;
using ::ink::strokes_internal::BarycentricLerp;
using ::ink::strokes_internal::Lerp;
using RgbaFloat = ::ink::Color::RgbaFloat;

ExtrudedVertex Lerp(const ExtrudedVertex& a, const ExtrudedVertex& b, float t) {
  return {
      .position = a.position + (b.position - a.position) * t,
      .new_non_position_attributes =
          Lerp(a.new_non_position_attributes, b.new_non_position_attributes, t),
      .color = Lerp(a.color, b.color, t),
      .texture_coords = Lerp(a.texture_coords, b.texture_coords, t),
      .secondary_texture_coords =
          Lerp(a.secondary_texture_coords, b.secondary_texture_coords, t),
  };
}

namespace {

Point CalculateNewTextureCoords(Point a, Point b, Point c,
                                std::array<float, 3> coords) {
  return {a.x * coords[0] + b.x * coords[1] + c.x * coords[2],
          a.y * coords[0] + b.y * coords[1] + c.y * coords[2]};
}

RgbaFloat CalculateNewColor(RgbaFloat a, RgbaFloat b, RgbaFloat c,
                            std::array<float, 3> coords) {
  return {
      a.r * coords[0] + b.r * coords[1] + c.r * coords[2],
      a.g * coords[0] + b.g * coords[1] + c.g * coords[2],
      a.b * coords[0] + b.b * coords[1] + c.b * coords[2],
      a.a * coords[0] + b.a * coords[1] + c.a * coords[2],
  };
}

}  // namespace

ExtrudedVertex BarycentricLerp(const ExtrudedVertex& a, const ExtrudedVertex& b,
                               const ExtrudedVertex& c, Point position) {
  std::optional<std::array<float, 3>> coords =
      geometry_internal::GetBarycentricCoordinates(
          Triangle{.p0 = a.position, .p1 = b.position, .p2 = c.position},
          position);
  ABSL_CHECK(coords.has_value());

  return {
      .position = position,
      .new_non_position_attributes = BarycentricLerp(
          a.new_non_position_attributes, b.new_non_position_attributes,
          c.new_non_position_attributes, *coords),
      .color = CalculateNewColor(a.color, b.color, c.color, *coords),
      .texture_coords = CalculateNewTextureCoords(
          a.texture_coords, b.texture_coords, c.texture_coords, *coords),
      .secondary_texture_coords = CalculateNewTextureCoords(
          a.secondary_texture_coords, b.secondary_texture_coords,
          c.secondary_texture_coords, *coords),
  };
}

}  // namespace ink::brush_tip_extruder_internal
