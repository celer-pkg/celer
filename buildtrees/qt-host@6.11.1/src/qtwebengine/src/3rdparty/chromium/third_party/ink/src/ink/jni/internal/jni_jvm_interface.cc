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

#include "ink/jni/internal/jni_jvm_interface.h"

#include <jni.h>

#include "absl/log/absl_check.h"
#include "ink/jni/internal/jni_defines.h"

namespace ink::jni {

namespace {

static jclass class_illegal_state_exception = nullptr;
static jclass class_illegal_argument_exception = nullptr;
static jclass class_no_such_element_exception = nullptr;
static jclass class_index_out_of_bounds_exception = nullptr;
static jclass class_unsupported_operation_exception = nullptr;
static jclass class_runtime_exception = nullptr;

static jclass class_immutable_vec = nullptr;
static jmethodID method_immutable_vec_init_x_y = nullptr;

static jclass class_mutable_vec = nullptr;
static jmethodID method_mutable_vec_set_x = nullptr;
static jmethodID method_mutable_vec_set_y = nullptr;

static jclass class_immutable_box = nullptr;
static jmethodID method_immutable_box_from_two_points = nullptr;

static jclass class_mutable_box = nullptr;
static jmethodID method_mutable_box_set_x_bounds = nullptr;
static jmethodID method_mutable_box_set_y_bounds = nullptr;

static jclass class_box_accumulator = nullptr;
static jmethodID method_box_accumulator_reset = nullptr;
static jmethodID method_box_accumulator_populate_from = nullptr;

static jclass class_immutable_parallelogram = nullptr;
static jmethodID method_immutable_parallelogram_from_center_dim_rot_shear =
    nullptr;

static jclass class_mutable_parallelogram = nullptr;
static jmethodID method_mutable_parallelogram_set_center_dim_rot_shear =
    nullptr;

static jclass class_brush_native = nullptr;
static jmethodID method_brush_native_compose_color_long_from_components =
    nullptr;

static jclass class_input_tool_type = nullptr;
static jmethodID method_input_tool_type_from = nullptr;

static jclass class_stroke_input = nullptr;
static jmethodID method_stroke_input_update = nullptr;

jmethodID GetMethodId(JNIEnv* env, jclass cached_class, const char* method_name,
                      const char* signature) {
  jmethodID method_id = env->GetMethodID(cached_class, method_name, signature);
  ABSL_CHECK_NE(method_id, nullptr) << "Method not found: " << method_name;
  return method_id;
}

jmethodID GetStaticMethodId(JNIEnv* env, jclass cached_class,
                            const char* method_name, const char* signature) {
  jmethodID method_id =
      env->GetStaticMethodID(cached_class, method_name, signature);
  ABSL_CHECK_NE(method_id, nullptr)
      << "Static method not found: " << method_name;
  return method_id;
}

jclass FindAndCacheClass(JNIEnv* env, const char* class_name) {
  jclass cached_class = env->FindClass(class_name);
  ABSL_CHECK_NE(cached_class, nullptr) << "Class not found: " << class_name;
  return static_cast<jclass>(env->NewGlobalRef(cached_class));
}

void DeleteCachedClass(JNIEnv* env, jclass& cached_class) {
  if (cached_class != nullptr) {
    env->DeleteGlobalRef(cached_class);
    cached_class = nullptr;
  }
}

}  // namespace

void UnloadJvmInterface(JNIEnv* env) {
  // There's not a corresponding LoadJvmInterface because loading is done
  // lazily. This library is monolithic, but the library that consumes it is
  // more modular. This avoids needing to attempt to load classes that are not
  // actually defined.

  DeleteCachedClass(env, class_illegal_state_exception);
  DeleteCachedClass(env, class_illegal_argument_exception);
  DeleteCachedClass(env, class_no_such_element_exception);
  DeleteCachedClass(env, class_index_out_of_bounds_exception);
  DeleteCachedClass(env, class_unsupported_operation_exception);
  DeleteCachedClass(env, class_runtime_exception);

  DeleteCachedClass(env, class_immutable_vec);
  method_immutable_vec_init_x_y = nullptr;

  DeleteCachedClass(env, class_mutable_vec);
  method_mutable_vec_set_x = nullptr;
  method_mutable_vec_set_y = nullptr;

  DeleteCachedClass(env, class_immutable_box);
  method_immutable_box_from_two_points = nullptr;

  DeleteCachedClass(env, class_mutable_box);
  method_mutable_box_set_x_bounds = nullptr;
  method_mutable_box_set_y_bounds = nullptr;

  DeleteCachedClass(env, class_box_accumulator);
  method_box_accumulator_reset = nullptr;
  method_box_accumulator_populate_from = nullptr;

  DeleteCachedClass(env, class_immutable_parallelogram);
  method_immutable_parallelogram_from_center_dim_rot_shear = nullptr;

  DeleteCachedClass(env, class_mutable_parallelogram);
  method_mutable_parallelogram_set_center_dim_rot_shear = nullptr;

  DeleteCachedClass(env, class_brush_native);
  method_brush_native_compose_color_long_from_components = nullptr;

  DeleteCachedClass(env, class_input_tool_type);
  method_input_tool_type_from = nullptr;

  DeleteCachedClass(env, class_stroke_input);
  method_stroke_input_update = nullptr;
}

jclass ClassIllegalStateException(JNIEnv* env) {
  if (class_illegal_state_exception == nullptr) {
    class_illegal_state_exception =
        FindAndCacheClass(env, "java/lang/IllegalStateException");
  }
  return class_illegal_state_exception;
}

jclass ClassIllegalArgumentException(JNIEnv* env) {
  if (class_illegal_argument_exception == nullptr) {
    class_illegal_argument_exception =
        FindAndCacheClass(env, "java/lang/IllegalArgumentException");
  }
  return class_illegal_argument_exception;
}

jclass ClassNoSuchElementException(JNIEnv* env) {
  if (class_no_such_element_exception == nullptr) {
    class_no_such_element_exception =
        FindAndCacheClass(env, "java/util/NoSuchElementException");
  }
  return class_no_such_element_exception;
}

jclass ClassIndexOutOfBoundsException(JNIEnv* env) {
  if (class_index_out_of_bounds_exception == nullptr) {
    class_index_out_of_bounds_exception =
        FindAndCacheClass(env, "java/lang/IndexOutOfBoundsException");
  }
  return class_index_out_of_bounds_exception;
}

jclass ClassUnsupportedOperationException(JNIEnv* env) {
  if (class_unsupported_operation_exception == nullptr) {
    class_unsupported_operation_exception =
        FindAndCacheClass(env, "java/lang/UnsupportedOperationException");
  }
  return class_unsupported_operation_exception;
}

jclass ClassRuntimeException(JNIEnv* env) {
  if (class_runtime_exception == nullptr) {
    class_runtime_exception =
        FindAndCacheClass(env, "java/lang/RuntimeException");
  }
  return class_runtime_exception;
}

jclass ClassImmutableVec(JNIEnv* env) {
  if (class_immutable_vec == nullptr) {
    class_immutable_vec =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/ImmutableVec");
  }
  return class_immutable_vec;
}

jmethodID MethodImmutableVecInitXY(JNIEnv* env) {
  if (method_immutable_vec_init_x_y == nullptr) {
    method_immutable_vec_init_x_y =
        GetMethodId(env, ClassImmutableVec(env), "<init>", "(FF)V");
  }
  return method_immutable_vec_init_x_y;
}

jclass ClassMutableVec(JNIEnv* env) {
  if (class_mutable_vec == nullptr) {
    class_mutable_vec =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/MutableVec");
  }
  return class_mutable_vec;
}

jmethodID MethodMutableVecSetX(JNIEnv* env) {
  if (method_mutable_vec_set_x == nullptr) {
    method_mutable_vec_set_x =
        GetMethodId(env, ClassMutableVec(env), "setX", "(F)V");
  }
  return method_mutable_vec_set_x;
}

jmethodID MethodMutableVecSetY(JNIEnv* env) {
  if (method_mutable_vec_set_y == nullptr) {
    method_mutable_vec_set_y =
        GetMethodId(env, ClassMutableVec(env), "setY", "(F)V");
  }
  return method_mutable_vec_set_y;
}

jclass ClassImmutableBox(JNIEnv* env) {
  if (class_immutable_box == nullptr) {
    class_immutable_box =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/ImmutableBox");
  }
  return class_immutable_box;
}

jmethodID MethodImmutableBoxFromTwoPoints(JNIEnv* env) {
  if (method_immutable_box_from_two_points == nullptr) {
    method_immutable_box_from_two_points = GetStaticMethodId(
        env, ClassImmutableBox(env), "fromTwoPoints",
        "(L" INK_PACKAGE "/geometry/Vec;L" INK_PACKAGE
        "/geometry/Vec;)L" INK_PACKAGE "/geometry/ImmutableBox;");
  }
  return method_immutable_box_from_two_points;
}

jclass ClassMutableBox(JNIEnv* env) {
  if (class_mutable_box == nullptr) {
    class_mutable_box =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/MutableBox");
  }
  return class_mutable_box;
}

jmethodID MethodMutableBoxSetXBounds(JNIEnv* env) {
  if (method_mutable_box_set_x_bounds == nullptr) {
    method_mutable_box_set_x_bounds =
        GetMethodId(env, ClassMutableBox(env), "setXBounds",
                    "(FF)L" INK_PACKAGE "/geometry/MutableBox;");
  }
  return method_mutable_box_set_x_bounds;
}

jmethodID MethodMutableBoxSetYBounds(JNIEnv* env) {
  if (method_mutable_box_set_y_bounds == nullptr) {
    method_mutable_box_set_y_bounds =
        GetMethodId(env, ClassMutableBox(env), "setYBounds",
                    "(FF)L" INK_PACKAGE "/geometry/MutableBox;");
  }
  return method_mutable_box_set_y_bounds;
}

jclass ClassBoxAccumulator(JNIEnv* env) {
  if (class_box_accumulator == nullptr) {
    class_box_accumulator =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/BoxAccumulator");
  }
  return class_box_accumulator;
}

jmethodID MethodBoxAccumulatorReset(JNIEnv* env) {
  if (method_box_accumulator_reset == nullptr) {
    method_box_accumulator_reset =
        GetMethodId(env, ClassBoxAccumulator(env), "reset",
                    "()L" INK_PACKAGE "/geometry/BoxAccumulator;");
  }
  return method_box_accumulator_reset;
}

jmethodID MethodBoxAccumulatorPopulateFrom(JNIEnv* env) {
  if (method_box_accumulator_populate_from == nullptr) {
    method_box_accumulator_populate_from =
        GetMethodId(env, ClassBoxAccumulator(env), "populateFrom",
                    "(FFFF)L" INK_PACKAGE "/geometry/BoxAccumulator;");
  }
  return method_box_accumulator_populate_from;
}

jclass ClassImmutableParallelogram(JNIEnv* env) {
  if (class_immutable_parallelogram == nullptr) {
    class_immutable_parallelogram =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/ImmutableParallelogram");
  }
  return class_immutable_parallelogram;
}

jmethodID MethodImmutableParallelogramFromCenterDimensionsRotationAndShear(
    JNIEnv* env) {
  if (method_immutable_parallelogram_from_center_dim_rot_shear == nullptr) {
    method_immutable_parallelogram_from_center_dim_rot_shear =
        GetStaticMethodId(env, ClassImmutableParallelogram(env),
                          "fromCenterDimensionsRotationAndShear",
                          "(L" INK_PACKAGE
                          "/geometry/ImmutableVec;FFFF)L" INK_PACKAGE
                          "/geometry/ImmutableParallelogram;");
  }
  return method_immutable_parallelogram_from_center_dim_rot_shear;
}

jclass ClassMutableParallelogram(JNIEnv* env) {
  if (class_mutable_parallelogram == nullptr) {
    class_mutable_parallelogram =
        FindAndCacheClass(env, INK_PACKAGE "/geometry/MutableParallelogram");
  }
  return class_mutable_parallelogram;
}

jmethodID MethodMutableParallelogramSetCenterDimensionsRotationAndShear(
    JNIEnv* env) {
  if (method_mutable_parallelogram_set_center_dim_rot_shear == nullptr) {
    method_mutable_parallelogram_set_center_dim_rot_shear =
        GetMethodId(env, ClassMutableParallelogram(env),
                    "setCenterDimensionsRotationAndShear", "(FFFFFF)V");
  }
  return method_mutable_parallelogram_set_center_dim_rot_shear;
}

jclass ClassBrushNative(JNIEnv* env) {
  if (class_brush_native == nullptr) {
    class_brush_native =
        FindAndCacheClass(env, INK_PACKAGE "/brush/BrushNative");
  }
  return class_brush_native;
}

jmethodID MethodBrushNativeComposeColorLongFromComponents(JNIEnv* env) {
  if (method_brush_native_compose_color_long_from_components == nullptr) {
    method_brush_native_compose_color_long_from_components =
        GetStaticMethodId(env, ClassBrushNative(env),
                          "composeColorLongFromComponents", "(IFFFF)J");
  }
  return method_brush_native_compose_color_long_from_components;
}

jclass ClassInputToolType(JNIEnv* env) {
  if (class_input_tool_type == nullptr) {
    class_input_tool_type =
        FindAndCacheClass(env, INK_PACKAGE "/brush/InputToolType");
  }
  return class_input_tool_type;
}

jmethodID MethodInputToolTypeFromInt(JNIEnv* env) {
  if (method_input_tool_type_from == nullptr) {
    method_input_tool_type_from =
        GetStaticMethodId(env, ClassInputToolType(env), "fromInt",
                          "(I)L" INK_PACKAGE "/brush/InputToolType;");
  }
  return method_input_tool_type_from;
}

jclass ClassStrokeInput(JNIEnv* env) {
  if (class_stroke_input == nullptr) {
    class_stroke_input =
        FindAndCacheClass(env, INK_PACKAGE "/strokes/StrokeInput");
  }
  return class_stroke_input;
}

jmethodID MethodStrokeInputUpdate(JNIEnv* env) {
  if (method_stroke_input_update == nullptr) {
    method_stroke_input_update =
        GetMethodId(env, ClassStrokeInput(env), "update",
                    "(FFJL" INK_PACKAGE "/brush/InputToolType;FFFF)V");
  }
  return method_stroke_input_update;
}

}  // namespace ink::jni
