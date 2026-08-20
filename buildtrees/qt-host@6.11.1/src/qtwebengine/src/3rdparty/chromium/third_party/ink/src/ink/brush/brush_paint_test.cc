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

#include "ink/brush/brush_paint.h"

#include <limits>
#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ink/brush/fuzz_domains.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

using ::absl_testing::IsOk;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();
constexpr absl::string_view kTestTextureId = "test-texture";

TEST(BrushPaintTest, TextureKeyframeSupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint::TextureKeyframe{.progress = 0},
      BrushPaint::TextureKeyframe{.progress = 1},
      BrushPaint::TextureKeyframe{.progress = 0, .size = Vec{1, 1}},
      BrushPaint::TextureKeyframe{.progress = 0, .offset = Vec{1, 1}},
      BrushPaint::TextureKeyframe{.progress = 0, .rotation = kHalfTurn},
      BrushPaint::TextureKeyframe{.progress = 0, .opacity = 0.5},
  }));
}

TEST(BrushPaintTest, TextureLayerSupportsAbslHash) {
  std::string id1 = "foo";
  std::string id2 = "bar";
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint::TextureLayer{.client_texture_id = id1},
      BrushPaint::TextureLayer{.client_texture_id = id2},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .mapping = BrushPaint::TextureMapping::kWinding},
      BrushPaint::TextureLayer{
          .client_texture_id = id1,
          .origin = BrushPaint::TextureOrigin::kFirstStrokeInput},
      BrushPaint::TextureLayer{
          .client_texture_id = id1,
          .size_unit = BrushPaint::TextureSizeUnit::kStrokeSize},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .wrap_x = BrushPaint::TextureWrap::kMirror},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .wrap_y = BrushPaint::TextureWrap::kClamp},
      BrushPaint::TextureLayer{.client_texture_id = id1, .size = {2, 2}},
      BrushPaint::TextureLayer{.client_texture_id = id1, .offset = {1, 1}},
      BrushPaint::TextureLayer{.client_texture_id = id1, .rotation = kHalfTurn},
      BrushPaint::TextureLayer{.client_texture_id = id1, .size_jitter = {2, 2}},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .offset_jitter = {1, 1}},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .rotation_jitter = kHalfTurn},
      BrushPaint::TextureLayer{.client_texture_id = id1, .opacity = 0.5},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .keyframes = {{.progress = 1}}},
      BrushPaint::TextureLayer{.client_texture_id = id1,
                               .blend_mode = BrushPaint::BlendMode::kXor},
  }));
}

TEST(BrushPaintTest, BrushPaintSupportsAbslHash) {
  std::string id1 = "foo";
  std::string id2 = "bar";
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint{},
      BrushPaint{{{.client_texture_id = id1}}},
      BrushPaint{{{.client_texture_id = id1}, {.client_texture_id = id2}}},
  }));
}

TEST(BrushPaintTest, TextureKeyframeEqualAndNotEqual) {
  BrushPaint::TextureKeyframe keyframe = {
      .progress = 1,
      .size = Vec{2, 2},
      .offset = Vec{1, 1},
      .rotation = kHalfTurn,
      .opacity = 0.5,
  };

  BrushPaint::TextureKeyframe other = keyframe;
  EXPECT_EQ(keyframe, other);

  other = keyframe;
  other.progress = 0;
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.size = Vec{7, 4};
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.offset = Vec{1, -1};
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.rotation = std::nullopt;
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.opacity = 0.25;
  EXPECT_NE(keyframe, other);
}

