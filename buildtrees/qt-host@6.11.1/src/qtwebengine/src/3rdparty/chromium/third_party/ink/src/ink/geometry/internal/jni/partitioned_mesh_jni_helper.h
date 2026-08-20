// Copyright 2025 Google LLC
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

#ifndef INK_GEOMETRY_INTERNAL_JNI_PARTITIONED_MESH_JNI_HELPER_H_
#define INK_GEOMETRY_INTERNAL_JNI_PARTITIONED_MESH_JNI_HELPER_H_

#include <jni.h>

#include "absl/log/absl_check.h"
#include "ink/geometry/partitioned_mesh.h"

namespace ink::jni {

// Creates a new stack-allocated copy of the `PartitionedMesh` and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin PartitionedMesh.
inline jlong NewNativePartitionedMesh(const PartitionedMesh& mesh) {
  return reinterpret_cast<jlong>(new PartitionedMesh(mesh));
}

// Creates a new stack-allocated empty `PartitionedMesh` and returns a pointer
// to it as a jlong, suitable for wrapping in a Kotlin PartitionedMesh.
inline jlong NewNativePartitionedMesh() {
  return NewNativePartitionedMesh(PartitionedMesh());
}

// Casts a Kotlin PartitionedMesh.nativePointer to a C++ PartitionedMesh. The
// returned PartitionedMesh is a const ref as the Kotlin PartitionedMesh is
// immutable.
inline const PartitionedMesh& CastToPartitionedMesh(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  return *reinterpret_cast<PartitionedMesh*>(native_pointer);
}

// Frees a Kotlin PartitionedMesh.nativePointer.
inline void DeleteNativePartitionedMesh(jlong native_pointer) {
  if (native_pointer == 0) return;
  delete reinterpret_cast<PartitionedMesh*>(native_pointer);
}

}  // namespace ink::jni

#endif  // INK_GEOMETRY_INTERNAL_JNI_PARTITIONED_MESH_JNI_HELPER_H_
