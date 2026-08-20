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

#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/internal/jni/partitioned_mesh_jni_helper.h"
#include "ink/geometry/intersects.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::AffineTransform;
using ::ink::Angle;
using ::ink::Intersects;
using ::ink::Point;
using ::ink::Quad;
using ::ink::Rect;
using ::ink::Segment;
using ::ink::Triangle;
using ::ink::jni::CastToPartitionedMesh;

}  // namespace

extern "C" {

JNI_METHOD(geometry, Intersection, jboolean, nativeVecSegmentIntersects)
(JNIEnv* env, jobject object, jfloat vec_x, jfloat vec_y,
 jfloat segment_start_x, jfloat segment_start_y, jfloat segment_end_x,
 jfloat segment_end_y) {
  Point point{vec_x, vec_y};
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  return Intersects(point, segment);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeVecTriangleIntersects)
(JNIEnv* env, jobject object, jfloat vec_x, jfloat vec_y, jfloat triangle_p0_x,
 jfloat triangle_p0_y, jfloat triangle_p1_x, jfloat triangle_p1_y,
 jfloat triangle_p2_x, jfloat triangle_p2_y) {
  Point point{vec_x, vec_y};
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  return Intersects(point, triangle);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeVecBoxIntersects)
(JNIEnv* env, jobject object, jfloat vec_x, jfloat vec_y, jfloat box_x_min,
 jfloat box_y_min, jfloat box_x_max, jfloat box_y_max) {
  Point point{vec_x, vec_y};
  Rect rect =
      Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  return Intersects(point, rect);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeVecParallelogramIntersects)
(JNIEnv* env, jobject object, jfloat vec_x, jfloat vec_y,
 jfloat parallelogram_center_x, jfloat parallelogram_center_y,
 jfloat parallelogram_width, jfloat parallelogram_height,
 jfloat parallelogram_angle_radian, jfloat parallelogram_shear_factor) {
  Point point{vec_x, vec_y};
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_center_x, parallelogram_center_y},
      parallelogram_width, parallelogram_height,
      Angle::Radians(parallelogram_angle_radian), parallelogram_shear_factor);
  return Intersects(point, quad);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeSegmentSegmentIntersects)
(JNIEnv* env, jobject object, jfloat segment_1_start_x,
 jfloat segment_1_start_y, jfloat segment_1_end_x, jfloat segment_1_end_y,
 jfloat segment_2_start_x, jfloat segment_2_start_y, jfloat segment_2_end_x,
 jfloat segment_2_end_y) {
  Segment segment1{{segment_1_start_x, segment_1_start_y},
                   {segment_1_end_x, segment_1_end_y}};
  Segment segment2{{segment_2_start_x, segment_2_start_y},
                   {segment_2_end_x, segment_2_end_y}};
  return Intersects(segment1, segment2);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeSegmentTriangleIntersects)
(JNIEnv* env, jobject object, jfloat segment_start_x, jfloat segment_start_y,
 jfloat segment_end_x, jfloat segment_end_y, jfloat triangle_p0_x,
 jfloat triangle_p0_y, jfloat triangle_p1_x, jfloat triangle_p1_y,
 jfloat triangle_p2_x, jfloat triangle_p2_y) {
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  return Intersects(segment, triangle);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeSegmentBoxIntersects)
(JNIEnv* env, jobject object, jfloat segment_start_x, jfloat segment_start_y,
 jfloat segment_end_x, jfloat segment_end_y, jfloat box_x_min, jfloat box_y_min,
 jfloat box_x_max, jfloat box_y_max) {
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  Rect rect =
      Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  return Intersects(segment, rect);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativeSegmentParallelogramIntersects)
(JNIEnv* env, jobject object, jfloat segment_start_x, jfloat segment_start_y,
 jfloat segment_end_x, jfloat segment_end_y, jfloat parallelogram_center_x,
 jfloat parallelogram_center_y, jfloat parallelogram_width,
 jfloat parallelogram_height, jfloat parallelogram_angle_radian,
 jfloat parallelogram_shear_factor) {
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_center_x, parallelogram_center_y},
      parallelogram_width, parallelogram_height,
      Angle::Radians(parallelogram_angle_radian), parallelogram_shear_factor);
  return Intersects(segment, quad);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeTriangleTriangleIntersects)
