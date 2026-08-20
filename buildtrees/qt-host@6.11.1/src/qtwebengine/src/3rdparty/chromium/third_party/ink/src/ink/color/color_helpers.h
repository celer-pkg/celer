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

#ifndef INK_COLOR_COLOR_HELPERS_H_
#define INK_COLOR_COLOR_HELPERS_H_

#include <cstdint>

#include "ink/color/color.h"

namespace ink {

// Extracts components from a packed uint32 with Alpha-Red-Green-Blue order.
constexpr Color::RgbaUint8 UnpackUint32ARGB(uint32_t argb) {
  return {.r = static_cast<uint8_t>((argb >> 16) & 0xff),
          .g = static_cast<uint8_t>((argb >> 8) & 0xff),
          .b = static_cast<uint8_t>(argb & 0xff),
          .a = static_cast<uint8_t>(argb >> 24)};
}

// Packs 8-bit components into a uint32 with Alpha-Red-Green-Blue order.
constexpr uint32_t PackUint32ARGB(const Color::RgbaUint8& color) {
  return static_cast<uint32_t>(color.a) << 24 |
         static_cast<uint32_t>(color.r) << 16 |
         static_cast<uint32_t>(color.g) << 8 | static_cast<uint32_t>(color.b);
}

}  // namespace ink

#endif  // INK_COLOR_COLOR_HELPERS_H_
