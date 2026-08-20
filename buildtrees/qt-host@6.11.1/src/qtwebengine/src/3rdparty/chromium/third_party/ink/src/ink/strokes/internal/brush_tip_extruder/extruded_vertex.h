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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_EXTRUDED_VERTEX_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_EXTRUDED_VERTEX_H_

#include "ink/color/color.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink::brush_tip_extruder_internal {

// Vertex type used internally for extrusion.
//
// This type will exist for the duration that extrusion needs to support both
// the legacy and new code paths for stroke creation. It is meant to hold the
// union of old and new attributes.
struct ExtrudedVertex {
  Point position = {0, 0};

  strokes_internal::StrokeVertex::NonPositionAttributes
      new_non_position_attributes;

  // Legacy-only attributes:
  ink::Color::RgbaFloat color = {0, 0, 0, 0};
  Point texture_coords = {0, 0};
  Point secondary_texture_coords = {0, 0};

  static ExtrudedVertex FromLegacy(
      const strokes_internal::LegacyVertex& vertex);
  strokes_internal::LegacyVertex ToLegacy() const;
};

bool operator==(const ExtrudedVertex& a, const ExtrudedVertex& b);
bool operator!=(const ExtrudedVertex& a, const ExtrudedVertex& b);

// Computes the linear interpolation between `a` and `b` when `t` is in the
// range [0, 1], and the linear extrapolation otherwise.
//
// Note that this naming follows the behavior of the C++20 `std::lerp`, which
// diverges from legacy code that would call this `Lerpnc`, with "nc"
// designating "non-clamping".
//
// TODO: b/270984127 - This should handle different categories of attributes
// differently. It should be aware of the legacy sentinel texture coordinate
// value and colors in particular should potentially be clamped instead of
// extrapolated.
ExtrudedVertex Lerp(const ExtrudedVertex& a, const ExtrudedVertex& b, float t);

// Computes the vertex that would have the given `position` using the
// barycentric coordinates of the point relative to the three vertices `a`, `b`
// and `c`. See https://en.wikipedia.org/wiki/Barycentric_coordinate_system.
//
// TODO: b/270984127 - Make this function handle triangles with zero area.
//
// TODO: b/270984127 - This should handle different categories of attributes
// differently. It should be aware of the legacy sentinel texture coordinate
// value and colors in particular should potentially be clamped instead of
// extrapolated.
ExtrudedVertex BarycentricLerp(const ExtrudedVertex& a, const ExtrudedVertex& b,
                               const ExtrudedVertex& c, Point position);

// ---------------------------------------------------------------------------
//                     Implementation details below

inline ExtrudedVertex ExtrudedVertex::FromLegacy(
    const strokes_internal::LegacyVertex& vertex) {
  return {
      .position = vertex.position,
      .color = {vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a},
      .texture_coords = vertex.texture_coords,
      .secondary_texture_coords = vertex.secondary_texture_coords};
}

inline strokes_internal::LegacyVertex ExtrudedVertex::ToLegacy() const {
  return {.position = position,
          .color = {color.r, color.g, color.b, color.a},
          .texture_coords = texture_coords,
          .secondary_texture_coords = secondary_texture_coords};
}

inline bool operator==(const ExtrudedVertex& a, const ExtrudedVertex& b) {
  return a.position == b.position &&
         a.new_non_position_attributes == b.new_non_position_attributes &&
         a.color.r == b.color.r && a.color.g == b.color.g &&
         a.color.b == b.color.b && a.color.a == b.color.a &&
         a.texture_coords == b.texture_coords &&
         a.secondary_texture_coords == b.secondary_texture_coords;
}

inline bool operator!=(const ExtrudedVertex& a, const ExtrudedVertex& b) {
  return !(a == b);
}

}  // namespace ink::brush_tip_extruder_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_EXTRUDER_EXTRUDED_VERTEX_H_
