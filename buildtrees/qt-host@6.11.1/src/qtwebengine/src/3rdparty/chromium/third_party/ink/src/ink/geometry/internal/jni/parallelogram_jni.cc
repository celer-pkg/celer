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

#include <array>
#include <utility>

#include "ink/geometry/angle.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/jni/rect_jni_helper.h"
#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/vec.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace {

using ::ink::Angle;
using ::ink::Envelope;
using ::ink::Point;
using ::ink::Quad;
using ::ink::Vec;
using ::ink::jni::ClassImmutableVec;
using ::ink::jni::CreateJImmutableBoxFromRectOrThrow;
using ::ink::jni::CreateJImmutableVecFromPointOrThrow;
using ::ink::jni::CreateJImmutableVecFromVecOrThrow;
using ::ink::jni::FillJMutableBoxFromRectOrThrow;
using ::ink::jni::FillJMutableVecFromPointOrThrow;
using ::ink::jni::FillJMutableVecFromVecOrThrow;

}  // namespace

extern "C" {

JNI_METHOD(geometry, ParallelogramNative, jobject, createBoundingBox)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);

  return CreateJImmutableBoxFromRectOrThrow(env,
                                            Envelope(quad).AsRect().value());
}

JNI_METHOD(geometry, ParallelogramNative, void, populateBoundingBox)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor, jobject mutable_box) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);

  FillJMutableBoxFromRectOrThrow(env, mutable_box,
                                 Envelope(quad).AsRect().value());
}

JNI_METHOD(geometry, ParallelogramNative, jobjectArray, createSemiAxes)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor,
 jclass immutable_vec_class) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);
  std::pair<Vec, Vec> axes = quad.SemiAxes();
  jobjectArray vector_array =
      env->NewObjectArray(2, ClassImmutableVec(env), nullptr);
  env->SetObjectArrayElement(
      vector_array, 0, CreateJImmutableVecFromVecOrThrow(env, axes.first));
  env->SetObjectArrayElement(
      vector_array, 1, CreateJImmutableVecFromVecOrThrow(env, axes.second));
  return vector_array;
}

JNI_METHOD(geometry, ParallelogramNative, void, populateSemiAxes)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor, jobject out_axis1,
 jobject out_axis2) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);
  std::pair<Vec, Vec> axes = quad.SemiAxes();
  FillJMutableVecFromVecOrThrow(env, out_axis1, axes.first);
  if (env->ExceptionCheck()) return;
  FillJMutableVecFromVecOrThrow(env, out_axis2, axes.second);
}

JNI_METHOD(geometry, ParallelogramNative, jobjectArray, createCorners)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);
  std::array<Point, 4> corners = quad.Corners();
  jobjectArray vector_array =
      env->NewObjectArray(4, ClassImmutableVec(env), nullptr);
  for (int i = 0; i < 4; ++i) {
    jobject corner = CreateJImmutableVecFromPointOrThrow(env, corners[i]);
    if (env->ExceptionCheck()) return nullptr;
    env->SetObjectArrayElement(vector_array, i, corner);
  }
  return vector_array;
}

JNI_METHOD(geometry, ParallelogramNative, void, populateCorners)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor, jobject out_corner1,
 jobject out_corner2, jobject out_corner3, jobject out_corner4) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);
  std::array<Point, 4> corners = quad.Corners();
  FillJMutableVecFromPointOrThrow(env, out_corner1, corners[0]);
  if (env->ExceptionCheck()) return;
  FillJMutableVecFromPointOrThrow(env, out_corner2, corners[1]);
  if (env->ExceptionCheck()) return;
  FillJMutableVecFromPointOrThrow(env, out_corner3, corners[2]);
  if (env->ExceptionCheck()) return;
  FillJMutableVecFromPointOrThrow(env, out_corner4, corners[3]);
}

JNI_METHOD(geometry, ParallelogramNative, jboolean, contains)
(JNIEnv* env, jobject object, jfloat center_x, jfloat center_y, jfloat width,
 jfloat height, jfloat rotation, jfloat shear_factor, jfloat point_x,
 jfloat point_y) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      {center_x, center_y}, width, height, Angle::Radians(rotation),
      shear_factor);
  return quad.Contains({point_x, point_y});
}

}  // extern "C"
