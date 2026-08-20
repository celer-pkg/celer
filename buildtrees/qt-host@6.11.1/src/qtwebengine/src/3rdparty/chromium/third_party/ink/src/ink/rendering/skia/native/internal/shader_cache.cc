#include "ink/rendering/skia/native/internal/shader_cache.h"

#include <utility>

#include "absl/base/nullability.h"
#include "absl/log/absl_log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/geometry/affine_transform.h"
#include "ink/rendering/skia/native/texture_bitmap_store.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"

namespace ink::skia_native_internal {
namespace {

SkTileMode ToSkTileMode(BrushPaint::TextureWrap wrap) {
  switch (wrap) {
    case BrushPaint::TextureWrap::kRepeat:
      return SkTileMode::kRepeat;
    case BrushPaint::TextureWrap::kMirror:
      return SkTileMode::kMirror;
    case BrushPaint::TextureWrap::kClamp:
      return SkTileMode::kClamp;
  }
  ABSL_LOG(FATAL) << "invalid `BrushPaint::TextureWrap` value: "
                  << static_cast<int>(wrap);
}

SkBlendMode ToSkBlendMode(BrushPaint::BlendMode blend_mode) {
  switch (blend_mode) {
    case BrushPaint::BlendMode::kModulate:
      return SkBlendMode::kModulate;
    case BrushPaint::BlendMode::kDstIn:
      return SkBlendMode::kDstIn;
    case BrushPaint::BlendMode::kDstOut:
      return SkBlendMode::kDstOut;
    case BrushPaint::BlendMode::kSrcAtop:
      return SkBlendMode::kSrcATop;
    case BrushPaint::BlendMode::kSrcIn:
      return SkBlendMode::kSrcIn;
    case BrushPaint::BlendMode::kSrcOver:
      return SkBlendMode::kSrcOver;
    case BrushPaint::BlendMode::kDstOver:
      return SkBlendMode::kDstOver;
    case BrushPaint::BlendMode::kSrc:
      return SkBlendMode::kSrc;
    case BrushPaint::BlendMode::kDst:
      return SkBlendMode::kDst;
    case BrushPaint::BlendMode::kSrcOut:
      return SkBlendMode::kSrcOut;
    case BrushPaint::BlendMode::kDstAtop:
      return SkBlendMode::kDstATop;
    case BrushPaint::BlendMode::kXor:
      return SkBlendMode::kXor;
  }
  ABSL_LOG(FATAL) << "invalid `BrushPaint::BlendMode` value: "
                  << static_cast<int>(blend_mode);
}

sk_sp<SkColorSpace> CreateColorSpace(ColorSpace color_space,
                                     Color::Format format) {
  bool is_linear = format != Color::Format::kGammaEncoded;
  switch (color_space) {
    case ColorSpace::kSrgb:
      return is_linear ? SkColorSpace::MakeSRGBLinear()
                       : SkColorSpace::MakeSRGB();
    case ColorSpace::kDisplayP3:
      return SkColorSpace::MakeRGB(
          is_linear ? SkNamedTransferFn::kLinear : SkNamedTransferFn::kSRGB,
          SkNamedGamut::kDisplayP3);
  }
  ABSL_LOG(FATAL) << "invalid `ColorSpace` value: "
                  << static_cast<int>(color_space);
}

SkMatrix ToSkMatrix(const AffineTransform& transform) {
  return SkMatrix::MakeAll(transform.A(), transform.B(), transform.C(),  //
                           transform.D(), transform.E(), transform.F(),  //
                           0, 0, 1);
}

// Computes the transform for a `TextureLayer` from texel space to size-unit
// space. This transform depends only on the `TextureLayer` and not on any
// properties of the particular stroke, so it can be computed up front.
//
// TODO: b/368283812 - We may need to refactor this once we implement texture
// jitter, since then more of this transform will differ from stroke to stroke.
AffineTransform ComputeTexelToSizeUnitTransform(
    const BrushPaint::TextureLayer& layer, int bitmap_width,
    int bitmap_height) {
  // Skia starts us in texel space (where each texel is a unit square). From
  // there, we first transform to UV space (where the texture image is a unit
  // square).
  AffineTransform texel_to_uv =
      AffineTransform::Scale(1.0f / static_cast<float>(bitmap_width),
                             1.0f / static_cast<float>(bitmap_height));
  // The texture offset is specified as fractions of the texture size; in other
  // words, it should be applied within texture UV space.
  AffineTransform uv_offset = AffineTransform::Translate(layer.offset);
  // Transform from UV space (where the texture image is a unit square) to
  // size-unit space (where distance is measured in the layer's chosen
  // `TextureSizeUnit`).
  AffineTransform uv_to_size_unit =
      AffineTransform::Scale(layer.size.x, layer.size.y);
  return uv_to_size_unit * uv_offset * texel_to_uv;
}

// Computes the transform for a `TextureLayer` from size-unit space to stroke
// space. This transform may depend on properties of the particular stroke, and
// so must be computed per-stroke.
AffineTransform ComputeSizeUnitToStrokeSpaceTransform(
    const BrushPaint::TextureLayer& layer, float brush_size,
    const StrokeInputBatch& inputs) {
  // Transform from size-unit space (where distance is measured in the layer's
  // chosen `TextureSizeUnit`) to stroke space (where distance is measured in
  // stroke coordinates).
  AffineTransform size_unit_to_stroke;
  switch (layer.size_unit) {
    case BrushPaint::TextureSizeUnit::kBrushSize:
      size_unit_to_stroke = AffineTransform::Scale(brush_size);
      break;
    case BrushPaint::TextureSizeUnit::kStrokeCoordinates:
      size_unit_to_stroke = AffineTransform::Identity();
      break;
    case BrushPaint::TextureSizeUnit::kStrokeSize:
      // TODO: b/336835642 - Implement support for `kStrokeSize`.
      break;
  }
  // While we're in stroke space, shift the origin to the position specified by
  // the layer.
  AffineTransform stroke_space_offset;
  switch (layer.origin) {
    case BrushPaint::TextureOrigin::kStrokeSpaceOrigin:
      break;
    case BrushPaint::TextureOrigin::kFirstStrokeInput:
      if (!inputs.IsEmpty()) {
        stroke_space_offset =
            AffineTransform::Translate(inputs.Get(0).position.Offset());
      }
      break;
    case BrushPaint::TextureOrigin::kLastStrokeInput:
      if (!inputs.IsEmpty()) {
        stroke_space_offset = AffineTransform::Translate(
            inputs.Get(inputs.Size() - 1).position.Offset());
      }
      break;
  }
  return stroke_space_offset * size_unit_to_stroke;
}

}  // namespace

ShaderCache::ShaderCache(const TextureBitmapStore* absl_nullable provider)
    : texture_provider_(provider) {}

sk_sp<SkBlender> ShaderCache::GetBlenderForPaint(const BrushPaint& paint) {
  if (paint.texture_layers.empty()) return nullptr;
  // `SkBlender::Mode` returns a singleton for each `SkBlendMode`, so no caching
  // is needed on our end.
  return SkBlender::Mode(ToSkBlendMode(paint.texture_layers.back().blend_mode));
}

absl::StatusOr<sk_sp<SkShader>> ShaderCache::GetShaderForPaint(
    const BrushPaint& paint, float brush_size, const StrokeInputBatch& inputs) {
  if (paint.texture_layers.empty()) return nullptr;
  SkBlendMode blend_mode;
  sk_sp<SkShader> paint_shader = nullptr;
  for (const BrushPaint::TextureLayer& layer : paint.texture_layers) {
    absl::StatusOr<sk_sp<SkShader>> layer_shader =
        GetShaderForLayer(layer, brush_size, inputs);
    if (!layer_shader.ok()) return layer_shader.status();
    if (paint_shader == nullptr) {
      paint_shader = *std::move(layer_shader);
    } else {
      paint_shader = SkShaders::Blend(blend_mode, *std::move(layer_shader),
                                      std::move(paint_shader));
    }
    blend_mode = ToSkBlendMode(layer.blend_mode);
  }
  return paint_shader;
}

absl::StatusOr<sk_sp<SkShader>> ShaderCache::GetShaderForLayer(
    const BrushPaint::TextureLayer& layer, float brush_size,
    const StrokeInputBatch& inputs) {
  sk_sp<SkShader>& cached_shader = layer_shaders_[layer];
  if (cached_shader == nullptr) {
    absl::StatusOr<sk_sp<SkShader>> shader = CreateBaseShaderForLayer(layer);
    if (!shader.ok()) return shader.status();
    cached_shader = *std::move(shader);
  }
  return cached_shader->makeWithLocalMatrix(ToSkMatrix(
      ComputeSizeUnitToStrokeSpaceTransform(layer, brush_size, inputs)));
}

absl::StatusOr<sk_sp<SkShader>> ShaderCache::CreateBaseShaderForLayer(
    const BrushPaint::TextureLayer& layer) {
  absl::StatusOr<sk_sp<SkImage>> image =
      GetImageForTexture(layer.client_texture_id);
  if (!image.ok()) return image.status();
  SkISize size = (*image)->dimensions();
  SkMatrix matrix = ToSkMatrix(
      ComputeTexelToSizeUnitTransform(layer, size.width(), size.height()));
  return SkShaders::Image(*std::move(image), ToSkTileMode(layer.wrap_x),
                          ToSkTileMode(layer.wrap_y), SkSamplingOptions(),
                          &matrix);
}

absl::StatusOr<sk_sp<SkImage>> ShaderCache::GetImageForTexture(
    absl::string_view texture_id) {
  if (texture_provider_ == nullptr) {
    return absl::FailedPreconditionError(absl::StrCat(
        "`TextureBitmapStore` is null, but asked to render texture: ",
        texture_id));
  }
  sk_sp<SkImage>& cached_image = texture_images_[texture_id];
  if (cached_image == nullptr) {
    absl::StatusOr<sk_sp<SkImage>> image =
        texture_provider_->GetTextureBitmap(texture_id);
    if (!image.ok()) return image.status();
    // Since cached_image is a shared pointer, assigning it populates the cache.
    cached_image = *std::move(image);
  }
  return cached_image;
}

sk_sp<SkColorSpace> ShaderCache::GetColorSpace(ColorSpace color_space,
                                               Color::Format format) {
  sk_sp<SkColorSpace>& cached_color_space =
      color_spaces_[std::make_pair(color_space, format)];
  if (cached_color_space == nullptr) {
    cached_color_space = CreateColorSpace(color_space, format);
  }
  return cached_color_space;
}

}  // namespace ink::skia_native_internal
