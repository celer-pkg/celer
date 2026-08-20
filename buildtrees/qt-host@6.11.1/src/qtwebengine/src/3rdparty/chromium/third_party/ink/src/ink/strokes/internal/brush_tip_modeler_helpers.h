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

#ifndef INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_HELPERS_H_
#define INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_HELPERS_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <variant>
#include <vector>

#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_tip.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/easing_implementation.h"
#include "ink/strokes/internal/noise_generator.h"
#include "ink/strokes/internal/stroke_input_modeler.h"

namespace ink::strokes_internal {

// Conceptually, the values manipulated by brush behavior nodes are nullable
// finite floats. For performance reasons, these are represented internally as
// plain `float`s (rather than, say, `std::optional<float>`), using NaN as a
// sentinel value to represent "null".
static_assert(std::numeric_limits<float>::has_quiet_NaN);
inline constexpr float kNullBehaviorNodeValue =
    std::numeric_limits<float>::quiet_NaN();

// Returns true if the given brush behavior node value is "null".
inline bool IsNullBehaviorNodeValue(float value) { return std::isnan(value); }

struct NoiseNodeImplementation {
  // The index into `BehaviorNodeContext::*_noise_generators_` for the latest
  // noise generator state of this noise node.
  size_t generator_index;
  // The below fields are copies of the same fields from the
  // `BrushBehavior::NoiseNode` that this struct helps implement.
  BrushBehavior::DampingSource vary_over;
  float base_period;
};

struct DampingNodeImplementation {
  // The index into `BehaviorNodeContext::*_damped_values_` for the latest
  // damped value of this damping node.
  size_t damping_index;
  // The below fields are copies of the same fields from the
  // `BrushBehavior::DampingNode` that this struct helps implement.
  BrushBehavior::DampingSource damping_source;
  float damping_gap;
};

struct TargetNodeImplementation {
  // The index into `BehaviorNodeContext::*_target_modifiers_` for the latest
  // modifier value of this target node.
  size_t target_index;
  // The below field is a copy of the same field from the
  // `BrushBehavior::TargetNode` that this struct helps implement.
  std::array<float, 2> target_modifier_range;
};

struct PolarTargetNodeImplementation {
  // The indices into `BehaviorNodeContext::target_modifiers` for the latest X/Y
  // modifier values of this target node.
  size_t target_x_index;
  size_t target_y_index;
  // The below fields are copies of the same fields from the
  // `BrushBehavior::PolarTargetNode` that this struct helps implement.
  std::array<float, 2> angle_range;
  std::array<float, 2> magnitude_range;
};

using BehaviorNodeImplementation =
    std::variant<BrushBehavior::SourceNode, BrushBehavior::ConstantNode,
                 NoiseNodeImplementation, BrushBehavior::FallbackFilterNode,
                 BrushBehavior::ToolTypeFilterNode, DampingNodeImplementation,
                 EasingImplementation, BrushBehavior::BinaryOpNode,
                 BrushBehavior::InterpolationNode, TargetNodeImplementation,
                 PolarTargetNodeImplementation>;

// Holds references to stroke data needed by `ProcessBehaviorNode()`, as well as
// references to mutable state that that function will need to update.
struct BehaviorNodeContext {
  const StrokeInputModeler::State& input_modeler_state;
  const ModeledStrokeInput& current_input;
  std::optional<Angle> current_travel_direction;
  float brush_size;
  // Distance/time from the start of the stroke up to the previous input (if
  // any).
  std::optional<InputMetrics> previous_input_metrics;
  std::vector<float>& stack;
  absl::Span<NoiseGenerator> noise_generators;
  absl::Span<float> damped_values;
  absl::Span<float> target_modifiers;
};

// Executes the specified node on the specified context. Note that although
// `context` is a const reference, the mutable objects that `context` references
// (`stack`, `damped_values`, and `target_modifiers`) will in general be
// modified by this function.
void ProcessBehaviorNode(const BehaviorNodeImplementation& node,
                         const BehaviorNodeContext& context);

// Constructs a `BrushTipState` at the given `position` using the non-behavior
// parameters of `brush_tip` with `brush_size`, and then applies
// `behavior_modifiers`.
//
// `behavior_modifiers` is expected to be the same size as `brush_tip.behaviors`
// and to hold the current modifier value for each behavior calculated using
// `UpdateBehaviorModifier()` above.
BrushTipState CreateTipState(Point position, std::optional<Angle> direction,
                             const BrushTip& brush_tip, float brush_size,
                             absl::Span<const BrushBehavior::Target> targets,
                             absl::Span<const float> target_modifiers);

// Computes the linear interpolation between `a` and `b` when `t` is in the
// range [0, 1], and the linear extrapolation otherwise.
ModeledStrokeInput Lerp(const ModeledStrokeInput& a,
                        const ModeledStrokeInput& b, float t);

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_BRUSH_TIP_MODELER_HELPERS_H_
