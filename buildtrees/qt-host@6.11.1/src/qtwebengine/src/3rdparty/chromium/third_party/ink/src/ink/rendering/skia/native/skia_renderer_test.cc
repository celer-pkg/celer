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

#include "ink/rendering/skia/native/skia_renderer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

// This test contains the cases that do not require a `GrDirectContext`.

TEST(SkiaRendererDrawableTest, DefaultConstructed) {
  SkiaRenderer::Drawable drawable;
  EXPECT_FALSE(drawable.HasBrushColor());
  EXPECT_THAT(drawable.ObjectToCanvas(),
              AffineTransformEq(AffineTransform::Identity()));
}

TEST(SkiaRendererDrawableTest, SetObjectToCanvas) {
  SkiaRenderer::Drawable drawable;
  ASSERT_THAT(drawable.ObjectToCanvas(),
              AffineTransformEq(AffineTransform::Identity()));

  AffineTransform transform =
      AffineTransform::RotateAboutPoint(kFullTurn / 8, {5, -9});
  drawable.SetObjectToCanvas(transform);
  EXPECT_THAT(drawable.ObjectToCanvas(), AffineTransformEq(transform));
}

TEST(SkiaRendererDrawableDeathTest, SetObjectToCanvas) {
  SkiaRenderer::Drawable drawable;
  ASSERT_FALSE(drawable.HasBrushColor());
  EXPECT_DEATH_IF_SUPPORTED(drawable.SetBrushColor(Color::GoogleBlue()), "");
}

}  // namespace
}  // namespace ink