(JNIEnv* env, jobject object, jfloat triangle_1_p0_x, jfloat triangle_1_p0_y,
 jfloat triangle_1_p1_x, jfloat triangle_1_p1_y, jfloat triangle_1_p2_x,
 jfloat triangle_1_p2_y, jfloat triangle_2_p0_x, jfloat triangle_2_p0_y,
 jfloat triangle_2_p1_x, jfloat triangle_2_p1_y, jfloat triangle_2_p2_x,
 jfloat triangle_2_p2_y) {
  Triangle triangle1{{triangle_1_p0_x, triangle_1_p0_y},
                     {triangle_1_p1_x, triangle_1_p1_y},
                     {triangle_1_p2_x, triangle_1_p2_y}};
  Triangle triangle2{{triangle_2_p0_x, triangle_2_p0_y},
                     {triangle_2_p1_x, triangle_2_p1_y},
                     {triangle_2_p2_x, triangle_2_p2_y}};
  return Intersects(triangle1, triangle2);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeTriangleBoxIntersects)
(JNIEnv* env, jobject object, jfloat triangle_p0_x, jfloat triangle_p0_y,
 jfloat triangle_p1_x, jfloat triangle_p1_y, jfloat triangle_p2_x,
 jfloat triangle_p2_y, jfloat box_x_min, jfloat box_y_min, jfloat box_x_max,
 jfloat box_y_max) {
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  Rect rect =
      Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  return Intersects(triangle, rect);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativeTriangleParallelogramIntersects)
(JNIEnv* env, jobject object, jfloat triangle_p0_x, jfloat triangle_p0_y,
 jfloat triangle_p1_x, jfloat triangle_p1_y, jfloat triangle_p2_x,
 jfloat triangle_p2_y, jfloat parallelogram_center_x,
 jfloat parallelogram_center_y, jfloat parallelogram_width,
 jfloat parallelogram_height, jfloat parallelogram_angle_radian,
 jfloat parallelogram_shear_factor) {
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_center_x, parallelogram_center_y},
      parallelogram_width, parallelogram_height,
      Angle::Radians(parallelogram_angle_radian), parallelogram_shear_factor);
  return Intersects(triangle, quad);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeBoxBoxIntersects)
(JNIEnv* env, jobject object, jfloat box_1_x_min, jfloat box_1_y_min,
 jfloat box_1_x_max, jfloat box_1_y_max, jfloat box_2_x_min, jfloat box_2_y_min,
 jfloat box_2_x_max, jfloat box_2_y_max) {
  Rect rect1 = Rect::FromTwoPoints({box_1_x_min, box_1_y_min},
                                   {box_1_x_max, box_1_y_max});
  Rect rect2 = Rect::FromTwoPoints({box_2_x_min, box_2_y_min},
                                   {box_2_x_max, box_2_y_max});
  return Intersects(rect1, rect2);
}

JNI_METHOD(geometry, Intersection, jboolean, nativeBoxParallelogramIntersects)
(JNIEnv* env, jobject object, jfloat box_x_min, jfloat box_y_min,
 jfloat box_x_max, jfloat box_y_max, jfloat parallelogram_center_x,
 jfloat parallelogram_center_y, jfloat parallelogram_width,
 jfloat parallelogram_height, jfloat parallelogram_angle_radian,
 jfloat parallelogram_shear_factor) {
  Rect rect =
      Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_center_x, parallelogram_center_y},
      parallelogram_width, parallelogram_height,
      Angle::Radians(parallelogram_angle_radian), parallelogram_shear_factor);
  return Intersects(rect, quad);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativeParallelogramParallelogramIntersects)
