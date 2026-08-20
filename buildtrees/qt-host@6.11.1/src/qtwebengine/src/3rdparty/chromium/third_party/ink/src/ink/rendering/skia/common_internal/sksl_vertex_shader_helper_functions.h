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

#ifndef INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_VERTEX_SHADER_HELPER_FUNCTIONS_H_
#define INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_VERTEX_SHADER_HELPER_FUNCTIONS_H_

#include "absl/strings/string_view.h"

namespace ink::skia_common_internal {

// SkSL source code for vertex shader helper functions.
//
// The functions in this string should be pure and not directly depend on the
// definitions of the mesh SkSL `Attributes` or `Varyings` structs.
//
// The source is held in a single string view made of adjacent raw string
// literals concatenated by the compiler, which allows insertion of
// documentation comments for each function.
inline constexpr absl::string_view kSkSLVertexShaderHelpers =
    // Constructs a mat2 from a float4 holding the matrix values as
    //
    // M = | values.x  values.z |
    //     | values.y  values.w |
    //
    // This function is useful when passing a 2x2 matrix as a float4 uniform,
    // which is helpful on Android for passing the values without using an extra
    // dedicated float array.
    R"(
    mat2 mat2FromFloat4ColumnMajor(const float4 values) {
      return mat2(values.xy, values.zw);
    })"

    // Returns the determinant of a 2x2 matrix.
    R"(
    float matrixDeterminant(const mat2 m) {
      return m[0][0] * m[1][1] - m[1][0] * m[0][1];
    })"

    // Returns a 2D vector rotated by +90 degrees.
    R"(
    float2 orthogonal(const float2 v) { return float2(-v.y, v.x); })"

    // Returns a new opacity by applying `opacityShift` to `baseOpacity`.
    // `opacityShift` is expected to be a value in the range [-1, 1].
    R"(
    float applyOpacityShift(const float opacityShift, const float baseOpacity) {
      return saturate((opacityShift + 1) * baseOpacity);
    })"

    // Returns a new *unpremultiplied* color by applying `hslShift` and
    // `opacityShift` to `colorUnpremul`. Both the input color and the output
    // color are unpremultiplied linear sRGB, and may include RGB values outside
    // the range [0, 1], e.g. for wide-gamut colors. The current implementation
    // performs the shift in the YIQ color space, using the Rec. 601 primaries,
    // though it might be better to use the sRGB primaries instead.
    //
    // NOTE: there is no separate `applyHSLShift()` taking two `float3`s to help
    // prevent accidentally passing arguments in the wrong order.
    R"(
    float4 applyHSLAndOpacityShift(const float3 hslShift,
                                   const float opacityShift,
                                   const float4 colorUnpremul) {
      float3 rgb = colorUnpremul.rgb;

      float y = dot(rgb, float3(0.299,  0.587,  0.114));
      float i = dot(rgb, float3(0.596, -0.275, -0.321));
      float q = dot(rgb, float3(0.212, -0.523,  0.311));

      // When colorUnpremul.rgb is pure black, (y, i, q) == (0, 0, 0), and
      // atan(0, 0) is undefined. To avoid that, we set hueRadians to 0 in that
      // case. Generally we aim to avoid branching in the shader for
      // performance, but in this case the branching is on a uniform value,
      // which is not an issue for performance like branching on a vertex
      // attribute value is.
      float hueRadians = colorUnpremul.rgb == float3(0, 0, 0) ? 0 : atan(q, i);

      float chroma = sqrt(i * i + q * q);

      hueRadians -= hslShift.x * radians(360);
      chroma *= (hslShift.y + 1);
      y += hslShift.z;
      i = chroma * cos(hueRadians);
      q = chroma * sin(hueRadians);

      float3 yiq = float3(y, i, q);
      rgb = float3(dot(yiq, float3(1,  0.956,  0.621)),
                   dot(yiq, float3(1, -0.272, -0.647)),
                   dot(yiq, float3(1, -1.107,  1.704)));
      return float4(rgb, applyOpacityShift(opacityShift, colorUnpremul.a));
    })"

    // Decodes the values of the side and forward margins given the side and
    // forward `labels`.
    //
    // See comments on `calculateAntialiasingAndPositionOutset()` below for some
    // more information on the definitions of "side", "forward" and "margin".
    //
    // LINT.IfChange(margin_encoding)
    R"(
    float2 decodeMargins(const float2 labels) {
      return (4.0 / 126.0) * max(abs(labels) - float2(1.0), float2(0.0));
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:margin_encoding,
    //     ../../../strokes/internal/stroke_vertex.h:margin_encoding)

    // Computes per-vertex properties needed for antialiasing and returns an
    // offset that should be added to `varyings.position`.
    //
    // For each triangle in a stroke mesh, we choose two of its three
    // barycentric coordinates to use for antialiasing and label them "side" and
    // "forward" coordinates. The "side" coordinate most closely points across
    // the width of the stroke, and the "forward" coordinate is most closely
    // parallel to the direction of travel.
    //
    // See documentation on `brush_tip_extruder_internal::DerivativeCalculator`
    // and `strokes_internal::StrokeVertex` for details on the derivative
    // calculation and vertex labels.
    //
    // The `mat2 objectToCanvasLinearComponent` is the upper-left 2x2 submatrix
    // of an affine transformation from object to canvas coordinates. It
    // includes the rotation, scale, and shear that will be applied to vertex
    // positions, and excludes the translation. I.e. the implementation below
    // does not yet support non-affine object-to-canvas transformations.
    //
    // The `float2 pixelsPerDimension` out-param will be set to the number of
    // pixels per unit-barycentric coordinate for the "side" and "forward"
    // coordinates. The pixels-per-dimension are related to the side and forward
    // derivatives as well as the object-to-canvas transform. For affine
    // transforms, the problem boils down to figuring out the shortest distance
    // between the origin and a line after applying a 2D transformation, T. This
    // is non-trivial, because the closest point may lie on a different part of
    // the line even under affine transformations. Each derivative value equals
    // the vector from a particular point to its projection on a line (see
    // `brush_tip_extruder_internal::DerivativeCalculator::CalculateAndSaveSideDerivative()`
    // and `geometry_internal::VectorFromPointToSegmentProjection()`). Because
    // the derivative vector, v, has already been calculated to be perpendicular
    // to the line in question, we can use it to also define the line by using a
    // vector that is orthogonal to v. If we define R as the 2x2 matrix rotation
    // by +90 degrees, then the line L is given by:
    //
    //          L(t) = v + t * Rv  for real-valued t
    //
    // By appling the formula for the shortest distance from a point to a line
    // using the origin and L(t), it can be shown that the distance under the
    // transformation T simplifies to:
    //
    //              |det(T)| * ‖v‖²
    //          D = ---------------
    //                   ‖TRv‖
    //
    // This can also be understood intuitively as h = 2A / b for a triangle.
    // Areas under affine transformations scale with a constant equal to the
    // determinant of the 2D matrix, and Rv gives the base vector of the
    // pretend triangle prior to the transformation.
    //
    // The `float4 normalizedToEdgeLRFB` out-param will be set to the distance,
    // in barycentric units, from this vertex to each of the four exterior edges
    // of the mesh. Topologically, the mesh is treated like a square, with sides
    // labeled left, right, front and back. Each component will be equal to
    // either 0 or 1 when this function returns, with 0 corresponding to being
    // along a particular exterior edge, and 1 being in the interior. For
    // example,
    //   * {0, 1, 1, 0} is along the back-left exterior
    //   * {1, 0, 1, 1} is along the right exterior
    //   * {1, 1, 1, 1} is in the interior
    //
    // We try to outset the positions of exterior vertices by some fraction of a
    // pixel in order for the mesh to not visibly shrink in size when
    // anti-aliased with this technique. Where the mesh is locally concave, a
    // vertex outset must be clamped to prevent adjacent vertices crossing
    // paths. This would create undesirable self-overlapping artifacts for
    // partially transparent strokes. The clamping is done using a "margin"
    // value that is encoded inside each vertex label. However, we gradually
    // ignore the margin as the stroke width drops from 0.5 to 0.25 pixels. At
    // that width, this entire section of the stroke will fade and any winding
    // artifacts should not be noticeable.
    //
    // Each component of the `float4 outsetPixelsLRFB` out-param will be set
    // to a value in the range [0, `pixelOutsetTarget`] that gives the outset in
    // pixels multiplied by the 0 or 1 component of 1 minus
    // `normalizedToEdgeLRFB`. See comments on the fragment shader helper
    // `simulatedPixelCoverage()` for how this is used.
    //
    // Finally, the calculated outsets along the side and forward directions are
    // combined, keeping in mind when the side and forward outsets may point in
    // the same direction. The final calculated outset is returned in
    // object/local-coordinates of the mesh.
    R"(
    float2 calculateAntialiasingAndPositionOutset(
        const float3 sideDerivativeAndLabel,
        const float3 forwardDerivativeAndLabel,
        const mat2 objectToCanvasLinearComponent,
        out float2 pixelsPerDimension,
        out float4 normalizedToEdgeLRFB,
        out float4 outsetPixelsLRFB) {
      float2 sideDerivative = sideDerivativeAndLabel.xy;
      float2 forwardDerivative = forwardDerivativeAndLabel.xy;

      pixelsPerDimension =
          abs(matrixDeterminant(objectToCanvasLinearComponent)) *
          float2(dot(sideDerivative, sideDerivative),
                 dot(forwardDerivative, forwardDerivative)) /
          max(float2(0.000001), float2(length(objectToCanvasLinearComponent *
                                              orthogonal(sideDerivative)),
                                       length(objectToCanvasLinearComponent *
                                              orthogonal(forwardDerivative))));

      float2 labels = float2(sideDerivativeAndLabel.z,
                             forwardDerivativeAndLabel.z);

      normalizedToEdgeLRFB = float4(labels.x > -0.005,   // distance to left
                                    labels.x <  0.005,   //             right
                                    labels.y > -0.005,   //             front
                                    labels.y <  0.005);  //             back

      float pixelOutsetTarget =
          targetAntialiasingPixelOutset(pixelsPerDimension.x);
      float2 outsetTargets = float2(pixelOutsetTarget) / pixelsPerDimension;
      float2 outsets = min(outsetTargets, decodeMargins(labels));
      outsets.x = mix(outsetTargets.x, outsets.x,
                      saturate(4.0 * pixelsPerDimension.x - 1.0));
      float4 fromEdge = float4(1.0) - normalizedToEdgeLRFB;
      outsetPixelsLRFB =
          pixelOutsetTarget * fromEdge * (outsets / outsetTargets).xxyy;

      float2 sideOutset = sign(labels.x) * outsets.x * sideDerivative;
      float2 forwardOutset = sign(labels.y) * outsets.y * forwardDerivative;
      float commonForwardMagnitude =
          saturate(dot(sideOutset, forwardOutset) /
                   max(0.000001, dot(forwardOutset, forwardOutset)));

      return sideOutset + (1.0 - commonForwardMagnitude) * forwardOutset;
    })"

    // Calculates the texture UV coordinates that should be used for a
    // particular vertex of a winding-textured mesh.
    //   * `surfaceUv` is the unpacked surface UV mesh attribute at this vertex,
    //     where U measures lateral position across the stroke or particle, and
    //     V measures forward position along the stroke or particle.
    //   * `particleAnimationOffset` is the unpacked per-particle animation
    //     progress offset at this vertex, from [0, 1).
    //   * `textureAnimationProgress` is the animation progress value for the
    //     entire mesh, from [0, 1].
    //   * `numTextureAnimationFrames` is the number of animation frames in the
    //     texture, and must be at least 1. If greater than 1, then the texture
    //     is assumed to be an atlas that is divided vertically (along its V
    //     dimension) into this many equal-sized frames.
    R"(
    float2 calculateWindingTextureUv(
        const float2 surfaceUv,
        const float particleAnimationOffset,
        const float textureAnimationProgress,
        const int numTextureAnimationFrames,
        const int numTextureAnimationRows,
        const int numTextureAnimationColumns) {
      // Progress overshooting 1 is handled by wrapping around to 0. Effectively
      // progress is mod 1.
      float progress =
          fract(textureAnimationProgress + particleAnimationOffset);

      float numFrames = float(numTextureAnimationFrames);
      float numRows = float(numTextureAnimationRows);
      float numColumns = float(numTextureAnimationColumns);
      float frameIndex = floor(progress * numFrames);
      float frameRowFractional = frameIndex / numColumns;
      float frameRow = floor(frameRowFractional);
      float frameColumn = floor(fract(frameRowFractional) * numColumns);
      return vec2((surfaceUv.x + frameColumn) / numColumns,
                  (surfaceUv.y + frameRow) / numRows);
    })"

    // ------------------------------------------------------------------------
    // Reused unpacking functions for specific `MeshFormat::AttributeType`
    //
    // Ink attributes that get packed into a number of unsigned bytes are
    // expected to come from a Skia attribute using the `kUByte4_unorm` type.
    // This format expects four unsigned byte values from the CPU (each in the
    // range [0, 255]). The values are then loaded into the shader as four
    // 16-bit floats, each in the range [0, 1].

    // Unpacks a `float2` that was packed into a single `float` according to
    // `MeshFormat::AttributeType::kFloat2PackedInOneFloat`. The components of
    // `unpackingTransform` are expected to be:
    //     {x-offset, x-scale, y-offset, y-scale}
    R"(
    float2 unpackFloat2PackedIntoFloat(const float4 unpackingTransform,
                                       const float packedValue) {
      float2 unpacked = float2(floor(packedValue / 4096.0),
                               fract(packedValue / 4096.0) * 4096.0);
      return unpackingTransform.yw * unpacked + unpackingTransform.xz;
    })"

    // Unpacks a `float2` that was packed into three unsigned bytes according to
    // `MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12`. The
    // components of `unpackingTransform` are expected to be:
    //     {x-offset, x-scale, y-offset, y-scale}
    R"(
    float2 unpackFloat2PackedIntoUByte3(const float4 unpackingTransform,
                                        const half3 packedValue) {
      float mixedXY = 15.9375 * float(packedValue.y);
      float2 unpacked =
          float2(4080.0 * float(packedValue.x) + floor(mixedXY),
                 4096.0 * fract(mixedXY) + 255.0 * float(packedValue.z));
      return unpackingTransform.yw * unpacked + unpackingTransform.xz;
    })"

    // ------------------------------------------------------------------------
    // Unpacking functions for particular shader vertex attributes

    // Unpacks a combined position-and-opacity value into a `float3` from one of
    // the "packed" types supported.
    // LINT.IfChange(opacity_packing)
    R"(
    float3 unpackPositionAndOpacityShift(const float4 unpackingTransform,
                                         const float2 packedValue) {
      return float3(
          unpackFloat2PackedIntoFloat(unpackingTransform, packedValue.x),
          packedValue.y);
    }
    float3 unpackPositionAndOpacityShift(const float4 unpackingTransform,
                                         const half4 packedValue) {
      return float3(
          unpackFloat2PackedIntoUByte3(unpackingTransform, packedValue.xyz),
          (255.0 / 127.0) * packedValue.w - 1.0);
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:margin_encoding)

    // Unpacks a combined derivative-and-label value into a `float3` from one of
    // the supported "packed" types.
    // LINT.IfChange(label_packing)
    R"(
    float3 unpackDerivativeAndLabel(const float4 unusedUnpackingTransform,
                                    const float3 unpackedValue) {
      return unpackedValue;
    }
    float3 unpackDerivativeAndLabel(const float4 unpackingTransform,
                                    const half4 packedValue) {
      return float3(
          unpackFloat2PackedIntoUByte3(unpackingTransform, packedValue.xyz),
          255.0 * packedValue.w - 128.0);
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:label_packing)

    // Unpacks an HSL color-shift value into a `float3` from one of the
    // supported "packed" types.
    // LINT.IfChange(hsl_packing)
    R"(
    float3 unpackHSLColorShift(const float3 unpackedValue) {
      return unpackedValue;
    }
    float3 unpackHSLColorShift(const half4 packedValue) {
      float mixedXY = 3.984375 * float(packedValue.y);
      float mixedYZ = 15.9375 * float(packedValue.z);
      return float3(1020.0 * float(packedValue.x) + floor(mixedXY),
                    1024.0 * fract(mixedXY) + floor(mixedYZ),
                    1024.0 * fract(mixedYZ) + 63.75 * float(packedValue.w)) /
                 511.0 -
             float3(1.0);
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:hsl_packing)

    // Unpacks a surface UV value into a `float2` from one of the supported
    // "packed" types.
    // LINT.IfChange(uv_packing)
    R"(
    float2 unpackSurfaceUv(const float2 unpackedValue) {
      return unpackedValue;
    })"
    // The [0, 1] UV values can be packed into three bytes, using 12 bits each:
    // UUUUUUUU UUUUVVVV VVVVVVVV. Each of those bytes is exposed to the shader
    // as a [0, 1] half float, which makes the unpacking a bit confusing:
    //   * The first step is to multiply the mixed middle half float by 15 and
    //     15/16 = 15.9375, so that the upper four bits of the middle byte form
    //     the integer part of `mixedUV` (0 to 15), and the lower four bits form
    //     the fractional part (0/16 to 15/16).
    //   * To get the 12 bits for U, we multiply the first half float by 0xff0 =
    //     4080 and add the integer part of `mixedUV` (which ranges from 0x000
    //     to 0x00f) to get a value that ranges from 0 to 0xfff = 4095. Finally,
    //     we divide by 4095 to recover the [0, 1] U value.
    //   * To get the 12 bits for V, we multiply the fractional part of
    //     `mixedUV` (which ranges from 0/16 to 15/16) by 0x1000 = 4096 to get a
    //     value that ranges from 0x000 to 0xf00, then add 0x0ff = 255 times the
    //     last half float to get a sum that ranges from 0 to 0xfff =
    //     4095. Finally, we divide by 4095 to recover the [0, 1] V value.
    R"(
    float2 unpackSurfaceUv(const half3 packedValue) {
      float mixedUV = 15.9375 * float(packedValue.y);
      return float2(
          (4080.0 * float(packedValue.x) + floor(mixedUV)) / 4095.0,
          (4096.0 * fract(mixedUV) + 255.0 * float(packedValue.z)) / 4095.0);
    })"
    // The [0, 1] UV values can instead be packed into four bytes, using 12 bits
    // for U and 20 bits for V: UUUUUUUU UUUUVVVV VVVVVVVV VVVVVVVV. Each of
    // those bytes is exposed to the shader as a [0, 1] half float, which makes
    // the unpacking a bit confusing:
    //   * The first step is to multiply the mixed second half float by 15 and
    //     15/16 = 15.9375, so that the upper four bits of the second byte form
    //     the integer part of `mixedUV` (0 to 15), and the lower four bits form
    //     the fractional part (0/16 to 15/16).
    //   * To get the 12 bits for U, we multiply the first half float by 0xff0 =
    //     4080 and add the integer part of `mixedUV` (which ranges from 0x000
    //     to 0x00f) to get a value that ranges from 0 to 0xfff = 4095. Finally,
    //     we divide by 4095 to recover the [0, 1] U value.
    //   * To get the 20 bits for V, we multiply the fractional part of
    //     `mixedUV` (which ranges from 0/16 to 15/16) by 0x100000 = 1048576 to
    //     get a value that ranges from 0x00000 to 0xf0000, then add 0x0ff00 =
    //     65280 times the third half float and 0x000ff = 255 times the last
    //     half float to get a sum that ranges from 0 to 0xfffff =
    //     1048575. Finally, we divide by 1048575 to recover the [0, 1] V value.
    R"(
    float2 unpackSurfaceUv(const half4 packedValue) {
      float mixedXY = 15.9375 * float(packedValue.y);
      return float2((4080.0 * float(packedValue.x) + floor(mixedXY)) / 4095.0,
                    (1048576.0 * fract(mixedXY) +
                     65280.0 * float(packedValue.z) +
                     255.0 * float(packedValue.w)) / 1048575.0);
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:uv_packing)

    // Unpacks an animation offset value into a `float` from one of the
    // supported "packed" types.
    // LINT.IfChange(anim_packing)
    R"(
    float unpackAnimationOffset(const float unpackedValue) {
      return unpackedValue;
    })"
    // A [0, 1) animation offset can be packed into one [0, 256) byte, where 0.0
    // maps to 0 and 1.0 would map to 256. This [0, 255] byte is exposed to the
    // shader as a [0, 1] half float. So to unpack, we multiply the [0, 1] half
    // float by 255/256 to get our [0, 1) animation offset.
    R"(
    float unpackAnimationOffset(const half packedValue) {
      return 255.0 * float(packedValue) / 256.0;
    })"
    // LINT.ThenChange(
    //     ../../../strokes/internal/stroke_vertex.cc:uv_packing)
    "";

}  // namespace ink::skia_common_internal

#endif  // INK_RENDERING_SKIA_COMMON_INTERNAL_SKSL_VERTEX_SHADER_HELPER_FUNCTIONS_H_
