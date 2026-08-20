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

#include "ink/color/color.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "ink/color/color_space.h"
#include "ink/color/fuzz_domains.h"
#include "ink/color/type_matchers.h"

namespace ink {
namespace {

using Format = ::ink::Color::Format;
using ::testing::ContainsRegex;
using ::testing::Eq;
using ::testing::MatchesRegex;
using ::testing::Not;
using ::testing::StrEq;

constexpr float kNan = std::numeric_limits<float>::quiet_NaN();

// The epsilon used for "nearly" comparisons in Color. Use this constant for
// creating inputs to tests, not as a tolerance for approximate matches.
constexpr float kColorNearlyEps = 7.629e-6;

constexpr Color::Format kAllFormats[] = {Format::kLinear, Format::kGammaEncoded,
                                         Format::kPremultipliedAlpha};
constexpr ColorSpace kAllColorSpaces[] = {ColorSpace::kSrgb,
                                          ColorSpace::kDisplayP3};

void IdenticalFromFloatsCompareEqual(const std::array<float, 4>& rgba) {
  for (auto format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha &&
        (rgba[3] <= 0.0f || std::isnan(rgba[3]))) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], format,
                                   color_space),
                  Eq(Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                                      format, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, IdenticalFromFloatsCompareEqual);

void NansGoToZeroAndAlphaIsClamped(const std::array<float, 4>& rgba) {
  std::array<float, 4> expected_equivalent_rgba = rgba;
  for (float& c : expected_equivalent_rgba) {
    if (std::isnan(c)) {
      c = 0.0f;
    }
  }
  expected_equivalent_rgba[3] =
      std::clamp(expected_equivalent_rgba[3], 0.0f, 1.0f);
  for (auto format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha &&
        (rgba[3] <= 0.0f || std::isnan(rgba[3]))) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], format,
                                   color_space),
                  Eq(Color::FromFloat(
                      expected_equivalent_rgba[0], expected_equivalent_rgba[1],
                      expected_equivalent_rgba[2], expected_equivalent_rgba[3],
                      format, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, NansGoToZeroAndAlphaIsClamped);

TEST(ColorTest, FromFloatAllZerosAlwaysMeansTransparentBlack) {
  for (auto format : kAllFormats) {
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromFloat(0.0f, 0.0f, 0.0f, 0.0f, format, color_space),
                  Eq(Color::FromFloat(0.0f, 0.0f, 0.0f, 0.0f, Format::kLinear,
                                      color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}

void ExpectFromFloatBackToFloatIsNear(
    const std::array<float, 4>& rgba,
    const std::array<float, 4>& expected_rgba) {
  for (Color::Format format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha &&
        (rgba[3] <= 0 || std::isnan(rgba[3]))) {
      // This is expected to crash; see the death tests.
      return;
    }

    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], format,
                                   color_space)
                      .AsFloat(format),
                  ChannelStructNear(expected_rgba, kColorTestEps))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}

void InGamutFromFloatBackToFloatIsIdentity(const std::array<float, 4>& rgba) {
  ExpectFromFloatBackToFloatIsNear(rgba, rgba);
}
FUZZ_TEST(ColorTest, InGamutFromFloatBackToFloatIsIdentity)
    .WithDomains(FourFloatsInZeroOne());

void OutOfRangeFromFloatBackToFloatIsIdentityAfterClampingAlpha(
    const std::array<float, 4>& rgba) {
  // Alpha will not round-trip; it gets clamped to [0, 1].
  std::array<float, 4> expected_rgba = rgba;
  expected_rgba[3] = std::clamp(expected_rgba[3], 0.0f, 1.0f);

  ExpectFromFloatBackToFloatIsNear(rgba, expected_rgba);
}
// Inputs are not guaranteed to be in-gamut, so we want to check out-of-gamut
// inputs (and those with alpha outside [0, 1]). Realistic ones are within the
// color gamut of human vision, at least, so we check a slightly larger region,
// [-5, 5]. Outside of this, we won't necessarily get a precise (within
// kColorTestEps) round-trip for kGammaEncoded or kPremultipliedAlpha, due to
// precision loss with conversion to and from linear values.
FUZZ_TEST(ColorTest, OutOfRangeFromFloatBackToFloatIsIdentityAfterClampingAlpha)
    .WithDomains(FourFloatsWithAbsoluteValueAtMost(5));

void FromFloatClampsNegativeAlphaToZero(float alpha) {
  for (auto color_space : kAllColorSpaces) {
    EXPECT_THAT(
        Color::FromFloat(0.5, 0.4, 0.3, alpha, Format::kLinear, color_space)
            .AsFloat(Format::kLinear),
        ChannelStructNear({0.5f, 0.4f, 0.3f, 0.0f}, kColorTestEps));
    EXPECT_THAT(Color::FromFloat(0.5, 0.4, 0.3, alpha, Format::kGammaEncoded,
                                 color_space)
                    .AsFloat(Format::kGammaEncoded),
                ChannelStructNear({0.5f, 0.4f, 0.3f, 0.0f}, kColorTestEps));
    // For premultiplied alpha=0, all color channels must be zero too.
    EXPECT_THAT(Color::FromFloat(0.0, 0.0, 0.0, alpha,
                                 Format::kPremultipliedAlpha, color_space)
                    .AsFloat(Format::kPremultipliedAlpha),
                ChannelStructNear({0.0f, 0.0f, 0.0f, 0.0f}, kColorTestEps));
  }
}
FUZZ_TEST(ColorTest, FromFloatClampsNegativeAlphaToZero)
    .WithDomains(fuzztest::NonPositive<float>());

TEST(ColorTest, FromFloatTreatsNanAlphaAsZero) {
  for (auto color_space : kAllColorSpaces) {
    EXPECT_THAT(
        Color::FromFloat(0.5, 0.4, 0.3, kNan, Format::kLinear, color_space)
            .AsFloat(Format::kLinear),
        ChannelStructNear({0.5f, 0.4f, 0.3f, 0.0f}, kColorTestEps));
    EXPECT_THAT(Color::FromFloat(0.5, 0.4, 0.3, kNan, Format::kGammaEncoded,
                                 color_space)
                    .AsFloat(Format::kGammaEncoded),
                ChannelStructNear({0.5f, 0.4f, 0.3f, 0.0f}, kColorTestEps));
    // For premultiplied alpha=0, all color channels must be zero too.
    EXPECT_THAT(Color::FromFloat(0.0, 0.0, 0.0, kNan,
                                 Format::kPremultipliedAlpha, color_space)
                    .AsFloat(Format::kPremultipliedAlpha),
                ChannelStructNear({0.0f, 0.0f, 0.0f, 0.0f}, kColorTestEps));
  }
}

TEST(ColorDeathTest, FromFloatAlphaZeroPremultipliedNonZeroColorsDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      Color::FromFloat(0.0f, 0.4f, 0.0f, 0.0f, Format::kPremultipliedAlpha),
      "[Pp]remultiplied.*alpha=0.*RGB=0");
}