(JNIEnv* env, jobject object, jfloat parallelogram_1_center_x,
 jfloat parallelogram_1_center_y, jfloat parallelogram_1_width,
 jfloat parallelogram_1_height, jfloat parallelogram_1_angle_in_radian,
 jfloat parallelogram_1_shear_factor, jfloat parallelogram_2_center_x,
 jfloat parallelogram_2_center_y, jfloat parallelogram_2_width,
 jfloat parallelogram_2_height, jfloat parallelogram_2_angle_in_radian,
 jfloat parallelogram_2_shear_factor) {
  Quad quad1 = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_1_center_x, parallelogram_1_center_y},
      parallelogram_1_width, parallelogram_1_height,
      Angle::Radians(parallelogram_1_angle_in_radian),
      parallelogram_1_shear_factor);
  Quad quad2 = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_2_center_x, parallelogram_2_center_y},
      parallelogram_2_width, parallelogram_2_height,
      Angle::Radians(parallelogram_2_angle_in_radian),
      parallelogram_2_shear_factor);
  return Intersects(quad1, quad2);
}

JNI_METHOD(geometry, Intersection, jboolean, nativePartitionedMeshVecIntersects)
(JNIEnv* env, jobject object, jlong partitioned_mesh_native_pointer,
 jfloat vec_x, jfloat vec_y, jfloat vec_to_partitionedMesh_transform_a,
 jfloat vec_to_partitionedMesh_transform_b,
 jfloat vec_to_partitionedMesh_transform_c,
 jfloat vec_to_partitionedMesh_transform_d,
 jfloat vec_to_partitionedMesh_transform_e,
 jfloat vec_to_partitionedMesh_transform_f) {
  Point point{vec_x, vec_y};

  AffineTransform transform(
      vec_to_partitionedMesh_transform_a, vec_to_partitionedMesh_transform_b,
      vec_to_partitionedMesh_transform_c, vec_to_partitionedMesh_transform_d,
      vec_to_partitionedMesh_transform_e, vec_to_partitionedMesh_transform_f);
  return Intersects(
      point, CastToPartitionedMesh(partitioned_mesh_native_pointer), transform);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativePartitionedMeshSegmentIntersects)
(JNIEnv* env, jobject object, jlong partitioned_mesh_native_pointer,
 jfloat segment_start_x, jfloat segment_start_y, jfloat segment_end_x,
 jfloat segment_end_y, jfloat segment_to_partitionedMesh_transform_a,
 jfloat segment_to_partitionedMesh_transform_b,
 jfloat segment_to_partitionedMesh_transform_c,
 jfloat segment_to_partitionedMesh_transform_d,
 jfloat segment_to_partitionedMesh_transform_e,
 jfloat segment_to_partitionedMesh_transform_f) {
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  AffineTransform transform(segment_to_partitionedMesh_transform_a,
                            segment_to_partitionedMesh_transform_b,
                            segment_to_partitionedMesh_transform_c,
                            segment_to_partitionedMesh_transform_d,
                            segment_to_partitionedMesh_transform_e,
                            segment_to_partitionedMesh_transform_f);
  return Intersects(segment,
                    CastToPartitionedMesh(partitioned_mesh_native_pointer),
                    transform);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativePartitionedMeshTriangleIntersects)
(JNIEnv* env, jobject object, jlong partitioned_mesh_native_pointer,
 jfloat triangle_p0_x, jfloat triangle_p0_y, jfloat triangle_p1_x,
 jfloat triangle_p1_y, jfloat triangle_p2_x, jfloat triangle_p2_y,
 jfloat triangle_to_partitionedMesh_transform_a,
 jfloat triangle_to_partitionedMesh_transform_b,
 jfloat triangle_to_partitionedMesh_transform_c,
 jfloat triangle_to_partitionedMesh_transform_d,
 jfloat triangle_to_partitionedMesh_transform_e,
 jfloat triangle_to_partitionedMesh_transform_f) {
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  AffineTransform transform(triangle_to_partitionedMesh_transform_a,
                            triangle_to_partitionedMesh_transform_b,
                            triangle_to_partitionedMesh_transform_c,
                            triangle_to_partitionedMesh_transform_d,
                            triangle_to_partitionedMesh_transform_e,
                            triangle_to_partitionedMesh_transform_f);
  return Intersects(triangle,
                    CastToPartitionedMesh(partitioned_mesh_native_pointer),
                    transform);
}

