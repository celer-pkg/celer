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

#ifndef INK_GEOMETRY_INTERNAL_LEGACY_TRIANGLE_CONTAINS_H_
#define INK_GEOMETRY_INTERNAL_LEGACY_TRIANGLE_CONTAINS_H_

#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"

namespace ink::geometry_internal {

// TODO: b/285173774 - This file contains a copy of a legacy geometry algorithm
// adapted to work on new types. It should be deleted once tessellation code can
// move to the equivalent algorithm in ../geometry/triangle.h

bool LegacyTriangleContains(const Triangle& triangle, Point p);

}  // namespace ink::geometry_internal

#endif  // INK_GEOMETRY_INTERNAL_LEGACY_TRIANGLE_CONTAINS_H_