TEST(ColorDeathTest, FromFloatAlphaNegativePremultipliedNonZeroColorsDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      Color::FromFloat(0.0f, 0.4f, 0.0f, -0.5f, Format::kPremultipliedAlpha),
      "[Pp]remultiplied.*alpha=0.*RGB=0");
}

TEST(ColorDeathTest, FromFloatAlphaNanPremultipliedNonZeroColorsDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      Color::FromFloat(0.0f, 0.4f, 0.0f, kNan, Format::kPremultipliedAlpha),
      "[Pp]remultiplied.*alpha=0.*RGB=0");
}

void FromFloatClampsAlphaGreaterThanOne(float alpha) {
  for (auto color_space : kAllColorSpaces) {
    EXPECT_THAT(
        Color::FromFloat(0.5, 0.4, 0.3, alpha, Format::kLinear, color_space)
            .AsFloat(Format::kLinear),
        ChannelStructNear({0.5f, 0.4f, 0.3f, 1.0f}, kColorTestEps));
    EXPECT_THAT(Color::FromFloat(0.5, 0.4, 0.3, alpha, Format::kGammaEncoded,
                                 color_space)
                    .AsFloat(Format::kGammaEncoded),
                ChannelStructNear({0.5f, 0.4f, 0.3f, 1.0f}, kColorTestEps));
    EXPECT_THAT(Color::FromFloat(0.5, 0.4, 0.3, alpha,
                                 Format::kPremultipliedAlpha, color_space)
                    .AsFloat(Format::kPremultipliedAlpha),
                ChannelStructNear({0.5f, 0.4f, 0.3f, 1.0f}, kColorTestEps));
  }
}
FUZZ_TEST(ColorTest, FromFloatClampsAlphaGreaterThanOne)
    .WithDomains(fuzztest::Filter([](float alpha) { return alpha >= 1.0f; },
                                  fuzztest::Positive<float>()));

