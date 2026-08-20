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

#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"
#include "ink/types/duration.h"

namespace {

using ::ink::BrushBehavior;
using ::ink::BrushTip;
using ::ink::Duration32;
using ::ink::brush_internal::ValidateBrushTip;
using ::ink::jni::CastToBrushBehavior;
using ::ink::jni::CastToBrushTip;
using ::ink::jni::DeleteNativeBrushTip;
using ::ink::jni::NewNativeBrushBehavior;
using ::ink::jni::NewNativeBrushTip;
using ::ink::jni::ThrowExceptionFromStatus;

}  // namespace

extern "C" {

// Constructs a native BrushTip and returns a pointer to it as a long. Throws an
// exception if the BrushTip validation fails.
JNI_METHOD(brush, BrushTipNative, jlong, create)
(JNIEnv* env, jobject thiz, jfloat scale_x, jfloat scale_y,
 jfloat corner_rounding, jfloat slant_radians, jfloat pinch,
 jfloat rotation_radians, jfloat opacity_multiplier,
 jfloat particle_gap_distance_scale, jlong particle_gap_duration_millis,
 jlongArray behavior_native_pointers_array) {
  std::vector<ink::BrushBehavior> behaviors;
  const jsize num_behaviors =
      env->GetArrayLength(behavior_native_pointers_array);
  behaviors.reserve(num_behaviors);
  jlong* behavior_pointers =
      env->GetLongArrayElements(behavior_native_pointers_array, nullptr);
  ABSL_CHECK(behavior_pointers != nullptr);
  for (jsize i = 0; i < num_behaviors; ++i) {
    behaviors.push_back(CastToBrushBehavior(behavior_pointers[i]));
  }
  // No need to copy back the array, which is not modified.
  env->ReleaseLongArrayElements(behavior_native_pointers_array,
                                behavior_pointers, JNI_ABORT);
  BrushTip tip{
      .scale = ink::Vec{scale_x, scale_y},
      .corner_rounding = corner_rounding,
      .slant = ink::Angle::Radians(slant_radians),
      .pinch = pinch,
      .rotation = ink::Angle::Radians(rotation_radians),
      .opacity_multiplier = opacity_multiplier,
      .particle_gap_distance_scale = particle_gap_distance_scale,
      .particle_gap_duration = Duration32::Millis(particle_gap_duration_millis),
      .behaviors = behaviors};
  if (absl::Status status = ValidateBrushTip(tip); !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return -1;  // Unused return value.
  }
  return NewNativeBrushTip(std::move(tip));
}

JNI_METHOD(brush, BrushTipNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeBrushTip(native_pointer);
}

JNI_METHOD(brush, BrushTipNative, jfloat, getScaleX)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).scale.x;
}

JNI_METHOD(brush, BrushTipNative, jfloat, getScaleY)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).scale.y;
}

JNI_METHOD(brush, BrushTipNative, jfloat, getCornerRounding)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).corner_rounding;
}

JNI_METHOD(brush, BrushTipNative, jfloat, getSlantRadians)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).slant.ValueInRadians();
}

JNI_METHOD(brush, BrushTipNative, jfloat, getPinch)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).pinch;
}

JNI_METHOD(brush, BrushTipNative, jfloat, getRotationRadians)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).rotation.ValueInRadians();
}

JNI_METHOD(brush, BrushTipNative, jfloat, getOpacityMultiplier)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).opacity_multiplier;
}

JNI_METHOD(brush, BrushTipNative, jfloat, getParticleGapDistanceScale)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).particle_gap_distance_scale;
}

JNI_METHOD(brush, BrushTipNative, jlong, getParticleGapDurationMillis)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).particle_gap_duration.ToMillis();
}

JNI_METHOD(brush, BrushTipNative, jint, getBehaviorCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToBrushTip(native_pointer).behaviors.size();
}

JNI_METHOD(brush, BrushTipNative, jlong, newCopyOfBrushBehavior)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index) {
  return NewNativeBrushBehavior(
      CastToBrushTip(native_pointer).behaviors[index]);
}

}  // extern "C"
