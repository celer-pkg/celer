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

#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/vec.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::Vec;
using ::ink::jni::CreateJImmutableVecFromVecOrThrow;
using ::ink::jni::FillJMutableVecFromVecOrThrow;

}  // namespace

extern "C" {

JNI_METHOD(geometry, VecNative, jobject, unitVec)
(JNIEnv* env, jobject object, jfloat vec_X, jfloat vec_Y) {
  return CreateJImmutableVecFromVecOrThrow(env, Vec{vec_X, vec_Y}.AsUnitVec());
}

JNI_METHOD(geometry, VecNative, void, populateUnitVec)
(JNIEnv* env, jobject object, jfloat vec_X, jfloat vec_Y,
 jobject output_mutable_vec) {
  FillJMutableVecFromVecOrThrow(env, output_mutable_vec,
                                Vec{vec_X, vec_Y}.AsUnitVec());
}

JNI_METHOD(geometry, VecNative, jfloat, absoluteAngleBetween)
(JNIEnv* env, jobject object, jfloat first_vec_X, jfloat first_vec_Y,
 jfloat second_vec_X, jfloat second_vec_Y) {
  return Vec::AbsoluteAngleBetween(Vec{first_vec_X, first_vec_Y},
                                   Vec{second_vec_X, second_vec_Y})
      .ValueInRadians();
}

JNI_METHOD(geometry, VecNative, jfloat, signedAngleBetween)
(JNIEnv* env, jobject object, jfloat first_vec_X, jfloat first_vec_Y,
 jfloat second_vec_X, jfloat second_vec_Y) {
  return Vec::SignedAngleBetween(Vec{first_vec_X, first_vec_Y},
                                 Vec{second_vec_X, second_vec_Y})
      .ValueInRadians();
}

}  // extern "C
