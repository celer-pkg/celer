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

#ifndef INK_COLOR_COLOR_H_
#define INK_COLOR_COLOR_H_

#include <array>
#include <cmath>
#include <cstdint>
#include <string>

#include "absl/log/absl_check.h"
#include "ink/color/color_space.h"

namespace ink {

// A color, as represented relative to the gamut of a particular color space.
// Out-of-gamut colors are representable in this class, including "impossible
// colors" — those with numerical coordinates in the color space that do not
// correspond to real colors at all.
class Color {
 public:
  // Format options for converting between numerical RGBA values and a Color.
  // They consist of a gamma format (linear or encoded) and an alpha format
  // (non-premultiplied or premultiplied). Both formats affect only the RGB
  // channels; the alpha channel is the same regardless of format.
  //
  // ## Gamma
  // When RGB values are **linear**, evenly spaced numbers result in tones that
  // are evenly spaced in *physical luminance*. Graphics computations require
  // linear inputs.
  //
  // When RGB values are **gamma-encoded**, evenly spaced numbers result in
  // tones that are evenly spaced in *perceptual lightness*; that is, they
  // produce a uniform-looking gradient for human viewers. In gamma-encoded
  // values, more bits (or, in the case of floating-point values, more of the
  // number line) are devoted to physically dark tones than physically light
  // tones. This format is almost always used for human-readable color
  // descriptions (e.g., color picker widgets).
  //
  // The actual transfer function used for "gamma" encoding and decoding is
  // defined by the color space, and (despite the colloquial name used here) is
  // not typically a pure power-law gamma curve.
  //
  // ## Alpha premultiplication
  // A color with **premultiplied alpha** is one in which the color channel
  // values (red, green, blue) have been multiplied by the alpha value. Note
  // that premultiplication changes the meanings of the color channel values:
  // the raw premultiplied values do not specify the coordinates of the point in
  // the color space's gamut that corresponds to the actual color described.
  //
  // LINT.IfChange(color_format)
  enum class Format {
    // Linear channel values with no alpha premultiplication. This format is for
    // graphics computations.
    kLinear,
    // Gamma-encoded channel values with no alpha premultiplication. This format
    // is for presentation of numerical values to humans.
    kGammaEncoded,
    // Linear channel values with color channels premultiplied by alpha. This
    // format is for graphics computations that require premultiplied inputs.
    kPremultipliedAlpha
    // The combination of gamma encoding plus alpha premultiplication is
    // deliberately disallowed because it does not have any proper use case:
    // gamma encoding is for presentation to human end-users, while
    // premultiplication is for use in shader code.
  };
  // LINT.ThenChange(fuzz_domains.cc:color_format)

  // Constructs a Color from float values in the nominal range [0.0, 1.0]. By
  // default, this constructor accepts gamma-encoded values, since this is the
  // format usually used for human-readable color descriptions. Note that:
  // - `alpha` values outside [0, 1] are interpreted as clamped to that range.
  // - NaN values for any argument are interpreted as 0.
  // - Negative and infinite values of `red`, `green`, and `blue` are permitted.
  // - For Format::kGammaEncoded, color channel values will be decoded. Decoding
  //   a value outside [0, 1] is not well defined and results are
  //   implementation-dependent.
  // - For Format::kPremultipliedAlpha, if alpha = 0 and any color channel is
  //   nonzero, this function will crash.
  static Color FromFloat(float red, float green, float blue, float alpha,
                         Format format = Format::kGammaEncoded,
                         ColorSpace color_space = ColorSpace::kSrgb);

  // Constructs a Color from int values in the range [0, 255], which are
  // interpreted as fixed-point values in [0.0, 1.0]. Note that, for
  // Format::kPremultipliedAlpha, if alpha = 0 and any color channel is nonzero,
  // this function will crash.
  static Color FromUint8(uint8_t red, uint8_t green, uint8_t blue,
                         uint8_t alpha, Format format = Format::kGammaEncoded,
                         ColorSpace color_space = ColorSpace::kSrgb);

