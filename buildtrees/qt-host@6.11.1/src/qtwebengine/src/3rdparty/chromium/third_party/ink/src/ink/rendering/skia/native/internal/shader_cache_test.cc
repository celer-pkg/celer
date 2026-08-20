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

#include "ink/rendering/skia/native/internal/shader_cache.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/fuzz_domains.h"
#include "ink/rendering/skia/native/texture_bitmap_store.h"
#include "ink/strokes/input/fuzz_domains.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"

namespace ink::skia_native_internal {
namespace {

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::IsNull;
using ::testing::NotNull;

constexpr absl::string_view kTestTextureId = "test";

// A TextureBitmapStore that always returns the same bitmap regardless of
// the texture ID.
class FakeBitmapStore : public TextureBitmapStore {
 public:
  explicit FakeBitmapStore(sk_sp<SkImage> image) : image_(std::move(image)) {}

  absl::StatusOr<sk_sp<SkImage>> GetTextureBitmap(
      absl::string_view texture_id) const override {
    return image_;
  }

 private:
  sk_sp<SkImage> image_;
};

absl::StatusOr<sk_sp<SkImage>> CreateImageFromSrgbLinearPixelData(
    int width, int height, absl::Span<const uint8_t> pixel_data) {
  SkImageInfo image_info = SkImageInfo::Make(
      width, height, SkColorType::kRGBA_8888_SkColorType,
      SkAlphaType::kUnpremul_SkAlphaType, SkColorSpace::MakeSRGBLinear());
  ABSL_CHECK_EQ(pixel_data.size(), image_info.computeMinByteSize());
  SkBitmap skia_bitmap;
  bool success = skia_bitmap.tryAllocPixels(image_info);
  if (!success) {
    return absl::InternalError(absl::StrCat("failed to allocate pixels for ",
                                            width, "x", height, " SkImage"));
  }
  std::memcpy(skia_bitmap.getPixels(), pixel_data.data(), pixel_data.size());
  skia_bitmap.setImmutable();
  return skia_bitmap.asImage();
}

TEST(ShaderCacheTest, GetShaderForEmptyBrushPaint) {
  ShaderCache cache(nullptr);
  absl::StatusOr<sk_sp<SkShader>> shader =
      cache.GetShaderForPaint(BrushPaint{}, 10, StrokeInputBatch());
  ASSERT_EQ(shader.status(), absl::OkStatus());
  EXPECT_THAT(*shader, IsNull());
}

TEST(ShaderCacheTest, TryGetTextureShaderWithoutTextureProvider) {
  ShaderCache cache(nullptr);
  absl::StatusOr<sk_sp<SkShader>> shader = cache.GetShaderForPaint(
      BrushPaint{{{.client_texture_id = std::string(kTestTextureId)}}}, 10,
      StrokeInputBatch());
  EXPECT_EQ(shader.status().code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(shader.status().message(),
              AllOf(HasSubstr("TextureBitmapStore"), HasSubstr("null")));
}

TEST(ShaderCacheTest, GetShaderForTexturedBrushPaint) {
  auto test_image = CreateImageFromSrgbLinearPixelData(
      /*width=*/2, /*height=*/1,
      std::vector<uint8_t>{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
  ASSERT_THAT(test_image.status(), absl::OkStatus());
  FakeBitmapStore provider(*test_image);
  ShaderCache cache(&provider);
  absl::StatusOr<sk_sp<SkShader>> shader = cache.GetShaderForPaint(
      BrushPaint{{{.client_texture_id = std::string(kTestTextureId)}}}, 10,
      StrokeInputBatch());
  ASSERT_EQ(shader.status(), absl::OkStatus());
  ASSERT_THAT(*shader, NotNull());
  SkImage* image = (*shader)->isAImage(nullptr, nullptr);
  ASSERT_THAT(image, NotNull());
  EXPECT_EQ(image->width(), 2);
  EXPECT_EQ(image->height(), 1);
  EXPECT_EQ(image->alphaType(), SkAlphaType::kUnpremul_SkAlphaType);
  EXPECT_EQ(image->colorType(), SkColorType::kRGBA_8888_SkColorType);
  ASSERT_THAT(image->colorSpace(), NotNull());
  EXPECT_TRUE(image->colorSpace()->gammaIsLinear());
  EXPECT_FALSE(image->colorSpace()->isSRGB());
}

void CanGetShaderForAnyValidInputs(const BrushPaint& brush_paint,
                                   float brush_size,
                                   const StrokeInputBatch& inputs) {
  auto test_image = CreateImageFromSrgbLinearPixelData(
      3, 2,
      {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
       0xdd, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
  ASSERT_THAT(test_image.status(), absl::OkStatus());
  FakeBitmapStore provider(*test_image);
  ShaderCache cache(&provider);
  absl::StatusOr<sk_sp<SkShader>> shader =
      cache.GetShaderForPaint(brush_paint, brush_size, inputs);
  EXPECT_EQ(shader.status(), absl::OkStatus());
}
FUZZ_TEST(ShaderCacheTest, CanGetShaderForAnyValidInputs)
    .WithDomains(ValidBrushPaint(), fuzztest::Positive<float>(),
                 ArbitraryStrokeInputBatch());

}  // namespace
}  // namespace ink::skia_native_internal
