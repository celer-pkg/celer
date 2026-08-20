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

#include <random>
#include <vector>

#include "absl/types/span.h"
#include "benchmark/benchmark.h"
#include "ink/geometry/internal/static_rtree.h"
#include "ink/geometry/rect.h"

namespace ink::geometry_internal {
namespace {

// Bounds computation functor for a `StaticRTree` of `Rect`s
auto rect_bounds = [](const Rect& r) { return r; };

// Returns a vector of `n_elements` pseudo-randomly generated `Rect`s, with
// centers uniformly distributed in the rect from {-100, -100} to {100, 100},
// and widths and heights uniformly distributed in the interval [.1, 10].
std::vector<Rect> MakeVectorOfRandomRects(int n_elements) {
  std::mt19937_64 rng_(0);
  auto rand_between = [&rng_](float a, float b) {
    return std::uniform_real_distribution<float>(a, b)(rng_);
  };
  std::vector<Rect> rects(n_elements);
  for (int i = 0; i < rects.size(); ++i) {
    rects[i] = Rect::FromCenterAndDimensions(
        {rand_between(-100, 100), rand_between(-100, 100)},
        rand_between(.1, 10), rand_between(.1, 10));
  }
  return rects;
}

void BM_ConstructFromRandomRects(benchmark::State& state) {
  std::vector<Rect> rects = MakeVectorOfRandomRects(state.range(0));
  for (auto s : state) {
    StaticRTree<Rect> rtree(rects, rect_bounds);
  }
}
BENCHMARK(BM_ConstructFromRandomRects)->Range(8, 16384);

void BM_VisitFirstIntersectingRect(benchmark::State& state) {
  std::vector<Rect> rects = MakeVectorOfRandomRects(state.range(0));
  StaticRTree<Rect> rtree(rects, rect_bounds);
  for (auto s : state) {
    rtree.VisitIntersectedElements(Rect::FromTwoPoints({-25, -25}, {75, 75}),
                                   [](const Rect&) { return true; });
  }
}
BENCHMARK(BM_VisitFirstIntersectingRect)->Range(8, 16384);

void BM_VisitAllIntersectingRects(benchmark::State& state) {
  std::vector<Rect> rects = MakeVectorOfRandomRects(state.range(0));
  StaticRTree<Rect> rtree(rects, rect_bounds);
  std::vector<Rect> output;
  output.reserve(rects.size());
  for (auto s : state) {
    state.PauseTiming();
    output.clear();
    state.ResumeTiming();
    rtree.VisitIntersectedElements(Rect::FromTwoPoints({-25, -25}, {75, 75}),
                                   [&output](const Rect& r) {
                                     output.push_back(r);
                                     return true;
                                   });
  }
}
BENCHMARK(BM_VisitAllIntersectingRects)->Range(8, 16384);

}  // namespace
}  // namespace ink::geometry_internal
