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

#include "ink/types/fuzz_domains.h"

#include "fuzztest/fuzztest.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {

fuzztest::Domain<Duration32> ArbitraryDuration32() {
  return fuzztest::Map(&Duration32::Seconds, fuzztest::Arbitrary<float>());
}

fuzztest::Domain<Duration32> FiniteNonNegativeDuration32() {
  return fuzztest::Filter(
      &Duration32::IsFinite,
      fuzztest::Map(&Duration32::Seconds, fuzztest::NonNegative<float>()));
}

fuzztest::Domain<PhysicalDistance> ArbitraryPhysicalDistance() {
  return fuzztest::Map(&PhysicalDistance::Centimeters,
                       fuzztest::Arbitrary<float>());
}

fuzztest::Domain<PhysicalDistance> FinitePositivePhysicalDistance() {
  return fuzztest::Filter(&PhysicalDistance::IsFinite,
                          fuzztest::Map(&PhysicalDistance::Centimeters,
                                        fuzztest::Positive<float>()));
}

}  // namespace ink
