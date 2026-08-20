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

#ifndef INK_STROKES_INTERNAL_LEGACY_VERTEX_H_
#define INK_STROKES_INTERNAL_LEGACY_VERTEX_H_

#include "ink/color/color.h"
#include "ink/geometry/point.h"

namespace ink::strokes_internal {

struct LegacyVertex {
  Point position = {0, 0};
  Color::RgbaFloat color = {0, 0, 0, 0};
  Point texture_coords = {0, 0};
  Point secondary_texture_coords = {0, 0};

  bool operator==(const LegacyVertex& other) const;
  bool operator!=(const LegacyVertex& other) const { return !(other == *this); }
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_LEGACY_VERTEX_H_
