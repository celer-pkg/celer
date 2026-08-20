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

#include "ink/color/color_space.h"

#include <array>
#include <cmath>
#include <string>

#include "absl/log/absl_log.h"
#include "absl/strings/str_format.h"

namespace ink {
namespace {

// Parameters that define the sRGB EOTF (decoding function), using the ICC
// type-3 curve parameterization.
//
// EOTF_sRGB(J) = { J / 12.92                 if J <= 0.04045;
//                  ((J + 0.055) / 1.055)^2.4 if J >  0.04045 }
//
// See https://color.org/chardata/rgb/sRGB.pdf and
// https://color.org/chardata/rgb/srgb.xalter.
constexpr double kSrgbA = 1.0 / 1.055;
constexpr double kSrgbB = 0.055 / 1.055;
constexpr double kSrgbC = 1.0 / 12.92;
constexpr double kSrgbD = 0.04045;
constexpr double kSrgbG = 2.4;

// The matrices below all have the following properties:
// - They convert between an RGB color space and XYZ-D65, or vice-versa.
// - They act on, and produce, linear coordinates in the named color spaces.
// - They are relative to the D65 white point; i.e., they have not been
//   chromatically adapted to D50 as is conventional for XYZ.
// - Their entries are laid out in column-major format; therefore, they appear
//   transposed in the literals below.
// These values were derived by converting the calculations in the open-source
// android.graphics.ColorSpace.Rgb into Numpy, and then executing them in high
// precision on the widely-published CIE xyz coordinates of the sRGB primaries,
// Display P3 primaries, and D65 white point.
constexpr std::array<double, 9> kSrgbToXyzD65 = {
    0.4123865632529916, 0.2126368216773238, 0.0193306201524840,  // Column 0
    0.3575914909206254, 0.7151829818412507, 0.1191971636402085,  // Column 1
    0.1804504912035637, 0.0721801964814255, 0.9503725870054357};
constexpr std::array<double, 9> kXyzD65ToSrgb = {
    3.2410032329763610,  -0.9692242522025170, +0.0556394198519755,  // Column 0
    -1.5373989694887868, +1.8759299836951764, -0.2040112061239100,  // Column 1
    -0.4986158819963633, +0.0415542263400848, +1.0571489771875330};
constexpr std::array<double, 9> kDisplayP3ToXyzD65 = {
    0.4865685656607022, 0.2289734426638599, 0.0000000000000000,  // Column 0
    0.2656727394591274, 0.6917516612331996, 0.0451142387760782,  // Column 1
    0.1981872402573512, 0.0792748961029405, 1.0437861320220500};
constexpr std::array<double, 9> kXyzD65ToDisplayP3 = {
    +2.4935091239346090, -0.8294732139295545, +0.0358512644339180,  // Column 0
    -0.9313881794047785, +1.7626305796003034, -0.0761839369220757,  // Column 1
    -0.4027127567416515, +0.0236242371055886, +0.9570295866943107};

std::array<double, 9> GetFromXyzD65(ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
      return kXyzD65ToSrgb;
    case ColorSpace::kDisplayP3:
      return kXyzD65ToDisplayP3;
  }
  ABSL_LOG(DFATAL) << "Unknown ColorSpace enum value "
                   << static_cast<int>(space);
  return {};
}

std::array<double, 9> GetToXyzD65(ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
      return kSrgbToXyzD65;
    case ColorSpace::kDisplayP3:
      return kDisplayP3ToXyzD65;
  }
  ABSL_LOG(DFATAL) << "Unknown ColorSpace enum value "
                   << static_cast<int>(space);
  return {};
}

}  // namespace

float GammaDecode(float encoded_value, ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
    case ColorSpace::kDisplayP3: {
      // sRGB and Display P3 use the same gamma curve: the sRGB curve.
      if (encoded_value < kSrgbD) {
        return kSrgbC * encoded_value;
      }
      return std::pow(kSrgbA * encoded_value + kSrgbB, kSrgbG);
    }
  }
  ABSL_LOG(DFATAL) << "Unknown ColorSpace enum value "
                   << static_cast<int>(space);
  return 0.0f;
}

float GammaEncode(float linear_value, ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
    case ColorSpace::kDisplayP3: {
      // sRGB and Display P3 use the same gamma curve: the sRGB curve.
      if (linear_value < kSrgbC * kSrgbD) {
        return linear_value / kSrgbC;
      }
      return (std::pow(linear_value, 1.0 / kSrgbG) - kSrgbB) / kSrgbA;
    }
  }
  ABSL_LOG(DFATAL) << "Unknown ColorSpace enum value "
                   << static_cast<int>(space);
  return 0.0f;
}

