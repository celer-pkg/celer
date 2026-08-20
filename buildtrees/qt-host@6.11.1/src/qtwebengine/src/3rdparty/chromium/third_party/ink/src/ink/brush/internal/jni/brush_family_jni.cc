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

#include <utility>
#include <variant>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_string_util.h"
#include "ink/jni/internal/jni_throw_util.h"

namespace {

using ::ink::BrushCoat;
using ::ink::BrushFamily;
using ::ink::jni::CastToBrushCoat;
using ::ink::jni::CastToBrushFamily;
using ::ink::jni::DeleteNativeBrushFamily;
using ::ink::jni::JStringView;
using ::ink::jni::NewNativeBrushCoat;
using ::ink::jni::NewNativeBrushFamily;
using ::ink::jni::ThrowExceptionFromStatus;

// 0 is reserved for internal use.
constexpr jint kSpringModel = 1;
constexpr jint kExperimentalRawPositionModel = 2;

BrushFamily::InputModel JIntToInputModel(jint input_model_value) {
  switch (input_model_value) {
    case kSpringModel:
      return BrushFamily::SpringModel();
    case kExperimentalRawPositionModel:
      return BrushFamily::ExperimentalRawPositionModel();
    default:
      ABSL_CHECK(false) << "Unknown input model value: " << input_model_value;
  }
}

jint InputModelToJInt(BrushFamily::SpringModel) { return kSpringModel; }

jint InputModelToJInt(BrushFamily::ExperimentalRawPositionModel) {
  return kExperimentalRawPositionModel;
}

jint InputModelToJInt(BrushFamily::InputModel input_model) {
  return std::visit([](const auto& model) { return InputModelToJInt(model); },
                    input_model);
}

}  // namespace

extern "C" {

// Construct a native BrushFamily and return a pointer to it as a long.
JNI_METHOD(brush, BrushFamilyNative, jlong,
           create)(JNIEnv* env, jobject object,
                   jlongArray coat_native_pointer_array,
                   jstring client_brush_family_id, jint input_model_value) {
  std::vector<BrushCoat> coats;
  const jsize num_coats = env->GetArrayLength(coat_native_pointer_array);
  coats.reserve(num_coats);
  jlong* coat_pointers =
      env->GetLongArrayElements(coat_native_pointer_array, nullptr);
  ABSL_CHECK(coat_pointers != nullptr);
  for (jsize i = 0; i < num_coats; ++i) {
    coats.push_back(CastToBrushCoat(coat_pointers[i]));
  }
  env->ReleaseLongArrayElements(
      coat_native_pointer_array, coat_pointers,
      // No need to copy back the array, which is not modified.
      JNI_ABORT);

  absl::StatusOr<BrushFamily> brush_family = BrushFamily::Create(
      coats, JStringView(env, client_brush_family_id).string_view(),
      JIntToInputModel(input_model_value));
  if (!brush_family.ok()) {
    ThrowExceptionFromStatus(env, brush_family.status());
    return 0;  // Unused return value.
  }

  return NewNativeBrushFamily(*std::move(brush_family));
}

JNI_METHOD(brush, BrushFamilyNative, void, free)
(JNIEnv* env, jobject object, jlong native_pointer) {
  DeleteNativeBrushFamily(native_pointer);
}

JNI_METHOD(brush, BrushFamilyNative, jstring, getClientBrushFamilyId)
(JNIEnv* env, jobject object, jlong native_pointer) {
  const BrushFamily& brush_family = CastToBrushFamily(native_pointer);
  return env->NewStringUTF(brush_family.GetClientBrushFamilyId().c_str());
}

JNI_METHOD(brush, BrushFamilyNative, jboolean, getInputModelInt)
(JNIEnv* env, jobject object, jlong native_pointer) {
  return InputModelToJInt(CastToBrushFamily(native_pointer).GetInputModel());
}

JNI_METHOD(brush, BrushFamilyNative, jlong, getBrushCoatCount)
(JNIEnv* env, jobject object, jlong native_pointer) {
  const BrushFamily& brush_family = CastToBrushFamily(native_pointer);
  return brush_family.GetCoats().size();
}

JNI_METHOD(brush, BrushFamilyNative, jlong, newCopyOfBrushCoat)
(JNIEnv* env, jobject object, jlong native_pointer, jint index) {
  const BrushFamily& brush_family = CastToBrushFamily(native_pointer);
  return NewNativeBrushCoat(brush_family.GetCoats()[index]);
}

}  // extern "C"