  // Constructs a Color from four 8-bit uint channels (in order: red, green,
  // blue, alpha) packed into a single uint32. Each channel value is in the
  // range [0, 255] and is interpreted as a fixed-point value in [0.0, 1.0].
  // Note that, for Format::kPremultipliedAlpha, if the alpha channel (last 8
  // bits) is 0 and any color channel is nonzero, this function will crash.
  static Color FromPackedUint32RGBA(uint32_t rgba,
                                    Format format = Format::kGammaEncoded,
                                    ColorSpace color_space = ColorSpace::kSrgb);

  static Color Black() { return FromPackedUint32RGBA(0x000000ff); }
  static Color Gray() { return FromPackedUint32RGBA(0x2a2a2aff); }
  static Color White() { return FromPackedUint32RGBA(0xffffffff); }
  static Color Transparent() { return FromPackedUint32RGBA(0x0); }
  static Color Red() { return FromPackedUint32RGBA(0xff0000ff); }
  static Color Orange() { return FromPackedUint32RGBA(0xffaa00ff); }
  static Color Yellow() { return FromPackedUint32RGBA(0xffff00ff); }
  static Color Green() { return FromPackedUint32RGBA(0x00ff00ff); }
  static Color Cyan() { return FromPackedUint32RGBA(0x00ffffff); }
  static Color Blue() { return FromPackedUint32RGBA(0x0000ffff); }
  static Color LightBlue() { return FromPackedUint32RGBA(0x7777ffff); }
  static Color Purple() { return FromPackedUint32RGBA(0xaa00ffff); }
  static Color Magenta() { return FromPackedUint32RGBA(0xff00ffff); }
  static Color GoogleBlue() { return FromPackedUint32RGBA(0x4285f4ff); }
  static Color GoogleRed() { return FromPackedUint32RGBA(0xea4335ff); }
  static Color GoogleYellow() { return FromPackedUint32RGBA(0xfbbc04ff); }
  static Color GoogleGreen() { return FromPackedUint32RGBA(0x34a853ff); }
  static Color GoogleGray() { return FromPackedUint32RGBA(0x9aa0a6ff); }
  static Color GoogleOrange() { return FromPackedUint32RGBA(0xfa7b17ff); }
  static Color GooglePink() { return FromPackedUint32RGBA(0xf439a0ff); }
  static Color GooglePurple() { return FromPackedUint32RGBA(0xa142f4ff); }
  static Color GoogleCyan() { return FromPackedUint32RGBA(0x24c1e0ff); }
  static Color DefaultDocumentBackground() {
    return FromPackedUint32RGBA(0xfafafaff);
  }

  // Defaults to opaque black.
  Color() : Color({0, 0, 0, 1}, ColorSpace::kSrgb) {}

  Color(const Color& other) = default;
  Color(Color&& other) = default;
  Color& operator=(const Color& other) = default;

  // Returns true if both colors are in the same color space and if the values
  // match exactly in all channels.
  bool operator==(const Color& other) const;
  bool operator!=(const Color& other) const { return !(*this == other); }

  // Returns true if the per-channel difference between `this` and `other`,
  // after conversion of `other` to the color space of `this', is very small.
  bool NearlyEquals(const Color& other) const;

  // Returns true if all color channels are in [0, 1].
  bool IsInGamut() const;

  // Returns true if all color channels are within a small distance of [0, 1].
  bool IsNearlyInGamut() const;

  // Returns a copy of this color with each channel, including alpha, clamped
  // to [0, 1].
  Color ClampedToGamut() const;

  // Returns a copy of this color with each color channel scaled by a constant
  // factor such that they are all <= 1, negative values clamped to 0, and alpha
  // clamped to [0, 1].
  Color ScaledToGamut() const;

  ColorSpace GetColorSpace() const { return color_space_; }

  // Returns a copy of this color converted into the target color space. The
  // alpha value is copied over unchanged.
  Color InColorSpace(ColorSpace target) const;

  // Returns the alpha value for this color, which will be in [0, 1].  Note that
  // this value is independent of the color space or format.
  float GetAlphaFloat() const { return rgba_[3]; }

