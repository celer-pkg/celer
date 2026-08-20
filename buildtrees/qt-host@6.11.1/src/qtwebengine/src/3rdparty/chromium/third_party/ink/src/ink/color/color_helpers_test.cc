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

#include "ink/color/color_helpers.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "ink/color/color.h"
#include "ink/color/type_matchers.h"

namespace ink {
namespace {

TEST(ColorHelpersTest, UnpackUint32ARGB) {
  uint32_t packed_argb = 0x12ab34cd;
  Color::RgbaUint8 unpacked = UnpackUint32ARGB(packed_argb);
  Color::RgbaUint8 expected = {.r = 0xab, .g = 0x34, .b = 0xcd, .a = 0x12};

  EXPECT_THAT(unpacked, ChannelStructEq(expected));
}

TEST(ColorHelpersTest, PackUint32ARGB) {
  Color::RgbaUint8 unpacked = {.r = 0x12, .g = 0xab, .b = 0x34, .a = 0xcd};
  uint32_t packed_argb = PackUint32ARGB(unpacked);
  EXPECT_EQ(packed_argb, 0xcd12ab34);
}

void UnpackThenPackUint32ARGBIsIdentity(uint32_t source) {
  uint32_t result = PackUint32ARGB(UnpackUint32ARGB(source));
  EXPECT_EQ(result, source);
}
FUZZ_TEST(ColorHelpersTest, UnpackThenPackUint32ARGBIsIdentity);

void PackThenUnpackUint32ARGBIsIdentity(Color::RgbaUint8 source) {
  Color::RgbaUint8 result = UnpackUint32ARGB(PackUint32ARGB(source));
  EXPECT_THAT(result, ChannelStructEq(source));
}
FUZZ_TEST(ColorHelpersTest, PackThenUnpackUint32ARGBIsIdentity);

}  // namespace
}  // namespace ink
