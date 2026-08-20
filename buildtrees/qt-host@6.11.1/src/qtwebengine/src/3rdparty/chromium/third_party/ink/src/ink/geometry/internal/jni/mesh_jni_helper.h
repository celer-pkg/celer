#ifndef INK_GEOMETRY_INTERNAL_JNI_MESH_JNI_HELPER_H_
#define INK_GEOMETRY_INTERNAL_JNI_MESH_JNI_HELPER_H_

#include <jni.h>

#include "absl/log/absl_check.h"
#include "ink/geometry/mesh.h"

namespace ink::jni {

// Creates a new stack-allocated copy of the `Mesh` and returns a pointer
// to it as a jlong, suitable for wrapping in a Kotlin Mesh.
inline jlong NewNativeMesh(const Mesh& mesh) {
  return reinterpret_cast<jlong>(new Mesh(mesh));
}

// Creates a new stack-allocated empty `Mesh` and returns a pointer
// to it as a jlong, suitable for wrapping in a Kotlin Mesh.
inline jlong NewNativeMesh() {
  // Note that URVO avoids the inefficiency of constructing this on the stack
  // and then copy-constructing it on the heap.
  return NewNativeMesh(Mesh());
}

// Casts a Kotlin Mesh.nativePointer to a C++ Mesh. The returned
// Mesh is a const ref as the Kotlin Mesh is immutable.
inline const Mesh& CastToMesh(jlong native_pointer) {
  ABSL_CHECK_NE(native_pointer, 0);
  return *reinterpret_cast<Mesh*>(native_pointer);
}

// Frees a Kotlin Mesh.nativePointer.
inline void DeleteNativeMesh(jlong native_pointer) {
  if (native_pointer == 0) return;
  delete reinterpret_cast<Mesh*>(native_pointer);
}

}  // namespace ink::jni

#endif  // INK_GEOMETRY_INTERNAL_JNI_MESH_JNI_HELPER_H_
