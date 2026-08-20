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

#include "ink/geometry/internal/modulo.h"

#include <cmath>

#include "absl/log/absl_check.h"

namespace ink::geometry_internal {

float FloatModulo(float a, float b) {
  ABSL_DCHECK(std::isfinite(b) && b > 0.f);
  float result = fmodf(a, b);
  // `fmodf` always matches the sign of the first argument, so `fmodf(a, b)`
  // returns a value in the range (-b, b).  We want [0, b), so wrap negative
  // values back to positive.
  if (result < 0.f) {
    result += b;
    // If `fmodf` returned a sufficiently small negative number, then adding `b`
    // will give us a result exactly equal to `b`, due to float rounding.
    // However, `FloatModulo` promises to return a value strictly less than `b`,
    // so we should return zero in this case.
    if (result == b) {
      result = 0.f;
    }
  }
  return result;
}

}  // namespace ink::geometry_internal
