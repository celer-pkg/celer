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

#ifndef INK_STROKES_STROKE_H_
#define INK_STROKES_STROKE_H_

#include "absl/status/status.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_family.h"
#include "ink/color/color.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/types/duration.h"

namespace ink {

// A `Stroke` is combination of a `StrokeInputBatch` that represents a
// user-drawn (or sometimes synthetic) path, a `Brush` that contains information
// on how that path should be converted into a geometric shape and rendered on
// screen, and a `PartitionedMesh`, which is the geometric shape calculated from
// the combination of the `StrokeInputBatch` and the `Brush`.
//
// Strokes can be constructed directly from a complete `StrokeInputBatch` or
// they can be constructed incrementally as input events are received in real
// time using `InProgressStroke`.
class Stroke {
 public:
  // Creates a stroke with the given `brush` and empty inputs and shape.
  explicit Stroke(const Brush& brush);

  // Creates a stroke using the given `brush` and `inputs` to generate the
  // shape.
  Stroke(const Brush& brush, const StrokeInputBatch& inputs);

  // Constructs with the given `brush`, `inputs`, and a pre-generated
  // `shape`.
  //
  // This is the intended way for deserialization to reconstruct a stroke. This
  // is not the recommended way to make a copy of an existing `Stroke`, as this
  // will always make an exact deep copy of the inputs and shape data. It is
  // also not feasible for the constructor to validate that the incoming `shape`
  // is visually the same as what would be generated from `brush` and `inputs`
  // from scratch.
  //
  // This CHECK-fails if `shape` doesn't have exactly one render group per brush
  // coat in `brush`.
  Stroke(const Brush& brush, const StrokeInputBatch& inputs,
         const PartitionedMesh& shape);

  Stroke(const Stroke& s) = default;
  Stroke(Stroke&& s) = default;
  Stroke& operator=(const Stroke& s) = default;
  Stroke& operator=(Stroke&& s) = default;

  const Brush& GetBrush() const { return brush_; }
  const BrushFamily& GetBrushFamily() const { return brush_.GetFamily(); }
  const StrokeInputBatch& GetInputs() const { return inputs_; }
  // Returns the `PartitionedMesh` for this stroke. This shape will have exactly
  // one render group per brush coat in `GetBrush()`.
  const PartitionedMesh& GetShape() const { return shape_; }

  // Returns the total input duration for this stroke.
  Duration32 GetInputDuration() const { return inputs_.GetDuration(); }

  // Sets both the `brush` and `inputs` for the stroke, always clearing the
  // shape and regenerating it if the new `inputs` are non-empty.
  void SetBrushAndInputs(const Brush& brush, const StrokeInputBatch& inputs);

  // Sets the `brush`, regenerating the mesh if needed.
  //
  // The mesh is regenerated if this call results in a change of the
  // `BrushTip`s, brush size, or brush epsilon.
  void SetBrush(const Brush& brush);

  // Sets the brush `family`, regenerating the mesh if the new family has a
  // different set of `BrushTip`s than the current brush tip.
  void SetBrushFamily(const BrushFamily& brush_family);

  // Sets the brush `color`. Never requires regenerating the shape.
  void SetBrushColor(const Color& color);

  // Sets the brush `size`, regenerating the shape if the new `size` is valid
  // and different from the current value.
  //
  // Returns an error and does not modify the stroke if `size` is not a finite
  // and positive value or if `size` is smaller than `epsilon`.
  absl::Status SetBrushSize(float size);

  // Sets the brush `epsilon`, regenerating the shape if the new `epsilon` is
  // valid and different from the current value.
  //
  // Returns an error and does not modify the stroke if `epsilon` is not a
  // finite and positive value or if `epsilon` is greater than `size`.
  absl::Status SetBrushEpsilon(float epsilon);

  // Sets the `inputs` for the stroke, regenerating the shape, or clearing the
  // shape if `inputs` is empty.
  void SetInputs(const StrokeInputBatch& inputs);

 private:
  // Regenerates the PartitionedMesh.
  void RegenerateShape();

  Brush brush_;
  StrokeInputBatch inputs_;
  PartitionedMesh shape_;
};

}  // namespace ink

#endif  // INK_STROKES_STROKE_H_
