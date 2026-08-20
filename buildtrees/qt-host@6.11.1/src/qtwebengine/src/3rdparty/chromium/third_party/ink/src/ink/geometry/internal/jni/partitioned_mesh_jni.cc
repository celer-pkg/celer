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

#include <jni.h>

#include <cstddef>
#include <vector>

#include "absl/types/span.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/jni/mesh_format_jni_helper.h"
#include "ink/geometry/internal/jni/mesh_jni_helper.h"
#include "ink/geometry/internal/jni/partitioned_mesh_jni_helper.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/triangle.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::AffineTransform;
using ::ink::Angle;
using ::ink::Mesh;
using ::ink::PartitionedMesh;
using ::ink::Point;
using ::ink::Quad;
using ::ink::Rect;
using ::ink::Triangle;
using ::ink::jni::CastToPartitionedMesh;
using ::ink::jni::DeleteNativePartitionedMesh;
using ::ink::jni::NewNativeMesh;
using ::ink::jni::NewNativeMeshFormat;
using ::ink::jni::NewNativePartitionedMesh;

}  // namespace

extern "C" {

JNI_METHOD(geometry, PartitionedMeshNative, jlongArray, newCopiesOfMeshes)
(JNIEnv* env, jobject object, jlong native_pointer, jint group_index) {
  absl::Span<const Mesh> meshes =
      CastToPartitionedMesh(native_pointer).RenderGroupMeshes(group_index);
  std::vector<jlong> meshes_ptrs(meshes.size());
  for (size_t i = 0; i < meshes.size(); ++i) {
    // Create new heap-allocated copies of each `Mesh` object, to be owned by
    // the `Mesh.kt` objects of the `PartitionedMesh.kt` that is under
    // construction. The C++ `Mesh` type is cheap to copy because internally it
    // keeps a `shared_ptr` to its (immutable) data.
    meshes_ptrs[i] = NewNativeMesh(meshes[i]);
  }
  jlongArray mesh_pointers = env->NewLongArray(meshes.size());
  env->SetLongArrayRegion(mesh_pointers, 0, meshes.size(), meshes_ptrs.data());
  return mesh_pointers;
}

JNI_METHOD(geometry, PartitionedMeshNative, jint, getRenderGroupCount)
(JNIEnv* env, jobject object, jlong native_pointer) {
  return CastToPartitionedMesh(native_pointer).RenderGroupCount();
}

JNI_METHOD(geometry, PartitionedMeshNative, jlong, getRenderGroupFormat)
(JNIEnv* env, jobject object, jlong native_pointer, jint group_index) {
  return NewNativeMeshFormat(
      CastToPartitionedMesh(native_pointer).RenderGroupFormat(group_index));
}

JNI_METHOD(geometry, PartitionedMeshNative, jint, getOutlineCount)
(JNIEnv* env, jobject object, jlong native_pointer, jint group_index) {
  return CastToPartitionedMesh(native_pointer).OutlineCount(group_index);
}

JNI_METHOD(geometry, PartitionedMeshNative, jint, getOutlineVertexCount)
(JNIEnv* env, jobject object, jlong native_pointer, jint group_index,
 jint outline_index) {
  return CastToPartitionedMesh(native_pointer)
      .Outline(group_index, outline_index)
      .size();
}

JNI_METHOD(geometry, PartitionedMeshNative, void,
           fillOutlineMeshIndexAndMeshVertexIndex)
(JNIEnv* env, jobject object, jlong native_pointer, jint group_index,
 jint outline_index, jint outline_vertex_index,
 jintArray out_mesh_index_and_mesh_vertex_index) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  absl::Span<const PartitionedMesh::VertexIndexPair> outline =
      partitioned_mesh.Outline(group_index, outline_index);
  PartitionedMesh::VertexIndexPair index_pair = outline[outline_vertex_index];
  jint mesh_index_and_mesh_vertex_index[] = {index_pair.mesh_index,
                                             index_pair.vertex_index};
  env->SetIntArrayRegion(out_mesh_index_and_mesh_vertex_index, 0, 2,
                         mesh_index_and_mesh_vertex_index);
}

// Allocate an empty `PartitionedMesh` and return a pointer to it.
JNI_METHOD(geometry, PartitionedMeshNative, jlong, create)
(JNIEnv* env, jobject object) { return NewNativePartitionedMesh(); }

