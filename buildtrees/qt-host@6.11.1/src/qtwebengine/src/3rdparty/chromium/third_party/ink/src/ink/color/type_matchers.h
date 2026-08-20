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

#ifndef INK_COLOR_TYPE_MATCHERS_H_
#define INK_COLOR_TYPE_MATCHERS_H_

#include <array>
#include <cstdint>

#include "gtest/gtest.h"
#include "ink/color/color.h"

namespace ink {

// Maximum absolute error for floats and doubles in color-related comparisons. A
// bit less than 2^-18, which is one quarter of the epsilon for a 16-bit
// fixed-point fraction. We target one quarter so that colors can be converted
// twice without loss of precision.
constexpr float kColorTestEps = 3.8e-6;

::testing::Matcher<std::array<float, 9>> NearIdentityMatrix(double eps);

::testing::Matcher<std::array<float, 4>> Vec4Near(
    const std::array<float, 4>& expected, float eps);

::testing::Matcher<float> FloatNearlyBetweenZeroAndOne(float eps);

::testing::Matcher<Color> ColorNearlyEquals(const Color& expected);

::testing::Matcher<Color::RgbaFloat> ChannelStructNear(
    const std::array<float, 4>& expected, float tolerance);

::testing::Matcher<Color::RgbaFloat> ChannelStructNearChannelStruct(
    const Color::RgbaFloat& expected, float tolerance);

::testing::Matcher<Color::RgbaFloat> ChannelStructEqFloats(
    const std::array<float, 4>& expected);

::testing::Matcher<Color::RgbaFloat> ChannelStructEqChannelStruct(
    const Color::RgbaFloat& expected);

::testing::Matcher<Color::RgbaUint8> ChannelStructEq(
    const Color::RgbaUint8& expected);
::testing::Matcher<Color::RgbaUint8> ChannelStructEqUint8s(
    const std::array<uint8_t, 4>& expected);

}  // namespace ink

#endif  // INK_COLOR_TYPE_MATCHERS_H_
