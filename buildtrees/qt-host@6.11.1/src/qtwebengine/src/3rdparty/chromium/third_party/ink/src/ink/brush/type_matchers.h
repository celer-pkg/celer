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

#ifndef INK_STROKES_BRUSH_TYPE_MATCHERS_H_
#define INK_STROKES_BRUSH_TYPE_MATCHERS_H_

#include <tuple>

#include "gtest/gtest.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"

namespace ink {

::testing::Matcher<BrushBehavior::Node> BrushBehaviorNodeEq(
    const BrushBehavior::Node& expected);
::testing::Matcher<std::tuple<BrushBehavior::Node, BrushBehavior::Node>>
BrushBehaviorNodeEq();

::testing::Matcher<BrushBehavior> BrushBehaviorEq(
    const BrushBehavior& expected);
::testing::Matcher<std::tuple<BrushBehavior, BrushBehavior>> BrushBehaviorEq();

::testing::Matcher<BrushTip> BrushTipEq(const BrushTip& expected);
::testing::Matcher<std::tuple<BrushTip, BrushTip>> BrushTipEq();

::testing::Matcher<BrushPaint> BrushPaintEq(const BrushPaint& expected);

::testing::Matcher<BrushPaint::TextureLayer> BrushPaintTextureLayerEq(
    const BrushPaint::TextureLayer& expected);
::testing::Matcher<
    std::tuple<BrushPaint::TextureLayer, BrushPaint::TextureLayer>>
BrushPaintTextureLayerEq();

::testing::Matcher<BrushCoat> BrushCoatEq(const BrushCoat& expected);
::testing::Matcher<std::tuple<BrushCoat, BrushCoat>> BrushCoatEq();

::testing::Matcher<BrushFamily> BrushFamilyEq(const BrushFamily& expected);

::testing::Matcher<Brush> BrushEq(const Brush& expected);

}  // namespace ink

#endif  // INK_STROKES_BRUSH_TYPE_MATCHERS_H_
