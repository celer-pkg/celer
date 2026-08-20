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

#ifndef INK_STORAGE_PARTITIONED_MESH_H_
#define INK_STORAGE_PARTITIONED_MESH_H_

#include "absl/status/statusor.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/storage/proto/mesh.pb.h"

namespace ink {

// Populates the `CodedModeledShape` proto by encoding the given
// `PartitionedMesh`.
//
// The `CodedModeledShape` proto need not be empty before calling this; this
// will effectively clear the proto first.
void EncodePartitionedMesh(const PartitionedMesh& shape,
                           ink::proto::CodedModeledShape& shape_proto);

// Decodes the `CodedModeledShape` proto into a `PartitionedMesh`. Returns an
// error if the proto is invalid.
absl::StatusOr<PartitionedMesh> DecodePartitionedMesh(
    const ink::proto::CodedModeledShape& shape_proto);

}  // namespace ink

#endif  // INK_STORAGE_PARTITIONED_MESH_H_
