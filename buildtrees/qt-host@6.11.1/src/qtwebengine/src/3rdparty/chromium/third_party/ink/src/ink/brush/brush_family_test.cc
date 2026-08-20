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

#include "ink/brush/brush_family.h"

#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ink/brush/brush_behavior.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/brush/easing_function.h"
#include "ink/brush/fuzz_domains.h"
#include "ink/brush/type_matchers.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Pointwise;
using ::testing::SizeIs;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();
constexpr absl::StatusCode kInvalidArgument =
    absl::StatusCode::kInvalidArgument;
constexpr absl::string_view kTestTextureId = "test-paint";

BrushTip CreatePressureTestTip() {
  return {
      .scale = {2, 1},
      .corner_rounding = 0.75,
      .rotation = kQuarterTurn,
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_out_of_range_behavior =
                  BrushBehavior::OutOfRange::kRepeat,
              .source_value_range = {0, 1},
          },
          BrushBehavior::ToolTypeFilterNode{
              .enabled_tool_types = {.touch = true, .stylus = true},
          },
          BrushBehavior::FallbackFilterNode{
              .is_fallback_for = BrushBehavior::OptionalInputProperty::kTilt,
          },
          BrushBehavior::ResponseNode{
              .response_curve = {EasingFunction::Predefined::kEaseInOut},
          },
          BrushBehavior::DampingNode{
              .damping_source = BrushBehavior::DampingSource::kTimeInSeconds,
              .damping_gap = 0.1,
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kWidthMultiplier,
              .target_modifier_range = {0, 2},
          },
      }}},
  };
}

BrushPaint CreateTestPaint() {
  return {.texture_layers = {
              {.client_texture_id = std::string(kTestTextureId),
               .mapping = BrushPaint::TextureMapping::kWinding,
               .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
               .size = {3, 5},
               .size_jitter = {0.1, 2},
               .keyframes = {{.progress = 0.1, .rotation = kFullTurn / 8}},
               .blend_mode = BrushPaint::BlendMode::kDstIn}}};
}

BrushCoat CreateTestCoat() {
  return {
      .tip = CreatePressureTestTip(),
      .paint = CreateTestPaint(),
  };
}

TEST(BrushFamilyTest, StringifyWithNoId) {
  absl::StatusOr<BrushFamily> family = BrushFamily::Create(
      BrushTip{.scale = {3, 3},
               .corner_rounding = 0,
               .opacity_multiplier = 0.7,
               .particle_gap_distance_scale = 0.1,
               .particle_gap_duration = Duration32::Seconds(2)},
      CreateTestPaint());
  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_EQ(absl::StrCat(*family),
            "BrushFamily(coats=[BrushCoat{tip=BrushTip{scale=<3, 3>, "
            "corner_rounding=0, opacity_multiplier=0.7, "
            "particle_gap_distance_scale=0.1, particle_gap_duration=2s}, "
            "paint=BrushPaint{texture_layers={TextureLayer{"
            "client_texture_id=test-paint, mapping=kWinding, "
            "origin=kStrokeSpaceOrigin, size_unit=kBrushSize, wrap_x=kRepeat, "
            "wrap_y=kRepeat, size=<3, 5>, offset=<0, 0>, rotation=0π, "
            "size_jitter=<0.1, 2>, offset_jitter=<0, 0>, rotation_jitter=0π, "
            "opacity=1, animation_frames=1, animation_rows=1, "
            "animation_columns=1, keyframes={TextureKeyframe{progress=0.1, "
            "rotation=0.25π}}, blend_mode=kDstIn}}}}])");
}

