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
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"

namespace ink::geometry_internal {
namespace {

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::IsNan;
using ::testing::Lt;

constexpr float kInf = std::numeric_limits<float>::infinity();
const float kNaN = std::nanf("");

fuzztest::Domain<float> FinitePositiveFloat() {
  return fuzztest::Filter([](float b) { return std::isfinite(b); },
                          fuzztest::Positive<float>());
}

TEST(ModuloTest, FloatModulo) {
  EXPECT_FLOAT_EQ(FloatModulo(0, 1), 0);
  EXPECT_FLOAT_EQ(FloatModulo(0.75, 1), 0.75);
  EXPECT_FLOAT_EQ(FloatModulo(1, 1), 0);
  EXPECT_FLOAT_EQ(FloatModulo(1.25, 1), 0.25);
  EXPECT_FLOAT_EQ(FloatModulo(-0.25, 1), 0.75);
  EXPECT_FLOAT_EQ(FloatModulo(-1.5, 1), 0.5);

  EXPECT_FLOAT_EQ(FloatModulo(0, 10), 0);
  EXPECT_FLOAT_EQ(FloatModulo(7.5, 10), 7.5);
  EXPECT_FLOAT_EQ(FloatModulo(10, 10), 0);
  EXPECT_FLOAT_EQ(FloatModulo(12.5, 10), 2.5);
  EXPECT_FLOAT_EQ(FloatModulo(-2.5, 10), 7.5);
  EXPECT_FLOAT_EQ(FloatModulo(-15, 10), 5);
}

void FloatModuloOfNonFiniteIsNan(float b) {
  EXPECT_THAT(FloatModulo(kInf, b), IsNan());
  EXPECT_THAT(FloatModulo(-kInf, b), IsNan());
  EXPECT_THAT(FloatModulo(kNaN, b), IsNan());
}
FUZZ_TEST(ModuloTest, FloatModuloOfNonFiniteIsNan)
    .WithDomains(FinitePositiveFloat());

void FloatModuloIsStrictlyInRange(float a, float b) {
  EXPECT_THAT(FloatModulo(a, b), AllOf(Ge(0.f), Lt(b)));
}
FUZZ_TEST(ModuloTest, FloatModuloIsStrictlyInRange)
    .WithDomains(fuzztest::Finite<float>(), FinitePositiveFloat());

}  // namespace
}  // namespace ink::geometry_internal
