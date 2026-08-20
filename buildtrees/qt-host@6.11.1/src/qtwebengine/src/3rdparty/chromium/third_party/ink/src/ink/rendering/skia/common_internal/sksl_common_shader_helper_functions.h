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

#ifndef INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_COMMON_SHADER_HELPER_FUNCTIONS_H_
#define INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_COMMON_SHADER_HELPER_FUNCTIONS_H_

#include "absl/strings/string_view.h"

namespace ink::skia_common_internal {

// SkSL source code for shader helper functions used by both vertex and fragment
// shader stages.
//
// The functions in this file should be pure and not directly depend on the
// definitions of the mesh SkSL `Attributes` or `Varyings` structs.
//
// The source is held in a single string view made of adjacent raw string
// literals concatenated by the compiler, which allows insertion of
// documentation comments for each function.
inline constexpr absl::string_view kSkSLCommonShaderHelpers =
    // Returns the target vertex outset in pixels based on the `widthInPixels`
    // of the current triangle.
    //
    // The pixel outset is calculated by the
    // `calculateAntialiasingAndPositionOutset()` vertex shader helper to
    // increase the size of triangles so that they do not visibly shrink when
    // calculating `simulatedPixelCoverage()` in the fragment shader.
    //
    // The goal of the outset is to make the triangle contain the center of any
    // pixel it intersects so that we get a fragment shader run at that pixel.
    // The shortest distance from the center of a pixel to one of its edges is
    // 0.5 px, but the distance to a corner is sqrt(2)/2 = 0.707106... px. This
    // larger distance produces smoother edges at widths greater than or equal
    // to a single pixel, but produces a greater amount of luminosity flicker
    // below that. So we compromise and transition back to a target outset of
    // 0.5 px when `widthInPixels` begins to drop below 1 px.
    R"(
    float targetAntialiasingPixelOutset(const float widthInPixels) {
      return mix(0.5, 0.707107, saturate(2.0 * (widthInPixels - 0.5)));
    }
)";

}  // namespace ink::skia_common_internal

#endif  // INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_COMMON_SHADER_HELPER_FUNCTIONS_H_
