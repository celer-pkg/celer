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

#include "benchmark/benchmark.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

void BM_AsUnitVec(benchmark::State& state) {
  Vec vec = {12.3, -45.6};
  for (auto s : state) {
    benchmark::DoNotOptimize(vec);
    Vec unit = vec.AsUnitVec();
    benchmark::DoNotOptimize(unit);
  }
}
BENCHMARK(BM_AsUnitVec);

}  // namespace
}  // namespace ink
