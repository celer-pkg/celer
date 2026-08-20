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

#include "ink/brush/brush.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/geometry/internal/jni/partitioned_mesh_jni_helper.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/jni/stroke_input_jni_helper.h"
#include "ink/strokes/internal/jni/stroke_jni_helper.h"
#include "ink/strokes/stroke.h"

namespace {

using ::ink::Stroke;
using ::ink::jni::CastToBrush;
using ::ink::jni::CastToPartitionedMesh;
using ::ink::jni::CastToStroke;
using ::ink::jni::CastToStrokeInputBatch;
using ::ink::jni::DeleteNativeStroke;
using ::ink::jni::NewNativePartitionedMesh;
using ::ink::jni::NewNativeStroke;
using ::ink::jni::NewNativeStrokeInputBatch;

}  // namespace

extern "C" {

JNI_METHOD(strokes, StrokeNative, jlong, createWithBrushAndInputs)
(JNIEnv* env, jobject object, jlong brush_native_pointer,
 jlong inputs_native_pointer) {
  return NewNativeStroke(Stroke(CastToBrush(brush_native_pointer),
                                CastToStrokeInputBatch(inputs_native_pointer)));
}

JNI_METHOD(strokes, StrokeNative, jlong, createWithBrushInputsAndShape)
(JNIEnv* env, jobject object, jlong brush_native_pointer,
 jlong inputs_native_pointer, jlong partitioned_mesh_native_pointer) {
  return NewNativeStroke(
      Stroke(CastToBrush(brush_native_pointer),
             CastToStrokeInputBatch(inputs_native_pointer),
             CastToPartitionedMesh(partitioned_mesh_native_pointer)));
}

// Make a heap-allocated shallow (doesn't replicate all the individual input
// points) copy of the `StrokeInputBatch` belonging to this `Stroke`. Return
// the raw pointer to this copy, so that it can be wrapped by a JVM
// `StrokeInputBatch`, which is responsible for freeing the copy when it is
// garbage collected and finalized.
JNI_METHOD(strokes, StrokeNative, jlong, newShallowCopyOfInputs)
(JNIEnv* env, jobject object, jlong native_pointer_to_stroke) {
  return NewNativeStrokeInputBatch(
      CastToStroke(native_pointer_to_stroke).GetInputs());
}

// Make a heap-allocated shallow (doesn't replicate all the individual meshes)
// copy of the `PartitionedMesh` belonging to this `Stroke`. Return the raw
// pointer to this copy, so that it can be wrapped by a JVM `PartitionedMesh`,
// which is responsible for freeing the copy when it is garbage collected and
// finalized.
JNI_METHOD(strokes, StrokeNative, jlong, newShallowCopyOfShape)
(JNIEnv* env, jobject object, jlong native_pointer_to_stroke) {
  return NewNativePartitionedMesh(
      CastToStroke(native_pointer_to_stroke).GetShape());
}

// Free the given `Stroke`.
JNI_METHOD(strokes, StrokeNative, void, free)
(JNIEnv* env, jobject object, jlong native_pointer_to_stroke) {
  DeleteNativeStroke(native_pointer_to_stroke);
}

}  // extern "C"
