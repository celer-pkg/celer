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

#include "ink/strokes/input/internal/stroke_input_validation_helpers.h"

#include "absl/status/status.h"
#include "absl/strings/substitute.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/types/duration.h"

namespace ink::stroke_input_internal {

absl::Status ValidateConsecutiveInputs(const StrokeInput& first,
                                       const StrokeInput& second) {
  if (first.tool_type != second.tool_type) {
    return absl::InvalidArgumentError(absl::Substitute(
        "All inputs must report the same value of `tool_type`. Got $0 and $1",
        static_cast<int>(first.tool_type), static_cast<int>(second.tool_type)));
  }

  if (first.position == second.position &&
      first.elapsed_time == second.elapsed_time) {
    return absl::InvalidArgumentError(
        absl::Substitute("Inputs must not have duplicate `position` and "
                         "`elapsed_time`. Got: {$0, $1}",
                         first.position, first.elapsed_time.ToSeconds()));
  }

  if (first.elapsed_time > second.elapsed_time) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Inputs must have non-decreasing `elapsed_time`. Got: "
        "$0, to be followed by: $1",
        first.elapsed_time.ToSeconds(), second.elapsed_time.ToSeconds()));
  }

  if (first.stroke_unit_length != second.stroke_unit_length) {
    return absl::InvalidArgumentError(
        absl::Substitute("All inputs must report the same value of "
                         "`stroke_unit_length`. Got $0 and $1",
                         first.stroke_unit_length, second.stroke_unit_length));
  }

  if (first.HasPressure() != second.HasPressure()) {
    return absl::InvalidArgumentError(
        "Either all or none of the inputs in a batch must report `pressure`.");
  }

  if (first.HasTilt() != second.HasTilt()) {
    return absl::InvalidArgumentError(
        "Either all or none of the inputs in a batch must report `tilt`.");
  }

  if (first.HasOrientation() != second.HasOrientation()) {
    return absl::InvalidArgumentError(
        "Either all or none of the inputs in a batch must report "
        "`orientation`.");
  }

  return absl::OkStatus();
}

}  // namespace ink::stroke_input_internal
