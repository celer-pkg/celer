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

#include "ink/jni/internal/jni_throw_util.h"

#include <jni.h>

#include <string>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/jni/internal/jni_jvm_interface.h"

namespace ink::jni {

namespace {

jclass ExceptionClassForStatusCode(JNIEnv* env, absl::StatusCode code) {
  // These should all be subclasses of RuntimeException.
  switch (code) {
    case absl::StatusCode::kFailedPrecondition:
      return ClassIllegalStateException(env);
    case absl::StatusCode::kInvalidArgument:
      return ClassIllegalArgumentException(env);
    case absl::StatusCode::kNotFound:
      return ClassNoSuchElementException(env);
    case absl::StatusCode::kOutOfRange:
      return ClassIndexOutOfBoundsException(env);
    case absl::StatusCode::kUnimplemented:
      return ClassUnsupportedOperationException(env);
    default:
      return ClassRuntimeException(env);
  }
}

}  // namespace

void ThrowExceptionFromStatus(JNIEnv* env, const absl::Status& status) {
  ABSL_CHECK(!status.ok()) << "Trying to throw an exception from OK status.";
  env->ThrowNew(ExceptionClassForStatusCode(env, status.code()),
                status.ToString().c_str());
}

}  // namespace ink::jni
