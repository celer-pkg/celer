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

#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/internal/jni/brush_jni_helper.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_string_util.h"
#include "ink/jni/internal/jni_throw_util.h"

namespace {

using ink::Angle;
using ink::BrushPaint;
using ink::Vec;
using ink::brush_internal::ValidateBrushPaint;
using ink::brush_internal::ValidateBrushPaintTextureLayer;
using ink::jni::CastToBrushPaint;
using ink::jni::CastToTextureLayer;
using ink::jni::DeleteNativeBrushPaint;
using ink::jni::DeleteNativeTextureLayer;
using ink::jni::JStringToStdString;
using ink::jni::NewNativeBrushPaint;
using ink::jni::NewNativeTextureLayer;
using ink::jni::ThrowExceptionFromStatus;

BrushPaint::TextureSizeUnit JIntToSizeUnit(jint val) {
  return static_cast<BrushPaint::TextureSizeUnit>(val);
}

BrushPaint::TextureOrigin JIntToOrigin(jint val) {
  return static_cast<BrushPaint::TextureOrigin>(val);
}

BrushPaint::TextureMapping JIntToMapping(jint val) {
  return static_cast<BrushPaint::TextureMapping>(val);
}

BrushPaint::TextureWrap JIntToWrap(jint val) {
  return static_cast<BrushPaint::TextureWrap>(val);
}

BrushPaint::BlendMode JIntToBlendMode(jint val) {
  return static_cast<BrushPaint::BlendMode>(val);
}

}  // namespace

extern "C" {

// Construct a native BrushPaint and return a pointer to it as a long.
JNI_METHOD(brush, BrushPaintNative, jlong, create)
(JNIEnv* env, jobject thiz, jlongArray texture_layer_native_pointers_array) {
  std::vector<BrushPaint::TextureLayer> texture_layers;
  ABSL_CHECK(texture_layer_native_pointers_array != nullptr);
  const jsize texture_layers_count =
      env->GetArrayLength(texture_layer_native_pointers_array);
  texture_layers.reserve(texture_layers_count);
  jlong* texture_layer_native_pointers =
      env->GetLongArrayElements(texture_layer_native_pointers_array, nullptr);
  ABSL_CHECK(texture_layer_native_pointers != nullptr);
  for (int i = 0; i < texture_layers_count; ++i) {
    texture_layers.push_back(
        CastToTextureLayer(texture_layer_native_pointers[i]));
  }
  env->ReleaseLongArrayElements(
      texture_layer_native_pointers_array, texture_layer_native_pointers,
      // No need to copy back the array, which is not modified.
      JNI_ABORT);
  BrushPaint brush_paint{.texture_layers = std::move(texture_layers)};
  if (absl::Status status = ValidateBrushPaint(brush_paint); !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  return NewNativeBrushPaint(std::move(brush_paint));
}

JNI_METHOD(brush, BrushPaintNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeBrushPaint(native_pointer);
}

JNI_METHOD(brush, BrushPaintNative, jint, getTextureLayerCount)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint& brush_paint = CastToBrushPaint(native_pointer);
  return brush_paint.texture_layers.size();
}

// Returns a pointer to a newly heap-allocated copy of the texture layer at the
// given instance on this BrushPaint.
JNI_METHOD(brush, BrushPaintNative, jlong, newCopyOfTextureLayer)
(JNIEnv* env, jobject thiz, jlong native_pointer, jint index) {
  const BrushPaint& brush_paint = CastToBrushPaint(native_pointer);
  return NewNativeTextureLayer(brush_paint.texture_layers[index]);
}

// ************ Native Implementation of BrushPaint TextureLayer ************

// Constructs a native BrushPaint TextureLayer and returns a pointer to it as a
// long.
JNI_METHOD(brush, TextureLayerNative, jlong, create)
(JNIEnv* env, jobject thiz, jstring client_texture_id, jfloat size_x,
 jfloat size_y, jfloat offset_x, jfloat offset_y, jfloat rotation_in_radians,
 jfloat opacity, jint animation_frames, jint animation_rows,
 jint animation_columns, jint size_unit, jint origin, jint mapping, jint wrap_x,
 jint wrap_y, jint blend_mode) {
  BrushPaint::TextureLayer texture_layer{
      .client_texture_id = JStringToStdString(env, client_texture_id),
      .mapping = JIntToMapping(mapping),
      .origin = JIntToOrigin(origin),
      .size_unit = JIntToSizeUnit(size_unit),
      .wrap_x = JIntToWrap(wrap_x),
      .wrap_y = JIntToWrap(wrap_y),
      .size = Vec{size_x, size_y},
      .offset = Vec{offset_x, offset_y},
      .rotation = Angle::Radians(rotation_in_radians),
      .opacity = opacity,
      .animation_frames = animation_frames,
      .animation_rows = animation_rows,
      .animation_columns = animation_columns,
      .blend_mode = JIntToBlendMode(blend_mode),
  };
  if (absl::Status status = ValidateBrushPaintTextureLayer(texture_layer);
      !status.ok()) {
    ThrowExceptionFromStatus(env, status);
    return 0;
  }
  return NewNativeTextureLayer(std::move(texture_layer));
}

JNI_METHOD(brush, TextureLayerNative, void, free)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  DeleteNativeTextureLayer(native_pointer);
}

JNI_METHOD(brush, TextureLayerNative, jstring, getClientTextureId)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return env->NewStringUTF(texture_layer.client_texture_id.c_str());
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getSizeX)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.size.x;
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getSizeY)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.size.y;
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getOffsetX)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.offset.x;
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getOffsetY)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.offset.y;
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getRotationInRadians)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.rotation.ValueInRadians();
}

JNI_METHOD(brush, TextureLayerNative, jfloat, getOpacity)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return texture_layer.opacity;
}

JNI_METHOD(brush, TextureLayerNative, jint, getAnimationFrames)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToTextureLayer(native_pointer).animation_frames;
}

JNI_METHOD(brush, TextureLayerNative, jint, getAnimationRows)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToTextureLayer(native_pointer).animation_rows;
}

JNI_METHOD(brush, TextureLayerNative, jint, getAnimationColumns)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  return CastToTextureLayer(native_pointer).animation_columns;
}

JNI_METHOD(brush, TextureLayerNative, jint, getSizeUnitInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.size_unit);
}

JNI_METHOD(brush, TextureLayerNative, jint, getOriginInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.origin);
}

JNI_METHOD(brush, TextureLayerNative, jint, getMappingInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.mapping);
}

JNI_METHOD(brush, TextureLayerNative, jint, getWrapXInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.wrap_x);
}

JNI_METHOD(brush, TextureLayerNative, jint, getWrapYInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.wrap_y);
}

JNI_METHOD(brush, TextureLayerNative, jint, getBlendModeInt)
(JNIEnv* env, jobject thiz, jlong native_pointer) {
  const BrushPaint::TextureLayer& texture_layer =
      CastToTextureLayer(native_pointer);
  return static_cast<jint>(texture_layer.blend_mode);
}

}  // extern "C"
