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

#ifndef INK_COLOR_COLOR_SPACE_H_
#define INK_COLOR_COLOR_SPACE_H_

#include <array>
#include <string>

namespace ink {

// A color space, which gives concrete meaning to raw color channel values.
//
// This should match the platform enum in ColorExtensions.kt.
enum class ColorSpace {
  // sRGB.
  kSrgb,

  // Display P3, which uses the DCI-P3 primaries and sRGB transfer function.
  kDisplayP3
};

// Decodes a nonlinear perceptual lightness value to a linear luminance using
// a color space's "gamma" function. This operation is also called the EOTF:
// electro-optical transfer function. Most callers should use
// Color::FromFloat() instead.
float GammaDecode(float encoded_value, ColorSpace space);

// Encodes a linear luminance value to a nonlinear perceptual lightness using
// a color space's "gamma" function. This operation is also called the OETF:
// opto-electronic transfer function. Most callers should use Color::AsFloat()
// instead.
float GammaEncode(float linear_value, ColorSpace space);

// Converts linear, non-premultiplied RGBA coordinates color space `source` to
// linear, non-premultiplied coordinates for the same color in `target`. Most
// callers should use Color::InColorSpace() instead.
std::array<float, 4> ConvertColor(
    const std::array<float, 4>& rgba_linear_nonpremultiplied, ColorSpace source,
    ColorSpace target);

// For external implementations (e.g., shaders) only. Prefer calling
// GammaDecode() or GammaEncode().
//
// Returns the parameters for this color space's EOTF (gamma decoding
// function) as an ICC type-3 parametric curve, as defined in ICC.1:2004-10,
// section 10.15.
std::array<float, 5> GetGammaDecodingParameters(ColorSpace space);

enum class MatrixLayout { kColumnMajor, kRowMajor };

// For external implementations (e.g., shaders) only. Prefer calling
// ConvertColor().
//
// Returns the matrix, in column-major format, that converts from color
// coordinates in this color space to coordinates in the XYZ space with the
// D65 white point.
std::array<float, 9> GetColorSpaceToXyzD65Matrix(ColorSpace space,
                                                 MatrixLayout layout);

// For external implementations (e.g., shaders) only. Prefer calling
// ConvertColor().
//
// Returns the matrix, in column-major format, that converts from color
// coordinates in the XYZ space with the D65 white point to coordinates in
// this color space.
std::array<float, 9> GetXyzD65ToColorSpaceMatrix(ColorSpace space,
                                                 MatrixLayout layout);

std::string ToFormattedString(ColorSpace space);

template <typename Sink>
void AbslStringify(Sink& sink, ColorSpace space) {
  sink.Append(ToFormattedString(space));
}

}  // namespace ink

#endif  // INK_COLOR_COLOR_SPACE_H_
