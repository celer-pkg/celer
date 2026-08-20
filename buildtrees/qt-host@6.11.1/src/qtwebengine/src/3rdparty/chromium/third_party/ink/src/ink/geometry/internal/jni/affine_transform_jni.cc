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

#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/jni/parallelogram_jni_helper.h"
#include "ink/geometry/quad.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::AffineTransform;
using ::ink::Angle;
using ::ink::Quad;
using ::ink::jni::CreateJImmutableParallelogramOrThrow;
using ::ink::jni::FillJMutableParallelogramOrThrow;

}  // namespace

extern "C" {

JNI_METHOD(geometry, AffineTransformNative, jobject,
           createTransformedParallelogram)
(JNIEnv* env, jobject object, jfloat affine_transform_A,
 jfloat affine_transform_B, jfloat affine_transform_C,
 jfloat affine_transform_D, jfloat affine_transform_E,
 jfloat affine_transform_F, jfloat quad_center_x, jfloat quad_center_y,
 jfloat quad_width, jfloat quad_height, jfloat quad_rotation,
 jfloat quad_shear_factor) {
  return CreateJImmutableParallelogramOrThrow(
      env,
      AffineTransform(affine_transform_A, affine_transform_B,
                      affine_transform_C, affine_transform_D,
                      affine_transform_E, affine_transform_F)
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {.x = quad_center_x, .y = quad_center_y}, quad_width, quad_height,
              Angle::Radians(quad_rotation), quad_shear_factor)));
}

JNI_METHOD(geometry, AffineTransformNative, void,
           populateTransformedParallelogram)
(JNIEnv* env, jobject object, jfloat affine_transform_A,
 jfloat affine_transform_B, jfloat affine_transform_C,
 jfloat affine_transform_D, jfloat affine_transform_E,
 jfloat affine_transform_F, jfloat quad_center_x, jfloat quad_center_y,
 jfloat quad_width, jfloat quad_height, jfloat quad_rotation,
 jfloat quad_shear_factor, jobject mutable_quad) {
  FillJMutableParallelogramOrThrow(
      env,
      AffineTransform(affine_transform_A, affine_transform_B,
                      affine_transform_C, affine_transform_D,
                      affine_transform_E, affine_transform_F)
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {.x = quad_center_x, .y = quad_center_y}, quad_width, quad_height,
              Angle::Radians(quad_rotation), quad_shear_factor)),
      mutable_quad);
}

}  // extern "C
