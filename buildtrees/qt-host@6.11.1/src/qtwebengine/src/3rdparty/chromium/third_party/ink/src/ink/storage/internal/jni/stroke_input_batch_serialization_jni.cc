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

#include <jni.h>

#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_proto_util.h"
#include "ink/jni/internal/jni_throw_util.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/storage/stroke_input_batch.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/jni/stroke_input_jni_helper.h"

namespace {

using ::ink::DecodeStrokeInputBatch;
using ::ink::EncodeStrokeInputBatch;
using ::ink::StrokeInputBatch;
using ::ink::jni::CastToStrokeInputBatch;
using ::ink::jni::NewNativeStrokeInputBatch;
using ::ink::jni::ParseProtoFromEither;
using ::ink::jni::SerializeProto;
using ::ink::jni::ThrowExceptionFromStatus;
using ::ink::proto::CodedStrokeInputBatch;

}  // namespace

extern "C" {

// Constructs a `StrokeInputBatch` from a serialized `CodedStrokeInputBatch`,
// which can be passed in as either a direct `ByteBuffer` or as an array of
// bytes. This returns the address of a heap-allocated `StrokeInputBatch`, which
// must later be freed by the caller.
JNI_METHOD(storage, StrokeInputBatchSerializationNative, jlong, newFromProto)
(JNIEnv* env, jclass klass, jobject direct_byte_buffer, jbyteArray byte_array,
 jint offset, jint length) {
  CodedStrokeInputBatch coded_input;
  if (absl::Status status = ParseProtoFromEither(
          env, direct_byte_buffer, byte_array, offset, length, coded_input);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  absl::StatusOr<StrokeInputBatch> input = DecodeStrokeInputBatch(coded_input);
  if (!input.ok()) {
    ThrowExceptionFromStatus(env, input.status());
    return 0;
  }
  return NewNativeStrokeInputBatch(*std::move(input));
}

JNI_METHOD(storage, StrokeInputBatchSerializationNative, jbyteArray, serialize)
(JNIEnv* env, jclass klass, jlong stroke_input_batch_native_pointer) {
  CodedStrokeInputBatch coded_input;
  EncodeStrokeInputBatch(
      CastToStrokeInputBatch(stroke_input_batch_native_pointer), coded_input);
  return SerializeProto(env, coded_input);
}

}  // extern "C"
