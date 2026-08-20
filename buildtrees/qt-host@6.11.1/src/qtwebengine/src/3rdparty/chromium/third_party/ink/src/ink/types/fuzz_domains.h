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

#ifndef INK_TYPES_FUZZ_DOMAINS_H_
#define INK_TYPES_FUZZ_DOMAINS_H_

#include <string>

#include "fuzztest/fuzztest.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {

// The domain of all durations, including negative and/or infinite durations.
fuzztest::Domain<Duration32> ArbitraryDuration32();
// The domain of all durations that are finite and non-negative.
fuzztest::Domain<Duration32> FiniteNonNegativeDuration32();

// The domain of all physical distances, including NaN and negative and/or
// infinite distances.
fuzztest::Domain<PhysicalDistance> ArbitraryPhysicalDistance();
// The domain of all physical distances that are finite and strictly positive.
fuzztest::Domain<PhysicalDistance> FinitePositivePhysicalDistance();

}  // namespace ink

#endif  // INK_TYPES_FUZZ_DOMAINS_H_
