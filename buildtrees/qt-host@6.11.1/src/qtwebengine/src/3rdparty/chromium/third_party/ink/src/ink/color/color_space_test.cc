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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "ink/color/fuzz_domains.h"
#include "ink/color/type_matchers.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ContainsRegex;
using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::FloatNear;
using ::testing::Ge;
using ::testing::Lt;
using ::testing::Ne;
using ::testing::StrEq;

MATCHER(IsFinite, "") { return std::isfinite(arg); }

// Takes two column-major 3x3 matrices, a and b, and returns their product.
std::array<float, 9> MultMat3ByMat3(std::array<float, 9> a,
                                    std::array<float, 9> b) {
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

void SrgbGammaEncodeDecodeAreInverses(float x) {
  EXPECT_THAT(GammaDecode(GammaEncode(x, ColorSpace::kSrgb), ColorSpace::kSrgb),
              FloatNear(x, kColorTestEps));
  EXPECT_THAT(GammaEncode(GammaDecode(x, ColorSpace::kSrgb), ColorSpace::kSrgb),
              FloatNear(x, kColorTestEps));
}
FUZZ_TEST(ColorSpaceTest, SrgbGammaEncodeDecodeAreInverses)
    .WithDomains(fuzztest::InRange(0.0f, 1.0f));

// The sRGB "gamma curve" for decoding is supposed to be similar to y = x^2.2,
// which is less than y = x over the open domain (0, 1).
void SrgbGammaDecodedValuesAreLower(float x) {
  EXPECT_THAT(GammaDecode(x, ColorSpace::kSrgb), Lt(x));
}
FUZZ_TEST(ColorSpaceTest, SrgbGammaDecodedValuesAreLower)
    .WithDomains(fuzztest::InRange(0.0001f, 0.9999f));

TEST(ColorSpaceTest, SrgbGammaIsIdentityAtZeroAndOne) {
  EXPECT_THAT(GammaDecode(0.0f, ColorSpace::kSrgb), Eq(0.0f));
  EXPECT_THAT(GammaEncode(0.0f, ColorSpace::kSrgb), Eq(0.0f));
  EXPECT_THAT(GammaDecode(1.0f, ColorSpace::kSrgb), Eq(1.0f));
  EXPECT_THAT(GammaEncode(1.0f, ColorSpace::kSrgb), Eq(1.0f));
}

void DisplayP3UsesSameTransferFunctionAsSrgb(float x) {
  EXPECT_THAT(GammaDecode(x, ColorSpace::kDisplayP3),
              Eq(GammaDecode(x, ColorSpace::kSrgb)));
  EXPECT_THAT(GammaEncode(x, ColorSpace::kDisplayP3),
              Eq(GammaEncode(x, ColorSpace::kSrgb)));
}
FUZZ_TEST(ColorSpaceTest, DisplayP3UsesSameTransferFunctionAsSrgb)
    .WithDomains(fuzztest::InRange(0.0f, 1.0f));

TEST(ColorSpaceDeathTest, GammaDecodeWithOutOfRangeEnumValueDies) {
  // TODO: b/173787033 - Simplify once --config=wasm supports death tests.
#ifdef GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(GammaDecode(0., static_cast<ColorSpace>(-1)),
                     "Unknown ColorSpace");
#else
  GTEST_SKIP() << "This build config does not support death tests.";
#endif  // GTEST_HAS_DEATH_TEST
}

TEST(ColorSpaceDeathTest, GammaEncodeWithOutOfRangeEnumValueDies) {
  // TODO: b/173787033 - Simplify once --config=wasm supports death tests.
#ifdef GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(GammaEncode(0.5, static_cast<ColorSpace>(-1)),
                     "Unknown ColorSpace");
#else
  GTEST_SKIP() << "This build config does not support death tests.";
#endif  // GTEST_HAS_DEATH_TEST
}

TEST(ColorSpaceTest, SrgbConversionMatricesAreInverses) {
  auto to_xyz = GetColorSpaceToXyzD65Matrix(ColorSpace::kSrgb,
                                            MatrixLayout::kColumnMajor);
  auto from_xyz = GetXyzD65ToColorSpaceMatrix(ColorSpace::kSrgb,
                                              MatrixLayout::kColumnMajor);
  EXPECT_THAT(MultMat3ByMat3(to_xyz, from_xyz),
              NearIdentityMatrix(kColorTestEps));
  EXPECT_THAT(MultMat3ByMat3(from_xyz, to_xyz),
              NearIdentityMatrix(kColorTestEps));
}

