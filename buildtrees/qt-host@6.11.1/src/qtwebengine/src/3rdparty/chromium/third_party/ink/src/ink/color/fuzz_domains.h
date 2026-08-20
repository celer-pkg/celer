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

#ifndef INK_COLOR_FUZZ_DOMAINS_H_
#define INK_COLOR_FUZZ_DOMAINS_H_

#include <array>

#include "fuzztest/fuzztest.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"

namespace ink {

// The domain of all arrays of 4 floats in the range [-limit, limit].
fuzztest::Domain<std::array<float, 4>> FourFloatsWithAbsoluteValueAtMost(
    float limit);

// The domain of all arrays of 4 floats in [0.0, 1.0].
fuzztest::Domain<std::array<float, 4>> FourFloatsInZeroOne();

// The domain of all valid ColorSpaces.
fuzztest::Domain<ColorSpace> ArbitraryColorSpace();

// The domain of all valid color formats.
fuzztest::Domain<Color::Format> ArbitraryColorFormat();

// The domain of all Colors, including those with infinite and/or out-of-gamut
// components.
fuzztest::Domain<Color> ArbitraryColor();

// The domain of Colors that are in the sRGB color space and are in-gamut.
fuzztest::Domain<Color> InGamutSrgbColor();

}  // namespace ink

#endif  // INK_COLOR_FUZZ_DOMAINS_H_
