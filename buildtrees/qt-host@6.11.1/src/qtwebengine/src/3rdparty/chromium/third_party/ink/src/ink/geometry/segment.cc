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

#include "ink/geometry/segment.h"

#include <optional>
#include <string>

#include "absl/strings/str_cat.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink {

Point Segment::Lerp(float ratio) const {
  const float reverse = 1.0f - ratio;
  return Point{start.x * reverse + end.x * ratio,
               start.y * reverse + end.y * ratio};
}

std::optional<float> Segment::Project(Point point) const {
  if (start == end) {
    return std::nullopt;
  }
  // Sometimes start is not exactly equal to the end, but close enough that the
  // magnitude-squared still is not positive due to floating-point
  // loss-of-precision.
  float magnitude_squared = Vector().MagnitudeSquared();
  if (magnitude_squared <= 0) {
    return std::nullopt;
  }
  return Vec::DotProduct(point - start, Vector()) / magnitude_squared;
}

namespace segment_internal {

std::string ToFormattedString(Segment segment) {
  return absl::StrCat("Segment[", segment.start, " -> ", segment.end, "]");
}

}  // namespace segment_internal
}  // namespace ink
