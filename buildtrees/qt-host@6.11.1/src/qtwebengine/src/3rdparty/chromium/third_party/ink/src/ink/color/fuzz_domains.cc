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

#include "ink/color/fuzz_domains.h"

#include <array>

#include "fuzztest/fuzztest.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"

namespace ink {
namespace {

using ::fuzztest::Arbitrary;
using ::fuzztest::ArrayOf;
using ::fuzztest::Domain;
using ::fuzztest::ElementOf;
using ::fuzztest::InRange;
using ::fuzztest::Map;

}  // namespace

Domain<std::array<float, 4>> FourFloatsWithAbsoluteValueAtMost(float limit) {
  return ArrayOf(InRange(-limit, limit), InRange(-limit, limit),
                 InRange(-limit, limit), InRange(-limit, limit));
}

Domain<std::array<float, 4>> FourFloatsInZeroOne() {
  return ArrayOf(InRange(0.0f, 1.0f), InRange(0.0f, 1.0f), InRange(0.0f, 1.0f),
                 InRange(0.0f, 1.0f));
}

// LINT.IfChange(color_space)
Domain<ColorSpace> ArbitraryColorSpace() {
  return ElementOf({
      ColorSpace::kSrgb,
      ColorSpace::kDisplayP3,
  });
}
// LINT.ThenChange(color_space.h:color_space)

Domain<Color> ArbitraryColor() {
  return Map(
      [](float r, float g, float b, float a, ColorSpace color_space) {
        // The Color::Format argument determines how the RGB arguments are
        // interpreted, but has no effect on how the decoded RGB values are
        // stored in the Color object.  Using kLinear here with Arbitrary<float>
        // RGB values allows us to generate all possible Color objects.
        return Color::FromFloat(r, g, b, a, Color::Format::kLinear,
                                color_space);
      },
      // The RGB arguments can be any float; Color::FromFloat will turn NaNs
      // into zero, but will not otherwise clamp the values.
      Arbitrary<float>(), Arbitrary<float>(), Arbitrary<float>(),
      // By contrast, Color::FromFloat clamps the alpha argument to [0, 1] (even
      // if it was NaN), so don't bother generating alpha values outside that
      // range (so as not to over-represent the endpoints).
      InRange(0.0f, 1.0f), ArbitraryColorSpace());
}

// LINT.IfChange(color_format)
Domain<Color::Format> ArbitraryColorFormat() {
  return ElementOf({
      Color::Format::kLinear,
      Color::Format::kGammaEncoded,
      Color::Format::kPremultipliedAlpha,
  });
}
// LINT.ThenChange(color.h:color_format)

Domain<Color> InGamutSrgbColor() {
  return Map(
      [](float r, float g, float b, float a) {
        return Color::FromFloat(r, g, b, a, Color::Format::kLinear,
                                ColorSpace::kSrgb);
      },
      InRange(0.0f, 1.0f), InRange(0.0f, 1.0f), InRange(0.0f, 1.0f),
      InRange(0.0f, 1.0f));
}

}  // namespace ink