TEST(BrushPaintTest, TextureLayerEqualAndNotEqual) {
  std::string id1 = "foo";
  std::string id2 = "bar";
  BrushPaint::TextureLayer layer = {
      .client_texture_id = id1,
      .mapping = BrushPaint::TextureMapping::kTiling,
      .origin = BrushPaint::TextureOrigin::kStrokeSpaceOrigin,
      .size_unit = BrushPaint::TextureSizeUnit::kStrokeCoordinates,
      .wrap_x = BrushPaint::TextureWrap::kRepeat,
      .wrap_y = BrushPaint::TextureWrap::kMirror,
      .size = {1, 1},
      .offset = {0, 0},
      .rotation = Angle(),
      .size_jitter = {0, 0},
      .offset_jitter = {0, 0},
      .rotation_jitter = Angle(),
      .opacity = 1,
      .keyframes = {},
      .blend_mode = BrushPaint::BlendMode::kModulate,
  };

  BrushPaint::TextureLayer other = layer;
  EXPECT_EQ(layer, other);

  other = layer;
  other.client_texture_id = id2;
  EXPECT_NE(layer, other);

  other = layer;
  other.mapping = BrushPaint::TextureMapping::kWinding;
  EXPECT_NE(layer, other);

  other = layer;
  other.origin = BrushPaint::TextureOrigin::kFirstStrokeInput;
  EXPECT_NE(layer, other);

  other = layer;
  other.size_unit = BrushPaint::TextureSizeUnit::kBrushSize;
  EXPECT_NE(layer, other);

  other = layer;
  other.wrap_x = BrushPaint::TextureWrap::kMirror;
  EXPECT_NE(layer, other);

  other = layer;
  other.wrap_y = BrushPaint::TextureWrap::kClamp;
  EXPECT_NE(layer, other);

  other = layer;
  other.size = {4, 5};
  EXPECT_NE(layer, other);

  other = layer;
  other.offset = {1, -1};
  EXPECT_NE(layer, other);

  other = layer;
  other.rotation = kHalfTurn;
  EXPECT_NE(layer, other);

  other = layer;
  other.size_jitter = {4, 5};
  EXPECT_NE(layer, other);

  other = layer;
  other.offset_jitter = {1, -1};
  EXPECT_NE(layer, other);

  other = layer;
  other.rotation_jitter = kHalfTurn;
  EXPECT_NE(layer, other);

  other = layer;
  other.opacity = 0.5;
  EXPECT_NE(layer, other);

  other = layer;
  other.keyframes.push_back({.progress = 0});
  EXPECT_NE(layer, other);

  other = layer;
  other.blend_mode = BrushPaint::BlendMode::kXor;
  EXPECT_NE(layer, other);
}

TEST(BrushPaintTest, BrushPaintEqualAndNotEqual) {
  std::string id1 = "foo";
  std::string id2 = "bar";
  BrushPaint paint = {{{.client_texture_id = id1}}};

  BrushPaint other = paint;
  EXPECT_EQ(paint, other);

  other = paint;
  other.texture_layers[0].client_texture_id = id2;
  EXPECT_NE(paint, other);

  other = paint;
  other.texture_layers.clear();
  EXPECT_NE(paint, other);

  other = paint;
  other.texture_layers.push_back({.client_texture_id = id2});
  EXPECT_NE(paint, other);
}

TEST(BrushPaintTest, StringifyTextureMapping) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureMapping::kWinding), "kWinding");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureMapping::kTiling), "kTiling");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureMapping>(99)),
            "TextureMapping(99)");
}

TEST(BrushPaintTest, StringifyTextureOrigin) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kStrokeSpaceOrigin),
            "kStrokeSpaceOrigin");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kFirstStrokeInput),
            "kFirstStrokeInput");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kLastStrokeInput),
            "kLastStrokeInput");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureOrigin>(99)),
            "TextureOrigin(99)");
}

TEST(BrushPaintTest, StringifyTextureSizeUnit) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kBrushSize),
            "kBrushSize");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kStrokeSize),
            "kStrokeSize");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kStrokeCoordinates),
            "kStrokeCoordinates");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureSizeUnit>(99)),
            "TextureSizeUnit(99)");
}

TEST(BrushPaintTest, StringifyTextureWrap) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureWrap::kRepeat), "kRepeat");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureWrap::kMirror), "kMirror");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureWrap::kClamp), "kClamp");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureWrap>(99)),
            "TextureWrap(99)");
}

TEST(BrushPaintTest, StringifyBlendMode) {
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kModulate), "kModulate");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kDstIn), "kDstIn");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kDstOut), "kDstOut");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcAtop), "kSrcAtop");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcIn), "kSrcIn");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcOver), "kSrcOver");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrc), "kSrc");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::BlendMode>(99)),
            "BlendMode(99)");
}

TEST(BrushPaintTest, StringifyTextureKeyFrame) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{}),
            "TextureKeyframe{progress=0}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{.progress = 0.3}),
            "TextureKeyframe{progress=0.3}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3, .size = std::optional<Vec>({4, 6})}),
            "TextureKeyframe{progress=0.3, size=<4, 6>}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2})}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2}),
                .rotation = kQuarterTurn}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
            "rotation=0.5π}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2}),
                .rotation = kQuarterTurn,
                .opacity = 0.6}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
            "rotation=0.5π, opacity=0.6}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .offset = std::optional<Vec>({2, 0.2}),
                .opacity = 0.6}),
            "TextureKeyframe{progress=0.3, offset=<2, 0.2>, opacity=0.6}");
}