TEST(ColorSpaceTest, DisplayP3ConversionMatricesAreInverses) {
  auto to_xyz = GetColorSpaceToXyzD65Matrix(ColorSpace::kDisplayP3,
                                            MatrixLayout::kColumnMajor);
  auto from_xyz = GetXyzD65ToColorSpaceMatrix(ColorSpace::kDisplayP3,
                                              MatrixLayout::kColumnMajor);
  EXPECT_THAT(MultMat3ByMat3(to_xyz, from_xyz),
              NearIdentityMatrix(kColorTestEps));
  EXPECT_THAT(MultMat3ByMat3(from_xyz, to_xyz),
              NearIdentityMatrix(kColorTestEps));
}

void ConvertSrgbToSrgbIsIdentity(const std::array<float, 4>& rgba) {
  // In this case, conversion is exact, not approximate.
  EXPECT_THAT(ConvertColor(rgba, ColorSpace::kSrgb, ColorSpace::kSrgb),
              ElementsAreArray(rgba));
}
FUZZ_TEST(ColorSpaceTest, ConvertSrgbToSrgbIsIdentity)
    .WithDomains(FourFloatsInZeroOne());

void ConvertDisplayP3ToDisplayP3IsIdentity(const std::array<float, 4>& rgba) {
  // In this case, conversion is exact, not approximate.
  EXPECT_THAT(
      ConvertColor(rgba, ColorSpace::kDisplayP3, ColorSpace::kDisplayP3),
      ElementsAreArray(rgba));
}
FUZZ_TEST(ColorSpaceTest, ConvertDisplayP3ToDisplayP3IsIdentity)
    .WithDomains(FourFloatsInZeroOne());

void SrgbToDisplayP3AndBackIsIdentity(const std::array<float, 4>& rgba) {
  EXPECT_THAT(ConvertColor(
                  ConvertColor(rgba, ColorSpace::kSrgb, ColorSpace::kDisplayP3),
                  ColorSpace::kDisplayP3, ColorSpace::kSrgb),
              Vec4Near(rgba, kColorTestEps));
}
FUZZ_TEST(ColorSpaceTest, SrgbToDisplayP3AndBackIsIdentity)
    .WithDomains(FourFloatsInZeroOne());

void DisplayP3ToSrgbAndBackIsIdentity(const std::array<float, 4>& rgba) {
  EXPECT_THAT(ConvertColor(
                  ConvertColor(rgba, ColorSpace::kDisplayP3, ColorSpace::kSrgb),
                  ColorSpace::kSrgb, ColorSpace::kDisplayP3),
              Vec4Near(rgba, kColorTestEps));
}
FUZZ_TEST(ColorSpaceTest, DisplayP3ToSrgbAndBackIsIdentity)
    .WithDomains(FourFloatsInZeroOne());

void SrgbColorsAreInsideDisplayP3Gamut(const std::array<float, 4>& rgba) {
  EXPECT_THAT(
      ConvertColor(rgba, ColorSpace::kSrgb, ColorSpace::kDisplayP3),
      ElementsAre(FloatNearlyBetweenZeroAndOne(kColorTestEps),
                  FloatNearlyBetweenZeroAndOne(kColorTestEps),
                  FloatNearlyBetweenZeroAndOne(kColorTestEps), Eq(rgba[3])));
}
FUZZ_TEST(ColorSpaceTest, SrgbColorsAreInsideDisplayP3Gamut)
    .WithDomains(FourFloatsInZeroOne());

TEST(ColorSpaceTest, WhiteIsTheSameInSrgbAndDisplayP3) {
  EXPECT_THAT(
      ConvertColor({1, 1, 1, 1}, ColorSpace::kSrgb, ColorSpace::kDisplayP3),
      Vec4Near({1, 1, 1, 1}, kColorTestEps));
  EXPECT_THAT(
      ConvertColor({1, 1, 1, 1}, ColorSpace::kDisplayP3, ColorSpace::kSrgb),
      Vec4Near({1, 1, 1, 1}, kColorTestEps));
}

TEST(ColorSpaceTest, BlackIsTheSameInSrgbAndDisplayP3) {
  EXPECT_THAT(
      ConvertColor({0, 0, 0, 0}, ColorSpace::kSrgb, ColorSpace::kDisplayP3),
      Vec4Near({0, 0, 0, 0}, kColorTestEps));
  EXPECT_THAT(
      ConvertColor({0, 0, 0, 0}, ColorSpace::kDisplayP3, ColorSpace::kSrgb),
      Vec4Near({0, 0, 0, 0}, kColorTestEps));
}

TEST(ColorSpaceDeathTest, ConvertColorToOutOfRangeEnumValueDies) {
  // TODO: b/173787033 - Simplify once --config=wasm supports death tests.
#ifdef GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(ConvertColor({0.5, 0.5, 0.5, 0.5}, ColorSpace::kSrgb,
                                  static_cast<ColorSpace>(-1)),
                     "Unknown ColorSpace");
