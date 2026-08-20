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

#ifndef INK_RENDERING_SKIA_NATIVE_SKIA_RENDERER_H_
#define INK_RENDERING_SKIA_NATIVE_SKIA_RENDERER_H_

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "absl/base/nullability.h"
#include "absl/container/inlined_vector.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/rendering/skia/native/internal/mesh_drawable.h"
#include "ink/rendering/skia/native/internal/mesh_specification_cache.h"
#include "ink/rendering/skia/native/internal/path_drawable.h"
#include "ink/rendering/skia/native/internal/shader_cache.h"
#include "ink/rendering/skia/native/texture_bitmap_store.h"
#include "ink/strokes/in_progress_stroke.h"
#include "ink/strokes/stroke.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"

namespace ink {

// A helper renderer for drawing Ink objects into an `SkCanvas`.
//
// This type is thread-compatible, but *not* thread-safe: all non-const access
// to a renderer object must be externally synchronized.
//
// The renderer is intended for hardware accelerated drawing using one of Skia's
// GPU backends. This requires every function that takes in a `GrDirectContext*`
// parameter to be called from the thread on which the context is active. The
// renderer has a fallback path for uses where a `GrDirectContext` is
// unavailable; see the bottom of this comment for more details.
//
// Drawing can be performed in one of two ways:
//
// Option 1:
//   Call the appropriate `SkiaRenderer::Draw()` function. This must be done
//   from the `GrDirectContext` thread if using one of Skia's GPU backends.
//
// Option 2:
//   a) Call the appropriate overload of `SkiaRenderer::CreateDrawable()` to
//      explicitly create a `Drawable` (from the `GrDirectContext` thread if
//      applicable).
//   b) Use the API of the returned `Drawable` to explictly update its
//      properties and draw it into an `SkCanvas` without needing to stay on a
//      particular thread.
//
// Depending on desired usage, option 1 or option 2 can be more convenient.
// Option 1 does not require manually tracking changes to drawn objects, but
// does require all drawing related operations to be done on a particular
// thread.
//
// The renderer supports a feature-limited CPU rendering fallback. Matching
// Skia's API patterns, CPU rendering is triggered by calling `Draw()` or
// `CreateDrawable()` with a null `GrDirectContext`. Note that CPU rasterized
// strokes are drawn as filled paths, and this results in some visual
// differences and limitations:
//   * `BrushBehavior`s targeting color and opacity are ignored.
//   * Individual strokes with a partially transparent brush color do not
//     accumulate opacity when overlapping themselves.
class SkiaRenderer {
 public:
  class Drawable;

  explicit SkiaRenderer(absl_nullable std::shared_ptr<TextureBitmapStore>
                            texture_provider = nullptr);

  SkiaRenderer(const SkiaRenderer&) = delete;
  SkiaRenderer(SkiaRenderer&&) = default;
  SkiaRenderer& operator=(const SkiaRenderer&) = delete;
  SkiaRenderer& operator=(SkiaRenderer&&) = default;
  ~SkiaRenderer() = default;

  // The following overloads draw a `stroke` with the given `object_to_canvas`
  // transform into the `canvas`.
  //
  // NOTE: This function calls `canvas.setMatrix()`, overwriting any current
  // matrix state. Callers who wish to make use of the `SkCanvas` matrix state
  // should wrap calls to this function with calls to `canvas.save()` and
  // `canvas.restore()`.
  //
  // TODO: b/286547863 - These functions currently generate temporary drawable
  // data on every call. This data will get cached on the `stroke` inside a
  // Skia implementation of a `RenderCache`, which would get initially attached
  // to the stroke by calling a separate renderer member function.
  absl::Status Draw(GrDirectContext* context, const InProgressStroke& stroke,
                    const AffineTransform& object_to_canvas, SkCanvas& canvas);
  absl::Status Draw(GrDirectContext* context, const Stroke& stroke,
                    const AffineTransform& object_to_canvas, SkCanvas& canvas);

