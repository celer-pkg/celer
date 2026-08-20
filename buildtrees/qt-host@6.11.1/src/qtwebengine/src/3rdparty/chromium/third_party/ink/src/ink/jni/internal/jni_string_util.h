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

#ifndef INK_JNI_INTERNAL_JNI_STRING_UTIL_H_
#define INK_JNI_INTERNAL_JNI_STRING_UTIL_H_

#include <jni.h>

#include <string>

#include "absl/strings/string_view.h"

namespace ink {
namespace jni {

// RAII wrapper that provides access to a `jstring` as an `absl::string_view`.
class JStringView {
 public:
  JStringView(JNIEnv* env, jstring j_string);
  ~JStringView();

  absl::string_view string_view() const { return string_view_; }

 private:
  JNIEnv* env_;
  jstring j_string_ = nullptr;
  absl::string_view string_view_;
};

// Gets the contents of a JVM `String` object as a UTF8-encoded `std::string`.
// Returns an empty `std::string` if the JVM `String` object is null.
inline std::string JStringToStdString(JNIEnv* env, jstring j_string) {
  return std::string(JStringView(env, j_string).string_view());
}

// Converts a `std::string` to a JVM `jbyteArray`.
jbyteArray StdStringToJByteArray(JNIEnv* env, const std::string& str);

// Converts a JVM `jbyteArray` to a `std::string`.
std::string JByteArrayToStdString(JNIEnv* env, jbyteArray byteArray);

}  // namespace jni
}  // namespace ink

#endif  // INK_JNI_INTERNAL_JNI_STRING_UTIL_H_
