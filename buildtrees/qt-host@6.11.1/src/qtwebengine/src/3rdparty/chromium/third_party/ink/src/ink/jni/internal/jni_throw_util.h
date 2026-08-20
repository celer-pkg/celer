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

#ifndef INK_JNI_INTERNAL_JNI_THROW_UTIL_H_
#define INK_JNI_INTERNAL_JNI_THROW_UTIL_H_

#include <jni.h>

#include "absl/status/status.h"

namespace ink::jni {

// Throws a Java exception, with the exception class and message determined from
// the given (non-OK) absl::Status.
//
// Note that C++ execution will continue on after this function returns; the
// caller should immediately return control back to the JVM after calling this
// (e.g. by returning a placeholder value from the JNI method) so that the Java
// exception can be processed.
void ThrowExceptionFromStatus(JNIEnv* env, const absl::Status& status);

}  // namespace ink::jni

#endif  // INK_JNI_INTERNAL_JNI_THROW_UTIL_H_
