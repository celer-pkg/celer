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

#ifndef INK_STROKES_INPUT_FUZZ_DOMAINS_H_
#define INK_STROKES_INPUT_FUZZ_DOMAINS_H_

#include <cstddef>

#include "fuzztest/fuzztest.h"
#include "ink/geometry/rect.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"

namespace ink {

// The domain of all tool types.
fuzztest::Domain<StrokeInput::ToolType> ArbitraryToolType();

// The domain of all StrokeInputs that are valid to include in a
// StrokeInputBatch.
fuzztest::Domain<StrokeInput> ValidStrokeInput();

// The domain of all StrokeInputBatches.
fuzztest::Domain<StrokeInputBatch> ArbitraryStrokeInputBatch();

// The domain of all StrokeInputBatches with at least the given number of
// inputs.
fuzztest::Domain<StrokeInputBatch> StrokeInputBatchWithMinSize(size_t min_size);

// The domain of StrokeInputBatches whose input positions are all within the
// given rectangle.
fuzztest::Domain<StrokeInputBatch> StrokeInputBatchInRect(Rect rect);

}  // namespace ink

#endif  // INK_STROKES_INPUT_FUZZ_DOMAINS_H_