  // Returns a copy of this color, but with its alpha value replaced by the
  // given alpha value. `alpha` values outside [0, 1] are interpreted as clamped
  // to that range, and a NaN `alpha` is interpreted as 0.
  Color WithAlphaFloat(float alpha) const;

  // Return type for `AsFloat()`. Raw channel values with no formally associated
  // color space, gamma type, or alpha type.
  struct RgbaFloat {
    float r;
    float g;
    float b;
    float a;
  };

  // Returns the channel values in the format requested, in this Color's color
  // space. (Call `InColorSpace()` first if you want coordinates in a different
  // color space.) Values are not clamped before returning. Alpha will be in the
  // range [0, 1]; all values except NaN are possible for color channels. Note
  // that gamma encoding is not well defined for values outside [0, 1].
  RgbaFloat AsFloat(Format format) const;

  // Return type for `AsUint8()`. Raw channel values with no formally associated
  // color space, gamma type, or alpha type. Channel values are in [0,1]-clamped
  // fixed-point format, as 8-bit uints in [0, 255].
  struct RgbaUint8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };

  // Returns the channel values, in [0, 1]-clamped fixed-point format, as 8-bit
  // uints in [0, 255]. The values returned are for this Color's color space; if
  // you want coordinates in a different color space, call InColorSpace() first.
  // The color is first converted to channel values in `format`, then the
  // channel values are clamped and converted to uint8. (The order of these
  // operations is only relevant for `kPremultipliedAlpha`.)
  RgbaUint8 AsUint8(Format format) const;

  // Like AsUint8, but packs the four uint8 values into a single RGBA uint32.
  uint32_t AsPackedUint32RGBA(Format format) const;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Color& color) {
    sink.Append(color.ToFormattedString());
  }

 private:
  // The four components of the color.
  //
  // Elements 0–2 are color coordinates relative to the gamut of `color_space_`,
  // in red-green-blue order. A value of 1.0 in a given channel means 100%
  // contribution from the gamut's corresponding color primary. The nominal
  // range for each channel value is [0.0, 1.0], but values outside of this
  // range are permitted; such points may represent out-of-gamut colors, out-of-
  // range luminescences, or even impossible colors. The coordinate values are
  // linear (gamma-decoded) and non-premultiplied, as required for interpreting
  // them as coordinates relative to a gamut.
  //
  // Element 3 is linear alpha.
  //
  // Invariants:
  // - No channel value is NaN.
  // - Alpha is in the range [0.0, 1.0].
  std::array<float, 4> rgba_ = {};

  // The color space in which to interpret the RGB channel values.
  ColorSpace color_space_ = ColorSpace::kSrgb;

  Color(std::array<float, 4> rgba, ColorSpace color_space)
      : rgba_(rgba), color_space_(color_space) {
    ABSL_DCHECK(!std::isnan(rgba_[0]));
    ABSL_DCHECK(!std::isnan(rgba_[1]));
    ABSL_DCHECK(!std::isnan(rgba_[2]));
    ABSL_DCHECK(!std::isnan(rgba_[3]));
    ABSL_DCHECK_GE(rgba_[3], 0.0f);
    ABSL_DCHECK_LE(rgba_[3], 1.0f);
  }

  std::string ToFormattedString() const;
};

std::string ToFormattedString(Color::Format format);
std::string ToFormattedString(Color::RgbaFloat rgba);
std::string ToFormattedString(Color::RgbaUint8 rgba);

template <typename Sink>
void AbslStringify(Sink& sink, Color::Format format) {
  sink.Append(ToFormattedString(format));
}

template <typename Sink>
void AbslStringify(Sink& sink, Color::RgbaFloat rgba) {
  sink.Append(ToFormattedString(rgba));
}

template <typename Sink>
void AbslStringify(Sink& sink, Color::RgbaUint8 rgba) {
  sink.Append(ToFormattedString(rgba));
}

}  // namespace ink

#endif  // INK_COLOR_COLOR_H_