TEST(BrushPaintTest, StringifyTextureLayer) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureLayer{}),
            "TextureLayer{client_texture_id=, mapping=kTiling, "
            "origin=kStrokeSpaceOrigin, size_unit=kStrokeCoordinates, "
            "wrap_x=kRepeat, wrap_y=kRepeat, "
            "size=<1, 1>, offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, "
            "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
            "animation_frames=1, animation_rows=1, animation_columns=1, "
            "keyframes={}, blend_mode=kModulate}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint::TextureLayer{.client_texture_id =
                                                std::string(kTestTextureId)}),
      "TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, "
      "wrap_y=kRepeat, size=<1, 1>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, blend_mode=kModulate}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .mapping = BrushPaint::TextureMapping::kWinding,
          .origin = BrushPaint::TextureOrigin::kFirstStrokeInput,
          .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
          .wrap_x = BrushPaint::TextureWrap::kMirror,
          .wrap_y = BrushPaint::TextureWrap::kClamp,
          .size = {3, 5},
          .offset = {2, 0.2},
          .rotation = kQuarterTurn,
          .size_jitter = {0.1, 0.2},
          .offset_jitter = {0.7, 0.3},
          .rotation_jitter = kFullTurn / 16,
          .opacity = 0.6,
          .animation_frames = 2,
          .animation_rows = 3,
          .animation_columns = 4,
          .keyframes = {{.progress = 0.2,
                         .size = std::optional<Vec>({2, 5}),
                         .rotation = kFullTurn / 16}},
          .blend_mode = BrushPaint::BlendMode::kDstIn}),
      "TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kFirstStrokeInput, size_unit=kBrushSize, "
      "wrap_x=kMirror, wrap_y=kClamp, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, size_jitter=<0.1, 0.2>, "
      "offset_jitter=<0.7, 0.3>, rotation_jitter=0.125π, opacity=0.6, "
      "animation_frames=2, animation_rows=3, animation_columns=4, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, "
      "rotation=0.125π}}, blend_mode=kDstIn}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .mapping = BrushPaint::TextureMapping::kWinding,
          .origin = BrushPaint::TextureOrigin::kLastStrokeInput,
          .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
          .wrap_x = BrushPaint::TextureWrap::kClamp,
          .wrap_y = BrushPaint::TextureWrap::kMirror,
          .size = {3, 5},
          .offset = {2, 0.2},
          .rotation = kQuarterTurn,
          .size_jitter = {0.1, 0.2},
          .offset_jitter = {0.7, 0.3},
          .rotation_jitter = kFullTurn / 16,
          .opacity = 0.6,
          .animation_frames = 2,
          .animation_rows = 3,
          .animation_columns = 4,
          .keyframes = {{.progress = 0.2,
                         .size = std::optional<Vec>({2, 5}),
                         .rotation = kFullTurn / 16},
                        {.progress = 0.4,
                         .offset = std::optional<Vec>({2, 0.2}),
                         .opacity = 0.4}},
          .blend_mode = BrushPaint::BlendMode::kSrcAtop}),
      "TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kLastStrokeInput, size_unit=kBrushSize, "
      "wrap_x=kClamp, wrap_y=kMirror, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, size_jitter=<0.1, 0.2>, "
      "offset_jitter=<0.7, 0.3>, rotation_jitter=0.125π, opacity=0.6, "
      "animation_frames=2, animation_rows=3, animation_columns=4, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, rotation=0.125π}, "
      "TextureKeyframe{progress=0.4, offset=<2, 0.2>, opacity=0.4}}, "
      "blend_mode=kSrcAtop}");
}

