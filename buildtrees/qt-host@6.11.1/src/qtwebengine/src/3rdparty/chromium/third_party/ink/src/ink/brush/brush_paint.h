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

#ifndef INK_STROKES_BRUSH_BRUSH_PAINT_H_
#define INK_STROKES_BRUSH_BRUSH_PAINT_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"

namespace ink {

// A `BrushPaint` consists of parameters that describe how a stroke mesh should
// be rendered.
struct BrushPaint {
  // Specification of how the texture should be applied to the stroke.
  //
  // This should match the platform enum in BrushPaint.kt.
  enum class TextureMapping {
    // The texture will repeat according to a 2D affine transformation of
    // vertex positions. Each copy of the texture will have the same size
    // and shape modulo reflections.
    kTiling,
    // The texture will morph to "wind along the path of the stroke." The
    // horizontal axis of texture space will lie along the width of the stroke
    // and the vertical axis will lie along the direction of travel of the
    // stroke at each point.
    kWinding,
  };

  // Specification of the origin point to use for the texture.
  //
  // This should match the platform enum in BrushPaint.kt.
  enum class TextureOrigin {
    // The texture origin is the origin of stroke space, however that happens to
    // be defined for a given stroke.
    kStrokeSpaceOrigin,
    // The texture origin is the first input position for the stroke.
    kFirstStrokeInput,
    // The texture origin is the last input position (including predicted
    // inputs) for the stroke. Note that this means that the texture origin for
    // an in-progress stroke will move as more inputs are added.
    kLastStrokeInput,
  };

  // Units for specifying `TextureLayer::size`.
  //
  // This should match the platform enum in BrushPaint.kt.
  enum class TextureSizeUnit {
    // As multiples of brush size.
    kBrushSize,
    // As multiples of the stroke "size".
    //
    // This has different meanings depending on the value of `TextureMapping`
    // for the given texture.
    //   * For `kTiling` textures, the stroke size is equal to the dimensions of
    //     the XY bounding rectangle of the mesh.
    //   * For `kWinding` textures, the stroke size components are given by
    //       x: stroke width, which may change over the course of the stroke if
    //          behaviors affect the tip geometry.
    //       y: the total distance traveled by the stroke.
    kStrokeSize,
    // In the same units as the provided `StrokeInput` position.
    kStrokeCoordinates,
  };

  // Texture wrapping modes for specifying `TextureLayer::wrap_x` and `wrap_y`.
  //
  // This should match the platform enum in BrushPaint.kt.
  enum class TextureWrap {
    // Repeats texture image horizontally/vertically.
    kRepeat,
    // Repeats texture image horizontally/vertically, alternating mirror images
    // so that adjacent edges always match.
    kMirror,
    // Points outside of the texture have the color of the nearest texture edge
    // point. This mode is typically most useful when the edge pixels of the
    // texture image are all the same, e.g. either transparent or a single solid
    // color.
    kClamp,
    // ClampToBorder/Decal/MirrorClampToEdge modes are intentionally omitted
    // here, because they're not supported by all graphics libraries; in
    // particular, Skia does not support ClampToBorder or MirrorClampToEdge, and
    // WebGL and WebGPU do not support any of them.
  };

  // Setting for how an incoming ("source" / "src") color should be combined
  // with the already present ("destination" / "dst") color at a given pixel.
  // LINT.IfChange(blend_mode)
  enum class BlendMode {
    // Source and destination are component-wise multiplied, including opacity.
    //
    // Alpha = Alpha_src * Alpha_dst
    // Color = Color_src * Color_dst
    kModulate,
    // Keeps destination pixels that cover source pixels. Discards remaining
    // source and destination pixels.
    //
    // Alpha = Alpha_src * Alpha_dst
    // Color = Alpha_src * Color_dst
    kDstIn,
    // Keeps the destination pixels not covered by source pixels. Discards
    // destination pixels that are covered by source pixels and all source
    // pixels.
    //
    // Alpha = (1 - Alpha_src) * Alpha_dst
    // Color = (1 - Alpha_src) * Color_dst
    kDstOut,
    // Discards source pixels that do not cover destination pixels. Draws
    // remaining pixels over destination pixels.
    //
    // Alpha = Alpha_dst
    // Color = Alpha_dst * Color_src + (1 - Alpha_src) * Color_dst
    kSrcAtop,
    // Keeps the source pixels that cover destination pixels. Discards remaining
    // source and destination pixels.
    //
    // Alpha = Alpha_src * Alpha_dst
    // Color = Color_src * Alpha_dst
    kSrcIn,

    // The following modes shouldn't normally be used for the last
    // `TextureLayer`, which defines the mode for blending the combined texture
    // with the (possibly adjusted per-vertex) brush color. That blend mode
    // needs the output Alpha to be a multiple of Alpha_dst so that per-vertex
    // adjustment for anti-aliasing is preserved correctly. Nonetheless, they
    // can sometimes be used for the last `TextureLayer` with care, for example
    // when the brush is designed such that the mesh outline for this coat of
    // paint will always fall within a transparent portion of the texture (which
    // is possible with e.g. a winding texture).

