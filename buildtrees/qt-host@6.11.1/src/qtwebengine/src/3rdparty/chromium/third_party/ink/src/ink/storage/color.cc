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

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/storage/proto/color.pb.h"

namespace ink {

void EncodeColor(const Color& color, proto::Color& color_proto) {
  Color::RgbaFloat rgba = color.AsFloat(Color::Format::kLinear);

  color_proto.set_r(rgba.r);
  color_proto.set_g(rgba.g);
  color_proto.set_b(rgba.b);

  // The Color type is not supposed to be able to store alpha outside [0, 1].
  // One of these checks failing indicates a programmer error within the Color
  // type itself.
  ABSL_DCHECK_GE(rgba.a, 0) << "in " << color;
  ABSL_DCHECK_LE(rgba.a, 1) << "in " << color;
  color_proto.set_a(rgba.a);

  color_proto.set_color_space(EncodeColorSpace(color.GetColorSpace()));
}

Color DecodeColor(const proto::Color& color_proto) {
  const ColorSpace color_space = DecodeColorSpace(color_proto.color_space());
  return Color::FromFloat(color_proto.r(), color_proto.g(), color_proto.b(),
                          color_proto.a(), Color::Format::kLinear, color_space);
}

proto::ColorSpace EncodeColorSpace(ColorSpace color_space) {
  switch (color_space) {
    case ColorSpace::kSrgb:
      return proto::ColorSpace::COLOR_SPACE_SRGB;
    case ColorSpace::kDisplayP3:
      return proto::ColorSpace::COLOR_SPACE_DISPLAY_P3;
  }
  ABSL_LOG(FATAL) << "Unknown ColorSpace " << color_space;
  return {};
}

ColorSpace DecodeColorSpace(const proto::ColorSpace& color_space_proto) {
  switch (color_space_proto) {
    case proto::ColorSpace::COLOR_SPACE_UNSPECIFIED:
      ABSL_LOG(WARNING) << "COLOR_SPACE_UNSPECIFIED; falling back to sRGB.";
      return ColorSpace::kSrgb;
    case proto::ColorSpace::COLOR_SPACE_SRGB:
      return ColorSpace::kSrgb;
    case proto::ColorSpace::COLOR_SPACE_DISPLAY_P3:
      return ColorSpace::kDisplayP3;
  }
  ABSL_LOG(WARNING) << "Unknown proto::ColorSpace " << color_space_proto
                    << "; falling back to sRGB.";
  return ColorSpace::kSrgb;
}

}  // namespace ink
