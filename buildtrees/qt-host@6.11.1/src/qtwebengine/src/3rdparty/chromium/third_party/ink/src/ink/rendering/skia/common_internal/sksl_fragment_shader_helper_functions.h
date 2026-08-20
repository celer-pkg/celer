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

#ifndef INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_FRAGMENT_SHADER_HELPER_FUNCTIONS_H_
#define INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_FRAGMENT_SHADER_HELPER_FUNCTIONS_H_

#include "absl/strings/string_view.h"

namespace ink::skia_common_internal {

// SkSL source code for fragment shader helper functions.
//
// The functions in this file should be pure and not directly depend on the
// definitions of the mesh SkSL `Attributes` or `Varyings` structs.
//
// The source is held in a single string view made of adjacent raw string
// literals concatenated by the compiler, which allows insertion of
// documentation comments for each function.
inline constexpr absl::string_view kSkSLFragmentShaderHelpers =
    // Returns the simulated pixel coverage of the current fragment so that it
    // can be used to perform antialiasing.
    //
    // Some of the description of `pixelsPerDimension` and
    // `normalizedToEdgeLRFB` can be found in comments on the vertex shader
    // helper `calculateAntialiasingAndPositionOutset()`.
    //
    // Coverage is calculated by first calculating the approximate distance in
    // pixels from the current fragment to each of the simulated left, right,
    // back, and front exterior edges. We must add the total outset to each of
    // the side and forward pixels-per-dimensions. To do this, we reconstruct
    // the value of each `outsetPixelsLRFB` component by dividing it by 1.0
    // minus the corresponding component of `normalizedToEdgeLRFB`. Taking the
    // y-components of `normalizedToEdgeLRFB` and `outsetPixelsLRFB` as
    // examples:
    //   * `1.0 - normalizedToEdgeLRFB.y` will range from 0 at the left-exterior
    //     of a triangle to 1 at the right-exterior. The current value is
    //     represented by X in the diagram below.
    //   * `outsetPixelsLRFB.y` will range from 0 at the left-exterior of a
    //     triangle to the original outset in pixels calculated along the
    //     right-exterior of the triangle. In the diagram below, the current
    //     value is represented by Y, and the value at the right-exterior is P.
    //   * We can reconstruct the value of P by dividing Y by X.
    //
    //    y
    //    ^
    //  P | - - - - -*
    //    |        / -
    //    |      /   -
    //  Y | - -*     -
    //    |  / -     -
    //    |/   -     -
    //    --------------> x
    //    0    X     1
    //
    // The pixels to each edge are calculated by multiplying the total pixels
    // per dimension by the normalized distance to the edge, and then by
    // dividing by twice the target outset. The `targetOutset` calculated from
    // the `targetAntialiasingPixelOutset()` common shader helper also gives us
    // half of the perceived edge width.
    //
    // The coverage in each of the left-right and front-back dimensions is
    // calculated by adding the pixel distance to opposite edges and
    // subtracting 1.0 (0.5 for each edge). An extra step is taken in this
    // calculation, because an entire triangle may be smaller than a pixel
    // along either dimension. This would be ok for a triangle with an exterior
    // edge, but would result in a false-positive reduction of opacity for a
    // triangle that is entirely in the interior.
    //
    // Finally, the total coverage is estimated by multiplying the coverage in
    // each of the left-right and front-back dimensions. This assumes that the
    // "left-right" axis is sufficiently close to perpendicular to the
    // "front-back" axis.
    R"(
    float simulatedPixelCoverage(const float2 pixelsPerDimension,
                                 const float4 normalizedToEdgeLRFB,
                                 const float4 outsetPixelsLRFB) {
      float targetOutset = targetAntialiasingPixelOutset(pixelsPerDimension.x);
      float4 fromEdge = float4(1.0) - normalizedToEdgeLRFB;
      float4 outset =
          min(outsetPixelsLRFB / max(fromEdge, 0.000001), targetOutset);
      float2 adjustedPixels = pixelsPerDimension + outset.xz + outset.yw;
      float4 pixelsToEdge = saturate(
          (adjustedPixels.xxyy * normalizedToEdgeLRFB) / (2.0 * targetOutset));
      float2 isInterior =
          step(1.9999, normalizedToEdgeLRFB.xz + normalizedToEdgeLRFB.yw);
      float2 coverage =
          mix(max(pixelsToEdge.xz + pixelsToEdge.yw - float2(1.0), 0.0),
              float2(1.0), isInterior);
      return coverage.x * coverage.y;
    }
)";

}  // namespace ink::skia_common_internal

#endif  // INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_FRAGMENT_SHADER_HELPER_FUNCTIONS_H_