TEST(BrushFamilyTest, StringifyWithId) {
  absl::StatusOr<BrushFamily> family = BrushFamily::Create(
      BrushTip{
          .scale = {3, 3}, .corner_rounding = 0, .opacity_multiplier = 0.7},
      CreateTestPaint(), "big-square");
  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_EQ(absl::StrCat(*family),
            "BrushFamily(coats=[BrushCoat{tip=BrushTip{scale=<3, 3>, "
            "corner_rounding=0, opacity_multiplier=0.7}, "
            "paint=BrushPaint{texture_layers={TextureLayer{client_texture_id="
            "test-paint, mapping=kWinding, "
            "origin=kStrokeSpaceOrigin, size_unit=kBrushSize, wrap_x=kRepeat, "
            "wrap_y=kRepeat, size=<3, 5>, offset=<0, 0>, rotation=0π, "
            "size_jitter=<0.1, 2>, offset_jitter=<0, 0>, rotation_jitter=0π, "
            "opacity=1, animation_frames=1, animation_rows=1, "
            "animation_columns=1, keyframes={TextureKeyframe{progress=0.1, "
            "rotation=0.25π}}, blend_mode=kDstIn}}}}], "
            "client_brush_family_id='big-square')");
}

TEST(BrushFamilyTest, CreateWithoutId) {
  BrushCoat coat = CreateTestCoat();

  absl::StatusOr<BrushFamily> family = BrushFamily::Create({coat});

  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_THAT(family->GetCoats(), ElementsAre(BrushCoatEq(coat)));
  EXPECT_THAT(family->GetClientBrushFamilyId(), "");
}

TEST(BrushFamilyTest, CreateWithId) {
  BrushCoat coat = CreateTestCoat();
  absl::StatusOr<BrushFamily> family =
      BrushFamily::Create({coat}, "test-family");

  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_THAT(family->GetCoats(), ElementsAre(BrushCoatEq(coat)));
  EXPECT_EQ(family->GetClientBrushFamilyId(), "test-family");
}

TEST(BrushFamilyTest, CreateWithNoCoats) {
  absl::StatusOr<BrushFamily> family = BrushFamily::Create({});
  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_THAT(family->GetCoats(), IsEmpty());
}

TEST(BrushFamilyTest, CreateWithMultipleCoats) {
  absl::StatusOr<BrushFamily> family =
      BrushFamily::Create({CreateTestCoat(), CreateTestCoat()});
  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_THAT(family->GetCoats(), SizeIs(2));
}