#else
  GTEST_SKIP() << "This build config does not support death tests.";
#endif  // GTEST_HAS_DEATH_TEST
}

TEST(ColorSpaceDeathTest, ConvertColorFromOutOfRangeEnumValueDies) {
  // TODO: b/173787033 - Simplify once --config=wasm supports death tests.
#ifdef GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(
      ConvertColor({0.5, 0.5, 0.5, 0.5}, static_cast<ColorSpace>(-1),
                   ColorSpace::kSrgb),
      "Unknown ColorSpace");
#else
  GTEST_SKIP() << "This build config does not support death tests.";
#endif  // GTEST_HAS_DEATH_TEST
}

TEST(ColorSpaceTest, DecodingParametersAreNontrivial) {
  EXPECT_THAT(GetGammaDecodingParameters(ColorSpace::kSrgb),
              AllOf(Each(IsFinite()), Contains(Ne(0.0f))));
}

TEST(ColorSpaceDeathTest, GettingDecodingParametersForOutOfRangeEnumValueDies) {
  // TODO: b/173787033 - Simplify once --config=wasm supports death tests.
#ifdef GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(GetGammaDecodingParameters(static_cast<ColorSpace>(-1)),
                     "Unknown ColorSpace");
#else
  GTEST_SKIP() << "This build config does not support death tests.";
#endif  // GTEST_HAS_DEATH_TEST
}

TEST(ColorSpaceTest, MatrixToXyzIsNontrivial) {
  EXPECT_THAT(GetColorSpaceToXyzD65Matrix(ColorSpace::kSrgb,
                                          MatrixLayout::kColumnMajor),
              AllOf(Each(IsFinite()), Contains(Ne(0.0f)).Times(Ge(3))));
  EXPECT_THAT(GetColorSpaceToXyzD65Matrix(ColorSpace::kDisplayP3,
                                          MatrixLayout::kColumnMajor),
              AllOf(Each(IsFinite()), Contains(Ne(0.0f)).Times(Ge(3))));
}

TEST(ColorSpaceTest, MatrixFromXyzIsNontrivial) {
  EXPECT_THAT(GetXyzD65ToColorSpaceMatrix(ColorSpace::kSrgb,
                                          MatrixLayout::kColumnMajor),
              AllOf(Each(IsFinite()), Contains(Ne(0.0f)).Times(Ge(3))));
  EXPECT_THAT(GetXyzD65ToColorSpaceMatrix(ColorSpace::kDisplayP3,
                                          MatrixLayout::kColumnMajor),
              AllOf(Each(IsFinite()), Contains(Ne(0.0f)).Times(Ge(3))));
}

TEST(ColorSpaceTest, MatrixToXyzRowAndColumnMajorAreConsistent) {
  const auto col_major = GetColorSpaceToXyzD65Matrix(
      ColorSpace::kSrgb, MatrixLayout::kColumnMajor);
  const auto row_major =
      GetColorSpaceToXyzD65Matrix(ColorSpace::kSrgb, MatrixLayout::kRowMajor);
  EXPECT_THAT(col_major, ElementsAre(row_major[0], row_major[3], row_major[6],
                                     row_major[1], row_major[4], row_major[7],
                                     row_major[2], row_major[5], row_major[8]));
  EXPECT_THAT(row_major, ElementsAre(col_major[0], col_major[3], col_major[6],
                                     col_major[1], col_major[4], col_major[7],
                                     col_major[2], col_major[5], col_major[8]));
}

TEST(ColorSpaceTest, MatrixFromXyzRowAndColumnMajorAreConsistent) {
  const auto col_major = GetXyzD65ToColorSpaceMatrix(
      ColorSpace::kSrgb, MatrixLayout::kColumnMajor);
  const auto row_major =
      GetXyzD65ToColorSpaceMatrix(ColorSpace::kSrgb, MatrixLayout::kRowMajor);
  EXPECT_THAT(col_major, ElementsAre(row_major[0], row_major[3], row_major[6],
                                     row_major[1], row_major[4], row_major[7],
                                     row_major[2], row_major[5], row_major[8]));
  EXPECT_THAT(row_major, ElementsAre(col_major[0], col_major[3], col_major[6],
                                     col_major[1], col_major[4], col_major[7],
                                     col_major[2], col_major[5], col_major[8]));
}

TEST(ColorSpaceTest, Stringify) {
  EXPECT_THAT(absl::StrCat(ColorSpace::kSrgb), StrEq("sRGB"));
  EXPECT_THAT(absl::StrCat(ColorSpace::kDisplayP3), StrEq("Display-P3"));
  EXPECT_THAT(absl::StrCat(static_cast<ColorSpace>(-1)),
              ContainsRegex("unknown"));
}

}  // namespace
}  // namespace ink
