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

#include "ink/geometry/internal/jni/box_accumulator_jni_helper.h"

#include <jni.h>

#include "ink/geometry/envelope.h"
#include "ink/geometry/rect.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace ink::jni {

void FillJBoxAccumulatorOrThrow(JNIEnv* env, const Envelope& envelope,
                                jobject box_accumulator) {
  if (envelope.IsEmpty()) {
    env->CallObjectMethod(box_accumulator, MethodBoxAccumulatorReset(env));
  } else {
    const Rect rect = *envelope.AsRect();
    env->CallObjectMethod(box_accumulator,
                          MethodBoxAccumulatorPopulateFrom(env), rect.XMin(),
                          rect.YMin(), rect.XMax(), rect.YMax());
  }
}

}  // namespace ink::jni