void FromFloatAndAsFloatAcceptAnyValues(const std::array<float, 4>& rgba) {
  for (auto from_format : kAllFormats) {
    if (from_format == Format::kPremultipliedAlpha &&
        (rgba[3] <= 0 || std::isnan(rgba[3]))) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto to_format : kAllFormats) {
      for (auto color_space : kAllColorSpaces) {
        // No expectation to test here; just asserting this doesn't crash.
        Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], from_format,
                         color_space)
            .AsFloat(to_format);
      }
    }
  }
}
FUZZ_TEST(ColorTest, FromFloatAndAsFloatAcceptAnyValues);

void AsFloatPremultipliedAlphaZeroIsAllZeros(const std::array<float, 3>& rgb) {
  EXPECT_THAT(Color::FromFloat(rgb[0], rgb[1], rgb[2], 0, Format::kLinear)
                  .AsFloat(Format::kPremultipliedAlpha),
              ChannelStructEqFloats({0.0f, 0.0f, 0.0f, 0.0f}));
}
FUZZ_TEST(ColorTest, AsFloatPremultipliedAlphaZeroIsAllZeros);

void FormatDoesNotAffectAlphaValue(float alpha) {
  for (auto color_space : kAllColorSpaces) {
    for (auto from_format : kAllFormats) {
      for (auto to_format : kAllFormats) {
        EXPECT_THAT(Color::FromFloat(0, 0, 0, alpha, from_format, color_space)
                        .AsFloat(to_format)
                        .a,
                    Eq(alpha));
      }
    }
  }
}
FUZZ_TEST(ColorTest, FormatDoesNotAffectAlphaValue)
    .WithDomains(fuzztest::InRange(0.0f, 1.0f));

void GetAlphaFloatIsFormatAgnostic(const Color& color) {
  for (Color::Format format : kAllFormats) {
    EXPECT_EQ(color.GetAlphaFloat(), color.AsFloat(format).a);
  }
}
FUZZ_TEST(ColorTest, GetAlphaFloatIsFormatAgnostic)
    .WithDomains(ArbitraryColor());

void GetAlphaFloatIsColorSpaceAgnostic(const Color& color) {
  for (ColorSpace color_space : kAllColorSpaces) {
    EXPECT_EQ(color.GetAlphaFloat(),
              color.InColorSpace(color_space).GetAlphaFloat());
  }
}
FUZZ_TEST(ColorTest, GetAlphaFloatIsColorSpaceAgnostic)
    .WithDomains(ArbitraryColor());

