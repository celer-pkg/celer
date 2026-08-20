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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/jni/box_accumulator_jni_helper.h"
#include "ink/geometry/internal/jni/mesh_format_jni_helper.h"
#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_throw_util.h"
#include "ink/strokes/in_progress_stroke.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/jni/in_progress_stroke_jni_helper.h"
#include "ink/strokes/internal/jni/stroke_input_jni_helper.h"
#include "ink/strokes/internal/jni/stroke_jni_helper.h"
#include "ink/types/duration.h"

namespace {

using ::ink::Duration32;
using ::ink::Envelope;
using ::ink::InProgressStroke;
using ::ink::MutableMesh;
using ::ink::Point;
using ::ink::StrokeInput;
using ::ink::StrokeInputBatch;
using ::ink::jni::CastToBrush;
using ::ink::jni::CastToInProgressStroke;
using ::ink::jni::CastToMutableStrokeInputBatch;
using ::ink::jni::CastToStrokeInputBatch;
using ::ink::jni::DeleteNativeInProgressStroke;
using ::ink::jni::FillJBoxAccumulatorOrThrow;
using ::ink::jni::FillJMutableVecFromPointOrThrow;
using ::ink::jni::NativeInProgressStrokeTriangleIndexDataCaches;
using ::ink::jni::NewNativeInProgressStroke;
using ::ink::jni::NewNativeMeshFormat;
using ::ink::jni::NewNativeStroke;
using ::ink::jni::ThrowExceptionFromStatus;
using ::ink::jni::UpdateJObjectInputOrThrow;

}  // namespace

extern "C" {

// Construct a native InProgressStroke and return a pointer to it as a long.
JNI_METHOD(strokes, InProgressStrokeNative, jlong, create)
(JNIEnv* env, jobject thiz) { return NewNativeInProgressStroke(); }

JNI_METHOD(strokes, InProgressStrokeNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeInProgressStroke(native_pointer);
}

JNI_METHOD(strokes, InProgressStrokeNative, void, clear)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  CastToInProgressStroke(native_pointer).Clear();
}

// Starts the stroke with a brush.
JNI_METHOD(strokes, InProgressStrokeNative, void, start)
(JNIEnv* env, jobject thiz, jlong native_pointer, jlong brush_native_pointer,
 jint noise_seed) {
  CastToInProgressStroke(native_pointer)
      .Start(CastToBrush(brush_native_pointer), noise_seed);
}

JNI_METHOD(strokes, InProgressStrokeNative, jboolean, enqueueInputs)
(JNIEnv* env, jobject thiz, jlong native_pointer, jlong real_inputs_pointer,
 jlong predicted_inputs_pointer) {
  InProgressStroke& in_progress_stroke = CastToInProgressStroke(native_pointer);
  const StrokeInputBatch& real_inputs =
      CastToStrokeInputBatch(real_inputs_pointer);
  const StrokeInputBatch& predicted_inputs =
      CastToStrokeInputBatch(predicted_inputs_pointer);
  if (absl::Status status =
          in_progress_stroke.EnqueueInputs(real_inputs, predicted_inputs);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return false;
  }
  return true;
}

JNI_METHOD(strokes, InProgressStrokeNative, jboolean, updateShape)
(JNIEnv* env, jobject thiz, jlong native_pointer,
 jlong j_current_elapsed_time_millis) {
  InProgressStroke& in_progress_stroke = CastToInProgressStroke(native_pointer);
  Duration32 current_elapsed_time =
      Duration32::Millis(j_current_elapsed_time_millis);
  if (absl::Status status =
          in_progress_stroke.UpdateShape(current_elapsed_time);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return false;
  }
  return true;
}

JNI_METHOD(strokes, InProgressStrokeNative, void, finishInput)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  CastToInProgressStroke(native_pointer).FinishInputs();
}

JNI_METHOD(strokes, InProgressStrokeNative, jboolean, isInputFinished)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToInProgressStroke(native_pointer).InputsAreFinished();
}

JNI_METHOD(strokes, InProgressStrokeNative, jboolean, isUpdateNeeded)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToInProgressStroke(native_pointer).NeedsUpdate();
}

