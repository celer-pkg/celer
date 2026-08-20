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

#include "ink/strokes/internal/stroke_input_modeler.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "absl/cleanup/cleanup.h"
#include "absl/log/absl_check.h"
#include "ink/brush/brush_family.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/stroke_modeler.h"
#include "ink_stroke_modeler/types.h"

namespace ink::strokes_internal {

constexpr float kDefaultLoopMitigationSpeedLowerBoundInCmPerSec = 0.0f;
constexpr float kDefaultLoopMitigationSpeedUpperBoundInCmPerSec = 25.0f;
constexpr float kDefaultLoopMitigationInterpolationStrengthAtSpeedLowerBound =
    1.0f;
constexpr float kDefaultLoopMitigationInterpolationStrengthAtSpeedUpperBound =
    0.5f;
const stroke_model::Duration kDefaultLoopMitigationMinSpeedSamplingWindow =
    stroke_model::Duration(0.04);
constexpr int kDefaultLoopMitigationMinDiscreteSpeedSamples = 10;

void StrokeInputModeler::StartStroke(const BrushFamily::InputModel& input_model,
                                     float brush_epsilon) {
  // The `stroke_modeler_` cannot be reset until we get the first input in order
  // to know the `StrokeInput::ToolType`.

  ABSL_CHECK_GT(brush_epsilon, 0);
  input_model_ = input_model;
  brush_epsilon_ = brush_epsilon;
  last_real_stroke_input_.reset();
  state_.tool_type = StrokeInput::ToolType::kUnknown;
  state_.stroke_unit_length = std::nullopt;
  state_.complete_elapsed_time = Duration32::Zero();
  state_.complete_traveled_distance = 0;
  state_.stable_input_count = 0;
  state_.real_input_count = 0;
  state_.total_real_distance = 0;
  state_.total_real_elapsed_time = Duration32::Zero();
  modeled_inputs_.clear();
}

namespace {

using stroke_model::PositionModelerParams;
using stroke_model::StylusStateModelerParams;

// LINT.IfChange(input_model_types)

PositionModelerParams::LoopContractionMitigationParameters
LoopContractionParams(const BrushFamily::SpringModel& spring_model,
                      std::optional<PhysicalDistance> stroke_unit_length) {
  // Without the stroke unit length, we cannot determine the speed of
  // the stroke inputs, so we cannot enable loop mitigation.
  if (!stroke_unit_length.has_value()) {
    return {.is_enabled = false};
  }
  return {
      .is_enabled = true,
      .speed_lower_bound = kDefaultLoopMitigationSpeedLowerBoundInCmPerSec /
                           stroke_unit_length->ToCentimeters(),
      .speed_upper_bound = kDefaultLoopMitigationSpeedUpperBoundInCmPerSec /
                           stroke_unit_length->ToCentimeters(),
      .interpolation_strength_at_speed_lower_bound =
          kDefaultLoopMitigationInterpolationStrengthAtSpeedLowerBound,
      .interpolation_strength_at_speed_upper_bound =
          kDefaultLoopMitigationInterpolationStrengthAtSpeedUpperBound,
      .min_speed_sampling_window = kDefaultLoopMitigationMinSpeedSamplingWindow,
      .min_discrete_speed_samples =
          kDefaultLoopMitigationMinDiscreteSpeedSamples,
  };
}

PositionModelerParams::LoopContractionMitigationParameters
LoopContractionParams(
    const BrushFamily::ExperimentalRawPositionModel& raw_position_model,
    std::optional<PhysicalDistance> stroke_unit_length) {
  return {
      .is_enabled = true,
      .speed_lower_bound = 0.0f,
      .speed_upper_bound = 0.0f,
      .interpolation_strength_at_speed_lower_bound = 0.0f,
      .interpolation_strength_at_speed_upper_bound = 0.0f,
      .min_speed_sampling_window = stroke_model::Duration(0),
      .min_discrete_speed_samples = 1,
  };
}

PositionModelerParams::LoopContractionMitigationParameters
MakeLoopContractionMitigationParameters(
    const BrushFamily::InputModel& input_model,
    std::optional<PhysicalDistance> stroke_unit_length) {
  return std::visit(
      [stroke_unit_length](auto&& input_model) {
        return LoopContractionParams(input_model, stroke_unit_length);
      },
      input_model);
}

StylusStateModelerParams StylusModelerParams(
    const BrushFamily::SpringModel& spring_model) {
  return {.use_stroke_normal_projection = true,
          .min_input_samples = 8,
          .min_sample_duration = stroke_model::Duration(0.04)};
}

StylusStateModelerParams StylusModelerParams(
    const BrushFamily::ExperimentalRawPositionModel& raw_position_model) {
  return {.use_stroke_normal_projection = true,
          .min_input_samples = 1,
          .min_sample_duration = stroke_model::Duration(0)};
}

StylusStateModelerParams MakeStylusStateModelerParams(
    const BrushFamily::InputModel& input_model) {
  return std::visit(
      [](auto&& input_model) -> stroke_model::StylusStateModelerParams {
        return StylusModelerParams(input_model);
      },
      input_model);
}

// LINT.ThenChange(../../brush/brush_family.h:input_model_types)

void ResetStrokeModeler(stroke_model::StrokeModeler& stroke_modeler,
                        const BrushFamily::InputModel& input_model,
                        float brush_epsilon,
                        std::optional<PhysicalDistance> stroke_unit_length) {
  // The minimum output rate was chosen to match legacy behavior, which was
  // in turn chosen to upsample enough to produce relatively smooth-looking
  // curves on 60 Hz touchscreens.
  constexpr double kMinOutputRateHz = 180;
  // We use the defaults for `PositionModelerParams` and
  // `StylusStateModelerParams`.
  ABSL_CHECK_OK(stroke_modeler.Reset(
      {// We turn off wobble smoothing because, in order to choose parameters
       // appropriately, we need to know the input rate and range of speeds that
       // we'll see for a stroke, which we don't have access to.
       .wobble_smoother_params = {.is_enabled = false},
       // We turn off loop contraction mitigation if the input model is not
       // spring model v2.
       .position_modeler_params = {.loop_contraction_mitigation_params =
                                       MakeLoopContractionMitigationParameters(
                                           input_model, stroke_unit_length)},
       // `brush_epsilon` is used for the stopping distance because once end of
       // the stroke is with `brush_epsilon` of the final input, further changes
       // are not considered visually distinct.
       .sampling_params = {.min_output_rate = kMinOutputRateHz,
                           .end_of_stroke_stopping_distance = brush_epsilon},
       // If we use loop mitigation, we need to use the new projection method.
       .stylus_state_modeler_params = MakeStylusStateModelerParams(input_model),
       // We disable the internal predictor on the `StrokeModeler`,
       // because it performs prediction after modeling. We wish to
       // accept external un-modeled prediction, as in the case of
       // platform provided prediction.
       .prediction_params = stroke_model::DisabledPredictorParams()}));
}

}  // namespace

void StrokeInputModeler::ExtendStroke(const StrokeInputBatch& real_inputs,
                                      const StrokeInputBatch& predicted_inputs,
                                      Duration32 current_elapsed_time) {
  ABSL_CHECK_GT(brush_epsilon_, 0) << "`StartStroke()` has not been called.";

  absl::Cleanup update_time_and_distance = [&]() {
    UpdateStateTimeAndDistance(current_elapsed_time);
  };

  if (real_inputs.IsEmpty() && predicted_inputs.IsEmpty() &&
      state_.real_input_count == modeled_inputs_.size()) {
    // We can return early, because there are no new inputs, and none of the
    // modeled inputs came from previous `predicted_inputs`. This allows us to
    // skip re-modeling the `last_real_stroke_input_`.
    return;
  }

  if (!last_real_stroke_input_.has_value()) {
    state_.tool_type = real_inputs.IsEmpty() ? predicted_inputs.GetToolType()
                                             : real_inputs.GetToolType();
    state_.stroke_unit_length = real_inputs.IsEmpty()
                                    ? predicted_inputs.GetStrokeUnitLength()
                                    : real_inputs.GetStrokeUnitLength();
    ResetStrokeModeler(stroke_modeler_, input_model_, brush_epsilon_,
                       real_inputs.GetStrokeUnitLength());
    stroke_modeler_has_input_ = false;
  }

  // Clear any "unstable" modeled inputs.
  modeled_inputs_.resize(state_.stable_input_count);

  // Re-model the current last real input as "stable" only if there are new
  // real inputs to process:
  if (last_real_stroke_input_.has_value() && !real_inputs.IsEmpty()) {
    ModelInput(*last_real_stroke_input_, /* last_input_in_update = */ false);
  }

  // Model all except the last new real input as "stable". The last one must
  // always be processed as "unstable", even in the case that current
  // `predicted_inputs` are non-empty, because a future update might have no new
  // predicted inputs.
  for (size_t i = 0; i + 1 < real_inputs.Size(); ++i) {
    ModelInput(real_inputs.Get(i), /* last_input_in_update = */ false);
  }

  // Save the state of the stroke modeler and model the remaining inputs as
  // "unstable".
  stroke_modeler_.Save();
  bool stroke_modeler_save_has_input = stroke_modeler_has_input_;
  state_.stable_input_count = modeled_inputs_.size();

  if (!real_inputs.IsEmpty()) {
    last_real_stroke_input_ = real_inputs.Get(real_inputs.Size() - 1);
    ModelInput(*last_real_stroke_input_,
               /* last_input_in_update = */ predicted_inputs.IsEmpty());
  } else if (last_real_stroke_input_.has_value()) {
    ModelInput(*last_real_stroke_input_,
               /* last_input_in_update = */ predicted_inputs.IsEmpty());
  }

  state_.real_input_count = modeled_inputs_.size();

  for (size_t i = 0; i < predicted_inputs.Size(); ++i) {
    ModelInput(predicted_inputs.Get(i),
               /* last_input_in_update = */ i == predicted_inputs.Size() - 1);
  }

  stroke_modeler_.Restore();
  stroke_modeler_has_input_ = stroke_modeler_save_has_input;
}

void StrokeInputModeler::ModelInput(const StrokeInput& input,
                                    bool last_input_in_update) {
  // The smoothing done by the `stroke_modeler_` causes the modeled results to
  // lag behind the current end of the stroke. This is usually made up for by
  // modeler's internal predictor, but we are disabling that to support external
  // prediction. Therefore, if we have more than one input in the stroke, we
  // need to model the last input in each update as a `kUp` event, which makes
  // the modeler catch up. This action is incompatible with further inputs, so
  // the `kUp` must be done after calling `stroke_modeler_.Save()`. This is why
  // this function should only be called with `last_input_in_update == true`
  // after the modeler save call. Note that the stroke modeler does model a
  // result for a `kDown` event, so a `kUp` is not necessary for a stroke
  // consisting of a single input.

  auto event_type = stroke_model::Input::EventType::kMove;
  if (!stroke_modeler_has_input_) {
    stroke_modeler_has_input_ = true;
    event_type = stroke_model::Input::EventType::kDown;
  } else if (last_input_in_update) {
    // The stroke modeler requires distinct `kDown` and `kUp` events, so we can
    // only pass a `kUp` if this is not the first input of the stroke.
    event_type = stroke_model::Input::EventType::kUp;
  }

  result_buffer_.clear();

  // `StrokeInputBatch` and `InProgressStroke` are designed to perform all the
  // necessary validation so that this operation should not fail.
  ABSL_CHECK_OK(stroke_modeler_.Update(
      {.event_type = event_type,
       .position = {input.position.x, input.position.y},
       .time = stroke_model::Time(input.elapsed_time.ToSeconds()),
       .pressure = input.pressure,
       .tilt = input.tilt.ValueInRadians(),
       .orientation = input.orientation.ValueInRadians()},
      result_buffer_));

  std::optional<Point> previous_position;
  float traveled_distance = 0;
  if (!modeled_inputs_.empty()) {
    previous_position = modeled_inputs_.back().position;
    traveled_distance = modeled_inputs_.back().traveled_distance;
  }

  for (const stroke_model::Result& result : result_buffer_) {
    Point position = {.x = result.position.x, .y = result.position.y};

    if (previous_position.has_value()) {
      float delta = (position - *previous_position).Magnitude();
      if (delta < brush_epsilon_) continue;

      traveled_distance += delta;
    }

    modeled_inputs_.push_back({
        .position = position,
        .velocity = {.x = result.velocity.x, .y = result.velocity.y},
        .acceleration = {.x = result.acceleration.x,
                         .y = result.acceleration.y},
        .traveled_distance = traveled_distance,
        .elapsed_time = Duration32::Seconds(result.time.Value()),
        .pressure = result.pressure,
        .tilt = Angle::Radians(result.tilt),
        .orientation = Angle::Radians(result.orientation),
    });

    previous_position = position;
  }
}

void StrokeInputModeler::UpdateStateTimeAndDistance(
    Duration32 current_elapsed_time) {
  if (modeled_inputs_.empty()) {
    state_.complete_elapsed_time = current_elapsed_time;
    return;
  }

  const auto& last_input = modeled_inputs_.back();
  state_.complete_elapsed_time =
      std::max(last_input.elapsed_time, current_elapsed_time);
  state_.complete_traveled_distance = last_input.traveled_distance;

  if (state_.real_input_count == 0) return;

  const auto& last_real_input = modeled_inputs_[state_.real_input_count - 1];
  state_.total_real_distance = last_real_input.traveled_distance;
  state_.total_real_elapsed_time = last_real_input.elapsed_time;
}

}  // namespace ink::strokes_internal
