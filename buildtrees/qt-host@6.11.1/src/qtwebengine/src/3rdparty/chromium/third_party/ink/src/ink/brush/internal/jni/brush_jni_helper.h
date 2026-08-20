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

#ifndef INK_STROKES_BRUSH_INTERNAL_JNI_BRUSH_JNI_HELPER_H_
#define INK_STROKES_BRUSH_INTERNAL_JNI_BRUSH_JNI_HELPER_H_

#include <jni.h>

#include "absl/log/absl_check.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"

namespace ink::jni {

// Creates a new stack-allocated copy of the given Brush and returns a pointer
// to it as a jlong, suitable for wrapping in a Kotlin Brush.
inline jlong NewNativeBrush(const Brush& brush) {
  return reinterpret_cast<jlong>(new Brush(brush));
}

// Casts a Kotlin Brush.nativePointer to a C++ Brush. The returned
// Brush is a const ref as the Kotlin Brush is immutable.
inline const Brush& CastToBrush(jlong brush_native_pointer) {
  ABSL_CHECK_NE(brush_native_pointer, 0);
  return *reinterpret_cast<Brush*>(brush_native_pointer);
}

// Frees a Kotlin Brush.nativePointer.
inline void DeleteNativeBrush(jlong brush_native_pointer) {
  if (brush_native_pointer == 0) return;
  delete reinterpret_cast<Brush*>(brush_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushFamily and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin BrushFamily.
inline jlong NewNativeBrushFamily(const BrushFamily& brush_family) {
  return reinterpret_cast<jlong>(new BrushFamily(brush_family));
}

// Casts a Kotlin BrushFamily.nativePointer to a C++ BrushFamily. The
// returned BrushFamily is a const ref as the Kotlin BrushFamily is immutable.
inline const BrushFamily& CastToBrushFamily(jlong brush_family_native_pointer) {
  ABSL_CHECK_NE(brush_family_native_pointer, 0);
  return *reinterpret_cast<BrushFamily*>(brush_family_native_pointer);
}

// Frees a Kotlin BrushFamily.nativePointer.
inline void DeleteNativeBrushFamily(jlong brush_family_native_pointer) {
  if (brush_family_native_pointer == 0) return;
  delete reinterpret_cast<BrushFamily*>(brush_family_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushCoat and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin BrushCoat.
inline jlong NewNativeBrushCoat(const BrushCoat& brush_coat) {
  return reinterpret_cast<jlong>(new BrushCoat(brush_coat));
}

// Casts a Kotlin BrushCoat.nativePointer to a C++ BrushCoat. The returned
// BrushCoat is a const ref as the Kotlin BrushCoat is immutable.
inline const BrushCoat& CastToBrushCoat(jlong brush_coat_native_pointer) {
  ABSL_CHECK_NE(brush_coat_native_pointer, 0);
  return *reinterpret_cast<BrushCoat*>(brush_coat_native_pointer);
}

// Frees a Kotlin BrushCoat.nativePointer.
inline void DeleteNativeBrushCoat(jlong brush_coat_native_pointer) {
  if (brush_coat_native_pointer == 0) return;
  delete reinterpret_cast<BrushCoat*>(brush_coat_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushPaint and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin BrushPaint.
inline jlong NewNativeBrushPaint(const BrushPaint& brush_paint) {
  return reinterpret_cast<jlong>(new BrushPaint(brush_paint));
}

// Casts a Kotlin BrushPaint.nativePointer to a C++ BrushPaint. The returned
// BrushPaint is a const ref as the Kotlin BrushPaint is immutable.
inline const BrushPaint& CastToBrushPaint(jlong brush_paint_native_pointer) {
  ABSL_CHECK_NE(brush_paint_native_pointer, 0);
  return *reinterpret_cast<BrushPaint*>(brush_paint_native_pointer);
}

// Frees a Kotlin BrushPaint.nativePointer.
inline void DeleteNativeBrushPaint(jlong brush_paint_native_pointer) {
  if (brush_paint_native_pointer == 0) return;
  delete reinterpret_cast<BrushPaint*>(brush_paint_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushPaint::TextureLayer and
// returns a pointer to it as a jlong, suitable for wrapping in a Kotlin
// BrushPaint.TextureLayer.
inline jlong NewNativeTextureLayer(
    const BrushPaint::TextureLayer& texture_layer) {
  return reinterpret_cast<jlong>(new BrushPaint::TextureLayer(texture_layer));
}

// Casts a Kotlin BrushBehavior.nativePointer to a C++
// BrushPaint::TextureLayer. The returned TextureLayer is a const ref as the
// Kotlin BrushPaint is immutable.
inline const BrushPaint::TextureLayer& CastToTextureLayer(
    jlong texture_layer_native_pointer) {
  ABSL_CHECK_NE(texture_layer_native_pointer, 0);
  return *reinterpret_cast<BrushPaint::TextureLayer*>(
      texture_layer_native_pointer);
}

// Frees a Kotlin BrushPaint::TextureLayer.nativePointer.
inline void DeleteNativeTextureLayer(jlong texture_layer_native_pointer) {
  if (texture_layer_native_pointer == 0) return;
  delete reinterpret_cast<BrushPaint::TextureLayer*>(
      texture_layer_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushTip and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin BrushTip.
inline jlong NewNativeBrushTip(const BrushTip& brush_tip) {
  return reinterpret_cast<jlong>(new BrushTip(brush_tip));
}

// Casts a Kotlin BrushTip.nativePointer to a C++ BrushTip. The returned
// BrushTip is a const ref as the Kotlin BrushTip is immutable.
inline const BrushTip& CastToBrushTip(jlong brush_tip_native_pointer) {
  ABSL_CHECK_NE(brush_tip_native_pointer, 0);
  return *reinterpret_cast<BrushTip*>(brush_tip_native_pointer);
}

// Frees a Kotlin BrushTip.nativePointer.
inline void DeleteNativeBrushTip(jlong brush_tip_native_pointer) {
  if (brush_tip_native_pointer == 0) return;
  delete reinterpret_cast<BrushTip*>(brush_tip_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushBehavior and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin BrushBehavior.
inline jlong NewNativeBrushBehavior(const BrushBehavior& brush_behavior) {
  return reinterpret_cast<jlong>(new BrushBehavior(brush_behavior));
}

// Casts a Kotlin BrushBehavior.nativePointer to a C++ BrushBehavior. The
// returned BrushBehavior is a const ref as the Kotlin BrushBehavior is
// immutable.
inline const BrushBehavior& CastToBrushBehavior(
    jlong brush_behavior_native_pointer) {
  ABSL_CHECK_NE(brush_behavior_native_pointer, 0);
  return *reinterpret_cast<BrushBehavior*>(brush_behavior_native_pointer);
}

// Frees a Kotlin BrushBehavior.nativePointer.
inline void DeleteNativeBrushBehavior(jlong brush_behavior_native_pointer) {
  if (brush_behavior_native_pointer == 0) return;
  delete reinterpret_cast<BrushBehavior*>(brush_behavior_native_pointer);
}

// Creates a new stack-allocated copy of the given BrushBehavior::Node and
// returns a pointer to it as a jlong, suitable for wrapping in a Kotlin
// BrushBehavior.Node.
inline jlong NewNativeBrushBehaviorNode(const BrushBehavior::Node& node) {
  return reinterpret_cast<jlong>(new BrushBehavior::Node(node));
}

// Casts a Kotlin BrushBehavior.Node.nativePointer to a C++
// BrushBehavior::Node. The returned Node is a const ref as the Kotlin
// BrushBehavior is immutable.
inline const BrushBehavior::Node& CastToBrushBehaviorNode(
    jlong node_native_pointer) {
  ABSL_CHECK_NE(node_native_pointer, 0);
  return *reinterpret_cast<BrushBehavior::Node*>(node_native_pointer);
}

// Frees a Kotlin BrushBehavior::Node.nativePointer.
inline void DeleteNativeBrushBehaviorNode(jlong node_native_pointer) {
  if (node_native_pointer == 0) return;
  delete reinterpret_cast<BrushBehavior::Node*>(node_native_pointer);
}

// Creates a new stack-allocated copy of the given EasingFunction and returns a
// pointer to it as a jlong, suitable for wrapping in a Kotlin EasingFunction.
inline jlong NewNativeEasingFunction(const EasingFunction& easing_function) {
  return reinterpret_cast<jlong>(new EasingFunction(easing_function));
}

// Casts a Kotlin EasingFunction.nativePointer to a C++ EasingFunction. The
// returned EasingFunction is a const ref as the Kotlin EasingFunction is
// immutable.
inline const EasingFunction& CastToEasingFunction(
    jlong easing_function_native_pointer) {
  ABSL_CHECK_NE(easing_function_native_pointer, 0);
  return *reinterpret_cast<EasingFunction*>(easing_function_native_pointer);
}

// Frees a Kotlin EasingFunction.nativePointer.
inline void DeleteNativeEasingFunction(jlong easing_function_native_pointer) {
  if (easing_function_native_pointer == 0) return;
  delete reinterpret_cast<EasingFunction*>(easing_function_native_pointer);
}

}  // namespace ink::jni

#endif  // INK_STROKES_BRUSH_INTERNAL_JNI_BRUSH_JNI_HELPER_H_
