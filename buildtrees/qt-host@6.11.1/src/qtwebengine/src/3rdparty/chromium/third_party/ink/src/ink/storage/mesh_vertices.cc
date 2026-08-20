// Copyright 2024-2025 Google LLC
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

#include "ink/storage/mesh_vertices.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/types/iterator_range.h"

namespace ink {

using ::ink::proto::CodedMesh;

CodedMeshVertexIterator::CodedMeshVertexIterator(
    CodedNumericRunIterator<float> x_stroke_space,
    CodedNumericRunIterator<float> y_stroke_space)
    : x_stroke_space_(x_stroke_space), y_stroke_space_(y_stroke_space) {
  UpdateValue();
}

CodedMeshVertexIterator& CodedMeshVertexIterator::operator++() {
  ++x_stroke_space_;
  ++y_stroke_space_;
  UpdateValue();
  return *this;
}

void CodedMeshVertexIterator::UpdateValue() {
  if (!x_stroke_space_.HasValue()) return;
  vertex_.position.x = *x_stroke_space_;
  vertex_.position.y = *y_stroke_space_;
}

absl::StatusOr<iterator_range<CodedMeshVertexIterator>> DecodeMeshVertices(
    const CodedMesh& mesh) {
  int num_vertices = mesh.x_stroke_space().deltas_size();
  if (mesh.y_stroke_space().deltas_size() != num_vertices) {
    return absl::InvalidArgumentError(
        "invalid mesh: mismatched numeric run lengths");
  }

  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
      x_stroke_space = DecodeFloatNumericRun(mesh.x_stroke_space());
  if (!x_stroke_space.ok()) {
    return x_stroke_space.status();
  }
  absl::StatusOr<iterator_range<CodedNumericRunIterator<float>>>
      y_stroke_space = DecodeFloatNumericRun(mesh.y_stroke_space());
  if (!y_stroke_space.ok()) {
    return y_stroke_space.status();
  }

  // TODO: b/259411109 - Also decode the (optional) color/texture vertex data.

  return iterator_range<CodedMeshVertexIterator>{
      CodedMeshVertexIterator(x_stroke_space->begin(), y_stroke_space->begin()),
      CodedMeshVertexIterator(x_stroke_space->end(), y_stroke_space->end())};
}

}  // namespace ink
