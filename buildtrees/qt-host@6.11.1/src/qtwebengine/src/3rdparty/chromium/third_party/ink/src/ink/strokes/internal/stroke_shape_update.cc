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

#include "ink/strokes/internal/stroke_shape_update.h"

#include <algorithm>
#include <cstdint>
#include <optional>

namespace ink::strokes_internal {
namespace {

// Returns the minimum value contained by one of two `optional`s, or `nullopt`
// if neither one has a value.
std::optional<uint32_t> GetMinValue(std::optional<uint32_t> a,
                                    std::optional<uint32_t> b) {
  if (!a.has_value()) return b;
  if (!b.has_value()) return a;
  return std::min(*a, *b);
}

}  // namespace

void StrokeShapeUpdate::Add(const StrokeShapeUpdate& other) {
  region.Add(other.region);
  first_index_offset =
      GetMinValue(first_index_offset, other.first_index_offset);
  first_vertex_offset =
      GetMinValue(first_vertex_offset, other.first_vertex_offset);
}

}  // namespace ink::strokes_internal