JNI_METHOD(geometry, Intersection, jboolean, nativePartitionedMeshBoxIntersects)
(JNIEnv* env, jobject object, jlong partitioned_mesh_native_pointer,
 jfloat box_x_min, jfloat box_y_min, jfloat box_x_max, jfloat box_y_max,
 jfloat box_to_partitionedMesh_transform_a,
 jfloat box_to_partitionedMesh_transform_b,
 jfloat box_to_partitionedMesh_transform_c,
 jfloat box_to_partitionedMesh_transform_d,
 jfloat box_to_partitionedMesh_transform_e,
 jfloat box_to_partitionedMesh_transform_f) {
  Rect rect =
      Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  AffineTransform transform(
      box_to_partitionedMesh_transform_a, box_to_partitionedMesh_transform_b,
      box_to_partitionedMesh_transform_c, box_to_partitionedMesh_transform_d,
      box_to_partitionedMesh_transform_e, box_to_partitionedMesh_transform_f);
  return Intersects(
      rect, CastToPartitionedMesh(partitioned_mesh_native_pointer), transform);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativePartitionedMeshParallelogramIntersects)
(JNIEnv* env, jobject object, jlong partitioned_mesh_native_pointer,
 jfloat parallelogram_center_x, jfloat parallelogram_center_y,
 jfloat parallelogram_width, jfloat parallelogram_height,
 jfloat parallelogram_angle_radian, jfloat parallelogram_shear_factor,
 jfloat parallelogram_to_partitionedMesh_transform_a,
 jfloat parallelogram_to_partitionedMesh_transform_b,
 jfloat parallelogram_to_partitionedMesh_transform_c,
 jfloat parallelogram_to_partitionedMesh_transform_d,
 jfloat parallelogram_to_partitionedMesh_transform_e,
 jfloat parallelogram_to_partitionedMesh_transform_f) {
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{parallelogram_center_x, parallelogram_center_y},
      parallelogram_width, parallelogram_height,
      Angle::Radians(parallelogram_angle_radian), parallelogram_shear_factor);
  AffineTransform transform(parallelogram_to_partitionedMesh_transform_a,
                            parallelogram_to_partitionedMesh_transform_b,
                            parallelogram_to_partitionedMesh_transform_c,
                            parallelogram_to_partitionedMesh_transform_d,
                            parallelogram_to_partitionedMesh_transform_e,
                            parallelogram_to_partitionedMesh_transform_f);
  return Intersects(
      quad, CastToPartitionedMesh(partitioned_mesh_native_pointer), transform);
}

JNI_METHOD(geometry, Intersection, jboolean,
           nativePartitionedMeshPartitionedMeshIntersects)
(JNIEnv* env, jobject object, jlong this_partitioned_mesh_native_pointer,
 jlong other_partitioned_mesh_native_pointer, jfloat this_to_common_transform_a,
 jfloat this_to_common_transform_b, jfloat this_to_common_transform_c,
 jfloat this_to_common_transform_d, jfloat this_to_common_transform_e,
 jfloat this_to_common_transform_f, jfloat other_to_common_transform_a,
 jfloat other_to_common_transform_b, jfloat other_to_common_transform_c,
 jfloat other_to_common_transform_d, jfloat other_to_common_transform_e,
 jfloat other_to_common_transform_f) {
  AffineTransform this_to_common_transform(
      this_to_common_transform_a, this_to_common_transform_b,
      this_to_common_transform_c, this_to_common_transform_d,
      this_to_common_transform_e, this_to_common_transform_f);
  AffineTransform other_to_common_transform(
      other_to_common_transform_a, other_to_common_transform_b,
      other_to_common_transform_c, other_to_common_transform_d,
      other_to_common_transform_e, other_to_common_transform_f);
  return Intersects(
      CastToPartitionedMesh(this_partitioned_mesh_native_pointer),
      this_to_common_transform,
      CastToPartitionedMesh(other_partitioned_mesh_native_pointer),
      other_to_common_transform);
}

}  // extern "C