  // Return a new `Drawable` created from an `InProgressStroke`.
  //
  // The returned drawable will have its transform set to `object_to_canvas` and
  // its brush-color set to that of the stroke. If `stroke` has not been
  // started, this function returns an "empty" drawable.
  //
  // TODO: b/284117747 - This function will be able to return an
  // invalid-argument error if rendering would fail due to an unsupported
  // `Brush`.
  //
  // NOTE: the drawable will not automatically track changes to the `stroke` and
  // must be manually recreated and/or updated.
  absl::StatusOr<Drawable> CreateDrawable(
      GrDirectContext* context, const InProgressStroke& stroke,
      const AffineTransform& object_to_canvas);

  // Returns a new `Drawable` created from the `Stroke`.
  //
  // The returned drawable will have its transform set to `object_to_canvas` and
  // its brush-color set to that of the stroke. If the `stroke` is empty, this
  // function returns an "empty" drawable.
  //
  // An invalid-argument error is returned if rendering would fail due to an
  // unsupported `stroke`.
  // TODO: b/284117747 - This function will be able to return an
  // invalid-argument error if rendering would fail due to an unsupported
  // `Brush`.
  //
  // NOTE: the drawable will not automatically track changes to the `stroke` and
  // must be manually recreated and/or updated.
  absl::StatusOr<Drawable> CreateDrawable(
      GrDirectContext* context, const Stroke& stroke,
      const AffineTransform& object_to_canvas);

  // TODO: b/284117747 - Add functions to "update" a `Drawable`.

 private:
  absl_nullable std::shared_ptr<TextureBitmapStore> texture_provider_;
  skia_native_internal::ShaderCache shader_cache_;
  skia_native_internal::MeshSpecificationCache specification_cache_;

  // Buffer of 16-bit integers used during index buffer creation when the
  // incoming mesh holds 32-bit indices.
  // TODO: b/294561921 - Remove once `InProgressStroke` uses 16-bit indices.
  std::vector<uint16_t> temporary_indices_;
};

// Type storing all information needed for drawing an Ink object into an
// `SkCanvas`.
//
// Objects of this type are returned by a `SkiaRenderer` on the
// `GrDirectContext` thread, and can subsequently be be used to draw into an
// `SkCanvas` on the thread of choice. This type is thread-compatible, but *not*
// thread-safe. All non-const access to a `Drawable` object must be externally
// synchronized.
class SkiaRenderer::Drawable {
 public:
  Drawable() = default;
  Drawable(const Drawable&) = default;
  Drawable(Drawable&&) = default;
  Drawable& operator=(const Drawable&) = default;
  Drawable& operator=(Drawable&&) = default;
  ~Drawable() = default;

  // Returns the complete transform from "object" coordinates to canvas
  // coordinates.
  const AffineTransform& ObjectToCanvas() const;

  // Sets the value of the complete transform from object coordinates to canvas
  // coordinates.
  void SetObjectToCanvas(const AffineTransform& object_to_canvas);

  // Draws the mesh-drawable into the provided `canvas` with the currently set
  // object-to-canvas transform.
  //
  // NOTE: This function calls `canvas.setMatrix()`, overwriting any current
  // matrix state. Callers who wish to make use of the `SkCanvas` matrix state
  // should wrap calls to this function with calls to `canvas.save()` and
  // `canvas.restore()`.
  void Draw(SkCanvas& canvas) const;

  // Returns true if the drawable has a brush-color property.
  //
  // All drawables created from an `InProgressStroke` or `Stroke` will have a
  // brush-color.
  bool HasBrushColor() const;

  // Sets the value of the brush-color property.
  //
  // CHECK-fails if the drawable does not have the property.
  void SetBrushColor(const Color& color);

  void SetImageFilter(sk_sp<SkImageFilter> image_filter);

 private:
  friend SkiaRenderer;

  // Union of internal implementation types for drawables.
  //
  // A `variant` is used instead of inheritance to save extra allocations /
  // indirections since a drawable can hold multiple meshes or paths.
  using Implementation = std::variant<skia_native_internal::MeshDrawable,
                                      skia_native_internal::PathDrawable>;

  Drawable(const AffineTransform& object_to_canvas,
           absl::InlinedVector<Implementation, 1> drawable_impls);

  AffineTransform object_to_canvas_;
  absl::InlinedVector<Implementation, 1> drawable_implementations_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline const AffineTransform& SkiaRenderer::Drawable::ObjectToCanvas() const {
  return object_to_canvas_;
}

}  // namespace ink

#endif  // INK_RENDERING_SKIA_NATIVE_SKIA_RENDERER_H_
