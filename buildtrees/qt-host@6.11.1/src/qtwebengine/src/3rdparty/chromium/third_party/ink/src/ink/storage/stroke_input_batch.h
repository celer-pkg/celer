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

#ifndef INK_STORAGE_STROKE_INPUT_BATCH_H_
#define INK_STORAGE_STROKE_INPUT_BATCH_H_

#include "absl/status/statusor.h"
#include "ink/storage/proto/stroke_input_batch.pb.h"
#include "ink/strokes/input/stroke_input_batch.h"

namespace ink {

// Populates the CodedStrokeInputBatch by encoding the given StrokeInputBatch.
//
// The CodedStrokeInputBatch need not be empty before calling this; this will
// effectively clear the CodedStrokeInputBatch first, but will reuse any
// existing allocations in the proto if possible.
void EncodeStrokeInputBatch(const StrokeInputBatch& input_batch,
                            ink::proto::CodedStrokeInputBatch& input_proto);

// Decodes the CodedStrokeInputBatch into a StrokeInputBatch. Returns an error
// if the proto is invalid.
absl::StatusOr<StrokeInputBatch> DecodeStrokeInputBatch(
    const ink::proto::CodedStrokeInputBatch& input_proto);

}  // namespace ink

#endif  // INK_STORAGE_STROKE_INPUT_BATCH_H_
