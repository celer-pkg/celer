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

#include "ink/geometry/internal/jni/parallelogram_jni_helper.h"

#include <jni.h>

#include "ink/geometry/internal/jni/vec_jni_helper.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace ink::jni {

jobject CreateJImmutableParallelogramOrThrow(JNIEnv* env, const Quad& quad) {
  jobject center = CreateJImmutableVecFromPointOrThrow(
      env, {quad.Center().x, quad.Center().y});
  if (env->ExceptionCheck()) return nullptr;
  return env->CallStaticObjectMethod(
      ClassImmutableParallelogram(env),
      MethodImmutableParallelogramFromCenterDimensionsRotationAndShear(env),
      center, quad.Width(), quad.Height(), quad.Rotation().ValueInRadians(),
      quad.ShearFactor());
}

void FillJMutableParallelogramOrThrow(JNIEnv* env, const Quad& quad,
                                      jobject mutable_parallelogram) {
  env->CallVoidMethod(
      mutable_parallelogram,
      MethodMutableParallelogramSetCenterDimensionsRotationAndShear(env),
      quad.Center().x, quad.Center().y, quad.Width(), quad.Height(),
      quad.Rotation().ValueInRadians(), quad.ShearFactor());
}

}  // namespace ink::jni
