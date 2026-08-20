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

#ifndef INK_STORAGE_COLOR_H_
#define INK_STORAGE_COLOR_H_

#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/storage/proto/color.pb.h"

namespace ink {

// Modifies `color_proto` so it represents the same color as `color`.
void EncodeColor(const Color& color, proto::Color& color_proto);

// Converts `color_proto` into an equivalent Color. Defaults to sRGB if the
// color space is invalid or unrecognized.
Color DecodeColor(const proto::Color& color_proto);

// Converts `color_space` into an equivalent `proto::ColorSpace`.
proto::ColorSpace EncodeColorSpace(ColorSpace color_space);

// Converts `color_space_proto` into an equivalent ColorSpace. Defaults to sRGB
// in the case of an invalid or unrecognized value.
ColorSpace DecodeColorSpace(const proto::ColorSpace& color_space_proto);

}  // namespace ink

#endif  // INK_STORAGE_COLOR_H_
