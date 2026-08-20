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

#include "ink/strokes/internal/legacy_vertex.h"

namespace ink::strokes_internal {

bool LegacyVertex::operator==(const LegacyVertex& other) const {
  return other.position == position && other.color.r == color.r &&
         other.color.g == color.g && other.color.b == color.b &&
         other.color.a == color.a && other.texture_coords == texture_coords &&
         other.secondary_texture_coords == secondary_texture_coords;
}

}  // namespace ink::strokes_internal
