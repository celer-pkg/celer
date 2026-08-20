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

#include "ink/color/type_matchers.h"

#include <array>
#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "ink/color/color.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::FloatNear;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Matcher;
using ::testing::Matches;

MATCHER_P(NearIdentityMatrixMatcher, eps,
          absl::StrCat(negation ? "isn't" : "is", " within ", eps,
                       " of the identity matrix")) {
  return Matches(ElementsAre(
      FloatNear(1.0, eps), FloatNear(0.0, eps), FloatNear(0.0, eps),
      FloatNear(0.0, eps), FloatNear(1.0, eps), FloatNear(0.0, eps),
      FloatNear(0.0, eps), FloatNear(0.0, eps), FloatNear(1.0, eps)))(arg);
}

MATCHER_P2(Vec4NearMatcher, expected, eps,
           absl::StrFormat("%s within %e of {%f, %f, %f, %f}",
                           negation ? "isn't" : "is", eps, expected[0],
                           expected[1], expected[2], expected[3])) {
  return Matches(ElementsAre(
      FloatNear(expected[0], eps), FloatNear(expected[1], eps),
      FloatNear(expected[2], eps), FloatNear(expected[3], eps)))(arg);
}

MATCHER_P(FloatNearlyBetweenZeroAndOneMatcher, eps,
          absl::StrFormat("%s within %e of the interval [0, 1]",
                          negation ? "isn't" : "is", eps)) {
  return Matches(AllOf(Ge(-eps), Le(1.0f + eps)))(arg);
}

MATCHER_P(ColorNearlyEqualsMatcher, expected, "") {
  return arg.NearlyEquals(expected);
}

MATCHER_P2(ChannelStructNearArrayMatcher, expected, tolerance,
           absl::StrFormat("%s within %f of {%f, %f, %f, %f}",
                           negation ? "isn't" : "is", tolerance, expected[0],
                           expected[1], expected[2], expected[3])) {
  return Matches(Vec4Near(expected, tolerance))(
      std::array<float, 4>{arg.r, arg.g, arg.b, arg.a});
}

MATCHER_P2(ChannelStructNearChannelStructMatcher, expected, tolerance,
           absl::StrFormat("%s within %f of {%f, %f, %f, %f}",
                           negation ? "isn't" : "is", tolerance, expected.r,
                           expected.g, expected.b, expected.a)) {
  return Matches(::testing::FieldsAre(
      FloatNear(expected.r, tolerance), FloatNear(expected.g, tolerance),
      FloatNear(expected.b, tolerance), FloatNear(expected.a, tolerance)))(arg);
}

MATCHER_P(ChannelStructEqFloatMatcher, expected, "") {
  return Matches(ElementsAreArray(expected))(
      std::array<float, 4>{arg.r, arg.g, arg.b, arg.a});
}

MATCHER_P(ChannelStructEqMatcher, expected, "") {
  return Matches(AllOf(Field(&Color::RgbaUint8::r, expected.r),
                       Field(&Color::RgbaUint8::g, expected.g),
                       Field(&Color::RgbaUint8::b, expected.b),
                       Field(&Color::RgbaUint8::a, expected.a)))(arg);
}

MATCHER_P(ChannelStructEqChannelStructMatcher, expected, "") {
  return Matches(AllOf(Field(&Color::RgbaFloat::r, expected.r),
                       Field(&Color::RgbaFloat::g, expected.g),
                       Field(&Color::RgbaFloat::b, expected.b),
                       Field(&Color::RgbaFloat::a, expected.a)))(arg);
}

MATCHER_P(ChannelStructEqUint8Matcher, expected, "") {
  return Matches(ElementsAreArray(expected))(
      std::array<uint8_t, 4>{arg.r, arg.g, arg.b, arg.a});
}

}  // namespace

Matcher<std::array<float, 9>> NearIdentityMatrix(double eps) {
  return NearIdentityMatrixMatcher(eps);
}

Matcher<std::array<float, 4>> Vec4Near(const std::array<float, 4>& expected,
                                       float eps) {
  return Vec4NearMatcher(expected, eps);
}

Matcher<float> FloatNearlyBetweenZeroAndOne(float eps) {
  return FloatNearlyBetweenZeroAndOneMatcher(eps);
}

Matcher<Color> ColorNearlyEquals(const Color& expected) {
  return ColorNearlyEqualsMatcher(expected);
}

Matcher<Color::RgbaFloat> ChannelStructNear(
    const std::array<float, 4>& expected, float tolerance) {
  return ChannelStructNearArrayMatcher(expected, tolerance);
}

Matcher<Color::RgbaFloat> ChannelStructNearChannelStruct(
    const Color::RgbaFloat& expected, float tolerance) {
  return ChannelStructNearChannelStructMatcher(expected, tolerance);
}

Matcher<Color::RgbaFloat> ChannelStructEqFloats(
    const std::array<float, 4>& expected) {
  return ChannelStructEqFloatMatcher(expected);
}

Matcher<Color::RgbaFloat> ChannelStructEqChannelStruct(
    const Color::RgbaFloat& expected) {
  return ChannelStructEqChannelStructMatcher(expected);
}

Matcher<Color::RgbaUint8> ChannelStructEq(const Color::RgbaUint8& expected) {
  return ChannelStructEqMatcher(expected);
}

Matcher<Color::RgbaUint8> ChannelStructEqUint8s(
    const std::array<uint8_t, 4>& expected) {
  return ChannelStructEqUint8Matcher(expected);
}

}  // namespace ink
