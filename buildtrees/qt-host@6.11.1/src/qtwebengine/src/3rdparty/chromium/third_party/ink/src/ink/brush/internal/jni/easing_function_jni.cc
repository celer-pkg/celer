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

#include <utility>
#include <variant>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/brush/easing_function.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/geometry/point.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"

namespace {

using ink::EasingFunction;
using ink::Point;
using ink::jni::CastToEasingFunction;
using ink::jni::DeleteNativeEasingFunction;
using ink::jni::NewNativeEasingFunction;
using ink::jni::ThrowExceptionFromStatus;

jlong ValidateAndHoistEasingFunctionOrThrow(
    EasingFunction::Parameters parameters, JNIEnv* env) {
  EasingFunction easing_function{.parameters = std::move(parameters)};
  if (absl::Status status =
          ink::brush_internal::ValidateEasingFunction(easing_function);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  return NewNativeEasingFunction(std::move(easing_function));
}

// Helper type for visitor pattern. This is a quick way of constructing a
// visitor instance with overloaded operator() from an initializer list of
// lambdas. An advantage of using this approach with visitor is that the
// compiler will check that the overloads are exhaustive.
template <class... Ts>
struct overloads : Ts... {
  using Ts::operator()...;
};

// Type-deduction guide required for overloads{...} to work properly as a
// constructor in some C++ versions.
template <class... Ts>
overloads(Ts...) -> overloads<Ts...>;

static constexpr int kPredefined = 0;
static constexpr int kCubicBezier = 1;
static constexpr int kLinear = 2;
static constexpr int kSteps = 3;

}  // namespace

extern "C" {

JNI_METHOD(brush, EasingFunctionNative, jlong, createPredefined)
(JNIEnv* env, jobject thiz, jint predefined_response_curve) {
  return ValidateAndHoistEasingFunctionOrThrow(
      static_cast<EasingFunction::Predefined>(predefined_response_curve), env);
}

JNI_METHOD(brush, EasingFunctionNative, jlong, createCubicBezier)
(JNIEnv* env, jobject thiz, jfloat x1, jfloat y1, jfloat x2, jfloat y2) {
  return ValidateAndHoistEasingFunctionOrThrow(
      EasingFunction::CubicBezier{.x1 = x1, .y1 = y1, .x2 = x2, .y2 = y2}, env);
}

JNI_METHOD(brush, EasingFunctionNative, jlong, createSteps)
(JNIEnv* env, jobject thiz, jint step_count, jint step_position) {
  return ValidateAndHoistEasingFunctionOrThrow(
      EasingFunction::Steps{
          .step_count = step_count,
          .step_position =
              static_cast<EasingFunction::StepPosition>(step_position),
      },
      env);
}

JNI_METHOD(brush, EasingFunctionNative, jlong, createLinear)
(JNIEnv* env, jobject thiz, jfloatArray points_array) {
  jsize num_points = env->GetArrayLength(points_array) / 2;
  jfloat* points_elements = env->GetFloatArrayElements(points_array, nullptr);
  ABSL_CHECK(points_elements != nullptr);
  std::vector<Point> points_vector;
  points_vector.reserve(num_points);
  for (int i = 0; i < num_points; ++i) {
    float x = points_elements[2 * i];
    float y = points_elements[2 * i + 1];
    points_vector.push_back(Point{x, y});
  }
  // Don't need to copy back the array, which is not modified.
  env->ReleaseFloatArrayElements(points_array, points_elements, JNI_ABORT);
  return ValidateAndHoistEasingFunctionOrThrow(
      EasingFunction::Linear{.points = std::move(points_vector)}, env);
}

JNI_METHOD(brush, EasingFunctionNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeEasingFunction(native_pointer);
}

JNI_METHOD(brush, EasingFunctionNative, jlong, getParametersType)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  constexpr auto visitor = overloads{
      [](const EasingFunction::Predefined&) { return kPredefined; },
      [](const EasingFunction::CubicBezier&) { return kCubicBezier; },
      [](const EasingFunction::Steps&) { return kSteps; },
      [](const EasingFunction::Linear&) { return kLinear; },
  };
  return static_cast<jint>(
      std::visit(visitor, CastToEasingFunction(native_pointer).parameters));
}

JNI_METHOD(brush, EasingFunctionNative, jint, getPredefinedValueInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return static_cast<jint>(std::get<EasingFunction::Predefined>(
      CastToEasingFunction(native_pointer).parameters));
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getCubicBezierX1)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return std::get<EasingFunction::CubicBezier>(
             CastToEasingFunction(native_pointer).parameters)
      .x1;
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getCubicBezierY1)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return std::get<EasingFunction::CubicBezier>(
             CastToEasingFunction(native_pointer).parameters)
      .y1;
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getCubicBezierX2)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return std::get<EasingFunction::CubicBezier>(
             CastToEasingFunction(native_pointer).parameters)
      .x2;
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getCubicBezierY2)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return std::get<EasingFunction::CubicBezier>(
             CastToEasingFunction(native_pointer).parameters)
      .y2;
}

JNI_METHOD(brush, EasingFunctionNative, jint, getLinearNumPoints)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return static_cast<jint>(std::get<EasingFunction::Linear>(
                               CastToEasingFunction(native_pointer).parameters)
                               .points.size());
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getLinearPointX)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index) {
  return std::get<EasingFunction::Linear>(
             CastToEasingFunction(native_pointer).parameters)
      .points[index]
      .x;
}

JNI_METHOD(brush, EasingFunctionNative, jfloat, getLinearPointY)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index) {
  return std::get<EasingFunction::Linear>(
             CastToEasingFunction(native_pointer).parameters)
      .points[index]
      .y;
}

JNI_METHOD(brush, EasingFunctionNative, jint, getStepsCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return std::get<EasingFunction::Steps>(
             CastToEasingFunction(native_pointer).parameters)
      .step_count;
}

JNI_METHOD(brush, EasingFunctionNative, jint, getStepsPositionInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return static_cast<jint>(std::get<EasingFunction::Steps>(
                               CastToEasingFunction(native_pointer).parameters)
                               .step_position);
}

}  // extern "C"
