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

#ifndef INK_STORAGE_MESH_VERTICES_H_
#define INK_STORAGE_MESH_VERTICES_H_

#include <cstddef>
#include <iterator>

#include "absl/status/statusor.h"
#include "ink/storage/numeric_run.h"
#include "ink/storage/proto/mesh.pb.h"
#include "ink/strokes/internal/legacy_vertex.h"
#include "ink/types/iterator_range.h"

namespace ink {

// An iterator over the decoded vertices of a CodedMesh proto. It is expected to
// be used via the DecodeMeshVertices function below.
//
// This class conforms to the named requirements of LegacyInputIterator
// (https://en.cppreference.com/w/cpp/named_req/InputIterator).
//
// Note that this is a proxy iterator. The decoded value does not live in the
// CodedMesh, but within the iterator, and so its lifetime is tied to the
// lifetime of the iterator itself. To prevent lifetime issues, `reference` is
// an alias to the value type, not a true reference.
class CodedMeshVertexIterator {
 public:
  using difference_type = ptrdiff_t;
  using value_type = strokes_internal::LegacyVertex;
  using pointer = const value_type*;
  using reference = const value_type;
  using iterator_category = std::input_iterator_tag;

  // NOLINTNEXTLINE - Suppress ClangTidy const-return-type.
  reference operator*() const { return vertex_; }
  pointer operator->() const { return &vertex_; }

  CodedMeshVertexIterator() = default;
  CodedMeshVertexIterator(const CodedMeshVertexIterator&) = default;
  CodedMeshVertexIterator& operator=(const CodedMeshVertexIterator&) = default;

  CodedMeshVertexIterator& operator++();
  CodedMeshVertexIterator operator++(int) {
    CodedMeshVertexIterator retval = *this;
    ++(*this);
    return retval;
  }

  friend bool operator==(const CodedMeshVertexIterator& lhs,
                         const CodedMeshVertexIterator& rhs) {
    return lhs.x_stroke_space_ == rhs.x_stroke_space_;
  }
  friend bool operator!=(const CodedMeshVertexIterator& lhs,
                         const CodedMeshVertexIterator& rhs) {
    return lhs.x_stroke_space_ != rhs.x_stroke_space_;
  }

 private:
  CodedMeshVertexIterator(CodedNumericRunIterator<float> x_stroke_space,
                          CodedNumericRunIterator<float> y_stroke_space);

  void UpdateValue();

  CodedNumericRunIterator<float> x_stroke_space_;
  CodedNumericRunIterator<float> y_stroke_space_;
  strokes_internal::LegacyVertex vertex_;

  friend absl::StatusOr<iterator_range<CodedMeshVertexIterator>>
  DecodeMeshVertices(const proto::CodedMesh& mesh);
};

// Given a CodedMesh proto, returns an iterator range over the decoded vertices.
// The proto object must outlive the returned range.  Returns an error if the
// proto is invalid (e.g. if its constituent numeric runs are invalid or of
// unequal lengths).
//
// Note that this function will completely ignore the triangle/outline index
// data in the CodedMesh proto; it only validates/decodes the vertex data.
absl::StatusOr<iterator_range<CodedMeshVertexIterator>> DecodeMeshVertices(
    const proto::CodedMesh& mesh);

}  // namespace ink

#endif  // INK_STORAGE_MESH_VERTICES_H_
