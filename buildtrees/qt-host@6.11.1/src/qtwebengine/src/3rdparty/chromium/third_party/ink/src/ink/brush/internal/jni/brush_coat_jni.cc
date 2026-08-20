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

#include "ink/brush/brush_coat.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::BrushCoat;
using ::ink::jni::CastToBrushCoat;
using ::ink::jni::CastToBrushPaint;
using ::ink::jni::CastToBrushTip;
using ::ink::jni::DeleteNativeBrushCoat;
using ::ink::jni::NewNativeBrushCoat;
using ::ink::jni::NewNativeBrushPaint;
using ::ink::jni::NewNativeBrushTip;

}  // namespace

extern "C" {

// Construct a native BrushCoat and return a pointer to it as a long.
JNI_METHOD(brush, BrushCoatNative, jlong, create)
(JNIEnv* env, jobject thiz, jlong tip_native_pointer,
 jlong paint_native_pointer) {
  return NewNativeBrushCoat(BrushCoat{
      .tip = CastToBrushTip(tip_native_pointer),
      .paint = CastToBrushPaint(paint_native_pointer),
  });
}

JNI_METHOD(brush, BrushCoatNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeBrushCoat(native_pointer);
}

JNI_METHOD(brush, BrushCoatNative, jlong, newCopyOfBrushTip)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return NewNativeBrushTip(CastToBrushCoat(native_pointer).tip);
}

JNI_METHOD(brush, BrushCoatNative, jlong, newCopyOfBrushPaint)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return NewNativeBrushPaint(CastToBrushCoat(native_pointer).paint);
}

}  // extern "C"