    // The source pixels are drawn over the destination pixels.
    //
    // Alpha = Alpha_src + (1 - Alpha_src) * Alpha_dst
    // Color = Color_src + (1 - Alpha_src) * Color_dst
    kSrcOver,
    // The source pixels are drawn behind the destination pixels.
    //
    // Alpha = Alpha_dst + (1 - Alpha_dst) * Alpha_src
    // Color = Color_dst + (1 - Alpha_dst) * Color_src
    kDstOver,
    // Keeps the source pixels and discards the destination pixels.
    //
    // When used on the last `TextureLayer`, this effectively causes the
    // texture(s) to ignore the brush's base color, which may sometimes be
    // useful for special effects in brushes with multiple coats of paint.
    //
    // Alpha = Alpha_src
    // Color = Color_src
    kSrc,
    // Keeps the destination pixels and discards the source pixels.
    //
    // This mode is unlikely to be useful, since it effectively causes the
    // renderer to just ignore this `TextureLayer` and all layers before it, but
    // it is included for completeness.
    //
    // Alpha = Alpha_dst
    // Color = Color_dst
    kDst,
    // Keeps the source pixels that do not cover destination pixels. Discards
    // destination pixels and all source pixels that cover destination pixels.
    //
    // Alpha = (1 - Alpha_dst) * Alpha_src
    // Color = (1 - Alpha_dst) * Color_src
    kSrcOut,
    // Discards destination pixels that aren't covered by source
    // pixels. Remaining destination pixels are drawn over source pixels.
    //
    // Alpha = Alpha_src
    // Color = Alpha_src * Color_dst + (1 - Alpha_dst) * Color_src
    kDstAtop,
    // Discards source and destination pixels that intersect; keeps source and
    // destination pixels that do not intersect.
    //
    // Alpha = (1 - Alpha_dst) * Alpha_src + (1 - Alpha_src) * Alpha_dst
    // Color = (1 - Alpha_dst) * Color_src + (1 - Alpha_src) * Color_dst
    kXor,

    // TODO: support some of the other Porter/Duff modes and non-separable blend
    // modes. For Android graphics.Canvas, properly supporting these won't be
    // possible until Android W at the earliest due to b/267164444.
  };
  // LINT.ThenChange(
  //   ../storage/proto/brush_family.proto:blend_mode,
  //   fuzz_domains.cc:blend_mode,
  // )

  // Keyframe values used by `TextureLayer` below.
  struct TextureKeyframe {
    // Percentage value in the range [0, 1] indicating animation progress.
    float progress;

    // Values of texture transform attributes to apply for this keyframe. If not
    // `nullopt`, these will override `TextureLayer::size`, `::offset`, and
    // `::rotation`.
    std::optional<Vec> size;
    std::optional<Vec> offset;
    std::optional<Angle> rotation;

    // Value of texture layer opacity to apply for this keyframe. This value
    // will override `TextureLayer::opacity`.
    std::optional<float> opacity;
  };

  struct TextureLayer {
    // String id that will be used by renderers to retrieve the color texture.
    std::string client_texture_id;

    TextureMapping mapping = TextureMapping::kTiling;
    TextureOrigin origin = TextureOrigin::kStrokeSpaceOrigin;
    TextureSizeUnit size_unit = TextureSizeUnit::kStrokeCoordinates;
    TextureWrap wrap_x = TextureWrap::kRepeat;
    TextureWrap wrap_y = TextureWrap::kRepeat;

    // The size of (one animation frame of) the texture, specified in
    // `size_unit`s.
    Vec size = {1, 1};
    // An offset into the texture, specified as fractions of the texture size.
    Vec offset = {0, 0};
    // Angle in radians specifying the rotation of the texture. The rotation is
    // carried out about the center of the texture's first repetition along both
    // axes.
    Angle rotation = Angle();

    // Transform "jitter" properties below specify the magnitude of random
    // offset that is applied to `size`, `offset`, and `rotation` on a
    // per-stroke basis. Each component of `size_jitter` must be less than or
    // equal to that of `size`.
    Vec size_jitter = {0, 0};
    Vec offset_jitter = {0, 0};
    Angle rotation_jitter = Angle();

    // Overall layer opacity.
    float opacity = 1;

    // The number of animation frames in this texture. Must be between 1 and
    // 2^24 (inclusive). If 1 (the default), then animation is effectively
    // disabled. If greater than 1, then the texture image is treated as a grid
    // of frame images, with dimensions `animation_rows` x `animation_columns`,
    // indexed in row-major order.
    int animation_frames = 1;

    // The number of rows in the grid of frame images. See `animation_frames`
    // for more details. Must be between 1 and 2^12 (inclusive).
    int animation_rows = 1;

    // The number of columns in the grid of frame images. See `animation_frames`
    // for more details. Must be between 1 and 2^12 (inclusive).
    int animation_columns = 1;

    // Animation keyframes; currently unused.
    //
    // TODO: b/373649343 - Decide if/how this should coexist with
    // `animation_frames` above.
    std::vector<TextureKeyframe> keyframes;

