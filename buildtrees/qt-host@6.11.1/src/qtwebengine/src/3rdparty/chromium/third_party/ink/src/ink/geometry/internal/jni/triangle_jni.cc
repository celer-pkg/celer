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

#include <jni.h>

#include "ink/geometry/point.h"
#include "ink/geometry/triangle.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::Point;
using ::ink::Triangle;

}  // namespace

extern "C" {

JNI_METHOD(geometry, TriangleNative, jboolean, contains)
(JNIEnv* env, jobject object, jfloat triangle_p0_x, jfloat triangle_p0_y,
 jfloat triangle_p1_x, jfloat triangle_p1_y, jfloat triangle_p2_x,
 jfloat triangle_p2_y, jfloat point_x, jfloat point_y) {
  Point point = Point{point_x, point_y};
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  return triangle.Contains(point);
}
}
