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

#ifndef INK_STROKES_BRUSH_BRUSH_COAT_H_
#define INK_STROKES_BRUSH_BRUSH_COAT_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/mesh_format.h"

namespace ink {

// A `BrushCoat` represents one coat of paint applied by a brush. It includes a
// `BrushPaint` and a `BrushTip` used to apply that paint. Multiple `BrushCoats`
// can be combined within a single brush; when a stroke drawn by a multi-coat
// brush is rendered, each coat of paint will be drawn entirely atop the
// previous coat, even if the stroke crosses over itself, as though each coat
// were painted in its entirety one at a time.
struct BrushCoat {
  BrushTip tip;
  BrushPaint paint;
};

namespace brush_internal {

// Determines whether the given BrushCoat struct is valid to be used in a
// BrushFamily, and returns an error if not.
absl::Status ValidateBrushCoat(const BrushCoat& coat);

// Returns the mesh attribute IDs that are required to properly render a mesh
// made with this brush coat. This will always include `kPosition` and certain
// other attribute IDs (`kSideDerivative`, `kSideLabel`, `kForwardDerivative`,
// `kForwardLabel`, and `kOpacityShift`), and may also include additional
// attribute IDs depending on the tip and paint settings.
std::vector<MeshFormat::AttributeId> GetRequiredAttributeIds(
    const BrushCoat& coat);

std::string ToFormattedString(const BrushCoat& coat);

}  // namespace brush_internal

template <typename Sink>
void AbslStringify(Sink& sink, const BrushCoat& coat) {
  sink.Append(brush_internal::ToFormattedString(coat));
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_COAT_H_