TEST(ColorTest, WithAlphaFloatClamps) {
  EXPECT_EQ(Color::Red().WithAlphaFloat(0.5).GetAlphaFloat(), 0.5);
  EXPECT_EQ(Color::Red().WithAlphaFloat(1.5).GetAlphaFloat(), 1.0);
  EXPECT_EQ(Color::Red().WithAlphaFloat(-.5).GetAlphaFloat(), 0.0);
  EXPECT_EQ(Color::Red().WithAlphaFloat(kNan).GetAlphaFloat(), 0.0);
}

void WithAlphaFloatOnlyAffectsAlpha(const Color& old_color, float alpha) {
  Color new_color = old_color.WithAlphaFloat(alpha);
  EXPECT_EQ(new_color.GetAlphaFloat(), alpha);
  EXPECT_EQ(new_color.GetColorSpace(), old_color.GetColorSpace());
  Color::RgbaFloat old_rgba = old_color.AsFloat(ink::Color::Format::kLinear);
  Color::RgbaFloat new_rgba = new_color.AsFloat(ink::Color::Format::kLinear);
  EXPECT_EQ(new_rgba.r, old_rgba.r);
  EXPECT_EQ(new_rgba.g, old_rgba.g);
  EXPECT_EQ(new_rgba.b, old_rgba.b);
}
FUZZ_TEST(ColorTest, WithAlphaFloatOnlyAffectsAlpha)
    .WithDomains(ArbitraryColor(), fuzztest::InRange(0.f, 1.f));