namespace {

// Multiplies two 3x3 matrices. a, b, and result are column-major.
std::array<double, 9> MultMat3ByMat3(const std::array<double, 9>& a,
                                     const std::array<double, 9>& b) {
  return {a[0] * b[0] + a[3] * b[1] + a[6] * b[2],
          a[1] * b[0] + a[4] * b[1] + a[7] * b[2],
          a[2] * b[0] + a[5] * b[1] + a[8] * b[2],
          a[0] * b[3] + a[3] * b[4] + a[6] * b[5],
          a[1] * b[3] + a[4] * b[4] + a[7] * b[5],
          a[2] * b[3] + a[5] * b[4] + a[8] * b[5],
          a[0] * b[6] + a[3] * b[7] + a[6] * b[8],
          a[1] * b[6] + a[4] * b[7] + a[7] * b[8],
          a[2] * b[6] + a[5] * b[7] + a[8] * b[8]};
}

// Multiplies a 3x3 matrix by the first 3 terms of a 4-vector. The fourth term
// is appended, unchanged, to the result. m is column-major.
std::array<float, 4> MultMat3ByVec4(const std::array<double, 9>& m,
                                    const std::array<float, 4>& v) {
  return {static_cast<float>(m[0] * v[0] + m[3] * v[1] + m[6] * v[2]),
          static_cast<float>(m[1] * v[0] + m[4] * v[1] + m[7] * v[2]),
          static_cast<float>(m[2] * v[0] + m[5] * v[1] + m[8] * v[2]), v[3]};
}

constexpr std::array<int, 9> kRowMajorToColumnMajorIndexMap = {0, 3, 6, 1, 4,
                                                               7, 2, 5, 8};

}  // namespace

std::array<float, 4> ConvertColor(
    const std::array<float, 4>& rgba_linear_nonpremultiplied, ColorSpace source,
    ColorSpace target) {
  if (source == target) {
    return rgba_linear_nonpremultiplied;
  }
  const std::array<double, 9> conversion_matrix =
      MultMat3ByMat3(GetFromXyzD65(target), GetToXyzD65(source));
  return MultMat3ByVec4(conversion_matrix, rgba_linear_nonpremultiplied);
}

std::array<float, 5> GetGammaDecodingParameters(ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
    case ColorSpace::kDisplayP3: {
      // sRGB and Display P3 use the same gamma curve: the sRGB curve.
      return {kSrgbA, kSrgbB, kSrgbC, kSrgbD, kSrgbG};
    }
  }
  ABSL_LOG(DFATAL) << "Unknown ColorSpace enum value " << space;
  return {};
}

std::array<float, 9> GetColorSpaceToXyzD65Matrix(ColorSpace space,
                                                 MatrixLayout layout) {
  std::array<double, 9> high_precision_col_major = GetToXyzD65(space);
  std::array<float, 9> output;
  switch (layout) {
    case MatrixLayout::kColumnMajor: {
      for (int i = 0; i < 9; ++i) {
        output[i] = static_cast<float>(high_precision_col_major[i]);
      }
      break;
    }
    case MatrixLayout::kRowMajor: {
      for (int i = 0; i < 9; ++i) {
        output[i] = static_cast<float>(
            high_precision_col_major[kRowMajorToColumnMajorIndexMap[i]]);
      }
      break;
    }
  }
  return output;
}

std::array<float, 9> GetXyzD65ToColorSpaceMatrix(ColorSpace space,
                                                 MatrixLayout layout) {
  std::array<double, 9> high_precision_col_major = GetFromXyzD65(space);
  std::array<float, 9> output;
  switch (layout) {
    case MatrixLayout::kColumnMajor: {
      for (int i = 0; i < 9; ++i) {
        output[i] = static_cast<float>(high_precision_col_major[i]);
      }
      break;
    }
    case MatrixLayout::kRowMajor: {
      for (int i = 0; i < 9; ++i) {
        output[i] = static_cast<float>(
            high_precision_col_major[kRowMajorToColumnMajorIndexMap[i]]);
      }
      break;
    }
  }
  return output;
}

std::string ToFormattedString(ColorSpace space) {
  switch (space) {
    case ColorSpace::kSrgb:
      return "sRGB";
    case ColorSpace::kDisplayP3:
      return "Display-P3";
  }
  return absl::StrFormat("[unknown ColorSpace %d]", static_cast<int>(space));
}

}  // namespace ink
