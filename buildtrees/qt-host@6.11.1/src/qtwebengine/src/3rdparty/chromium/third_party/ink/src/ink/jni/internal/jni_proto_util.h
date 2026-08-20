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

#ifndef INK_JNI_INTERNAL_JNI_PROTO_UTIL_H_
#define INK_JNI_INTERNAL_JNI_PROTO_UTIL_H_

#include <jni.h>

#include "absl/status/status.h"
#include "google/protobuf/message_lite.h"

namespace ink {
namespace jni {

// Attempts to parse a serialized proto from either a direct java.nio.ByteBuffer
// or a jbyteArray, one of which must be non-null. If the proto doesn't parse,
// returns a non-OK absl::Status. `offset` is the starting point of the data
// within the `serialized_proto` array, and `size` is how far beyond `offset`
// the data continues.
absl::Status ParseProtoFromEither(JNIEnv* env,
                                  jobject serialized_proto_direct_buffer,
                                  jbyteArray serialized_proto_array,
                                  jint offset, jint size,
                                  google::protobuf::MessageLite& dest);

// Attempts to parse a serialized proto. If the proto doesn't parse, returns
// a non-OK absl::Status.
absl::Status ParseProtoFromByteArray(JNIEnv* env, jbyteArray serialized_proto,
                                     google::protobuf::MessageLite& dest);

// Attempts to parse a serialized proto. If the proto doesn't parse, returns
// a non-OK absl::Status. `offset` is the starting point of the data within the
// `serialized_proto` array, and `size` is how far beyond `offset` the data
// continues.
absl::Status ParseProtoFromByteArray(JNIEnv* env, jbyteArray serialized_proto,
                                     jint offset, jint size,
                                     google::protobuf::MessageLite& dest);

// Attempts to parse a serialized proto from a direct java.nio.ByteBuffer. If
// the proto doesn't parse, returns a non-OK absl::Status. `offset` is the
// starting point of the data within the `serialized_proto` array, and `size`
// is how far beyond `offset` the data continues.
// Note: This has a different name than `ParseProto`, as jbyteArray is a type of
// jobject so the two definitions would clash rather than serve as overloads.
absl::Status ParseProtoFromBuffer(JNIEnv* env,
                                  jobject serialized_proto_direct_buffer,
                                  jint offset, jint size,
                                  google::protobuf::MessageLite& dest);

// Serializes a proto into a newly-allocated Java byte array.
[[nodiscard]] jbyteArray SerializeProto(JNIEnv* env,
                                        const google::protobuf::MessageLite& src);

}  // namespace jni
}  // namespace ink

#endif  // INK_JNI_INTERNAL_JNI_PROTO_UTIL_H_
