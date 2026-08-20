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

#include "ink/geometry/rect.h"

#include <cmath>
#include <string>

#include "absl/log/absl_check.h"
#include "absl/strings/str_format.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"

namespace ink {

Point Rect::Center() const {
  // Halve each bound before adding, rather than adding first and then halving
  // the sum, to avoid overflow for very large Rects.
  float x = 0.5f * x_min_ + 0.5f * x_max_;
  float y = 0.5f * y_min_ + 0.5f * y_max_;
  // If x_min_ = -inf and x_max_ = inf, then x will be NaN, but 0 would be a
  // more sensible result for the center of an infinite Rect (and same for y).
  return Point{std::isnan(x) ? 0.f : x, std::isnan(y) ? 0.f : y};
}

Segment Rect::GetEdge(int index) const {
  switch (index) {
    case 0:
      return Segment{Point{x_min_, y_min_}, Point{x_max_, y_min_}};
    case 1:
      return Segment{Point{x_max_, y_min_}, Point{x_max_, y_max_}};
    case 2:
      return Segment{Point{x_max_, y_max_}, Point{x_min_, y_max_}};
    case 3:
      return Segment{Point{x_min_, y_max_}, Point{x_min_, y_min_}};
    default:
      ABSL_CHECK(false) << "Index " << index << " out of bounds";
  }
}

Rect Rect::ContainingRectWithAspectRatio(float aspect_ratio) const {
  ABSL_CHECK(aspect_ratio > 0) << "Aspect ratio cannot be <= 0";
  float corrected_width;
  float corrected_height;
  if (aspect_ratio > AspectRatio()) {
    corrected_height = Height();
    corrected_width = Height() * aspect_ratio;
  } else {
    corrected_width = Width();
    corrected_height = Width() / aspect_ratio;
  }
  return Rect::FromCenterAndDimensions(Center(), corrected_width,
                                       corrected_height);
}

Rect Rect::InteriorRectWithAspectRatio(float aspect_ratio) const {
  ABSL_CHECK(aspect_ratio >= 0) << "Aspect ratio cannot be < 0";
  float corrected_width;
  float corrected_height;
  if (aspect_ratio > AspectRatio()) {
    corrected_width = Width();
    corrected_height = Width() / aspect_ratio;
  } else {
    corrected_height = Height();
    corrected_width = Height() * aspect_ratio;
  }
  return Rect::FromCenterAndDimensions(Center(), corrected_width,
                                       corrected_height);
}

std::string Rect::ToFormattedString() const {
  return absl::StrFormat("Rect[%v by %v from (%v, %v) to (%v, %v)]", Width(),
                         Height(), x_min_, y_min_, x_max_, y_max_);
}

}  // namespace ink
