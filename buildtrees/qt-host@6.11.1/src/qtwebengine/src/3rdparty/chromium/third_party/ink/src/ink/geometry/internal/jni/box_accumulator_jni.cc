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

#include <jni.h>

#include "ink/geometry/angle.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/internal/jni/box_accumulator_jni_helper.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/jni/internal/jni_defines.h"

namespace {

using ::ink::Angle;
using ::ink::Envelope;
using ::ink::Point;
using ::ink::Quad;
using ::ink::Rect;
using ::ink::Segment;
using ::ink::Triangle;
using ::ink::jni::FillJBoxAccumulatorOrThrow;

Envelope BuildEnvelopeFromBounds(jboolean envelope_has_bounds,
                                 jfloat envelope_bounds_x_min,
                                 jfloat envelope_bounds_y_min,
                                 jfloat envelope_bounds_x_max,
                                 jfloat envelope_bounds_y_max) {
  return (envelope_has_bounds
              ? Envelope(Rect::FromTwoPoints(
                    {envelope_bounds_x_min, envelope_bounds_y_min},
                    {envelope_bounds_x_max, envelope_bounds_y_max}))
              : Envelope());
}

}  // namespace

extern "C" {

JNI_METHOD(geometry, BoxAccumulatorNative, void, addSegment)
(JNIEnv* env, jobject object, jboolean envelope_has_bounds,
 jfloat envelope_bounds_x_min, jfloat envelope_bounds_y_min,
 jfloat envelope_bounds_x_max, jfloat envelope_bounds_y_max,
 jfloat segment_start_x, jfloat segment_start_y, jfloat segment_end_x,
 jfloat segment_end_y, jobject output) {
  Envelope envelope = BuildEnvelopeFromBounds(
      envelope_has_bounds, envelope_bounds_x_min, envelope_bounds_y_min,
      envelope_bounds_x_max, envelope_bounds_y_max);
  Segment segment{{segment_start_x, segment_start_y},
                  {segment_end_x, segment_end_y}};
  envelope.Add(segment);
  FillJBoxAccumulatorOrThrow(env, envelope, output);
}

JNI_METHOD(geometry, BoxAccumulatorNative, void, addTriangle)
(JNIEnv* env, jobject object, jboolean envelope_has_bounds,
 jfloat envelope_bounds_x_min, jfloat envelope_bounds_y_min,
 jfloat envelope_bounds_x_max, jfloat envelope_bounds_y_max,
 jfloat triangle_p0_x, jfloat triangle_p0_y, jfloat triangle_p1_x,
 jfloat triangle_p1_y, jfloat triangle_p2_x, jfloat triangle_p2_y,
 jobject output) {
  Envelope envelope = BuildEnvelopeFromBounds(
      envelope_has_bounds, envelope_bounds_x_min, envelope_bounds_y_min,
      envelope_bounds_x_max, envelope_bounds_y_max);
  Triangle triangle{{triangle_p0_x, triangle_p0_y},
                    {triangle_p1_x, triangle_p1_y},
                    {triangle_p2_x, triangle_p2_y}};
  envelope.Add(triangle);
  FillJBoxAccumulatorOrThrow(env, envelope, output);
}

JNI_METHOD(geometry, BoxAccumulatorNative, void, addParallelogram)
(JNIEnv* env, jobject object, jboolean envelope_has_bounds,
 jfloat envelope_bounds_x_min, jfloat envelope_bounds_y_min,
 jfloat envelope_bounds_x_max, jfloat envelope_bounds_y_max,
 jfloat quad_center_x, jfloat quad_center_y, jfloat quad_width,
 jfloat quad_height, jfloat quad_angle_radian, jfloat quad_shear_factor,
 jobject output) {
  Envelope envelope = BuildEnvelopeFromBounds(
      envelope_has_bounds, envelope_bounds_x_min, envelope_bounds_y_min,
      envelope_bounds_x_max, envelope_bounds_y_max);
  Quad quad = Quad::FromCenterDimensionsRotationAndShear(
      Point{quad_center_x, quad_center_y}, quad_width, quad_height,
      Angle::Radians(quad_angle_radian), quad_shear_factor);
  envelope.Add(quad);
  FillJBoxAccumulatorOrThrow(env, envelope, output);
}

JNI_METHOD(geometry, BoxAccumulatorNative, void, addPoint)
(JNIEnv* env, jobject object, jboolean envelope_has_bounds,
 jfloat envelope_bounds_x_min, jfloat envelope_bounds_y_min,
 jfloat envelope_bounds_x_max, jfloat envelope_bounds_y_max, jfloat point_x,
 jfloat point_y, jobject output) {
  Envelope envelope = BuildEnvelopeFromBounds(
      envelope_has_bounds, envelope_bounds_x_min, envelope_bounds_y_min,
      envelope_bounds_x_max, envelope_bounds_y_max);
  Point point = Point{point_x, point_y};
  envelope.Add(point);
  FillJBoxAccumulatorOrThrow(env, envelope, output);
}

JNI_METHOD(geometry, BoxAccumulatorNative, void, addOptionalBox)
(JNIEnv* env, jobject object, jboolean envelope_has_bounds,
 jfloat envelope_bounds_x_min, jfloat envelope_bounds_y_min,
 jfloat envelope_bounds_x_max, jfloat envelope_bounds_y_max,
 jboolean box_has_bounds, jfloat box_x_min, jfloat box_y_min, jfloat box_x_max,
 jfloat box_y_max, jobject output) {
  if (!box_has_bounds) return;
  Envelope envelope = BuildEnvelopeFromBounds(
      envelope_has_bounds, envelope_bounds_x_min, envelope_bounds_y_min,
      envelope_bounds_x_max, envelope_bounds_y_max);
  Rect rect =
      ink::Rect::FromTwoPoints({box_x_min, box_y_min}, {box_x_max, box_y_max});
  envelope.Add(rect);
  FillJBoxAccumulatorOrThrow(env, envelope, output);
}
}
