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

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"

using ::testing::AllOf;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Ge;
using ::testing::Le;
using ::testing::Pointwise;

namespace ink::strokes_internal {
namespace {

TEST(NoiseGeneratorTest, RandomSequenceIsFixedForAGivenSeed) {
  // Given the same parameters, the output of a NoiseGenerator should never
  // change, even across Ink library releases.
  NoiseGenerator generator(12345);
  std::vector<float> actual;
  for (int i = 0; i < 30; ++i) {
    actual.push_back(generator.CurrentOutputValue());
    generator.AdvanceInputBy(0.1);
  }
  std::vector<float> expected = {
      0.323644608, 0.332764149, 0.357517153, 0.393995285, 0.438290149,
      0.486493349, 0.534696579, 0.578991473, 0.615469575, 0.640222609,
      0.649342120, 0.642169000, 0.622699142, 0.594006658, 0.559165835,
      0.521250844, 0.483335823, 0.448494971, 0.419802576, 0.400332719,
      0.393159628, 0.388005435, 0.374015540, 0.353398860, 0.328364313,
      0.301120877, 0.273877412, 0.248842880, 0.228226215, 0.214236364};
  EXPECT_THAT(actual, Pointwise(FloatEq(), expected));
}

TEST(NoiseGeneratorTest, UsesAll64SeedBits) {
  // Two different seed values should (in most cases, but in particular in this
  // specific case) result in different values generated.  We shouldn't, for
  // example, just ignore the top or bottom 32 out of 64 seed bits.
  NoiseGenerator generator1(0x10000000DeadBeefLL);
  NoiseGenerator generator2(0x20000000DeadBeefLL);
  NoiseGenerator generator3(0x10000000DeadBeadLL);
  EXPECT_NE(generator2.CurrentOutputValue(), generator1.CurrentOutputValue());
  EXPECT_NE(generator3.CurrentOutputValue(), generator1.CurrentOutputValue());
}

// Tests that all values emitted from the NoiseGenerator are in [0, 1].
void EmitsValuesBetweenZeroAndOne(uint64_t seed, float advance_by) {
  NoiseGenerator generator(seed);
  for (int i = 0; i < 1000; ++i) {
    generator.AdvanceInputBy(advance_by);
    EXPECT_THAT(generator.CurrentOutputValue(), AllOf(Ge(0), Le(1)));
  }
}
FUZZ_TEST(NoiseGeneratorTest, EmitsValuesBetweenZeroAndOne)
    .WithDomains(fuzztest::Arbitrary<uint64_t>(),
                 fuzztest::InRange<float>(0, 3));

// Tests that small advances through the sequence result in small changes to the
// output value.
void EmitsContinuousValues(uint64_t seed) {
  NoiseGenerator generator(seed);
  float prev_value = generator.CurrentOutputValue();
  for (int i = 0; i < 1000; ++i) {
    generator.AdvanceInputBy(0.01);
    float value = generator.CurrentOutputValue();
    EXPECT_THAT(value, FloatNear(prev_value, 0.05));
    prev_value = value;
  }
}
FUZZ_TEST(NoiseGeneratorTest, EmitsContinuousValues)
    .WithDomains(fuzztest::Arbitrary<uint64_t>());

// Tests that you can copy a NoiseGenerator (even mid-way through its sequence),
// and from that point on the two generators will emit identical sequences.
void CopiedGeneratorEmitsSameSequence(uint64_t seed, float advance_by) {
  NoiseGenerator generator1(seed);
  for (int i = 0; i < 100; ++i) {
    generator1.AdvanceInputBy(advance_by);
  }
  NoiseGenerator generator2 = generator1;
  for (int i = 0; i < 1000; ++i) {
    generator1.AdvanceInputBy(advance_by);
    generator2.AdvanceInputBy(advance_by);
    EXPECT_THAT(generator2.CurrentOutputValue(),
                FloatEq(generator1.CurrentOutputValue()));
  }
}
FUZZ_TEST(NoiseGeneratorTest, CopiedGeneratorEmitsSameSequence)
    .WithDomains(fuzztest::Arbitrary<uint64_t>(),
                 fuzztest::InRange<float>(0, 3));

}  // namespace
}  // namespace ink::strokes_internal
