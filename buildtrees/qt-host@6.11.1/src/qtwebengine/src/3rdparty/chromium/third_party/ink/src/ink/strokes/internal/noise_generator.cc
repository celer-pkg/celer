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

#include "ink/strokes/internal/noise_generator.h"

#include <cmath>
#include <cstdint>
#include <random>

#include "absl/log/absl_check.h"
#include "ink/geometry/internal/algorithms.h"

namespace ink::strokes_internal {

NoiseGenerator::NoiseGenerator(uint64_t seed) {
  // `std::seed_seq` ignores all but the bottom 32 bits of each entry, so we
  // need to split our 64-bit seed value into two 32-bit entries.
  std::seed_seq seq{seed & 0xffffffff, seed >> 32};
  prng_.seed(seq);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  prev_value_ = distribution(prng_);
  next_value_ = distribution(prng_);
}

float NoiseGenerator::CurrentOutputValue() const {
  ABSL_DCHECK_GE(progress_, 0.0f);
  ABSL_DCHECK_LT(progress_, 1.0f);
  // Use a smoothstep function (https://en.wikipedia.org/wiki/Smoothstep) to
  // connect the current two random values from the PRNG, so as to make the
  // noise function smooth as well as continuous.
  return ::ink::geometry_internal::Lerp(
      prev_value_, next_value_,
      progress_ * progress_ * (3.0f - 2.0f * progress_));
}

void NoiseGenerator::AdvanceInputBy(float advance_by) {
  ABSL_DCHECK_GE(advance_by, 0.0f);
  progress_ += advance_by;
  // Whenever `progress_` rolls over 1, we need to generate the next lattice
  // value.
  if (progress_ >= 1.0f) {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    // If `progress_` rolls over 1 by more than 1, because we advanced the input
    // by a large number all at once, then we need to generate multiple new
    // lattice points; in theory, we should generate a new lattice point for
    // every integer we skip past. However, (1) that generally doesn't happen in
    // typical Ink usage (we're typically advancing by small fractions each
    // call), and (2) then we risk a call like `AdvanceInputBy(1e30f)` grinding
    // the CPU to a halt. So instead, we generate at most two new lattice
    // points, even if we're rolling past more than two integers.
    if (progress_ >= 2.0f) {
      next_value_ = distribution(prng_);
    }

    prev_value_ = next_value_;
    next_value_ = distribution(prng_);

    // Set `progress_` equal to its fractional part (i.e. `progress_` mod 1).
    float unused;
    progress_ = std::modf(progress_, &unused);
  }
}

}  // namespace ink::strokes_internal
