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

#include "ink/geometry/internal/jni/vec_jni_helper.h"

#include <jni.h>

#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace ink::jni {

jobject CreateJImmutableVecFromVecOrThrow(JNIEnv* env, Vec vec) {
  return env->NewObject(ClassImmutableVec(env), MethodImmutableVecInitXY(env),
                        vec.x, vec.y);
}

jobject CreateJImmutableVecFromPointOrThrow(JNIEnv* env, Point point) {
  return env->NewObject(ClassImmutableVec(env), MethodImmutableVecInitXY(env),
                        point.x, point.y);
}

void FillJMutableVecFromVecOrThrow(JNIEnv* env, jobject mutable_vec, Vec vec) {
  env->CallVoidMethod(mutable_vec, MethodMutableVecSetX(env), vec.x);
  if (env->ExceptionCheck()) return;
  env->CallVoidMethod(mutable_vec, MethodMutableVecSetY(env), vec.y);
}

void FillJMutableVecFromPointOrThrow(JNIEnv* env, jobject mutable_vec,
                                     Point point) {
  env->CallVoidMethod(mutable_vec, MethodMutableVecSetX(env), point.x);
  if (env->ExceptionCheck()) return;
  env->CallVoidMethod(mutable_vec, MethodMutableVecSetY(env), point.y);
}

}  // namespace ink::jni
