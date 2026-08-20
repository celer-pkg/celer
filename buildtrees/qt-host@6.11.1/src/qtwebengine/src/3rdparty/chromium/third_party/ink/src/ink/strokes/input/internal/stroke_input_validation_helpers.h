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

#ifndef INK_STROKES_INPUT_INTERNAL_STROKE_INPUT_VALIDATION_HELPERS_H_
#define INK_STROKES_INPUT_INTERNAL_STROKE_INPUT_VALIDATION_HELPERS_H_

#include "absl/status/status.h"
#include "ink/strokes/input/stroke_input.h"

namespace ink::stroke_input_internal {

// Validates that a pair of inputs can be consecutive in a `StrokeInputBatch`.
//
// This includes checking that `first` and `second`:
//   * Have the same `tool_type`
//   * Do not have duplicate `x`, `y`, `elapsed_time`
//   * Have non-decreasing `elapsed_time`
//   * Have the same format of reported optional `pressure`, `tilt`, and
//     `orientation`
absl::Status ValidateConsecutiveInputs(const StrokeInput& first,
                                       const StrokeInput& second);

}  // namespace ink::stroke_input_internal

#endif  // INK_STROKES_INPUT_INTERNAL_STROKE_INPUT_VALIDATION_HELPERS_H_
