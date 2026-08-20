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

#ifndef INK_STROKES_INTERNAL_NOISE_GENERATOR_H_
#define INK_STROKES_INTERNAL_NOISE_GENERATOR_H_

#include <cstdint>
#include <random>

namespace ink::strokes_internal {

// A random gradient noise function that maps input values in [0, inf) to output
// values in [0, 1]. The shape of the output function is continuous (that is,
// small increments in the input value will result in small increments in the
// output value, never big jumps), and smooth (that is, the first derivative is
// also continuous, so there are no "sharp corners" in the shape of the
// function).
class NoiseGenerator {
 public:
  // Two `NoiseGenerator` instances created with the same seed will generate the
  // same random function, even across different hardware or Ink library
  // releases.
  //
  // If, in the future, we want to use a newer noise-generation implementation
  // without breaking existing strokes, we can add a new enum parameter here to
  // select the underlying implementation to use.
  explicit NoiseGenerator(uint64_t seed);

  // `NoiseGenerator` is cheaply copiable, and the copied generator will output
  // the exact same sequence as the original.
  NoiseGenerator(const NoiseGenerator&) = default;
  NoiseGenerator& operator=(const NoiseGenerator&) = default;
  ~NoiseGenerator() = default;

  // Returns the current output value of the generator.
  float CurrentOutputValue() const;

  // Advances the input value (which starts at zero when the `NoiseGenerator` is
  // constructed) forward by the given amount (which must be non-negative), thus
  // changing the value returned by `CurrentOutputValue()`.  Calling this with
  // zero is a no-op.
  void AdvanceInputBy(float advance_by);

 private:
  // The underlying PRNG used to generate lattice values for our 1D gradient
  // noise function. A few notes on the choice of PRNG implementation here:
  //
  //   * We can't use absl's PRNG implementations, because they explicitly and
  //     intentionally are not stable across processes, let alone library
  //     versions (see https://abseil.io/docs/cpp/guides/random).
  //   * `NoiseGenerator` needs to be small and cheap to copy, due to how it's
  //     used in `BrushTipModeler` (where all `NoiseGenerators` for a stroke
  //     need to be frequently saved/restored as volatile portions of the stroke
  //     are re-extruded). This makes "good" PRNGs with large memory footprints
  //     (like Mersenne Twister) unattractive here.
  //   * For typical usage in Ink brushes, we won't be generating very many
  //     random values (hundreds rather than billions), and the quality of the
  //     randomness isn't especially critical, so even a PRNG with a relatively
  //     short period is acceptable. All that matters is that it's seed-stable,
  //     small, and "good enough".
  //
  // All considered, we've chosen to use an LCG, which uses a tiny amount of
  // memory (see https://en.wikipedia.org/wiki/Linear_congruential_generator).
  // C++'s `minstd_rand` uses a specific set of LCG parameters (a=48271, c=0,
  // m=2^31-1) that is reasonably good and won't change in the future.
  std::minstd_rand prng_;
  // The current input value, mod 1.
  float progress_ = 0.f;
  // The last two [0, 1) floats to be emitted by the PRNG. The output value is a
  // smoothstep interpolation between these two values, using `progress_` as the
  // interpolation variable. Whenever `progress_` wraps around 1, we move
  // `next_value_` into `prev_value_` and generate a new `next_value_` from the
  // PRNG.
  float prev_value_;
  float next_value_;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_NOISE_GENERATOR_H_
