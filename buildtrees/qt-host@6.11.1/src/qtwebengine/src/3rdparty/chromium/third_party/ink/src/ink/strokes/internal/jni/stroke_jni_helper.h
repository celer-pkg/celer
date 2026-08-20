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

#ifndef INK_STROKES_INTERNAL_JNI_STROKE_JNI_HELPER_H_
#define INK_STROKES_INTERNAL_JNI_STROKE_JNI_HELPER_H_

#include <jni.h>

#include "absl/log/absl_check.h"
#include "ink/strokes/stroke.h"

namespace ink::jni {

// Creates a new stack-allocated copy of the `Stroke` and returns a pointer
// to it as a jlong, suitable for wrapping in a Kotlin Stroke.
inline jlong NewNativeStroke(const Stroke& stroke) {
  return reinterpret_cast<jlong>(new Stroke(stroke));
}

// Casts a Kotlin Stroke.nativePointer to a C++ Stroke. The returned
// Stroke is a const ref as the Kotlin Stroke is immutable.
inline const Stroke& CastToStroke(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  return *reinterpret_cast<Stroke*>(native_pointer);
}

// Frees a Kotlin Stroke.nativePointer.
inline void DeleteNativeStroke(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  delete reinterpret_cast<Stroke*>(native_pointer);
}

}  // namespace ink::jni

#endif  // INK_STROKES_INTERNAL_JNI_STROKE_JNI_HELPER_H_
