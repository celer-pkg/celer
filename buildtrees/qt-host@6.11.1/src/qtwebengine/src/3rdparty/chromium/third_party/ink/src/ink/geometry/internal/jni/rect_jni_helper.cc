
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

#include "ink/geometry/internal/jni/rect_jni_helper.h"

#include <jni.h>

#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/rect.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace ink::jni {

jobject CreateJImmutableBoxFromRectOrThrow(JNIEnv* env, Rect rect) {
  return env->CallStaticObjectMethod(
      ClassImmutableBox(env), MethodImmutableBoxFromTwoPoints(env),
      CreateJImmutableVecFromPointOrThrow(env, {rect.XMin(), rect.YMin()}),
      CreateJImmutableVecFromPointOrThrow(env, {rect.XMax(), rect.YMax()}));
}

void FillJMutableBoxFromRectOrThrow(JNIEnv* env, jobject mutable_box,
                                    Rect rect) {
  env->CallObjectMethod(mutable_box, MethodMutableBoxSetXBounds(env),
                        rect.XMin(), rect.XMax());
  if (env->ExceptionCheck()) return;
  env->CallObjectMethod(mutable_box, MethodMutableBoxSetYBounds(env),
                        rect.YMin(), rect.YMax());
}

}  // namespace ink::jni
