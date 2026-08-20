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

#ifndef INK_GEOMETRY_SEGMENT_H_
#define INK_GEOMETRY_SEGMENT_H_

#include <cmath>
#include <optional>
#include <string>

#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"

namespace ink {

// This struct represents a directed line segment between two points.
struct Segment {
  Point start{0, 0};
  Point end{0, 0};

  // Returns the vector from the start of the segment to the end.
  //
  // Note that due to potential float overflow, this can return an infinite
  // vector even if the segment endpoints are finite.
  Vec Vector() const { return end - start; }

  // Returns the length of the segment.
  //
  // Note that due to potential float overflow, this can return infinity even if
  // the segment endpoints are finite.
  float Length() const { return Vector().Magnitude(); }

  // Returns the point halfway along the segment.
  //
  // If the segment endpoints are finite, this is guaranteed to avoid overflow
  // (i.e. it won't erroneously return an infinite point).
  Point Midpoint() const { return Lerp(0.5f); }

  // Returns the point on the segment at the given ratio of the segment's
  // length, measured from the start point. You may also think of this as
  // linearly interpolating from the start of the segment to the end. Values
  // outside the interval [0, 1] will be extrapolated along the infinite line
  // passing through this segment.
  //
  // If the segment endpoints are finite, and the ratio is in the interval [0,
  // 1], this is guaranteed to avoid overflow (i.e. it won't erroneously return
  // an infinite point).
  Point Lerp(float ratio) const;

  // Returns the "ratio" along the infinite line that coincides with this
  // segment, at which it is closest to the given point. This is the inverse of
  // `Lerp`. If you need the closest point on the segment itself, you can clamp
  // the value to the interval [0, 1], i.e.:
  //   std::clamp(segment.Project(point).value_or(0.f), 0.f, 1.f);
  // Returns std::nullopt if the start and end are the same or close enough that
  // Vector().MagnitudeSquared() <= 0 (which means the projection cannot be
  // reliably computed).
  std::optional<float> Project(Point point) const;
};

// Segments are considered equivalent only when their starting points are
// identical and their ending points are identical. Segments who have the same
// endpoints but run opposite directions are not considered equivalent.
bool operator==(const Segment& lhs, const Segment& rhs);
bool operator!=(const Segment& lhs, const Segment& rhs);

namespace segment_internal {
std::string ToFormattedString(Segment segment);
}  // namespace segment_internal

template <typename Sink>
void AbslStringify(Sink& sink, Segment segment) {
  sink.Append(segment_internal::ToFormattedString(segment));
}

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const Segment& lhs, const Segment& rhs) {
  return lhs.start == rhs.start && lhs.end == rhs.end;
}

inline bool operator!=(const Segment& lhs, const Segment& rhs) {
  return lhs.start != rhs.start || lhs.end != rhs.end;
}

}  // namespace ink

#endif  // INK_GEOMETRY_SEGMENT_H_