JNI_METHOD(strokes, InProgressStrokeNative, jlong, newStrokeFromCopy)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return NewNativeStroke(CastToInProgressStroke(native_pointer).CopyToStroke());
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getInputCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToInProgressStroke(native_pointer).InputCount();
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getRealInputCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  return in_progress_stroke.RealInputCount();
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getPredictedInputCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  return in_progress_stroke.PredictedInputCount();
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, populateInputs)
(JNIEnv* env, jobject thiz, jlong native_pointer,
 jlong mutable_stroke_input_batch_pointer, jint from, jint to) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  StrokeInputBatch& batch =
      CastToMutableStrokeInputBatch(mutable_stroke_input_batch_pointer);
  batch.Clear();
  const StrokeInputBatch& inputs = in_progress_stroke.GetInputs();
  for (int i = from; i < to; ++i) {
    // The input here should have already been validated.
    ABSL_CHECK_OK(batch.Append(inputs.Get(i)));
  }
  return in_progress_stroke.GetInputs().Size();
}

JNI_METHOD(strokes, InProgressStrokeNative, void, getAndOverwriteInput)
(JNIEnv* env, jobject thiz, jlong native_pointer, jobject j_input, jint index,
 jclass input_tool_type_class) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  StrokeInput input = in_progress_stroke.GetInputs().Get(index);
  UpdateJObjectInputOrThrow(env, input, j_input);
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getBrushCoatCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToInProgressStroke(native_pointer).BrushCoatCount();
}

JNI_METHOD(strokes, InProgressStrokeNative, void, getMeshBounds)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jobject j_out_envelope) {
  FillJBoxAccumulatorOrThrow(
      env, CastToInProgressStroke(native_pointer).GetMeshBounds(coat_index),
      j_out_envelope);
}

JNI_METHOD(strokes, InProgressStrokeNative, void, fillUpdatedRegion)
(JNIEnv* env, jobject thiz, jlong native_pointer, jobject j_out_envelope) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  const Envelope& updated_region = in_progress_stroke.GetUpdatedRegion();
  FillJBoxAccumulatorOrThrow(env, updated_region, j_out_envelope);
}

JNI_METHOD(strokes, InProgressStrokeNative, void, resetUpdatedRegion)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  CastToInProgressStroke(native_pointer).ResetUpdatedRegion();
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getOutlineCount)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index) {
  return CastToInProgressStroke(native_pointer)
      .GetCoatOutlines(coat_index)
      .size();
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getOutlineVertexCount)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint outline_index) {
  return CastToInProgressStroke(native_pointer)
      .GetCoatOutlines(coat_index)[outline_index]
      .size();
}

JNI_METHOD(strokes, InProgressStrokeNative, void, fillOutlinePosition)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint outline_index, jint outline_vertex_index, jobject out_position) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  absl::Span<const uint32_t> outline =
      in_progress_stroke.GetCoatOutlines(coat_index)[outline_index];
  // TODO: b/294561921 - Implement multiple meshes.
  Point position = in_progress_stroke.GetMesh(coat_index)
                       .VertexPosition(outline[outline_vertex_index]);
  FillJMutableVecFromPointOrThrow(env, out_position, position);
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getMeshPartitionCount)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index) {
  // TODO: b/294561921 - Implement multiple meshes.
  return 1;
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getVertexCount)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint mesh_index) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  // TODO: b/294561921 - Implement multiple meshes.
  return in_progress_stroke.GetMesh(coat_index).VertexCount();
}

JNI_METHOD(strokes, InProgressStrokeNative, jobject, getRawVertexData)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint mesh_index) {
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);
  // TODO: b/294561921 - Implement multiple meshes.
  const absl::Span<const std::byte> raw_vertex_data =
      in_progress_stroke.GetMesh(coat_index).RawVertexData();
  if (raw_vertex_data.data() == nullptr) return nullptr;
  return env->NewDirectByteBuffer(
      // NewDirectByteBuffer needs a non-const void*. The resulting buffer is
      // writeable, but it will be wrapped at the Kotlin layer in a read-only
      // buffer that delegates to this one.
      const_cast<std::byte*>(raw_vertex_data.data()), raw_vertex_data.size());
}