TEST(BrushPaintTest, StringifyBrushPaint) {
  EXPECT_EQ(absl::StrCat(BrushPaint{}), "BrushPaint{texture_layers={}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{.texture_layers = {{}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, "
      "wrap_y=kRepeat, size=<1, 1>, offset=<0, 0>, "
      "rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=1, animation_frames=1, animation_rows=1, "
      "animation_columns=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(
          BrushPaint{.texture_layers = {{.client_texture_id =
                                             std::string(kTestTextureId)}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, wrap_y=kRepeat, "
      "size=<1, 1>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, "
      "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
      "animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, "
      "blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, wrap_x=kRepeat, wrap_y=kRepeat, "
      "size=<1, 1>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5}}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<0, 0>, "
      "rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=1, animation_frames=1, animation_rows=1, "
      "animation_columns=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .size = {3, 5}}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, wrap_y=kRepeat, "
      "size=<3, 5>, offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, "
      "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
      "animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .size = {3, 5},
                              .offset = {2, 0.2}}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, wrap_y=kRepeat, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0π, size_jitter=<0, 0>, "
      "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
      "animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, "
      "blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kQuarterTurn,
                              .opacity = 0.6}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, wrap_x=kRepeat, wrap_y=kRepeat, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, size_jitter=<0, 0>, "
      "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=0.6, "
      "animation_frames=1, animation_rows=1, animation_columns=1, "
      "keyframes={}, "
      "blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .blend_mode = BrushPaint::BlendMode::kSrcIn}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=1, animation_frames=1, animation_rows=1, "
      "animation_columns=1, keyframes={}, blend_mode=kSrcIn}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kQuarterTurn,
                              .opacity = 0.6}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0.5π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=0.6, animation_frames=1, animation_rows=1, "
      "animation_columns=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kQuarterTurn,
                              .size_jitter = {0.1, 0.2},
                              .offset_jitter = {0.7, 0.3},
                              .rotation_jitter = kFullTurn / 16,
                              .opacity = 0.6,
                              .blend_mode = BrushPaint::BlendMode::kSrcIn}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0.5π, size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, animation_frames=1, "
      "animation_rows=1, animation_columns=1, keyframes={}, "
      "blend_mode=kSrcIn}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers =
              {{.client_texture_id = std::string(kTestTextureId),
                .mapping = BrushPaint::TextureMapping::kWinding,
                .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
                .size = {3, 5},
                .offset = {2, 0.2},
                .rotation = kQuarterTurn,
                .size_jitter = {0.1, 0.2},
                .offset_jitter = {0.7, 0.3},
                .rotation_jitter = kFullTurn / 16,
                .opacity = 0.6,
                .keyframes = {{.progress = 0.3,
                               .size = std::optional<Vec>({4, 6}),
                               .offset = std::optional<Vec>({2, 0.2}),
                               .rotation = kQuarterTurn,
                               .opacity = 0.6}}}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0.5π, size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, animation_frames=1, "
      "animation_rows=1, animation_columns=1, "
      "keyframes={TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
      "rotation=0.5π, opacity=0.6}}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers =
              {{.client_texture_id = std::string(kTestTextureId),
                .mapping = BrushPaint::TextureMapping::kWinding,
                .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
                .size = {3, 5},
                .offset = {2, 0.2},
                .rotation = kQuarterTurn,
                .size_jitter = {0.1, 0.2},
                .offset_jitter = {0.7, 0.3},
                .rotation_jitter = kFullTurn / 16,
                .opacity = 0.6,
                .blend_mode = BrushPaint::BlendMode::kSrcIn},
               {.client_texture_id = std::string(kTestTextureId),
                .mapping = BrushPaint::TextureMapping::kTiling,
                .size_unit = BrushPaint::TextureSizeUnit::kStrokeSize,
                .size = {1, 4},
                .opacity = 0.7,
                .keyframes = {{.progress = 0.2,
                               .size = std::optional<Vec>({2, 5}),
                               .rotation = kFullTurn / 16},
                              {.progress = 0.4,
                               .offset = std::optional<Vec>({2, 0.2}),
                               .opacity = 0.4}},
                .blend_mode = BrushPaint::BlendMode::kDstIn}}}),
      "BrushPaint{texture_layers={TextureLayer{client_texture_id=test-texture, "
      "mapping=kWinding, origin=kStrokeSpaceOrigin, size_unit=kBrushSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0.5π, size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, animation_frames=1, "
      "animation_rows=1, animation_columns=1, keyframes={}, "
      "blend_mode=kSrcIn}, "
      "TextureLayer{client_texture_id=test-texture, "
      "mapping=kTiling, "
      "origin=kStrokeSpaceOrigin, size_unit=kStrokeSize, "
      "wrap_x=kRepeat, wrap_y=kRepeat, size=<1, 4>, "
      "offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=0.7, animation_frames=1, animation_rows=1, "
      "animation_columns=1, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, rotation=0.125π}, "
      "TextureKeyframe{progress=0.4, offset=<2, 0.2>, opacity=0.4}}, "
      "blend_mode=kDstIn}}}");
}

