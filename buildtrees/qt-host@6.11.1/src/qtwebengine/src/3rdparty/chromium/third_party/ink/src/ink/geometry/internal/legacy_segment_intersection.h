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

#ifndef INK_GEOMETRY_INTERNAL_LEGACY_SEGMENT_INTERSECTIONS_H_
#define INK_GEOMETRY_INTERNAL_LEGACY_SEGMENT_INTERSECTIONS_H_

#include <array>
#include <optional>

#include "absl/strings/str_cat.h"
#include "ink/geometry/segment.h"

namespace ink::geometry_internal {

// TODO: b/285173774 - This file contains copies of legacy geometry algorithms
// adapted to work on new types. These should be deleted once tessellation code
// can move to the equivalent algorithms in ../geometry/intersects.h.

struct LegacySegmentIntersection {
  // The length ratio parameter intervals over which the segments are coincident
  // (see Segment::Eval()). The intervals will be ordered with respect to
  // segment1, i.e. segment1_interval[0] corresponds to segment2_interval[0],
  // and segment1_interval[0] will be less than or equal to
  // segment1_interval[1].
  // If the segments intersect at a single point, the first and second element
  // of each interval will be equal.
  std::array<float, 2> segment1_interval{{-1, -1}};
  std::array<float, 2> segment2_interval{{-1, -1}};

  // The coincident portion of the segments. This may be a degenerate segment.
  Segment intx;
};

template <typename Sink>
void AbslStringify(Sink& sink, LegacySegmentIntersection intx) {
  sink.Append(absl::StrCat("[", intx.segment1_interval[0], ", ",
                           intx.segment1_interval[1], "], ", "[",
                           intx.segment2_interval[0], ", ",
                           intx.segment2_interval[1], "], ", intx.intx));
}

std::optional<LegacySegmentIntersection> LegacyIntersection(const Segment& a,
                                                            const Segment& b);

inline bool LegacyIntersects(const Segment& a, const Segment& b) {
  return LegacyIntersection(a, b).has_value();
}

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_LEGACY_SEGMENT_INTERSECTION_H_
