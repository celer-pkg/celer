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

#ifndef INK_GEOMETRY_TRIANGLE_H_
#define INK_GEOMETRY_TRIANGLE_H_

#include <string>

#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/vec.h"

namespace ink {

// A Triangle, defined by its three corners.
struct Triangle {
  Point p0;
  Point p1;
  Point p2;

  // Returns the signed area of the Triangle. If the Triangle's
  // points wind in a positive direction (as defined by `Angle`),
  // then the Triangle's area will be positive. Otherwise, it will
  // be negative.
  float SignedArea() const { return .5 * Vec::Determinant(p1 - p0, p2 - p1); }

  // Returns true if the given point is contained within the
  // Triangle. Points that lie exactly on the Triangle's boundary
  // are considered to be contained.
  bool Contains(Point point) const;

  // Returns the segment of the Triangle between the point at `index` and
  // the point at `index` + 1 modulo 3.
  // `index` must be 0, 1, or 2.
  Segment GetEdge(int index) const;
};

bool operator==(const Triangle& lhs, const Triangle& rhs);
bool operator!=(const Triangle& lhs, const Triangle& rhs);

namespace triangle_internal {
std::string ToFormattedString(const Triangle& t);
}  // namespace triangle_internal

template <typename Sink>
void AbslStringify(Sink& sink, const Triangle& t) {
  sink.Append(triangle_internal::ToFormattedString(t));
}

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const Triangle& lhs, const Triangle& rhs) {
  return lhs.p0 == rhs.p0 && lhs.p1 == rhs.p1 && lhs.p2 == rhs.p2;
}
inline bool operator!=(const Triangle& lhs, const Triangle& rhs) {
  return !(lhs == rhs);
}

}  // namespace ink

#endif  // INK_GEOMETRY_TRIANGLE_H_