void IdenticalFromUint8sCompareEqual(const std::array<uint8_t, 4>& rgba) {
  for (auto format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha && rgba[3] == 0) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromUint8(rgba[0], rgba[1], rgba[2], rgba[3], format,
                                   color_space),
                  Eq(Color::FromUint8(rgba[0], rgba[1], rgba[2], rgba[3],
                                      format, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, IdenticalFromUint8sCompareEqual);

TEST(ColorTest, FromUint8AllZerosAlwaysMeansTransparentBlack) {
  for (auto format : kAllFormats) {
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(
          Color::FromUint8(0, 0, 0, 0, format, color_space),
          Eq(Color::FromUint8(0, 0, 0, 0, Format::kLinear, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}

void FromUint8BackToUint8IsIdentity(const std::array<uint8_t, 4>& rgba) {
  for (auto format : {Format::kLinear, Format::kGammaEncoded}) {
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromUint8(rgba[0], rgba[1], rgba[2], rgba[3], format,
                                   color_space)
                      .AsUint8(format),
                  ChannelStructEqUint8s(rgba))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, FromUint8BackToUint8IsIdentity);

TEST(ColorDeathTest, FromUint8AlphaZeroPremultipliedNonZeroColorsDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      Color::FromUint8(0, 27, 0, 0, Format::kPremultipliedAlpha),
      "[Pp]remultiplied.*alpha=0.*RGB=0");
}

TEST(ColorTest, AsUint8Rounds) {
  const Color c =
      Color::FromFloat(127.0f / 255.0f, 135.5f / 255.0f, 254.49f / 255.0f,
                       200.5f / 255.0f, Format::kLinear);
  EXPECT_THAT(c.AsUint8(Format::kLinear),
              ChannelStructEqUint8s({127, 136, 254, 201}));
}

void AsUint8WorksWithAnyValues(const std::array<float, 4>& rgba) {
  for (auto from_format : kAllFormats) {
    if (from_format == Format::kPremultipliedAlpha &&
        (rgba[3] <= 0 || std::isnan(rgba[3]))) {
      // This is expected to crash; see the death test above.
      continue;
    }
    for (auto to_format : kAllFormats) {
      for (auto color_space : kAllColorSpaces) {
        // No expectation to test here; just asserting this doesn't crash.
        Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], from_format,
                         color_space)
            .AsUint8(to_format);
      }
    }
  }
}
FUZZ_TEST(ColorTest, AsUint8WorksWithAnyValues);

void IdenticalFromPackedUint32RGBAsCompareEqual(uint32_t rgba) {
  for (auto format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha && (rgba & 0xff) == 0) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(Color::FromPackedUint32RGBA(rgba, format, color_space),
                  Eq(Color::FromPackedUint32RGBA(rgba, format, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, IdenticalFromPackedUint32RGBAsCompareEqual);

TEST(ColorTest, FromPackedUint32RGBAAllZerosAlwaysMeansTransparentBlack) {
  for (auto format : kAllFormats) {
    for (auto color_space : kAllColorSpaces) {
      EXPECT_THAT(
          Color::FromPackedUint32RGBA(0x0, format, color_space),
          Eq(Color::FromPackedUint32RGBA(0x0, Format::kLinear, color_space)))
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}

void FromPackedUint32RGBAToPackedUint32RGBAIsIdentity(uint32_t rgba) {
  for (auto format : kAllFormats) {
    if (format == Format::kPremultipliedAlpha && (rgba & 0xff) == 0) {
      // This is expected to crash; see the death tests.
      continue;
    }
    for (auto color_space : kAllColorSpaces) {
      EXPECT_EQ(Color::FromPackedUint32RGBA(rgba, format, color_space)
                    .AsPackedUint32RGBA(format),
                rgba)
          << absl::StrFormat("for case %v %v", format, color_space);
    }
  }
}
FUZZ_TEST(ColorTest, FromPackedUint32RGBAToPackedUint32RGBAIsIdentity);

TEST(ColorDeathTest,
     FromPackedUint32RGBAAlphaZeroPremultipliedNonZeroColorsDies) {
  EXPECT_DEATH_IF_SUPPORTED(
      Color::FromPackedUint32RGBA(0x1a2b3c00, Format::kPremultipliedAlpha),
      "[Pp]remultiplied.*alpha=0.*RGB=0");
}

TEST(ColorTest, PredefinedColorsDoNotCrash) {
  Color test_color;
  test_color = Color::Black();
  test_color = Color::Gray();
  test_color = Color::White();
  test_color = Color::Transparent();
  test_color = Color::Red();
  test_color = Color::Orange();
  test_color = Color::Yellow();
  test_color = Color::Green();
  test_color = Color::Cyan();
  test_color = Color::Blue();
  test_color = Color::LightBlue();
  test_color = Color::Purple();
  test_color = Color::Magenta();
  test_color = Color::GoogleBlue();
  test_color = Color::GoogleRed();
  test_color = Color::GoogleYellow();
  test_color = Color::GoogleGreen();
  test_color = Color::GoogleGray();
  test_color = Color::GoogleOrange();
  test_color = Color::GooglePink();
  test_color = Color::GooglePurple();
  test_color = Color::GoogleCyan();
  test_color = Color::DefaultDocumentBackground();
}

TEST(ColorTest, ColorsInDifferentColorSpacesAreNotEqual) {
  Color in_srgb = Color::FromUint8(0, 0, 0, 255);
  ASSERT_EQ(in_srgb.GetColorSpace(), ColorSpace::kSrgb);
  Color in_p3 = in_srgb.InColorSpace(ColorSpace::kDisplayP3);
  EXPECT_THAT(in_srgb.AsFloat(Format::kLinear),
              ChannelStructEqFloats({0.0f, 0.0f, 0.0f, 1.0f}));
  EXPECT_THAT(in_p3.AsFloat(Format::kLinear),
              ChannelStructEqFloats({0.0f, 0.0f, 0.0f, 1.0f}));
  EXPECT_NE(in_srgb, in_p3);
  EXPECT_NE(in_p3, in_srgb);
}

void EqualityRequiresAnExactMatch(const std::array<float, 4>& rgba) {
  Color lhs = Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3]);
  Color rhs = Color::FromFloat(rgba[0] + kColorNearlyEps * 0.1f, rgba[1],
                               rgba[2], rgba[3]);
  EXPECT_NE(lhs, rhs);
}
FUZZ_TEST(ColorTest, EqualityRequiresAnExactMatch)
    .WithDomains(FourFloatsInZeroOne());

TEST(ColorTest, NearlyEqualsRejectsDifferentColors) {
  const Color lhs = Color::FromFloat(0.7, 0.6, 0.5, 0.4, Format::kLinear);
  const Color rhs = Color::FromFloat(0.7, 0.61, 0.5, 0.4, Format::kLinear);
  EXPECT_THAT(lhs, Not(ColorNearlyEquals(rhs)));
}

void NearlyEqualsAcceptsSlightlyDifferentColors(
    const std::array<float, 4>& rgba) {
  const Color lhs =
      Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3], Format::kLinear);
  const Color rhs = Color::FromFloat(rgba[0] + kColorNearlyEps * 0.9f, rgba[1],
                                     rgba[2], rgba[3], Format::kLinear);
  EXPECT_THAT(lhs, ColorNearlyEquals(rhs));
  EXPECT_THAT(rhs, ColorNearlyEquals(lhs));
}
FUZZ_TEST(ColorTest, NearlyEqualsAcceptsSlightlyDifferentColors)
    .WithDomains(FourFloatsInZeroOne());

void NearlyEqualsAcceptsDifferentColorSpaces(const std::array<float, 4>& rgba) {
  Color c = Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                             Format::kLinear, ColorSpace::kSrgb);
  Color d = Color::FromFloat(rgba[0] + kColorNearlyEps * 0.5f, rgba[1], rgba[2],
                             rgba[3], Format::kLinear, ColorSpace::kSrgb);
  Color c_p3 = c.InColorSpace(ColorSpace::kDisplayP3);
  Color d_p3 = d.InColorSpace(ColorSpace::kDisplayP3);
  EXPECT_THAT(c, ColorNearlyEquals(c_p3));
  EXPECT_THAT(c, ColorNearlyEquals(d_p3));
  EXPECT_THAT(d, ColorNearlyEquals(c_p3));
  EXPECT_THAT(d, ColorNearlyEquals(d_p3));
  EXPECT_THAT(c_p3, ColorNearlyEquals(c));
  EXPECT_THAT(c_p3, ColorNearlyEquals(d));
  EXPECT_THAT(d_p3, ColorNearlyEquals(c));
  EXPECT_THAT(d_p3, ColorNearlyEquals(d));
}
FUZZ_TEST(ColorTest, NearlyEqualsAcceptsDifferentColorSpaces)
    .WithDomains(FourFloatsInZeroOne());

TEST(ColorTest, IsInGamutIsFalseForOutOfGamutColors) {
  EXPECT_FALSE(Color::FromFloat(0.5, 1.1, 0.4, 0.5).IsInGamut());
  EXPECT_FALSE(Color::FromFloat(0.5, -0.1, 0.4, 0.5).IsInGamut());
}

void IsInGamutIsTrueForSimpleColors(const std::array<float, 4>& rgba) {
  for (auto color_space : kAllColorSpaces) {
    EXPECT_TRUE(Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                                 Format::kLinear, color_space)
                    .IsInGamut())
        << absl::StrFormat("for case %v", color_space);
  }
}
FUZZ_TEST(ColorTest, IsInGamutIsTrueForSimpleColors)
    .WithDomains(FourFloatsInZeroOne());

TEST(ColorTest, IsNearlyInGamutIsFalseForOutOfGamutColors) {
  EXPECT_FALSE(Color::FromFloat(0.5, 1.1, 0.4, 0.5).IsNearlyInGamut());
  EXPECT_FALSE(Color::FromFloat(0.5, -0.1, 0.4, 0.5).IsNearlyInGamut());
}

TEST(ColorTest, IsNearlyInGamutAcceptsSlightlyOutOfGamutColors) {
  EXPECT_TRUE(
      Color::FromFloat(1.000005f, -0.000005f, 0.0f, 1.0f, Format::kLinear)
          .IsNearlyInGamut());
}

TEST(ColorTest, ClampToGamut) {
  EXPECT_EQ(Color::FromFloat(1.8f, -0.000005f, -2.5f, 1.0f, Format::kLinear)
                .ClampedToGamut(),
            Color::FromFloat(1.0f, 0.0f, 0.0f, 1.0f, Format::kLinear));
}

void ClampedToGamutIsInGamut(const Color& color) {
  Color clamped = color.ClampedToGamut();
  EXPECT_TRUE(clamped.IsInGamut())
      << "Where clamped is: " << testing::PrintToString(clamped);
}
FUZZ_TEST(ColorTest, ClampedToGamutIsInGamut).WithDomains(ArbitraryColor());

TEST(ColorTest, ScaleToGamut) {
  const Color actual_2 =
      Color::FromFloat(0.5f, 2.5f, 0.75f, 0.3f, Format::kLinear)
          .ScaledToGamut();
  const Color expected_2 =
      Color::FromFloat(0.2f, 1.0f, 0.3f, 0.3f, Format::kLinear);
  EXPECT_THAT(actual_2, ColorNearlyEquals(expected_2));
}

void ScaledToGamutIsInGamut(const Color& color) {
  Color scaled = color.ScaledToGamut();
  EXPECT_TRUE(scaled.IsInGamut())
      << "Where scaled is: " << testing::PrintToString(scaled);
}
FUZZ_TEST(ColorTest, ScaledToGamutIsInGamut).WithDomains(ArbitraryColor());

void RoundTripColorSpaceConversionIsNearlyEqual(
    const std::array<float, 4>& rgba) {
  const Color srgb = Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                                      Format::kLinear, ColorSpace::kSrgb);
  const Color srgb_round_tripped =
      srgb.InColorSpace(ColorSpace::kDisplayP3).InColorSpace(ColorSpace::kSrgb);
  EXPECT_THAT(srgb_round_tripped, ColorNearlyEquals(srgb));

  const Color p3 = Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                                    Format::kLinear, ColorSpace::kDisplayP3);
  const Color p3_round_tripped =
      p3.InColorSpace(ColorSpace::kSrgb).InColorSpace(ColorSpace::kDisplayP3);
  EXPECT_THAT(p3_round_tripped, ColorNearlyEquals(p3));
}
FUZZ_TEST(ColorTest, RoundTripColorSpaceConversionIsNearlyEqual)
    .WithDomains(FourFloatsInZeroOne());