JNI_METHOD(geometry, PartitionedMeshNative, jfloat,
           partitionedMeshTriangleCoverage)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat triangle_p0_x,
 jfloat triangle_p0_y, jfloat triangle_p1_x, jfloat triangle_p1_y,
 jfloat triangle_p2_x, jfloat triangle_p2_y,
 jfloat triangle_to_partitionedMesh_transform_a,
 jfloat triangle_to_partitionedMesh_transform_b,
 jfloat triangle_to_partitionedMesh_transform_c,
 jfloat triangle_to_partitionedMesh_transform_d,
 jfloat triangle_to_partitionedMesh_transform_e,
 jfloat triangle_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  AffineTransform transform(triangle_to_partitionedMesh_transform_a,
                            triangle_to_partitionedMesh_transform_b,
                            triangle_to_partitionedMesh_transform_c,
                            triangle_to_partitionedMesh_transform_d,
                            triangle_to_partitionedMesh_transform_e,
                            triangle_to_partitionedMesh_transform_f);
  return partitioned_mesh.Coverage(triangle, transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jfloat, partitionedMeshBoxCoverage)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat rect_x_min,
 jfloat rect_y_min, jfloat rect_x_max, jfloat rect_y_max,
 jfloat rect_to_partitionedMesh_transform_a,
 jfloat rect_to_partitionedMesh_transform_b,
 jfloat rect_to_partitionedMesh_transform_c,
 jfloat rect_to_partitionedMesh_transform_d,
 jfloat rect_to_partitionedMesh_transform_e,
 jfloat rect_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  AffineTransform transform(
      rect_to_partitionedMesh_transform_a, rect_to_partitionedMesh_transform_b,
      rect_to_partitionedMesh_transform_c, rect_to_partitionedMesh_transform_d,
      rect_to_partitionedMesh_transform_e, rect_to_partitionedMesh_transform_f);
  return partitioned_mesh.Coverage(rect, transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jfloat,
           partitionedMeshParallelogramCoverage)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat quad_center_x,
 jfloat quad_center_y, jfloat quad_width, jfloat quad_height,
 jfloat quad_angle_radian, jfloat quad_shear_factor,
 jfloat quad_to_partitionedMesh_transform_a,
 jfloat quad_to_partitionedMesh_transform_b,
 jfloat quad_to_partitionedMesh_transform_c,
 jfloat quad_to_partitionedMesh_transform_d,
 jfloat quad_to_partitionedMesh_transform_e,
 jfloat quad_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{quad_center_x, quad_center_y}, quad_width, quad_height,
      Angle::Radians(quad_angle_radian), quad_shear_factor);
  AffineTransform transform(
      quad_to_partitionedMesh_transform_a, quad_to_partitionedMesh_transform_b,
      quad_to_partitionedMesh_transform_c, quad_to_partitionedMesh_transform_d,
      quad_to_partitionedMesh_transform_e, quad_to_partitionedMesh_transform_f);
  return partitioned_mesh.Coverage(quad, transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jfloat,
           partitionedMeshPartitionedMeshCoverage)