// Returns a direct byte buffer of the triangle index data in 16-bit format.
// Automatically converts 32-bit to 16-bit data internally if needed, which
// omits any triangles beyond the point where index values first exceed the
// 16-bit maximum value. Those triangles are still present in the underlying
// data and will be included in `CopyToStroke`, but will not be returned by this
// method (which is typically used for rendering).
// TODO: b/294561921 - Simplify this when the underlying index data is in 16 bit
//   values.
JNI_METHOD(strokes, InProgressStrokeNative, jobject, getRawTriangleIndexData)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint mesh_index) {
  ABSL_CHECK_EQ(mesh_index, 0) << "Unsupported mesh index: " << mesh_index;
  const InProgressStroke& in_progress_stroke =
      CastToInProgressStroke(native_pointer);

  const MutableMesh& mesh = in_progress_stroke.GetMesh(coat_index);
  size_t index_stride = mesh.IndexStride();
  ABSL_CHECK(index_stride == sizeof(uint32_t))
      << "Unsupported index stride: " << index_stride;

  // If necessary, expand the list of caches.
  std::vector<std::vector<uint16_t>>& triangle_index_data_caches =
      NativeInProgressStrokeTriangleIndexDataCaches(native_pointer);
  if (coat_index >= static_cast<int>(triangle_index_data_caches.size())) {
    triangle_index_data_caches.resize(coat_index + 1);
  }

  std::vector<uint16_t>& triangle_index_data_cache =
      triangle_index_data_caches[coat_index];
  // Clear the contents, but don't give up any of the capacity because it will
  // be filled again right away.
  triangle_index_data_cache.clear();
  const absl::Span<const std::byte> raw_index_data = mesh.RawIndexData();
  uint32_t index_count = 3 * mesh.TriangleCount();
  for (uint32_t i = 0; i < index_count; ++i) {
    uint32_t i_byte = sizeof(uint32_t) * i;
    uint32_t triangle_index_32;
    // Interpret each set of 4 bytes as a 32-bit integer.
    std::memcpy(&triangle_index_32, &raw_index_data[i_byte], sizeof(uint32_t));
    // If that 32-bit integer would not fit into a 16-bit integer, then stop
    // copying content and return all the triangles that have been processed so
    // far.
    if (triangle_index_32 > std::numeric_limits<uint16_t>::max()) {
      // The offending index may have been in the middle of a triangle, so
      // rewind back to the previous multiple of 3 to just return whole
      // triangles.
      uint32_t i_last_multiple_of_3 = (i / 3) * 3;
      triangle_index_data_cache.erase(
          triangle_index_data_cache.begin() + i_last_multiple_of_3,
          triangle_index_data_cache.end());
      ABSL_LOG_EVERY_N_SEC(WARNING, 1)
          << "Triangle index data exceeds 16-bit limit, truncating.";
      break;
    }
    uint16_t triangle_index_16 = triangle_index_32;
    triangle_index_data_cache.push_back(triangle_index_16);
  }

  ABSL_CHECK_EQ(triangle_index_data_cache.size() % 3, 0u);

  // This direct byte buffer is writeable, but it will be wrapped at the Kotlin
  // layer in a read-only buffer that delegates to this one.
  if (triangle_index_data_cache.data() == nullptr) return nullptr;
  return env->NewDirectByteBuffer(
      triangle_index_data_cache.data(),
      triangle_index_data_cache.size() * sizeof(uint16_t));
}

JNI_METHOD(strokes, InProgressStrokeNative, jint, getTriangleIndexStride)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint mesh_index) {
  // The data is converted from uint32_t above in getRawTriangleIndexData.
  return sizeof(uint16_t);
}

// Return a newly allocated copy of the given `Mesh`'s `MeshFormat`.
JNI_METHOD(strokes, InProgressStrokeNative, jlong, newCopyOfMeshFormat)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint coat_index,
 jint mesh_index) {
  // TODO: b/294561921 - Implement multiple meshes.
  return NewNativeMeshFormat(
      CastToInProgressStroke(native_pointer).GetMesh(coat_index).Format());
}

}  // extern "C"
