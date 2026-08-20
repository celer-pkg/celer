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

#include <jni.h>

#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::Point;
using ::ink::Rect;
using ::ink::jni::CreateJImmutableVecFromPointOrThrow;
using ::ink::jni::FillJMutableVecFromPointOrThrow;

}  // namespace

extern "C" {

JNI_METHOD(geometry, BoxNative, jobject, createCenter)
(JNIEnv* env, jobject object, float rect_x_min, jfloat rect_y_min,
 jfloat rect_x_max, jfloat rect_y_max) {
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  Point point = rect.Center();

  return CreateJImmutableVecFromPointOrThrow(env, point);
}

JNI_METHOD(geometry, BoxNative, void, populateCenter)
(JNIEnv* env, jobject object, float rect_x_min, jfloat rect_y_min,
 jfloat rect_x_max, jfloat rect_y_max, jobject mutable_vec) {
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  Point point = rect.Center();

  FillJMutableVecFromPointOrThrow(env, mutable_vec, point);
}

JNI_METHOD(geometry, BoxNative, jboolean, containsPoint)
(JNIEnv* env, jobject object, jfloat rect_x_min, jfloat rect_y_min,
 jfloat rect_x_max, jfloat rect_y_max, jfloat point_x, jfloat point_y) {
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  return rect.Contains(Point{point_x, point_y});
}

JNI_METHOD(geometry, BoxNative, jboolean, containsBox)
(JNIEnv* env, jobject object, jfloat rect_x_min, jfloat rect_y_min,
 jfloat rect_x_max, jfloat rect_y_max, jfloat other_x_min, jfloat other_y_min,
 jfloat other_x_max, jfloat other_y_max) {
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  Rect other_rect = Rect::FromTwoPoints({other_x_min, other_y_min},
                                        {other_x_max, other_y_max});
  return rect.Contains(other_rect);
}

}  // extern "C