TEST(ColorTest, Stringify) {
  EXPECT_THAT(
      absl::StrCat(
          Color::FromFloat(0.6, 0.4, 0.7, 0.8, Color::Format::kLinear)),
      ContainsRegex(R"regex(Color.*0\.6.*0\.4.*0\.7.*0\.8.*sRGB)regex"));
}

TEST(FormatTest, Stringify) {
  EXPECT_THAT(absl::StrCat(Color::Format::kLinear), StrEq("kLinear"));
  EXPECT_THAT(absl::StrCat(Color::Format::kGammaEncoded),
              StrEq("kGammaEncoded"));
  EXPECT_THAT(absl::StrCat(Color::Format::kPremultipliedAlpha),
              StrEq("kPremultipliedAlpha"));
  EXPECT_THAT(absl::StrCat(static_cast<Color::Format>(-1)),
              ContainsRegex("unknown"));
}

TEST(RgbaFloatTest, Stringify) {
  EXPECT_THAT(absl::StrCat(Color::RgbaFloat{0.5, 0.75, -0.25, 1.25}),
              MatchesRegex(
                  R"regex(RgbaFloat\{0\.50* 0\.750* -0\.250* 1\.250*\})regex"));
}

TEST(RgbaUint8Test, Stringify) {
  EXPECT_THAT(absl::StrCat(Color::RgbaUint8{0, 28, 197, 255}),
              StrEq("RgbaUint8{0 28 197 255}"));
}

}  // namespace
}  // namespace ink
