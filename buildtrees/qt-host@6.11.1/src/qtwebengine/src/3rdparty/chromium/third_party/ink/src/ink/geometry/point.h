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

#ifndef INK_GEOMETRY_POINT_H_
#define INK_GEOMETRY_POINT_H_

#include <string>

#include "ink/geometry/vec.h"

namespace ink {

// A location in 2-dimensional space.
struct Point {
  float x = 0;
  float y = 0;

  // Returns the offset vector from the origin to this point.
  Vec Offset() const { return Vec{x, y}; }
};

inline constexpr Point kOrigin{0, 0};

bool operator==(Point lhs, Point rhs);
bool operator!=(Point lhs, Point rhs);

Vec operator-(Point lhs, Point rhs);

Point operator+(Point p, Vec v);
Point operator+(Vec v, Point p);
Point operator-(Point p, Vec v);

Point &operator+=(Point &p, Vec v);
Point &operator-=(Point &p, Vec v);

namespace point_internal {
std::string ToFormattedString(Point p);
}  // namespace point_internal

template <typename Sink>
void AbslStringify(Sink &sink, Point p) {
  sink.Append(point_internal::ToFormattedString(p));
}

template <typename H>
H AbslHashValue(H h, Point p) {
  return H::combine(std::move(h), p.x, p.y);
}

////////////////////////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////////////////////////

inline bool operator==(Point lhs, Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline bool operator!=(Point lhs, Point rhs) { return !(lhs == rhs); }

inline Vec operator-(Point lhs, Point rhs) {
  return Vec{.x = lhs.x - rhs.x, .y = lhs.y - rhs.y};
}

inline Point operator+(Point p, Vec v) {
  return {.x = v.x + p.x, .y = v.y + p.y};
}
inline Point operator+(Vec v, Point p) {
  return {.x = v.x + p.x, .y = v.y + p.y};
}
inline Point operator-(Point p, Vec v) {
  return {.x = p.x - v.x, .y = p.y - v.y};
}

inline Point &operator+=(Point &p, Vec v) {
  p.x += v.x;
  p.y += v.y;
  return p;
}
inline Point &operator-=(Point &p, Vec v) {
  p.x -= v.x;
  p.y -= v.y;
  return p;
}

}  // namespace ink

#endif  // INK_GEOMETRY_POINT_H_
