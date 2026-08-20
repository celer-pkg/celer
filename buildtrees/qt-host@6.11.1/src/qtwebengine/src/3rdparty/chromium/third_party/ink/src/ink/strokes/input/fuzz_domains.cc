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

#include "ink/strokes/input/fuzz_domains.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "fuzztest/fuzztest.h"
#include "absl/status/statusor.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/fuzz_domains.h"
#include "ink/types/physical_distance.h"

namespace ink {

namespace {

// The domain of all valid, non-absent stroke unit lengths.
fuzztest::Domain<PhysicalDistance> ValidStrokeUnitLength() {
  return FinitePositivePhysicalDistance();
}

// The domain of all valid, non-absent stroke input pressure values.
fuzztest::Domain<float> ValidPressure() { return fuzztest::InRange(0.f, 1.f); }

// The domain of all valid, non-absent stroke input tilt values.
fuzztest::Domain<Angle> ValidTilt() {
  return AngleInRange(Angle(), kQuarterTurn);
}

// The domain of all valid, non-absent stroke input orientation values.
fuzztest::Domain<Angle> ValidOrientation() { return NormalizedAngle(); }

// Given a sequence of (position, time) pairs, returns a copy of the sequence
// that is sorted by the time values.
std::vector<std::pair<Point, Duration32>> XytsSortedByTime(
    const std::vector<std::pair<Point, Duration32>>& unsorted) {
  std::vector<std::pair<Point, Duration32>> sorted = unsorted;
  std::sort(
      sorted.begin(), sorted.end(),
      [](std::pair<Point, Duration32> lhs, std::pair<Point, Duration32> rhs) {
        return lhs.second < rhs.second;
      });
  return sorted;
}

// A domain over sequences of (position, time) pairs such that (1) time values
// are non-decreasing, and (2) all (position, time) pairs are unique.
fuzztest::Domain<std::vector<std::pair<Point, Duration32>>> ValidXytSequence(
    const fuzztest::Domain<Point>& position_domain, size_t min_size) {
  return fuzztest::Map(
      &XytsSortedByTime,
      fuzztest::UniqueElementsVectorOf(
          fuzztest::PairOf(position_domain, FiniteNonNegativeDuration32()))
          .WithMinSize(min_size));
}

fuzztest::Domain<StrokeInputBatch> StrokeInputBatchWithPositionsAndMinSize(
    const fuzztest::Domain<Point>& position_domain, size_t min_size) {
  return fuzztest::FlatMap(
      [](const std::vector<std::pair<Point, Duration32>>& xyts) {
        return fuzztest::Map(
            [xyts](StrokeInput::ToolType tool_type,
                   PhysicalDistance stroke_unit_length, uint32_t noise_seed,
                   const std::optional<std::vector<float>>& pressures,
                   const std::optional<std::vector<Angle>>& tilts,
                   const std::optional<std::vector<Angle>>& orientations) {
              std::vector<StrokeInput> inputs;
              inputs.reserve(xyts.size());
              for (size_t i = 0; i < xyts.size(); ++i) {
                inputs.push_back(StrokeInput{
                    .tool_type = tool_type,
                    .position = xyts[i].first,
                    .elapsed_time = xyts[i].second,
                    .stroke_unit_length = stroke_unit_length,
                    .pressure = pressures.has_value()
                                    ? (*pressures)[i]
                                    : StrokeInput::kNoPressure,
                    .tilt =
                        tilts.has_value() ? (*tilts)[i] : StrokeInput::kNoTilt,
                    .orientation = orientations.has_value()
                                       ? (*orientations)[i]
                                       : StrokeInput::kNoOrientation,
                });
              }
              return StrokeInputBatch::Create(inputs, noise_seed).value();
            },
            ArbitraryToolType(),
            fuzztest::OneOf(fuzztest::Just(StrokeInput::kNoStrokeUnitLength),
                            ValidStrokeUnitLength()),
            fuzztest::Arbitrary<uint32_t>(),
            fuzztest::OptionalOf(
                fuzztest::VectorOf(ValidPressure()).WithSize(xyts.size())),
            fuzztest::OptionalOf(
                fuzztest::VectorOf(ValidTilt()).WithSize(xyts.size())),
            fuzztest::OptionalOf(
                fuzztest::VectorOf(ValidOrientation()).WithSize(xyts.size())));
      },
      ValidXytSequence(position_domain, min_size));
}

}  // namespace

// LINT.IfChange(tool_types)
fuzztest::Domain<StrokeInput::ToolType> ArbitraryToolType() {
  return fuzztest::ElementOf({
      StrokeInput::ToolType::kUnknown,
      StrokeInput::ToolType::kMouse,
      StrokeInput::ToolType::kTouch,
      StrokeInput::ToolType::kStylus,
  });
}
// LINT.ThenChange(stroke_input.h:tool_types)

fuzztest::Domain<StrokeInput> ValidStrokeInput() {
  return fuzztest::StructOf<StrokeInput>(
      ArbitraryToolType(), FinitePoint(), FiniteNonNegativeDuration32(),
      fuzztest::OneOf(fuzztest::Just(StrokeInput::kNoStrokeUnitLength),
                      ValidStrokeUnitLength()),
      fuzztest::OneOf(fuzztest::Just(StrokeInput::kNoPressure),
                      ValidPressure()),
      fuzztest::OneOf(fuzztest::Just(StrokeInput::kNoTilt), ValidTilt()),
      fuzztest::OneOf(fuzztest::Just(StrokeInput::kNoOrientation),
                      ValidOrientation()));
}

fuzztest::Domain<StrokeInputBatch> ArbitraryStrokeInputBatch() {
  return StrokeInputBatchWithMinSize(0);
}

fuzztest::Domain<StrokeInputBatch> StrokeInputBatchWithMinSize(
    size_t min_size) {
  return StrokeInputBatchWithPositionsAndMinSize(FinitePoint(), min_size);
}

fuzztest::Domain<StrokeInputBatch> StrokeInputBatchInRect(Rect rect) {
  return StrokeInputBatchWithPositionsAndMinSize(PointInRect(rect), 0);
}

}  // namespace ink