    // The rule by which the texture layers up to and including this one are
    // combined with the subsequent layer.
    //
    // I.e. `BrushPaint::texture_layers[index].blend_mode` will be used to
    // combine "src", which is the result of blending layers [0..index], with
    // "dst", which is the layer at index + 1. If index refers to the last
    // texture layer, then the layer at "index + 1" is the brush color layer.
    BlendMode blend_mode = BlendMode::kModulate;
  };

  std::vector<TextureLayer> texture_layers;
};

bool operator==(const BrushPaint::TextureKeyframe& lhs,
                const BrushPaint::TextureKeyframe& rhs);
bool operator!=(const BrushPaint::TextureKeyframe& lhs,
                const BrushPaint::TextureKeyframe& rhs);

bool operator==(const BrushPaint::TextureLayer& lhs,
                const BrushPaint::TextureLayer& rhs);
bool operator!=(const BrushPaint::TextureLayer& lhs,
                const BrushPaint::TextureLayer& rhs);

bool operator==(const BrushPaint& lhs, const BrushPaint& rhs);
bool operator!=(const BrushPaint& lhs, const BrushPaint& rhs);

namespace brush_internal {

// Determines whether the given `BrushPaint` struct is valid to be used in a
// `BrushFamily`, and returns an error if not.
absl::Status ValidateBrushPaint(const BrushPaint& paint);
// Determines whether the given `BrushPaint` struct is valid to be used in a
// `BrushFamily` assuming that the `BrushPaint::TextureLayer`s are valid.
absl::Status ValidateBrushPaintTopLevel(const BrushPaint& paint);
// Determines whether the given `BrushPaint::TextureLayer` struct is valid to be
// used in a `BrushPaint`, and returns an error if not.
absl::Status ValidateBrushPaintTextureLayer(
    const BrushPaint::TextureLayer& layer);

std::string ToFormattedString(BrushPaint::TextureMapping texture_mapping);
std::string ToFormattedString(BrushPaint::TextureOrigin texture_origin);
std::string ToFormattedString(BrushPaint::TextureSizeUnit texture_size_unit);
std::string ToFormattedString(BrushPaint::TextureWrap texture_wrap);
std::string ToFormattedString(BrushPaint::BlendMode blend_mode);
std::string ToFormattedString(const BrushPaint::TextureKeyframe& keyframe);
std::string ToFormattedString(const BrushPaint::TextureLayer& texture_layer);
std::string ToFormattedString(const BrushPaint& paint);

}  // namespace brush_internal

template <typename Sink>
void AbslStringify(Sink& sink, BrushPaint::TextureMapping texture_mapping) {
  sink.Append(brush_internal::ToFormattedString(texture_mapping));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushPaint::TextureOrigin texture_origin) {
  sink.Append(brush_internal::ToFormattedString(texture_origin));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushPaint::TextureSizeUnit texture_size_unit) {
  sink.Append(brush_internal::ToFormattedString(texture_size_unit));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushPaint::TextureWrap texture_wrap) {
  sink.Append(brush_internal::ToFormattedString(texture_wrap));
}

template <typename Sink>
void AbslStringify(Sink& sink, BrushPaint::BlendMode blend_mode) {
  sink.Append(brush_internal::ToFormattedString(blend_mode));
}

template <typename Sink>
void AbslStringify(Sink& sink, const BrushPaint::TextureKeyframe& key_frame) {
  sink.Append(brush_internal::ToFormattedString(key_frame));
}

template <typename Sink>
void AbslStringify(Sink& sink, const BrushPaint::TextureLayer& texture_layer) {
  sink.Append(brush_internal::ToFormattedString(texture_layer));
}

template <typename Sink>
void AbslStringify(Sink& sink, const BrushPaint& paint) {
  sink.Append(brush_internal::ToFormattedString(paint));
}

template <typename H>
H AbslHashValue(H h, const BrushPaint::TextureKeyframe& keyframe) {
  return H::combine(std::move(h), keyframe.progress, keyframe.size,
                    keyframe.offset, keyframe.rotation, keyframe.opacity);
}

template <typename H>
H AbslHashValue(H h, const BrushPaint::TextureLayer& layer) {
  return H::combine(std::move(h), layer.client_texture_id, layer.mapping,
                    layer.origin, layer.size_unit, layer.wrap_x, layer.wrap_y,
                    layer.size, layer.offset, layer.rotation, layer.size_jitter,
                    layer.offset_jitter, layer.rotation_jitter, layer.opacity,
                    layer.keyframes, layer.blend_mode);
}

template <typename H>
H AbslHashValue(H h, const BrushPaint& paint) {
  return H::combine(std::move(h), paint.texture_layers);
}

inline bool operator!=(const BrushPaint::TextureKeyframe& lhs,
                       const BrushPaint::TextureKeyframe& rhs) {
  return !(lhs == rhs);
}

inline bool operator!=(const BrushPaint::TextureLayer& lhs,
                       const BrushPaint::TextureLayer& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const BrushPaint& lhs, const BrushPaint& rhs) {
  return lhs.texture_layers == rhs.texture_layers;
}

inline bool operator!=(const BrushPaint& lhs, const BrushPaint& rhs) {
  return !(lhs == rhs);
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_PAINT_H_
