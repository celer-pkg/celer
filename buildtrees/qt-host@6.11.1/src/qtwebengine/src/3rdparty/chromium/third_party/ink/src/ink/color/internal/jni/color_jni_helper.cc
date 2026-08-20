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

#include "ink/color/internal/jni/color_jni_helper.h"

#include "absl/log/absl_check.h"
#include "ink/color/color_space.h"

namespace ink::jni {

ColorSpace JIntToColorSpace(jint color_space_id) {
  switch (color_space_id) {
    case kJniColorSpaceIdSrgb:
      return ColorSpace::kSrgb;
    case kJniColorSpaceIdDisplayP3:
      return ColorSpace::kDisplayP3;
    default:
      ABSL_CHECK(false) << "Unknown color space id: " << color_space_id;
  }
}

jint ColorSpaceToJInt(ColorSpace color_space) {
  switch (color_space) {
    case ColorSpace::kSrgb:
      return kJniColorSpaceIdSrgb;
    case ColorSpace::kDisplayP3:
      return kJniColorSpaceIdDisplayP3;
  }
  ABSL_CHECK(false) << "Unknown color space: " << color_space;
}

bool ColorSpaceIsSupportedInJetpack(ColorSpace color_space) {
  // Currently this is defensive coding, all of the color spaces supported in
  // Ink C++ code are also supported in the Color class used in Jetpack.
  switch (color_space) {
    case ColorSpace::kSrgb:
    case ColorSpace::kDisplayP3:
      return true;
  }
  return false;
}

}  // namespace ink::jni
