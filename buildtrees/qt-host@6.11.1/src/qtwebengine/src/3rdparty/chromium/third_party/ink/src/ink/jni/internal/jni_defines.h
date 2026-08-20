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

#ifndef INK_JNI_INTERNAL_JNI_DEFINES_H_
#define INK_JNI_INTERNAL_JNI_DEFINES_H_

// For open source, we use the androidx.ink package.
#define INK_PACKAGE "androidx/ink"

#define JNI_METHOD(module, clazz, return_type, method_name) \
  JNIEXPORT return_type JNICALL                             \
      Java_androidx_ink_##module##_##clazz##_##method_name

#define JNI_METHOD_INNER(module, clazz, inner_clazz, return_type, method_name) \
  JNI_METHOD(module, clazz##_00024##inner_clazz, return_type, method_name)

#endif  // INK_JNI_INTERNAL_JNI_DEFINES_H_
