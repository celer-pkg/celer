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

#include "ink/brush/brush_coat.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/fuzz_domains.h"
#include "ink/geometry/mesh_format.h"

namespace ink {
namespace {

using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::UnorderedElementsAre;

constexpr absl::string_view kTestTextureId = "test-paint";

TEST(BrushCoatTest, Stringify) {
  EXPECT_EQ(absl::StrCat(BrushCoat{.tip = BrushTip{}}),
            "BrushCoat{tip=BrushTip{scale=<1, 1>, corner_rounding=1}, "
            "paint=BrushPaint{texture_layers={}}}");
}

TEST(BrushCoatTest, CoatWithDefaultTipAndPaintIsValid) {
  absl::Status status = brush_internal::ValidateBrushCoat(
      BrushCoat{.tip = BrushTip{}, .paint = BrushPaint{}});
  EXPECT_EQ(status, absl::OkStatus());
}

TEST(BrushCoatTest, CoatWithInvalidTipIsInvalid) {
  absl::Status status = brush_internal::ValidateBrushCoat(
      BrushCoat{.tip = BrushTip{.pinch = -1}, .paint = BrushPaint{}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("pinch"));
}

void CanValidateValidBrushCoat(const BrushCoat& coat) {
  EXPECT_EQ(absl::OkStatus(), brush_internal::ValidateBrushCoat(coat));
}
FUZZ_TEST(EasingFunctionTest, CanValidateValidBrushCoat)
    .WithDomains(ValidBrushCoat());

TEST(BrushCoatTest, GetRequiredAttributeIdsDefaultBrushCoat) {
  EXPECT_THAT(brush_internal::GetRequiredAttributeIds(BrushCoat()),
              UnorderedElementsAre(MeshFormat::AttributeId::kPosition,
                                   MeshFormat::AttributeId::kSideDerivative,
                                   MeshFormat::AttributeId::kSideLabel,
                                   MeshFormat::AttributeId::kForwardDerivative,
                                   MeshFormat::AttributeId::kForwardLabel,
                                   MeshFormat::AttributeId::kOpacityShift));
}

TEST(BrushCoatTest, GetRequiredAttributeIdsWithoutColorShift) {
  BrushTip tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  BrushCoat coat = {.tip = tip};
  EXPECT_THAT(brush_internal::GetRequiredAttributeIds(coat),
              Not(Contains(MeshFormat::AttributeId::kColorShiftHsl)));
}

TEST(BrushCoatTest, GetRequiredAttributeIdsWithColorShift) {
  constexpr BrushBehavior::Target kColorShiftTargets[] = {
      BrushBehavior::Target::kHueOffsetInRadians,
      BrushBehavior::Target::kSaturationMultiplier,
      BrushBehavior::Target::kLuminosity,
  };
  for (BrushBehavior::Target target : kColorShiftTargets) {
    BrushTip tip = {
        .behaviors = {BrushBehavior{{
            BrushBehavior::SourceNode{
                .source = BrushBehavior::Source::kNormalizedPressure,
                .source_value_range = {0, 1},
            },
            BrushBehavior::TargetNode{
                .target = target,
                .target_modifier_range = {0, 1},
            },
        }}},
    };
    BrushCoat coat = {.tip = tip};
    EXPECT_THAT(brush_internal::GetRequiredAttributeIds(coat),
                Contains(MeshFormat::AttributeId::kColorShiftHsl));
  }
}

TEST(BrushCoatTest, GetRequiredAttributeIdsWithoutWindingTextures) {
  BrushPaint paint = {
      .texture_layers = {BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .mapping = BrushPaint::TextureMapping::kTiling,
      }},
  };
  BrushCoat coat = {.tip = BrushTip(), .paint = paint};
  EXPECT_THAT(brush_internal::GetRequiredAttributeIds(coat),
              Not(Contains(MeshFormat::AttributeId::kSurfaceUv)));
}

TEST(BrushCoatTest, GetRequiredAttributeIdsWithWindingTextures) {
  BrushPaint paint = {
      .texture_layers = {BrushPaint::TextureLayer{
          .client_texture_id = std::string(kTestTextureId),
          .mapping = BrushPaint::TextureMapping::kWinding,
      }},
  };
  BrushCoat coat = {.tip = BrushTip(), .paint = paint};
  EXPECT_THAT(brush_internal::GetRequiredAttributeIds(coat),
              Contains(MeshFormat::AttributeId::kSurfaceUv));
}

}  // namespace
}  // namespace ink
