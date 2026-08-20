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

#ifndef INK_STROKES_INTERNAL_STROKE_INPUT_MODELER_H_
#define INK_STROKES_INTERNAL_STROKE_INPUT_MODELER_H_

#include <cstddef>
#include <optional>
#include <vector>

#include "absl/types/span.h"
#include "ink/brush/brush_family.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"
#include "ink_stroke_modeler/stroke_modeler.h"
#include "ink_stroke_modeler/types.h"

namespace ink::strokes_internal {

// The result of modeling `StrokeInput` by the `StrokeInputModeler`.
//
// The `velocity` and `acceleration` are the modeled instantaneous
// velocity/acceleration of the input, and `traveled_distance` is the modeled
// total distance traveled since the start of the stroke. The other members are
// the modeled analogues of member variables of `StrokeInput` with the same
// names.
struct ModeledStrokeInput {
  Point position;
  Vec velocity;
  Vec acceleration;
  float traveled_distance = 0;
  Duration32 elapsed_time;
  float pressure = StrokeInput::kNoPressure;
  Angle tilt = StrokeInput::kNoTilt;
  Angle orientation = StrokeInput::kNoOrientation;
};

// Measures the input distance/time from one point on the stroke input to
// another.
struct InputMetrics {
  // The distance traveled by the input pointer, in stroke space units.
  float traveled_distance = 0;
  // The input time elapsed.
  Duration32 elapsed_time;
};

// A wrapper over the open-sourced Ink `StrokeModeler` that accepts Strokes API
// `StrokeInput` and custom raw prediction instead of using the built-in
// predictors.
//
// This type requires that the incremental `StrokeInputBatch` objects are
// already checked to form a valid input sequence before being passed to
// `ExtendStroke()`.
//
// Each call to `ExtendStroke()` generates some number of `ModeledInput` that
// are categorized as either "stable" or "unstable". Stable modeled inputs are
// not modified until the start of a new stroke, and unstable modeled inputs are
// usually replaced by the next call to `ExtendStroke()`. All stable modeled
// inputs come as a result of modeling real, as opposed to predicted, input.
// However, not every real modeled input will be stable.
class StrokeInputModeler {
 public:
  // The current state of modeling all `StrokeInput` for a stroke by the
  // `StrokeInputModeler`.
  struct State {
    // The current tool type of the stroke.
    //
    // When the current stroke has no inputs, the return value is `kUnknown`.
    StrokeInput::ToolType tool_type = StrokeInput::ToolType::kUnknown;
    // The physical distance that the pointer must travel in order to produce an
    // input motion of one stroke unit. For stylus/touch, this is the real-world
    // distance that the stylus/fingertip must move in physical space; for
    // mouse, this is the visual distance that the mouse pointer must travel
    // along the surface of the display.
    //
    // A value of `std::nullopt` indicates that the relationship between stroke
    // space and physical space is unknown (possibly because the current stroke
    // has no inputs yet) or ill-defined.
    std::optional<PhysicalDistance> stroke_unit_length;
    // The modeled time elapsed from the start of the stroke until either "now"
    // or the last modeled input, whichever comes later.
    //
    // This value may be different from the `current_elapsed_time` value passed
    // to `ExtendStroke()` due to modeling and prediction. If `inputs` is not
    // empty, this value will always be greater than or equal to
    // `inputs.back().elapsed_time`.
    Duration32 complete_elapsed_time = Duration32::Zero();
    // The modeled distance traveled from the start of the stroke to the last
    // modeled input.
    float complete_traveled_distance = 0;
    // The total elapsed time for real inputs only.
    Duration32 total_real_elapsed_time = Duration32::Zero();
    // The total traveled distance for real inputs only.
    float total_real_distance = 0;
    // The number of "stable" elements at the start of `inputs`.
    //
    // These will not be removed or modified by subsequent calls to
    // `ExtendStroke()`, which means the values of this member variable over the
    // course of a single stroke will be non-decreasing.
    size_t stable_input_count = 0;
    // The number of elements at the start of `inputs` above that were
    // a result of modeling only the `real_inputs`.
    //
    // This number will always be greater than the value of `stable_input_count`
    // when the stroke is not empty. As with the stable count, the values
    // of this member variable will be non-decreasing over the course of a
    // single stroke.
    size_t real_input_count = 0;
  };

  StrokeInputModeler() = default;
  StrokeInputModeler(const StrokeInputModeler&) = delete;
  StrokeInputModeler& operator=(const StrokeInputModeler&) = delete;
  StrokeInputModeler(StrokeInputModeler&&) = default;
  StrokeInputModeler& operator=(StrokeInputModeler&&) = default;
  ~StrokeInputModeler() = default;

  // Clears any ongoing stroke and sets up the modeler to accept new stroke
  // input.
  //
  // The value of `brush_epsilon` is CHECK-validated to be greater than zero and
  // is used as the minimum distance between resulting `ModeledStrokeInput`.
  // This function must be called before starting to call `ExtendStroke()`.
  void StartStroke(const BrushFamily::InputModel& input_model,
                   float brush_epsilon);

  // Models new real and predicted inputs and adds them to the current stroke.
  //
  // The `current_elapsed_time` should be the duration from the start of the
  // stroke until "now". The `elapsed_time` of inputs may be "in the future"
  // relative to this duration.
  //
  // This always clears any previously generated unstable modeled inputs. Either
  // or both of `real_inputs` and `predicted_inputs` may be empty. CHECK-fails
  // if `StartStroke()` has not been called at least once.
  void ExtendStroke(const StrokeInputBatch& real_inputs,
                    const StrokeInputBatch& predicted_inputs,
                    Duration32 current_elapsed_time);

  const State& GetState() const { return state_; }
  absl::Span<const ModeledStrokeInput> GetModeledInputs() const {
    return modeled_inputs_;
  }

 private:
  // Models a single `input`.
  //
  // The value of `last_input_in_update` indicates whether this is the last
  // input being modeled from a single call to `ExtendStroke()`. This last input
  // must always be "unstable".
  void ModelInput(const StrokeInput& input, bool last_input_in_update);

  // Updates `state_` elapsed time and distance properties.
  void UpdateStateTimeAndDistance(Duration32 current_elapsed_time);

  BrushFamily::InputModel input_model_ = BrushFamily::DefaultInputModel();
  // We use `brush_epsilon` to set up the parameters for `stroke_modeler_`, and
  // to determine the minimum distance that a new `stroke_model::Result` must
  // travel from the previous accepted one in order to be turned into a
  // `ModeledStrokeInput`.
  float brush_epsilon_ = 0;
  stroke_model::StrokeModeler stroke_modeler_;
  std::vector<stroke_model::Result> result_buffer_;
  std::optional<StrokeInput> last_real_stroke_input_;
  // All modeled inputs for a stroke.
  std::vector<ModeledStrokeInput> modeled_inputs_;
  bool stroke_modeler_has_input_ = false;
  State state_;
};

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_STROKE_INPUT_MODELER_H_
