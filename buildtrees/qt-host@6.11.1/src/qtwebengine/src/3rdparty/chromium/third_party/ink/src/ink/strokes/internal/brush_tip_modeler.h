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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "absl/base/nullability.h"
#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/angle.h"
#include "ink/strokes/internal/brush_tip_modeler_helpers.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/stroke_input_modeler.h"
#include "ink/types/duration.h"

namespace ink::strokes_internal {

// The `BrushTipModeler` uses the `StrokeInputModeler::State` for a stroke to
// model a moving `BrushTip` with a particular brush size.
//
// Modeling the brush tip consists of
//   1. Calling `StartStroke()` to set up the modeler and select the `BrushTip`
//      and size.
//   2. Updating the ongoing stroke by:
//      a) Passing the current `StrokeInputModeler::State` to `UpdateStroke()`.
//      b) Consuming the generated `BrushTipState`s for each update returned by
//         `NewFixedTipStates()` and `VolatileTipStates()`.
//   3. Preferrably reusing the tip modeler for the next stroke by going back to
//      step 1.
//
//  Each `ModeledStrokeInput` will typically be used to generate one
//  `BrushTipState`. However, in general, each input may be used to create zero
//  or more states, and each state may be created from an interpolation of two
//  inputs.
//
// After each update, each `BrushTipState` falls into one of two categories:
// "fixed" or "volatile". A tip state is considered fixed if and only if it
// satisfies all of the following:
//   1. It was generated from only "stable" modeled input(s).
//   2. It was generated from modeled input(s) that are sufficiently far behind
//      the end of the stroke that the current `BrushTip` would generate the
//      same result for all further updates.
//
// Fixed tip states can be used to make permanent additions to stroke geometry,
// while any geometry made using volatile tip states must be undoable.
//
// All tip states generated from unstable modeled input are volatile. Typically
// most tip states that are generated from stable input will be fixed, but it
// is also possible for a given `BrushTip` to cause all such tip states to be
// volatile.
class BrushTipModeler {
 public:
  BrushTipModeler() = default;
  BrushTipModeler(const BrushTipModeler&) = delete;
  BrushTipModeler& operator=(const BrushTipModeler&) = delete;
  BrushTipModeler(BrushTipModeler&&) = default;
  BrushTipModeler& operator=(BrushTipModeler&&) = default;
  ~BrushTipModeler() = default;

  // Clears any ongoing stroke and sets up the modeler to accept new stroke
  // input.
  //
  // Calling `StartStroke()` is required before calling `UpdateStroke()`. The
  // `brush_tip` parameter must not be `nullptr` and must remain valid for the
  // duration of the stroke. `brush_size` must be finite and greater than 0.
  void StartStroke(const BrushTip* absl_nonnull brush_tip, float brush_size,
                   uint32_t noise_seed = 0);

  // Updates the tip modeler with the current `StrokeInputModeler::State` and
  // current inputs.
  //
  // Requires that `StartStroke()` has been called at least once since this
  // modeler was constructed.
  void UpdateStroke(const StrokeInputModeler::State& input_modeler_state,
                    absl::Span<const ModeledStrokeInput> inputs);

  // Returns true if the brush tip has any behaviors whose source values could
  // continue to change with the further passage of time (even in the absence of
  // any new inputs).
  bool HasUnfinishedTimeBehaviors(
      const StrokeInputModeler::State& input_modeler_state) const;

  // Returns tip states that have become fixed as a result of the most recent
  // call to `UpdateStroke()`.
  //
  // This means all prior fixed tip states have been discarded and will not be
  // returned. Some or all of the returned states may have been returned by
  // `VolatileTipStates()` on previous updates. All fixed tip states are
  // generated from stable modeled input.
  absl::Span<const BrushTipState> NewFixedTipStates() const;

  // Returns the current volatile tip states as a result of `UpdateStroke()`.
  //
  // All tip states generated from unstable modeled input will be returned by
  // this function in addition to any tip states made from stable modeled input
  // that are too close to the end of the stroke for the current `BrushTip`.
  absl::Span<const BrushTipState> VolatileTipStates() const;

 private:
  // Helper methods for the `std::visit` call in `StartStroke`.
  void AppendBehaviorNode(const BrushBehavior::SourceNode& node);
  void AppendBehaviorNode(const BrushBehavior::ConstantNode& node);
  void AppendBehaviorNode(const BrushBehavior::NoiseNode& node);
  void AppendBehaviorNode(const BrushBehavior::FallbackFilterNode& node);
  void AppendBehaviorNode(const BrushBehavior::ToolTypeFilterNode& node);
  void AppendBehaviorNode(const BrushBehavior::DampingNode& node);
  void AppendBehaviorNode(const BrushBehavior::ResponseNode& node);
  void AppendBehaviorNode(const BrushBehavior::BinaryOpNode& node);
  void AppendBehaviorNode(const BrushBehavior::InterpolationNode& node);
  void AppendBehaviorNode(const BrushBehavior::TargetNode& node);
  void AppendBehaviorNode(const BrushBehavior::PolarTargetNode& node);

