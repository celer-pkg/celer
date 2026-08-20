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

#ifndef INK_STROKES_INTERNAL_STROKE_SHAPE_UPDATE_H_
#define INK_STROKES_INTERNAL_STROKE_SHAPE_UPDATE_H_

#include <cstdint>
#include <optional>

#include "ink/geometry/envelope.h"

namespace ink::strokes_internal {

// A helper type used inside `StrokeShapeBuilder` to manage data on incrememntal
// changes to the stroke mesh being built.
//
// Update information consists of:
//   * The bounding region of any updated parts of a stroke shape's mesh. This
//     is useful to pass on to renderers to calculate and scissor down to only
//     the "damaged" region of the screen.
//   * The first triangle index and first vertex of the stroke's shape that has
//     been updated (newly appended or modified). This is useful for efficiently
//     updating GPU buffers by sending only the new / updated data to the GPU
//     whenever possible.
//
// The index and vertex are tracked by their respective offset inside a mesh.
struct StrokeShapeUpdate {
  // Adds the `other` update to `this` by calculating the joined updated region
  // and the minima of first updated index and vertex offsets.
  void Add(const StrokeShapeUpdate& other);

  Envelope region;
  std::optional<uint32_t> first_index_offset;
  std::optional<uint32_t> first_vertex_offset;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_STROKE_SHAPE_UPDATE_H_
