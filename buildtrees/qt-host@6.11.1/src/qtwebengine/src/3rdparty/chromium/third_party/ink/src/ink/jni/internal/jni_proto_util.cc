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

#include "ink/jni/internal/jni_proto_util.h"

#include <jni.h>

#include <cstddef>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "google/protobuf/message_lite.h"

namespace ink {
namespace jni {

absl::Status ParseProtoFromByteArray(JNIEnv* env, jbyteArray serialized_proto,
                                     google::protobuf::MessageLite& dest) {
  return ParseProtoFromByteArray(env, serialized_proto, 0,
                                 env->GetArrayLength(serialized_proto), dest);
}

absl::Status ParseProtoFromByteArray(JNIEnv* env, jbyteArray serialized_proto,
                                     jint offset, jint size,
                                     google::protobuf::MessageLite& dest) {
  // TODO(b/290213178): See how often this copies using the isCopy pointer
  //   argument, and consider using critical arrays to minimize copying.
  ABSL_CHECK(serialized_proto != nullptr);
  jbyte* bytes = env->GetByteArrayElements(serialized_proto, nullptr);
  ABSL_CHECK(bytes);
  bool success = dest.ParseFromArray(bytes + offset, size);
  env->ReleaseByteArrayElements(serialized_proto, bytes, 0);
  if (!success) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Failed to parse ", dest.GetTypeName(), " proto from byte[]."));
  }
  return absl::OkStatus();
}

absl::Status ParseProtoFromBuffer(JNIEnv* env,
                                  jobject serialized_proto_direct_buffer,
                                  jint offset, jint size,
                                  google::protobuf::MessageLite& dest) {
  ABSL_CHECK(serialized_proto_direct_buffer != nullptr);
  void* addr = env->GetDirectBufferAddress(serialized_proto_direct_buffer);
  ABSL_CHECK(addr != nullptr);
  // Cannot do arithmetic on void* due to unknown size of elements, so
  // explicitly cast to clarify that the offset (and size) are in bytes.
  jbyte* bytes = static_cast<jbyte*>(addr);
  bool success = dest.ParseFromArray(bytes + offset, size);
  if (!success) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Failed to parse ", dest.GetTypeName(), " proto from direct buffer."));
  }
  return absl::OkStatus();
}

absl::Status ParseProtoFromEither(JNIEnv* env,
                                  jobject serialized_proto_direct_buffer,
                                  jbyteArray serialized_proto_array,
                                  jint offset, jint size,
                                  google::protobuf::MessageLite& dest) {
  if (serialized_proto_direct_buffer != nullptr) {
    return ParseProtoFromBuffer(env, serialized_proto_direct_buffer, offset,
                                size, dest);
  } else {
    return ParseProtoFromByteArray(env, serialized_proto_array, offset, size,
                                   dest);
  }
}

jbyteArray SerializeProto(JNIEnv* env, const google::protobuf::MessageLite& src) {
  size_t size = src.ByteSizeLong();
  jbyteArray serialized_proto = env->NewByteArray(size);
  ABSL_CHECK(serialized_proto);
  jbyte* bytes = env->GetByteArrayElements(serialized_proto, nullptr);
  ABSL_CHECK(bytes);
  bool success = src.SerializeToArray(bytes, size);
  env->ReleaseByteArrayElements(serialized_proto, bytes, 0);
  ABSL_CHECK(success);
  return serialized_proto;
}

}  // namespace jni
}  // namespace ink
