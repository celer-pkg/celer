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
#include <string>

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/strings/str_format.h"
#include "ink/color/color_space.h"

namespace ink {
namespace {

constexpr int kChannelRed = 0;
constexpr int kChannelGreen = 1;
constexpr int kChannelBlue = 2;
constexpr int kChannelAlpha = 3;
constexpr int kNumChannelsColorOnly = 3;
constexpr int kNumChannelsIncludingAlpha = 4;

// Epsilon for "Nearly" comparisons: the maximum absolute deviation in channel
// value that would round to zero if that channel were quantized to a 16-bit
// fixed-point value. This quantity is approximately 1/(2 * (2^16 - 1)).
constexpr float kNearlyZero = 7.62951e-6;

float NanToZero(float x) {
  if (std::isnan(x)) {
    return 0;
  }
  return x;
}

}  // namespace

Color Color::FromFloat(float red, float green, float blue, float alpha,
                       Format format, ColorSpace color_space) {
  red = NanToZero(red);
  green = NanToZero(green);
  blue = NanToZero(blue);
  alpha = std::clamp(NanToZero(alpha), 0.0f, 1.0f);
  if (format == Format::kPremultipliedAlpha && alpha == 0.0f &&
      (red != 0.0f || green != 0.0f || blue != 0.0f)) {
    // If alpha is zero and the inputs were correctly premultiplied, then all
    // color channels must necessarily be zero too.
    ABSL_LOG(FATAL) << absl::StrFormat(
        "Premultiplied alpha=0 must have RGB=0. Got RGBA={%f, %f, %f, %f}.",
        red, green, blue, alpha);
  }
  if (format == Format::kPremultipliedAlpha) {
    // Un-premultiply only if alpha != 0. The check above assures that the only
    // other case is RGBA=(0,0,0,0), which indicates transparent black, which
    // doesn't need any special handling.
    if (alpha != 0.0f) {
      red /= alpha;
      green /= alpha;
      blue /= alpha;
    }
  } else if (format == Format::kGammaEncoded) {
    red = GammaDecode(red, color_space);
    green = GammaDecode(green, color_space);
    blue = GammaDecode(blue, color_space);
  }
  return Color({red, green, blue, alpha}, color_space);
}

Color Color::FromUint8(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha,
                       Format format, ColorSpace color_space) {
  if (format == Format::kPremultipliedAlpha && alpha == 0.0f &&
      (red != 0 || green != 0 || blue != 0)) {
    // If alpha is zero and the inputs were correctly premultiplied, then all
    // color channels must necessarily be zero too.
    ABSL_LOG(FATAL) << absl::StrFormat(
        "Premultiplied alpha=0 must have RGB=0. Got RGBA={%d, %d, %d, %d}.",
        red, green, blue, alpha);
  }
  return FromFloat(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f,
                   format, color_space);
}

Color Color::FromPackedUint32RGBA(uint32_t rgba, Format format,
                                  ColorSpace color_space) {
  return FromUint8(static_cast<uint8_t>(rgba >> 24),
                   static_cast<uint8_t>((rgba >> 16) & 0xff),
                   static_cast<uint8_t>((rgba >> 8) & 0xff),
                   static_cast<uint8_t>(rgba & 0xff), format, color_space);
}

bool Color::operator==(const Color &other) const {
  if (color_space_ != other.color_space_) {
    return false;
  }
  for (int i = 0; i < kNumChannelsIncludingAlpha; ++i) {
    if (rgba_[i] != other.rgba_[i]) {
      return false;
    }
  }
  return true;
}

bool Color::NearlyEquals(const Color &other) const {
  const Color converted = other.InColorSpace(color_space_);
  for (int i = 0; i < kNumChannelsIncludingAlpha; ++i) {
    if (std::fabs(converted.rgba_[i] - rgba_[i]) > kNearlyZero) {
      return false;
    }
  }
  return true;
}

bool Color::IsInGamut() const {
  for (int i = 0; i < kNumChannelsColorOnly; ++i) {
    if (rgba_[i] < 0.0f || rgba_[i] > 1.0f) {
      return false;
    }
  }
  return true;
}

bool Color::IsNearlyInGamut() const {
  for (int i = 0; i < kNumChannelsColorOnly; ++i) {
    if (rgba_[i] < -kNearlyZero || rgba_[i] > 1.0f + kNearlyZero) {
      return false;
    }
  }
  return true;
}

Color Color::ClampedToGamut() const {
  Color copy(*this);
  // Modify all channels, including alpha.
  for (float &c : copy.rgba_) {
    c = std::clamp(c, 0.0f, 1.0f);
  }
  return copy;
}

Color Color::ScaledToGamut() const {
  Color copy(*this);

  // Clamp all channels, including alpha, up to zero.
  for (float &c : copy.rgba_) {
    c = std::max(0.0f, c);
  }

  // Clamp alpha down to one.
  copy.rgba_[kChannelAlpha] = std::min(1.0f, copy.rgba_[kChannelAlpha]);

  // Scale color channels.
  const float max_val =
      std::max(copy.rgba_[kChannelRed],
               std::max(copy.rgba_[kChannelGreen], copy.rgba_[kChannelBlue]));
  if (max_val > 1.0f) {
    for (int i = 0; i < kNumChannelsColorOnly; ++i) {
      copy.rgba_[i] = copy.rgba_[i] / max_val;
    }
  }

  return copy;
}

Color Color::InColorSpace(ColorSpace target) const {
  std::array<float, 4> converted_rgba =
      ConvertColor(rgba_, color_space_, target);
  // Infinite inputs may convert to NaN; use the factory to deal with those.
  return Color::FromFloat(converted_rgba[0], converted_rgba[1],
                          converted_rgba[2], converted_rgba[3], Format::kLinear,
                          target);
}

Color Color::WithAlphaFloat(float alpha) const {
  return Color::FromFloat(rgba_[0], rgba_[1], rgba_[2], alpha, Format::kLinear,
                          color_space_);
}

Color::RgbaFloat Color::AsFloat(Format format) const {
  RgbaFloat rgba = {rgba_[0], rgba_[1], rgba_[2], rgba_[3]};
  if (format == Format::kGammaEncoded) {
    rgba.r = GammaEncode(rgba.r, color_space_);
    rgba.g = GammaEncode(rgba.g, color_space_);
    rgba.b = GammaEncode(rgba.b, color_space_);
  } else if (format == Format::kPremultipliedAlpha) {
    ABSL_DCHECK_GE(rgba.a, 0.0f);
    ABSL_DCHECK_LE(rgba.a, 1.0f);
    if (rgba.a == 0.0f) {
      // This branch needs special handling because infinite channel values are
      // permitted, and zero times infinity is NaN. We want it to be zero.
      rgba.r = 0.0f;
      rgba.g = 0.0f;
      rgba.b = 0.0f;
    } else {
      rgba.r *= rgba.a;
      rgba.g *= rgba.a;
      rgba.b *= rgba.a;
    }
  }
  return rgba;
}

namespace {
uint8_t FloatToUint8(float value) {
  return static_cast<uint8_t>(
      std::clamp(std::round(255.0f * value), 0.0f, 255.0f));
}
}  // namespace

Color::RgbaUint8 Color::AsUint8(Format format) const {
  RgbaFloat floats = AsFloat(format);
  RgbaUint8 ints;
  ints.r = FloatToUint8(floats.r);
  ints.g = FloatToUint8(floats.g);
  ints.b = FloatToUint8(floats.b);
  ints.a = FloatToUint8(floats.a);
  return ints;
}

uint32_t Color::AsPackedUint32RGBA(Format format) const {
  RgbaUint8 ints = AsUint8(format);
  return (static_cast<uint32_t>(ints.r) << 24) |
         (static_cast<uint32_t>(ints.g) << 16) |
         (static_cast<uint32_t>(ints.b) << 8) | static_cast<uint32_t>(ints.a);
}

std::string Color::ToFormattedString() const {
  return absl::StrFormat("Color({%f, %f, %f, %f}, %v)", rgba_[0], rgba_[1],
                         rgba_[2], rgba_[3], color_space_);
}

std::string ToFormattedString(Color::Format format) {
  switch (format) {
    case Color::Format::kLinear:
      return "kLinear";
    case Color::Format::kGammaEncoded:
      return "kGammaEncoded";
    case Color::Format::kPremultipliedAlpha:
      return "kPremultipliedAlpha";
  }
  return absl::StrFormat("unknown Color::Format %d", static_cast<int>(format));
}

std::string ToFormattedString(Color::RgbaFloat rgba) {
  return absl::StrFormat("RgbaFloat{%f %f %f %f}", rgba.r, rgba.g, rgba.b,
                         rgba.a);
}

std::string ToFormattedString(Color::RgbaUint8 rgba) {
  return absl::StrFormat("RgbaUint8{%d %d %d %d}", rgba.r, rgba.g, rgba.b,
                         rgba.a);
}

}  // namespace ink