TEST(BrushFamilyTest, CreateWithTooManyCoats) {
  std::vector<BrushCoat> coats(BrushFamily::MaxBrushCoats() + 1,
                               CreateTestCoat());
  absl::StatusOr<BrushFamily> family = BrushFamily::Create(coats);
  EXPECT_EQ(family.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(family.status().message(), HasSubstr("coats.size()"));
}

TEST(BrushFamilyTest, CreateWithInvalidTipScale) {
  {
    absl::Status status =
        BrushFamily::Create({.scale = {kInfinity, 1}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.scale = {1, kInfinity}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }

  {
    absl::Status status =
        BrushFamily::Create({.scale = {kNan, 1}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.scale = {1, kNan}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }

  {
    absl::Status status = BrushFamily::Create({.scale = {-1, 1}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }
  {
    absl::Status status = BrushFamily::Create({.scale = {1, -1}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }

  {
    absl::Status status = BrushFamily::Create({.scale = {0, 0}}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("scale"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipCornerRounding) {
  {
    absl::Status status =
        BrushFamily::Create({.corner_rounding = -kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("corner_rounding"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.corner_rounding = kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("corner_rounding"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.corner_rounding = kNan}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("corner_rounding"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.corner_rounding = -1}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("corner_rounding"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.corner_rounding = 2}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("corner_rounding"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipPinch) {
  {
    absl::Status status =
        BrushFamily::Create({.pinch = -kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pinch"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.pinch = kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pinch"));
  }
  {
    absl::Status status = BrushFamily::Create({.pinch = kNan}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pinch"));
  }
  {
    absl::Status status = BrushFamily::Create({.pinch = -1}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pinch"));
  }
  {
    absl::Status status = BrushFamily::Create({.pinch = 2}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("pinch"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipRotation) {
  {
    absl::Status status =
        BrushFamily::Create({.rotation = Angle::Radians(kInfinity)}, {})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("rotation"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.rotation = -Angle::Radians(kInfinity)}, {})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("rotation"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.rotation = -Angle::Radians(kNan)}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("rotation"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipOpacityMultiplier) {
  {
    absl::Status status =
        BrushFamily::Create({.opacity_multiplier = -kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("opacity_multiplier"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.opacity_multiplier = kInfinity}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("opacity_multiplier"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.opacity_multiplier = kNan}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("opacity_multiplier"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.opacity_multiplier = -1}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("opacity_multiplier"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.opacity_multiplier = 5}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("opacity_multiplier"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipSlant) {
  {
    absl::Status status =
        BrushFamily::Create({.slant = Angle::Radians(kInfinity)}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("slant"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.slant = -Angle::Radians(kInfinity)}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("slant"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.slant = -Angle::Radians(kNan)}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("slant"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.slant = -kHalfTurn}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("slant"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.slant = kHalfTurn}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("slant"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipParticleGapDistanceScale) {
  {
    absl::Status status =
        BrushFamily::Create({.particle_gap_distance_scale = kInfinity}, {})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("particle_gap_distance_scale"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.particle_gap_distance_scale = kNan}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("particle_gap_distance"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.particle_gap_distance_scale = -1.f}, {}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("particle_gap_distance"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidTipParticleGapDuration) {
  {
    absl::Status status =
        BrushFamily::Create({.particle_gap_duration = Duration32::Infinite()},
                            {})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("particle_gap_duration"));
  }
  {
    absl::Status status =
        BrushFamily::Create({.particle_gap_duration = -Duration32::Seconds(1)},
                            {})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("particle_gap_duration"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorSource) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = static_cast<BrushBehavior::Source>(-1),
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kSizeMultiplier,
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorTarget) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = static_cast<BrushBehavior::Target>(-1),
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorOutOfRange) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kNormalizedPressure,
              .source_out_of_range_behavior =
                  static_cast<BrushBehavior::OutOfRange>(-1),
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kSizeMultiplier,
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_out_of_range_behavior"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorSourceValueRange) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);

  source_node->source_value_range = {kInfinity, 0};
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));

  source_node->source_value_range = {0, kInfinity};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));

  source_node->source_value_range = {kNan, 0};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));

  source_node->source_value_range = {0, kNan};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));

  source_node->source_value_range = {0, 0};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));

  source_node->source_value_range = {5, 5};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("source_value_range"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorSourceAndOutOfRangeBehavior) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source_value_range = {0, 1},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, 1},
          },
      }}},
  };
  BrushBehavior::SourceNode* source_node =
      &std::get<BrushBehavior::SourceNode>(brush_tip.behaviors[0].nodes[0]);

  source_node->source = BrushBehavior::Source::kTimeSinceInputInSeconds;
  source_node->source_out_of_range_behavior =
      BrushBehavior::OutOfRange::kRepeat;
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(),
              HasSubstr("kTimeSinceInput*` must only be used with "
                        "`source_out_of_range_behavior` of `kClamp"));

  source_node->source = BrushBehavior::Source::kTimeSinceInputInMillis;
  source_node->source_out_of_range_behavior =
      BrushBehavior::OutOfRange::kMirror;
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(),
              HasSubstr("kTimeSinceInput*` must only be used with "
                        "`source_out_of_range_behavior` of `kClamp"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorTargetModifierRange) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
          },
      }}},
  };
  BrushBehavior::TargetNode* target_node =
      &std::get<BrushBehavior::TargetNode>(brush_tip.behaviors[0].nodes[1]);

  target_node->target_modifier_range = {kInfinity, 0};
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));

  target_node->target_modifier_range = {0, kInfinity};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));

  target_node->target_modifier_range = {kNan, 0};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));

  target_node->target_modifier_range = {0, kNan};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));

  target_node->target_modifier_range = {0, 0};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));

  target_node->target_modifier_range = {5, 5};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("target_modifier_range"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorPredefinedResponseCurve) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::ResponseNode{
              .response_curve = {static_cast<EasingFunction::Predefined>(-1)},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("Predefined"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorCubicBezierResponseCurve) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::ResponseNode{},
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  BrushBehavior::ResponseNode* response_node =
      &std::get<BrushBehavior::ResponseNode>(brush_tip.behaviors[0].nodes[1]);

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = -1, .y1 = 0, .x2 = 1, .y2 = 1};
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = 2, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = kInfinity, .y1 = 0, .x2 = 1, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = kInfinity, .x2 = 1, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = kInfinity, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = 1, .y2 = kInfinity};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = kNan, .y1 = 0, .x2 = 1, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = kNan, .x2 = 1, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = kNan, .y2 = 1};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));

  response_node->response_curve.parameters =
      EasingFunction::CubicBezier{.x1 = 0, .y1 = 0, .x2 = 1, .y2 = kNan};
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("CubicBezier"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorLinearResponseCurve) {
  auto create_family_with_linear = [](const std::vector<Point>& points) {
    return BrushFamily::Create(
        BrushTip{
            .behaviors = {BrushBehavior{{
                BrushBehavior::SourceNode{
                    .source = BrushBehavior::Source::kOrientationInRadians,
                    .source_value_range = {0, 3},
                },
                BrushBehavior::ResponseNode{
                    .response_curve = {EasingFunction::Linear{.points =
                                                                  points}},
                },
                BrushBehavior::TargetNode{
                    .target = BrushBehavior::Target::kPinchOffset,
                    .target_modifier_range = {0, .2},
                },
            }}},
        },
        BrushPaint{});
  };
  // Non-finite Y-position:
  {
    absl::Status status = create_family_with_linear({{0, kNan}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  {
    absl::Status status = create_family_with_linear({{0, kInfinity}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  {
    absl::Status status = create_family_with_linear({{0, -kInfinity}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("y-position"));
  }
  // Non-finite X-position:
  {
    absl::Status status = create_family_with_linear({{kNan, 0}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = create_family_with_linear({{kInfinity, 0}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = create_family_with_linear({{-kInfinity, 0}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  // X-position out of range:
  {
    absl::Status status = create_family_with_linear({{-0.1, 0}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  {
    absl::Status status = create_family_with_linear({{1.1, 0}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("x-position"));
  }
  // X-positions that aren't monotonicly non-decreasing:
  {
    absl::Status status =
        create_family_with_linear({{0.75, 0}, {0.25, 1}}).status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("monotonic"));
  }
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorStepsResponseCurve) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::ResponseNode{},
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  BrushBehavior::ResponseNode* response_node =
      &std::get<BrushBehavior::ResponseNode>(brush_tip.behaviors[0].nodes[1]);

  response_node->response_curve.parameters = EasingFunction::Steps{
      .step_count = 0,
      .step_position = EasingFunction::StepPosition::kJumpEnd,
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("Steps"));

  response_node->response_curve.parameters = EasingFunction::Steps{
      .step_count = 1,
      .step_position = static_cast<EasingFunction::StepPosition>(-1),
  };
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("Steps"));

  response_node->response_curve.parameters = EasingFunction::Steps{
      .step_count = 1,
      .step_position = EasingFunction::StepPosition::kJumpNone,
  };
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("Steps"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorDampingGap) {
  BrushTip brush_tip = BrushTip{
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::DampingNode{
              .damping_source = BrushBehavior::DampingSource::kTimeInSeconds,
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  BrushBehavior::DampingNode* damping_node =
      &std::get<BrushBehavior::DampingNode>(brush_tip.behaviors[0].nodes[1]);

  damping_node->damping_gap = -0.1;
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("damping_gap"));

  damping_node->damping_gap = kInfinity;
  status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("damping_gap"));
}

TEST(BrushFamilyTest, CreateWithInvalidEnabledToolTypes) {
  BrushTip brush_tip = BrushTip{
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::ToolTypeFilterNode{
              .enabled_tool_types = {.unknown = false,
                                     .mouse = false,
                                     .touch = false,
                                     .stylus = false},
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("enabled_tool_types"));
}

TEST(BrushFamilyTest, CreateWithInvalidBehaviorFallbackSource) {
  BrushTip brush_tip = {
      .behaviors = {BrushBehavior{{
          BrushBehavior::SourceNode{
              .source = BrushBehavior::Source::kOrientationInRadians,
              .source_value_range = {0, 3},
          },
          BrushBehavior::FallbackFilterNode{
              .is_fallback_for =
                  static_cast<BrushBehavior::OptionalInputProperty>(-1),
          },
          BrushBehavior::TargetNode{
              .target = BrushBehavior::Target::kPinchOffset,
              .target_modifier_range = {0, .2},
          },
      }}},
  };
  absl::Status status = BrushFamily::Create(brush_tip, BrushPaint{}).status();
  EXPECT_EQ(status.code(), kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("is_fallback_for"));
}

TEST(BrushFamilyTest, DefaultConstruction) {
  BrushFamily family;
  EXPECT_THAT(family.GetCoats(),
              ElementsAre(BrushCoatEq(BrushCoat{.tip = BrushTip{}})));
  EXPECT_THAT(family.GetClientBrushFamilyId(), IsEmpty());
}

TEST(BrushFamilyTest, CopyAndMove) {
  {
    auto family =
        BrushFamily::Create(CreatePressureTestTip(), CreateTestPaint(),
                            "/brush-family:test-family");
    ASSERT_EQ(absl::OkStatus(), family.status());

    BrushFamily copied_family = *family;
    EXPECT_THAT(copied_family.GetCoats(),
                Pointwise(BrushCoatEq(), family->GetCoats()));
    EXPECT_EQ(copied_family.GetClientBrushFamilyId(),
              family->GetClientBrushFamilyId());
  }
  {
    auto family =
        BrushFamily::Create(CreatePressureTestTip(), CreateTestPaint(),
                            "/brush-family:test-family");
    ASSERT_EQ(absl::OkStatus(), family.status());

    BrushFamily copied_family;
    copied_family = *family;
    EXPECT_THAT(copied_family.GetCoats(),
                Pointwise(BrushCoatEq(), family->GetCoats()));
    EXPECT_EQ(copied_family.GetClientBrushFamilyId(),
              family->GetClientBrushFamilyId());
  }
  {
    auto family =
        BrushFamily::Create(CreatePressureTestTip(), CreateTestPaint(),
                            "/brush-family:test-family");
    ASSERT_EQ(absl::OkStatus(), family.status());

    BrushFamily copied_family = *family;
    BrushFamily moved_family = *std::move(family);
    EXPECT_THAT(moved_family.GetCoats(),
                Pointwise(BrushCoatEq(), copied_family.GetCoats()));
    EXPECT_EQ(moved_family.GetClientBrushFamilyId(),
              copied_family.GetClientBrushFamilyId());
  }
  {
    auto family =
        BrushFamily::Create(CreatePressureTestTip(), CreateTestPaint(),
                            "/brush-family:test-family");
    ASSERT_EQ(absl::OkStatus(), family.status());

    BrushFamily copied_family = *family;
    BrushFamily moved_family;
    moved_family = *std::move(family);
    EXPECT_THAT(moved_family.GetCoats(),
                Pointwise(BrushCoatEq(), copied_family.GetCoats()));
    EXPECT_EQ(moved_family.GetClientBrushFamilyId(),
              copied_family.GetClientBrushFamilyId());
  }
}

TEST(BrushFamilyTest, CreateWithInvalidBrushPaint) {
  // `TextureLayer::mapping` has invalid enum value
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers =
                 {{.client_texture_id = std::string(kTestTextureId),
                   .mapping = static_cast<BrushPaint::TextureMapping>(-1),
                   .size = {1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::texture_layers::mapping"));
  }
  // `TextureLayer::origin` has invalid enum value
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .origin =
                                     static_cast<BrushPaint::TextureOrigin>(-1),
                                 .size = {1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::texture_layers::origin"));
  }
  // TextureLayer::size_unit has invalid enum value
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers =
                 {{.client_texture_id = std::string(kTestTextureId),
                   .size_unit = static_cast<BrushPaint::TextureSizeUnit>(-1),
                   .size = {1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::texture_layers::size_unit"));
  }
  // `TextureLayer::size` has negative component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {-1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("BrushPaint::TextureLayer::size"));
  }
  // `TextureLayer::size` has zero-size component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {0, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("BrushPaint::TextureLayer::size"));
  }
  // `TextureLayer::size` is non-finite.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {3, kInfinity}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("BrushPaint::TextureLayer::size"));
  }
  // `TextureLayer::offset` is infinite.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .offset = {kInfinity, 0.4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::offset"));
  }
  // `TextureLayer::offset` is NaN.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .offset = {1, kNan}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::offset"));
  }
  // `TextureLayer::size_jitter` has negative component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .size_jitter = {-0.1, 0.4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::size_jitter"));
  }
  // `TextureLayer::size_jitter` is greater than size component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .size_jitter = {0.1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::size_jitter"));
  }
  // `TextureLayer::offset_jitter` has negative component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .offset = {0.8, 0.7},
                                 .offset_jitter = {-0.1, 0.4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::offset_jitter"));
  }
  // `TextureLayer::offset_jitter` is greater than 1.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .offset = {0.8, 0.7},
                                 .offset_jitter = {0.1, 4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::offset_jitter"));
  }
  // `TextureLayer::size_jitter` has negative component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .size_jitter = {-0.1, 0.4}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::size_jitter"));
  }
  // `TextureLayer::opacity` is negative.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .opacity = -1}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::opacity"));
  }
  // `TextureLayer::opacity` is negative.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .opacity = -1}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::opacity"));
  }
  // `TextureLayer::opacity` is greater than 1.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .opacity = 3}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureLayer::opacity"));
  }
  // `TextureLayer::TextureKeyframes::progress` is greater than 1.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .keyframes = {{.progress = 3}}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureKeyframe::progress"));
  }
  // `TextureLayer::TextureKeyframes::size` has infinite component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .keyframes = {{.size = std::optional<Vec>(
                                                    {4, kInfinity})}}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureKeyframe::size"));
  }
  // `TextureLayer::TextureKeyframes::offset` has negative component.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers =
                 {{.client_texture_id = std::string(kTestTextureId),
                   .size = {1, 3},
                   .keyframes = {{.offset = std::optional<Vec>({-2, 4})}}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureKeyframe::offset"));
  }
  // `TextureLayer::TextureKeyframes::rotation` has infinite radians.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers =
                 {{.client_texture_id = std::string(kTestTextureId),
                   .size = {1, 3},
                   .keyframes = {{.rotation = Angle::Radians(kInfinity)}}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureKeyframe::rotation"));
  }
  // `TextureLayer::TextureKeyframes::opacity` is greater than 1.
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 3},
                                 .keyframes = {{.opacity = 3}}}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::TextureKeyframe::opacity"));
  }
  // `TextureLayer::blend_mode` has invalid enum value
  {
    absl::Status status =
        BrushFamily::Create(
            BrushTip{.scale = {3, 3}, .corner_rounding = 0},
            {.texture_layers = {{.client_texture_id =
                                     std::string(kTestTextureId),
                                 .size = {1, 4},
                                 .blend_mode =
                                     static_cast<BrushPaint::BlendMode>(-1)}}})
            .status();
    EXPECT_EQ(status.code(), kInvalidArgument);
    EXPECT_THAT(status.message(),
                HasSubstr("BrushPaint::texture_layers::blend_mode"));
  }
}

void CanCreateAnyValidBrushFamily(absl::Span<const BrushCoat> coats,
                                  absl::string_view id) {
  absl::StatusOr<BrushFamily> family = BrushFamily::Create(coats, id);
  ASSERT_EQ(family.status(), absl::OkStatus());
  EXPECT_THAT(family->GetCoats(), Pointwise(BrushCoatEq(), coats));
  EXPECT_EQ(family->GetClientBrushFamilyId(), id);
}
FUZZ_TEST(BrushFamilyTest, CanCreateAnyValidBrushFamily)
    .WithDomains(fuzztest::VectorOf(ValidBrushCoat())
                     .WithMaxSize(BrushFamily::MaxBrushCoats()),
                 fuzztest::Arbitrary<std::string>());

}  // namespace
}  // namespace ink
