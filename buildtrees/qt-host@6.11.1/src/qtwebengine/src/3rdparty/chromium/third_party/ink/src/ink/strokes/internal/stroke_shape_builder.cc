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

#include "ink/strokes/internal/stroke_shape_builder.h"

#include <cstdint>

#include "absl/types/span.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/brush_tip_extruder.h"
#include "ink/strokes/internal/brush_tip_modeler.h"
#include "ink/strokes/internal/stroke_outline.h"
#include "ink/strokes/internal/stroke_shape_update.h"
#include "ink/types/duration.h"

namespace ink::strokes_internal {
namespace {

bool IsWindingTextureCoat(const BrushCoat& coat) {
  // We can only handle winding textures when there is a single texture layer.
  return coat.paint.texture_layers.size() == 1 &&
         coat.paint.texture_layers[0].mapping ==
             BrushPaint::TextureMapping::kWinding;
}

}  // namespace

void StrokeShapeBuilder::StartStroke(const BrushFamily::InputModel& input_model,
                                     const BrushCoat& coat, float brush_size,
                                     float brush_epsilon, uint32_t noise_seed) {
  // The `input_modeler_`, `tip_modeler_` and `tip_extruder_` CHECK-validate
  // `brush_tip` being not null, and `brush_size` and `brush_epsilon` being
  // greater than zero.
  input_modeler_.StartStroke(input_model, brush_epsilon);

  mesh_bounds_.Reset();
  outlines_.clear();

  bool is_winding_texture_brush = IsWindingTextureCoat(coat);
  bool is_winding_texture_particle_brush =
      is_winding_texture_brush &&
      (coat.tip.particle_gap_distance_scale != 0 ||
       coat.tip.particle_gap_duration != Duration32::Zero());
  tip_.modeler.StartStroke(&coat.tip, brush_size, noise_seed);
  tip_.extruder.StartStroke(brush_epsilon, is_winding_texture_particle_brush,
                            mesh_);
}

StrokeShapeUpdate StrokeShapeBuilder::ExtendStroke(
    const StrokeInputBatch& real_inputs,
    const StrokeInputBatch& predicted_inputs, Duration32 current_elapsed_time) {
  input_modeler_.ExtendStroke(real_inputs, predicted_inputs,
                              current_elapsed_time);
  StrokeShapeUpdate update;
  mesh_bounds_.Reset();

  outlines_.clear();
  BrushTipModeler& tip_modeler = tip_.modeler;
  BrushTipExtruder& tip_extruder = tip_.extruder;
  tip_modeler.UpdateStroke(input_modeler_.GetState(),
                           input_modeler_.GetModeledInputs());
  update.Add(tip_extruder.ExtendStroke(tip_modeler.NewFixedTipStates(),
                                       tip_modeler.VolatileTipStates()));
  mesh_bounds_.Add(tip_extruder.GetBounds());
  for (const StrokeOutline& outline : tip_extruder.GetOutlines()) {
    const absl::Span<const uint32_t>& indices = outline.GetIndices();
    if (!indices.empty()) {
      outlines_.push_back(indices);
    }
  }

  return update;
}

bool StrokeShapeBuilder::HasUnfinishedTimeBehaviors() const {
  return tip_.modeler.HasUnfinishedTimeBehaviors(input_modeler_.GetState());
}

}  // namespace ink::strokes_internal
