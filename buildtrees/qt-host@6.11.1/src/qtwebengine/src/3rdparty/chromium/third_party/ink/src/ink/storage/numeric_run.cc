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

#include "ink/storage/numeric_run.h"

#include <cmath>
#include <cstdint>
#include <limits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/storage/proto/coded_numeric_run.pb.h"
#include "ink/types/iterator_range.h"

namespace ink {
namespace {

bool IsInt32(float number) {
  return number >= static_cast<float>(std::numeric_limits<int32_t>::min()) &&
         number <= static_cast<float>(std::numeric_limits<int32_t>::max()) &&
         number == static_cast<float>(static_cast<int32_t>(number));
}

}  // namespace

absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
DecodeFloatNumericRun(const proto::CodedNumericRun& run) {
  if (!std::isfinite(run.offset())) {
    return absl::InvalidArgumentError(
        "invalid float numeric run: non-finite offset");
  }
  if (!std::isfinite(run.scale())) {
    return absl::InvalidArgumentError(
        "invalid float numeric run: non-finite scale");
  }
  return iterator_range<CodedNumericRunIterator<float>>{
      CodedNumericRunIterator<float>(&run, 0),
      CodedNumericRunIterator<float>(&run, run.deltas_size())};
}

absl::StatusOr<iterator_range<CodedNumericRunIterator<int32_t>>>
DecodeIntNumericRun(const proto::CodedNumericRun& run) {
  if (!IsInt32(run.offset())) {
    return absl::InvalidArgumentError(
        "invalid int numeric run: non-integer offset");
  }
  if (!IsInt32(run.scale())) {
    return absl::InvalidArgumentError(
        "invalid int numeric run: non-integer scale");
  }
  return iterator_range<CodedNumericRunIterator<int32_t>>{
      CodedNumericRunIterator<int32_t>(&run, 0),
      CodedNumericRunIterator<int32_t>(&run, run.deltas_size())};
}

}  // namespace ink
