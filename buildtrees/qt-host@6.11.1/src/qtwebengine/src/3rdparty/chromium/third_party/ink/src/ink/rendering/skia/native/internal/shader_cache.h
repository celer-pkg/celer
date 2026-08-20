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

#ifndef INK_RENDERING_SKIA_NATIVE_INTERNAL_SHADER_CACHE_H_
#define INK_RENDERING_SKIA_NATIVE_INTERNAL_SHADER_CACHE_H_

#include <string>
#include <utility>

#include "absl/base/nullability.h"
#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/rendering/skia/native/texture_bitmap_store.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"

namespace ink::skia_native_internal {

class ShaderCache {
 public:
  // If non-null, `texture_provider` must outlive the `ShaderCache`.
  explicit ShaderCache(const TextureBitmapStore* absl_nullable provider);

  ShaderCache(const ShaderCache&) = delete;
  ShaderCache(ShaderCache&&) = default;
  ShaderCache& operator=(const ShaderCache&) = delete;
  ShaderCache& operator=(ShaderCache&&) = default;
  ~ShaderCache() = default;

  // Returns the `SkBlender` object (which may be nullptr) that should be used
  // for the given `BrushPaint`.
  sk_sp<SkBlender> GetBlenderForPaint(const BrushPaint& paint);

  // Returns the `SkShader` object (which may be nullptr) that should be used
  // for the given `BrushPaint` and stroke properties.
  absl::StatusOr<sk_sp<SkShader>> GetShaderForPaint(
      const BrushPaint& paint, float brush_size,
      const StrokeInputBatch& inputs);

 private:
  // Returns the texture shader that should be used for the given `TextureLayer`
  // and stroke properties, including the full local matrix needed.
  absl::StatusOr<sk_sp<SkShader>> GetShaderForLayer(
      const BrushPaint::TextureLayer& layer, float brush_size,
      const StrokeInputBatch& inputs);

  // Helper method for `GetShaderForLayer`. Creates a new `SkShader` object for
  // the given `TextureLayer`, with a local matrix consisting of the portion of
  // the transform that is inherent to the `TextureLayer` and doesn't depend on
  // the properties of a particular stroke (and thus can be cached).
  absl::StatusOr<sk_sp<SkShader>> CreateBaseShaderForLayer(
      const BrushPaint::TextureLayer& layer);

  // Returns an `SkImage` object with the bitmap data for the given texture
  // ID. The `SkImage` object will be cached, so that the same instance is
  // returned for the same texture ID.
  absl::StatusOr<sk_sp<SkImage>> GetImageForTexture(
      absl::string_view texture_id);

  // Returns the `SkColorSpace` corresponding to the given Ink `ColorSpace` and
  // `Color::Format`. The `SkColorSpace` object will be cached, so that the same
  // instance is returned for the same parameters.
  sk_sp<SkColorSpace> GetColorSpace(ColorSpace color_space,
                                    Color::Format format);

  const TextureBitmapStore* absl_nullable texture_provider_ = nullptr;
  absl::flat_hash_map<std::pair<ColorSpace, Color::Format>, sk_sp<SkColorSpace>>
      color_spaces_;
  absl::flat_hash_map<std::string, sk_sp<SkImage>> texture_images_;
  absl::flat_hash_map<BrushPaint::TextureLayer, sk_sp<SkShader>> layer_shaders_;
};

}  // namespace ink::skia_native_internal

#endif  // INK_RENDERING_SKIA_NATIVE_INTERNAL_SHADER_CACHE_H_
