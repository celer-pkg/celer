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

#include "ink/strokes/stroke.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/color/color.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/internal/stroke_shape_builder.h"
#include "ink/strokes/internal/stroke_vertex.h"

namespace ink {
namespace {

using ::ink::strokes_internal::StrokeShapeBuilder;
using ::ink::strokes_internal::StrokeVertex;

bool BrushCoatTipsAreEqual(absl::Span<const BrushCoat> coats1,
                           absl::Span<const BrushCoat> coats2) {
  if (coats1.size() != coats2.size()) {
    return false;
  }
  for (size_t i = 0; i < coats1.size(); ++i) {
    if (coats1[i].tip != coats2[i].tip) {
      return false;
    }
  }
  return true;
}

}  // namespace

Stroke::Stroke(const Brush& brush)
    : brush_(brush),
      shape_(PartitionedMesh::WithEmptyGroups(brush_.CoatCount())) {}

Stroke::Stroke(const Brush& brush, const StrokeInputBatch& inputs)
    : brush_(brush), inputs_(inputs) {
  RegenerateShape();
}

Stroke::Stroke(const Brush& brush, const StrokeInputBatch& inputs,
               const PartitionedMesh& shape)
    : brush_(brush), inputs_(inputs), shape_(shape) {
  ABSL_CHECK_EQ(shape_.RenderGroupCount(), brush_.CoatCount())
      << "`shape` must have one render group per brush coat in `brush`";
}

void Stroke::SetBrushAndInputs(const Brush& brush,
                               const StrokeInputBatch& inputs) {
  brush_ = brush;
  inputs_ = inputs;
  RegenerateShape();
}

void Stroke::SetBrush(const Brush& brush) {
  bool needs_regenerate =
      brush.GetSize() != brush_.GetSize() ||
      brush.GetEpsilon() != brush_.GetEpsilon() ||
      !BrushCoatTipsAreEqual(brush.GetCoats(), brush_.GetCoats());

  brush_ = brush;
  if (needs_regenerate) {
    RegenerateShape();
  }
}

void Stroke::SetBrushFamily(const BrushFamily& brush_family) {
  bool needs_regenerate =
      !BrushCoatTipsAreEqual(brush_family.GetCoats(), brush_.GetCoats());
  brush_.SetFamily(brush_family);
  if (needs_regenerate) {
    RegenerateShape();
  }
}

void Stroke::SetBrushColor(const Color& color) { brush_.SetColor(color); }

absl::Status Stroke::SetBrushSize(float size) {
  if (size == brush_.GetSize()) {
    return absl::OkStatus();
  }
  absl::Status status = brush_.SetSize(size);
  if (!status.ok()) {
    return status;
  }
  RegenerateShape();
  return absl::OkStatus();
}

absl::Status Stroke::SetBrushEpsilon(float epsilon) {
  if (epsilon == brush_.GetEpsilon()) {
    return absl::OkStatus();
  }
  absl::Status status = brush_.SetEpsilon(epsilon);
  if (!status.ok()) {
    return status;
  }
  RegenerateShape();
  return absl::OkStatus();
}

void Stroke::SetInputs(const StrokeInputBatch& inputs) {
  inputs_.Clear();
  ABSL_CHECK_OK(inputs_.Append(inputs));
  RegenerateShape();
}

namespace {

// Resources for stroke shape generation grouped into a struct for simpler
// `thread_local` variable creation in `RegenerateShape()` below.
struct ShapeGenerationResources {
  std::vector<StrokeShapeBuilder> builders;
  std::vector<StrokeVertex::CustomPackingArray> custom_packing_arrays;
  std::vector<PartitionedMesh::MutableMeshGroup> mesh_groups;
};

}  // namespace

void Stroke::RegenerateShape() {
  // Create thread local stroke shape resources to save allocations if
  // `thread_local` is supported, which is almost always. If not, fall back to a
  // regular local variable.
#ifdef ABSL_HAVE_THREAD_LOCAL
  thread_local
#endif
      ShapeGenerationResources shape_gen;

  absl::Span<const BrushCoat> coats = brush_.GetCoats();
  size_t num_coats = coats.size();
  if (num_coats == 0 || inputs_.IsEmpty()) {
    shape_ = PartitionedMesh::WithEmptyGroups(brush_.CoatCount());
    return;
  }

  // If necessary, expand the thread-local builders vector to the number of
  // brush coats. In order to cache all the allocations within, we never shrink
  // this vector.
  if (shape_gen.builders.size() < num_coats) {
    shape_gen.builders.resize(num_coats);
  }
  shape_gen.custom_packing_arrays.clear();
  shape_gen.custom_packing_arrays.reserve(num_coats);
  shape_gen.mesh_groups.clear();
  shape_gen.mesh_groups.reserve(num_coats);

  for (size_t i = 0; i < num_coats; ++i) {
    StrokeShapeBuilder& builder = shape_gen.builders[i];
    builder.StartStroke(brush_.GetFamily().GetInputModel(), coats[i],
                        brush_.GetSize(), brush_.GetEpsilon(),
                        inputs_.GetNoiseSeed());
    builder.ExtendStroke(inputs_, StrokeInputBatch(), inputs_.GetDuration());

    const MutableMesh& mesh = builder.GetMesh();
    shape_gen.custom_packing_arrays.push_back(
        StrokeVertex::MakeCustomPackingArray(mesh.Format()));

    shape_gen.mesh_groups.push_back({
        .mesh = &mesh,
        .outlines = builder.GetOutlines(),
        .packing_params = shape_gen.custom_packing_arrays.back().Values(),
    });
  }

  absl::StatusOr<PartitionedMesh> partitioned_mesh =
      PartitionedMesh::FromMutableMeshGroups(shape_gen.mesh_groups);
  if (partitioned_mesh.ok()) {
    shape_ = *std::move(partitioned_mesh);
  } else {
    ABSL_LOG(WARNING) << "Failed to create PartitionedMesh: "
                      << partitioned_mesh.status();
    shape_ = PartitionedMesh::WithEmptyGroups(brush_.CoatCount());
  }

  ABSL_DCHECK_EQ(shape_.RenderGroupCount(), brush_.CoatCount());
}

}  // namespace ink
