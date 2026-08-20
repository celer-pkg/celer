// Copyright 2025 Google LLC
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

#ifndef INK_STROKES_INTERNAL_JNI_IN_PROGRESS_STROKE_JNI_HELPER_H_
#define INK_STROKES_INTERNAL_JNI_IN_PROGRESS_STROKE_JNI_HELPER_H_

#include <jni.h>

#include <cstdint>
#include <vector>

#include "absl/log/absl_check.h"
#include "ink/strokes/in_progress_stroke.h"

namespace ink::jni {

namespace internal {

// Associates an `InProgressStroke` with a cached triangle index buffer instance
// that is used for converting 32-bit indices to 16-bit indices, without needing
// to allocate a new buffer for this conversion every time.

// TODO: b/294561921 - Delete this when the underlying index data is in 16-bit
//   values.
struct InProgressStrokeWrapper {
  InProgressStroke in_progress_stroke;

  // For each brush coat, holds data previously returned by
  // NativeGetRawTriangleIndexData as a direct byte buffer, so that
  // conversion from 32-bit to 16-bit indices does not need to happen at the JVM
  // layer with JVM allocations.
  std::vector<std::vector<uint16_t>> triangle_index_data_caches;
};

}  // namespace internal

// Creates a new stack-allocated
// `ink::jni::internal::InProgressStrokeWrapper` containing an empty
// `InProgressStroke` and returns a pointer to it as a jlong, suitable for
// wrapping in a Kotlin InProgressStroke.
inline jlong NewNativeInProgressStroke() {
  return reinterpret_cast<jlong>(new internal::InProgressStrokeWrapper());
}

// Casts a Kotlin InProgressStroke.nativePointer to a mutable reference to a
// C++ InProgressStroke.
inline InProgressStroke& CastToInProgressStroke(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  return reinterpret_cast<internal::InProgressStrokeWrapper*>(native_pointer)
      ->in_progress_stroke;
}

// Returns a reference to the triangle index data caches for the given
// Kotlin InProgressStroke.nativePointer.
inline std::vector<std::vector<uint16_t>>&
NativeInProgressStrokeTriangleIndexDataCaches(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  return reinterpret_cast<internal::InProgressStrokeWrapper*>(native_pointer)
      ->triangle_index_data_caches;
}

// Frees a Kotlin InProgressStroke.nativePointer.
inline void DeleteNativeInProgressStroke(jlong native_pointer) {
  if (native_pointer == 0) return;
  delete reinterpret_cast<internal::InProgressStrokeWrapper*>(native_pointer);
}

}  // namespace ink::jni

#endif  // INK_STROKES_INTERNAL_JNI_IN_PROGRESS_STROKE_JNI_HELPER_H_