(JNIEnv* env, jobject object, jlong native_pointer,
 jlong other_partitioned_mesh_pointer, jfloat other_to_this_transform_a,
 jfloat other_to_this_transform_b, jfloat other_to_this_transform_c,
 jfloat other_to_this_transform_d, jfloat other_to_this_transform_e,
 jfloat other_to_this_transform_f) {
  const PartitionedMesh& this_partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  const PartitionedMesh& other_partitioned_mesh =
      CastToPartitionedMesh(other_partitioned_mesh_pointer);
  AffineTransform transform(
      other_to_this_transform_a, other_to_this_transform_b,
      other_to_this_transform_c, other_to_this_transform_d,
      other_to_this_transform_e, other_to_this_transform_f);
  return this_partitioned_mesh.Coverage(other_partitioned_mesh, transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jboolean,
           partitionedMeshTriangleCoverageIsGreaterThan)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat triangle_p0_x,
 jfloat triangle_p0_y, jfloat triangle_p1_x, jfloat triangle_p1_y,
 jfloat triangle_p2_x, jfloat triangle_p2_y, jfloat coverage_threshold,
 jfloat triangle_to_partitionedMesh_transform_a,
 jfloat triangle_to_partitionedMesh_transform_b,
 jfloat triangle_to_partitionedMesh_transform_c,
 jfloat triangle_to_partitionedMesh_transform_d,
 jfloat triangle_to_partitionedMesh_transform_e,
 jfloat triangle_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  AffineTransform transform(triangle_to_partitionedMesh_transform_a,
                            triangle_to_partitionedMesh_transform_b,
                            triangle_to_partitionedMesh_transform_c,
                            triangle_to_partitionedMesh_transform_d,
                            triangle_to_partitionedMesh_transform_e,
                            triangle_to_partitionedMesh_transform_f);
  return partitioned_mesh.CoverageIsGreaterThan(triangle, coverage_threshold,
                                                transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jboolean,
           partitionedMeshBoxCoverageIsGreaterThan)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat rect_x_min,
 jfloat rect_y_min, jfloat rect_x_max, jfloat rect_y_max,
 jfloat coverage_threshold, jfloat rect_to_partitionedMesh_transform_a,
 jfloat rect_to_partitionedMesh_transform_b,
 jfloat rect_to_partitionedMesh_transform_c,
 jfloat rect_to_partitionedMesh_transform_d,
 jfloat rect_to_partitionedMesh_transform_e,
 jfloat rect_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Rect rect =
      Rect::FromTwoPoints({rect_x_min, rect_y_min}, {rect_x_max, rect_y_max});
  AffineTransform transform(
      rect_to_partitionedMesh_transform_a, rect_to_partitionedMesh_transform_b,
      rect_to_partitionedMesh_transform_c, rect_to_partitionedMesh_transform_d,
      rect_to_partitionedMesh_transform_e, rect_to_partitionedMesh_transform_f);
  return partitioned_mesh.CoverageIsGreaterThan(rect, coverage_threshold,
                                                transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jboolean,
           partitionedMeshParallelogramCoverageIsGreaterThan)
(JNIEnv* env, jobject object, jlong native_pointer, jfloat quad_center_x,
 jfloat quad_center_y, jfloat quad_width, jfloat quad_height,
 jfloat quad_angle_radian, jfloat quad_shear_factor, jfloat coverage_threshold,
 jfloat quad_to_partitionedMesh_transform_a,
 jfloat quad_to_partitionedMesh_transform_b,
 jfloat quad_to_partitionedMesh_transform_c,
 jfloat quad_to_partitionedMesh_transform_d,
 jfloat quad_to_partitionedMesh_transform_e,
 jfloat quad_to_partitionedMesh_transform_f) {
  const PartitionedMesh& partitioned_mesh =
      CastToPartitionedMesh(native_pointer);
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{quad_center_x, quad_center_y}, quad_width, quad_height,
      Angle::Radians(quad_angle_radian), quad_shear_factor);
  AffineTransform transform(
      quad_to_partitionedMesh_transform_a, quad_to_partitionedMesh_transform_b,
      quad_to_partitionedMesh_transform_c, quad_to_partitionedMesh_transform_d,
      quad_to_partitionedMesh_transform_e, quad_to_partitionedMesh_transform_f);
  return partitioned_mesh.CoverageIsGreaterThan(quad, coverage_threshold,
                                                transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, jboolean,
           partitionedMeshPartitionedMeshCoverageIsGreaterThan)
(JNIEnv* env, jobject object, jlong native_pointer,
 jlong other_partitioned_mesh_pointer, jfloat coverage_threshold,
 jfloat other_to_this_transform_a, jfloat other_to_this_transform_b,
 jfloat other_to_this_transform_c, jfloat other_to_this_transform_d,
 jfloat other_to_this_transform_e, jfloat other_to_this_transform_f) {
  AffineTransform transform(
      other_to_this_transform_a, other_to_this_transform_b,
      other_to_this_transform_c, other_to_this_transform_d,
      other_to_this_transform_e, other_to_this_transform_f);
  return CastToPartitionedMesh(native_pointer)
      .CoverageIsGreaterThan(
          CastToPartitionedMesh(other_partitioned_mesh_pointer),
          coverage_threshold, transform);
}

JNI_METHOD(geometry, PartitionedMeshNative, void, initializeSpatialIndex)
(JNIEnv* env, jobject object, jlong native_pointer) {
  return CastToPartitionedMesh(native_pointer).InitializeSpatialIndex();
}

JNI_METHOD(geometry, PartitionedMeshNative, jboolean, isSpatialIndexInitialized)
(JNIEnv* env, jobject object, jlong native_pointer) {
  return CastToPartitionedMesh(native_pointer).IsSpatialIndexInitialized();
}

JNI_METHOD(geometry, PartitionedMeshNative, void, free)
(JNIEnv* env, jobject object, jlong native_pointer) {
  DeleteNativePartitionedMesh(native_pointer);
}

}  // extern "C"