  // Returns the maximum values of distance traveled and time elapsed for
  // modeled inputs that can be used to generate fixed tip states.
  InputMetrics CalculateMaxFixedInputMetrics(
      const StrokeInputModeler::State& input_modeler_state,
      absl::Span<const ModeledStrokeInput> inputs) const;

  // Processes a single `ModeledStrokeInput` and sets up particle emission if
  // enabled.
  void ProcessSingleInput(
      const StrokeInputModeler::State& input_modeler_state,
      const ModeledStrokeInput& current_input,
      std::optional<Angle> current_travel_direction,
      const ModeledStrokeInput* absl_nullable previous_input,
      std::optional<InputMetrics>& last_modeled_tip_state_metrics);

  // Appends a single new element to the `saved_tip_states_` based on the
  // current `input`.
  void AddNewTipState(
      const StrokeInputModeler::State& input_modeler_state,
      const ModeledStrokeInput& input, std::optional<Angle> travel_direction,
      std::optional<InputMetrics> previous_input_metrics,
      std::optional<InputMetrics>& last_modeled_tip_state_metrics);

  // Appends a "gap" tip state for when the tip modeler is emitting particles.
  //
  // The `BrushTipExtruder` inserts a "break" in mesh geometry whenever the
  // dimensions of the tip are below the value of brush epsilon. We piggyback on
  // this feature and insert a zero-sized tip state to disconnect all of the
  // non-zero-sized states, turning them into particles.
  void AppendParticleGapTipState() {
    saved_tip_states_.push_back(BrushTipState{.width = 0, .height = 0});
  }

  // Index into the `input_modeler_state.inputs` passed to `UpdateStroke()` for
  // the next `ModeledStrokeInput` that should be used to generate a fixed
  // `BrushTipState`.
  size_t input_index_for_next_fixed_state_ = 0;
  // The minimum distance traveled and time elapsed between emitted particles,
  // when one or both of the values is non-zero.
  InputMetrics particle_gap_metrics_;
  // The distance traveled and time elapsed used to model the most recent fixed
  // tip state.  If emitting particles, these values will generally be different
  // from those in `*last_fixed_input_`.
  std::optional<InputMetrics> last_fixed_modeled_tip_state_metrics_;

  std::vector<BrushTipState> saved_tip_states_;
  // The number of `BrushTipState` at the start of `saved_tip_states_` that are
  // considered fixed.
  size_t new_fixed_tip_state_count_ = 0;

  const BrushTip* absl_nullable brush_tip_ = nullptr;
  float brush_size_ = 0;
  // Per-stroke random seed value for seeding generators for noise behaviors (if
  // the brush tip has any). All tips/coats in the same brush use the same
  // per-stroke seed for a given stroke.
  uint32_t noise_seed_ = 0;

  // Cached values from `brush_tip_` that give the upper bounds on distance and
  // time remaining that are affected by the tip's behaviors.
  float distance_remaining_behavior_upper_bound_ = 0;
  Duration32 time_remaining_behavior_upper_bound_ = Duration32::Zero();
  float distance_fraction_behavior_upper_bound_ = 0;
  // Flag for whether the current `brush_tip_` has behaviors that depend on
  // properties of subsequent modeled inputs, like the travel direction.
  bool behaviors_depend_on_next_input_ = false;

  std::vector<BehaviorNodeImplementation> behavior_nodes_;
  std::vector<float> behavior_stack_;
  // These next two vectors must always be the same size:
  std::vector<NoiseGenerator> current_noise_generators_;
  std::vector<NoiseGenerator> fixed_noise_generators_;
  // These next two vectors must always be the same size:
  std::vector<float> current_damped_values_;
  std::vector<float> fixed_damped_values_;
  // These next three vectors must always be the same size:
  std::vector<BrushBehavior::Target> behavior_targets_;
  std::vector<float> current_target_modifiers_;
  std::vector<float> fixed_target_modifiers_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline absl::Span<const BrushTipState> BrushTipModeler::NewFixedTipStates()
    const {
  return absl::MakeSpan(saved_tip_states_.data(), new_fixed_tip_state_count_);
}

inline absl::Span<const BrushTipState> BrushTipModeler::VolatileTipStates()
    const {
  auto span = absl::MakeSpan(saved_tip_states_);
  span.remove_prefix(new_fixed_tip_state_count_);
  return span;
}

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_H_
