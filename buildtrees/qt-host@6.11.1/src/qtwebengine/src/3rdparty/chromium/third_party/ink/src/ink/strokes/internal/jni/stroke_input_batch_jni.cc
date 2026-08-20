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

#include <optional>

#include "absl/status/status.h"
#include "ink/geometry/angle.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/jni/stroke_input_jni_helper.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

#define STROKE_INPUT_BATCH_JNI_METHOD(return_type, method_name) \
  JNI_METHOD(strokes, StrokeInputBatchNative, return_type, method_name)

#define MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(return_type, method_name) \
  JNI_METHOD(strokes, MutableStrokeInputBatchNative, return_type, method_name)

namespace {

using ::ink::Angle;
using ::ink::Duration32;
using ::ink::PhysicalDistance;
using ::ink::StrokeInput;
using ::ink::StrokeInputBatch;
using ::ink::jni::CastToMutableStrokeInputBatch;
using ::ink::jni::CastToStrokeInputBatch;
using ::ink::jni::DeleteNativeStrokeInputBatch;
using ::ink::jni::JIntToToolType;
using ::ink::jni::NewNativeStrokeInputBatch;
using ::ink::jni::ThrowExceptionFromStatus;
using ::ink::jni::ToolTypeToJInt;
using ::ink::jni::UpdateJObjectInputOrThrow;

}  // namespace

extern "C" {

// ******** Native Implementation of Immutable/Mutable StrokeInputBatch ********
STROKE_INPUT_BATCH_JNI_METHOD(jlong, create)
(JNIEnv* env, jobject thiz) { return NewNativeStrokeInputBatch(); }

STROKE_INPUT_BATCH_JNI_METHOD(void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeStrokeInputBatch(native_pointer);
}

STROKE_INPUT_BATCH_JNI_METHOD(jint, getSize)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToStrokeInputBatch(native_pointer).Size();
}

STROKE_INPUT_BATCH_JNI_METHOD(void, populate)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index, jobject j_input) {
  StrokeInput input = CastToStrokeInputBatch(native_pointer).Get(index);
  UpdateJObjectInputOrThrow(env, input, j_input);
}

STROKE_INPUT_BATCH_JNI_METHOD(jlong, getDurationMillis)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToStrokeInputBatch(native_pointer).GetDuration().ToMillis();
}

STROKE_INPUT_BATCH_JNI_METHOD(jint, getToolType)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return ToolTypeToJInt(CastToStrokeInputBatch(native_pointer).GetToolType());
}

STROKE_INPUT_BATCH_JNI_METHOD(jfloat, getStrokeUnitLengthCm)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  std::optional<PhysicalDistance> stroke_unit_length =
      CastToStrokeInputBatch(native_pointer).GetStrokeUnitLength();
  if (!stroke_unit_length.has_value()) return 0;
  return stroke_unit_length->ToCentimeters();
}

STROKE_INPUT_BATCH_JNI_METHOD(jboolean, hasStrokeUnitLength)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToStrokeInputBatch(native_pointer).HasStrokeUnitLength();
}

STROKE_INPUT_BATCH_JNI_METHOD(jboolean, hasPressure)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const StrokeInputBatch& batch = CastToStrokeInputBatch(native_pointer);
  return batch.HasPressure();
}

STROKE_INPUT_BATCH_JNI_METHOD(jboolean, hasTilt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const StrokeInputBatch& batch = CastToStrokeInputBatch(native_pointer);
  return batch.HasTilt();
}

STROKE_INPUT_BATCH_JNI_METHOD(jboolean, hasOrientation)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const StrokeInputBatch& batch = CastToStrokeInputBatch(native_pointer);
  return batch.HasOrientation();
}

STROKE_INPUT_BATCH_JNI_METHOD(jint, getNoiseSeed)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const StrokeInputBatch& batch = CastToStrokeInputBatch(native_pointer);
  return batch.GetNoiseSeed();
}

// ************ Native Implementation of MutableStrokeInputBatch ************
MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(jboolean, appendSingle)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint tool_type, jfloat x,
 jfloat y, jlong elapsed_time_millis, jfloat stroke_unit_length_cm,
 jfloat pressure, jfloat tilt, jfloat orientation) {
  StrokeInput input = {.tool_type = JIntToToolType(tool_type),
                       .position = {x, y},
                       .elapsed_time = Duration32::Millis(elapsed_time_millis),
                       .stroke_unit_length =
                           PhysicalDistance::Centimeters(stroke_unit_length_cm),
                       .pressure = pressure,
                       .tilt = Angle::Radians(tilt),
                       .orientation = Angle::Radians(orientation)};

  if (absl::Status status =
          CastToMutableStrokeInputBatch(native_pointer).Append(input);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return false;
  }
  return true;
}

MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(jboolean, appendBatch)
(JNIEnv* env, jobject thiz, jlong native_pointer,
 jlong append_from_native_pointer) {
  if (absl::Status status =
          CastToMutableStrokeInputBatch(native_pointer)
              .Append(CastToStrokeInputBatch(append_from_native_pointer));
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return false;
  }
  return true;
}

MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(void, clear)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  CastToMutableStrokeInputBatch(native_pointer).Clear();
}

MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(jlong, newCopy)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return NewNativeStrokeInputBatch(CastToStrokeInputBatch(native_pointer));
}

MUTABLE_STROKE_INPUT_BATCH_JNI_METHOD(void, setNoiseSeed)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint seed) {
  CastToMutableStrokeInputBatch(native_pointer).SetNoiseSeed(seed);
}

}  // extern "C"
