// Copyright 2024 Google LLC
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

#include "ink/geometry/angle.h"
#include "ink/jni/internal/jni_defines.h"

extern "C" {

JNI_METHOD(geometry, Angle, jfloat, nativeNormalized)
(JNIEnv* env, jobject object, jfloat angle_radians) {
  ink::Angle angle = ink::Angle::Radians(angle_radians);
  return angle.Normalized().ValueInRadians();
}

JNI_METHOD(geometry, Angle, jfloat, nativeNormalizedAboutZero)
(JNIEnv* env, jobject object, jfloat angle_radians) {
  ink::Angle angle = ink::Angle::Radians(angle_radians);
  return angle.NormalizedAboutZero().ValueInRadians();
}
}