TEST(BrushPaintTest, InvalidTextureLayerRotation) {
  absl::Status status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                          .rotation = Angle::Radians(kInfinity)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation` must be finite"));

  status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                          .rotation = Angle::Radians(kNan)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation` must be finite"));
}

TEST(BrushPaintTest, InvalidTextureLayerRotationJitter) {
  absl::Status status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                          .rotation_jitter = Angle::Radians(kInfinity)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation_jitter` must be finite"));

  status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.client_texture_id = std::string(kTestTextureId),
                          .rotation_jitter = Angle::Radians(kNan)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation_jitter` must be finite"));
}

TEST(BrushPaintTest, InvalidTextureLayerTextureWrap) {
  absl::Status status =
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .wrap_x = static_cast<BrushPaint::TextureWrap>(123)});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(),
              HasSubstr("wrap_x` holds non-enumerator value"));

  status =
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .wrap_y = static_cast<BrushPaint::TextureWrap>(123)});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(),
              HasSubstr("wrap_y` holds non-enumerator value"));
}

TEST(BrushPaintTest, InvalidTextureLayerAnimationFrames) {
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_frames = -1}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_frames` must be in the interval [1, 2^24]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_frames = 0}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_frames` must be in the interval [1, 2^24]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_frames = (1 << 24) + 1}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_frames` must be in the interval [1, 2^24]")));
}

TEST(BrushPaintTest, InvalidTextureLayerAnimationGridDimensions) {
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_rows = -1}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("animation_rows` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_rows = 0}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("animation_rows` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_rows = (1 << 12) + 1}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("animation_rows` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_columns = -1}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_columns` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_columns = 0}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_columns` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_columns = (1 << 12) + 1}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr("animation_columns` must be in the interval [1, 2^12]")));
  EXPECT_THAT(
      brush_internal::ValidateBrushPaintTextureLayer(BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .animation_frames = 7,
          .animation_rows = 2,
          .animation_columns = 3}),
      StatusIs(
          absl::StatusCode::kInvalidArgument,
          HasSubstr(
              "TextureLayer::animation_frames` must be less than or equal to "
              "the product of `animation_rows` and `animation_columns`")));
}

TEST(BrushPaintTest, MismatchedTextureMappings) {
  EXPECT_THAT(brush_internal::ValidateBrushPaint(BrushPaint{
                  {{.client_texture_id = std::string(kTestTextureId),
                    .mapping = BrushPaint::TextureMapping::kTiling},
                   {.client_texture_id = std::string(kTestTextureId),
                    .mapping = BrushPaint::TextureMapping::kWinding}}}),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("TextureLayer::mapping` must be the same")));
}

TEST(BrushPaintTest, MismatchedAnimationFrames) {
  EXPECT_THAT(
      brush_internal::ValidateBrushPaint(
          BrushPaint{{{.client_texture_id = std::string(kTestTextureId),
                       .animation_frames = 12,
                       .animation_rows = 3,
                       .animation_columns = 4},
                      {.client_texture_id = std::string(kTestTextureId),
                       .animation_frames = 8,
                       .animation_rows = 3,
                       .animation_columns = 4}}}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("TextureLayer::animation_frames` must be the same")));
}

TEST(BrushPaintTest, MismatchedAnimationRows) {
  EXPECT_THAT(
      brush_internal::ValidateBrushPaint(
          BrushPaint{{{.client_texture_id = std::string(kTestTextureId),
                       .animation_rows = 12},
                      {.client_texture_id = std::string(kTestTextureId),
                       .animation_rows = 8}}}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("TextureLayer::animation_rows` must be the same")));
}

TEST(BrushPaintTest, MismatchedAnimationColumns) {
  EXPECT_THAT(
      brush_internal::ValidateBrushPaint(
          BrushPaint{{{.client_texture_id = std::string(kTestTextureId),
                       .animation_columns = 12},
                      {.client_texture_id = std::string(kTestTextureId),
                       .animation_columns = 8}}}),
      StatusIs(absl::StatusCode::kInvalidArgument,
               HasSubstr("TextureLayer::animation_columns` must be the same")));
}

void CanValidateAnyValidBrushPaint(const BrushPaint& paint) {
  EXPECT_THAT(brush_internal::ValidateBrushPaint(paint), IsOk());
}
FUZZ_TEST(BrushPaintTest, CanValidateAnyValidBrushPaint)
    .WithDomains(ValidBrushPaint());

}  // namespace
}  // namespace ink
