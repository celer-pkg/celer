// Copyright 2024-2025 Google LLC
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

#include "ink/storage/color.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/storage/proto/color.pb.h"
#include "ink/storage/proto_matchers.h"

namespace ink {
namespace {

using ::testing::FloatEq;

TEST(ColorTest, EncodeColorSpaceWorksForValidColorSpaces) {
  EXPECT_EQ(EncodeColorSpace(ColorSpace::kSrgb),
            proto::ColorSpace::COLOR_SPACE_SRGB);
  EXPECT_EQ(EncodeColorSpace(ColorSpace::kDisplayP3),
            proto::ColorSpace::COLOR_SPACE_DISPLAY_P3);
}

TEST(ColorDeathTest, EncodeColorSpaceDiesOnInvalidEnumValue) {
  EXPECT_DEATH_IF_SUPPORTED(EncodeColorSpace(static_cast<ColorSpace>(-1)),
                            "Unknown");
}

TEST(ColorTest, DecodeColorSpaceWorksForValidProtoColorSpaces) {
  EXPECT_EQ(DecodeColorSpace(proto::ColorSpace::COLOR_SPACE_SRGB),
            ColorSpace::kSrgb);

  EXPECT_EQ(DecodeColorSpace(proto::ColorSpace::COLOR_SPACE_DISPLAY_P3),
            ColorSpace::kDisplayP3);
}

TEST(ColorTest, DecodeColorSpaceReturnsSrgbForInvalidColorSpace) {
  EXPECT_EQ(DecodeColorSpace(proto::ColorSpace::COLOR_SPACE_UNSPECIFIED),
            ColorSpace::kSrgb);

  EXPECT_EQ(DecodeColorSpace(static_cast<proto::ColorSpace>(-1)),
            ColorSpace::kSrgb);
}

void EncodeColorWorksForArbitraryColors(const std::array<float, 4>& rgba) {
  Color c = Color::FromFloat(rgba[0], rgba[1], rgba[2], rgba[3],
                             Color::Format::kLinear, ColorSpace::kDisplayP3);

  proto::Color proto;
  EncodeColor(c, proto);

  // proto::Color channels are linear and non-premultiplied.
  Color::RgbaFloat expected_rgba = c.AsFloat(Color::Format::kLinear);
  proto::Color expected_proto;
  expected_proto.set_color_space(proto::ColorSpace::COLOR_SPACE_DISPLAY_P3);
  expected_proto.set_r(expected_rgba.r);
  expected_proto.set_g(expected_rgba.g);
  expected_proto.set_b(expected_rgba.b);
  expected_proto.set_a(expected_rgba.a);

  EXPECT_THAT(proto, EqualsProtoTreatingNaNsAsEqual(expected_proto));
}
FUZZ_TEST(ColorTest, EncodeColorWorksForArbitraryColors);

void DecodeColorWorksForArbitraryProto(const proto::Color& proto) {
  const Color actual = DecodeColor(proto);

  Color::RgbaFloat actual_rgba = actual.AsFloat(Color::Format::kLinear);

  // The decoding process turns NaNs into zeros.
  float expected_r = std::isnan(proto.r()) ? 0 : proto.r();
  float expected_g = std::isnan(proto.g()) ? 0 : proto.g();
  float expected_b = std::isnan(proto.b()) ? 0 : proto.b();
  EXPECT_THAT(actual_rgba.r, FloatEq(expected_r));
  EXPECT_THAT(actual_rgba.g, FloatEq(expected_g));
  EXPECT_THAT(actual_rgba.b, FloatEq(expected_b));

  // The decoding process clamps alpha to [0, 1].
  float expected_alpha;
  if (std::isnan(proto.a())) {
    expected_alpha = 0;
  } else {
    expected_alpha = std::clamp(proto.a(), 0.0f, 1.0f);
  }
  EXPECT_THAT(actual_rgba.a, FloatEq(expected_alpha));

  ColorSpace expected_color_space;
  switch (proto.color_space()) {
    // Invalid proto enum values are defaulted to sRGB.
    case proto::ColorSpace::COLOR_SPACE_UNSPECIFIED:
    case proto::ColorSpace::COLOR_SPACE_SRGB: {
      expected_color_space = ColorSpace::kSrgb;
      break;
    }
    case proto::ColorSpace::COLOR_SPACE_DISPLAY_P3: {
      expected_color_space = ColorSpace::kDisplayP3;
      break;
    }
  }
  EXPECT_EQ(actual.GetColorSpace(), expected_color_space);
}
FUZZ_TEST(ColorTest, DecodeColorWorksForArbitraryProto);

}  // namespace
}  // namespace ink
