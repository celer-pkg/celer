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

#include "ink/brush/brush_tip.h"

#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/brush/brush_behavior.h"
#include "ink/geometry/angle.h"
#include "ink/types/duration.h"

namespace ink {
namespace {

TEST(BrushTipTest, Stringify) {
  EXPECT_EQ(absl::StrCat(BrushTip{}),
            "BrushTip{scale=<1, 1>, corner_rounding=1}");
  EXPECT_EQ(absl::StrCat(BrushTip{
                .scale = {0.5f, 2.f},
                .corner_rounding = 0.25f,
                .slant = Angle::Degrees(45),
                .pinch = 0.75f,
                .rotation = Angle::Degrees(90),
                .opacity_multiplier = 0.7f,
            }),
            "BrushTip{scale=<0.5, 2>, corner_rounding=0.25, slant=0.25π, "
            "pinch=0.75, rotation=0.5π, opacity_multiplier=0.7}");
  EXPECT_EQ(
      absl::StrCat(BrushTip{
          .scale = {1.25f, 0.75f},
          .corner_rounding = 0.f,
          .opacity_multiplier = 0.7f,
          .behaviors =
              {
                  BrushBehavior{{
                      BrushBehavior::SourceNode{
                          .source = BrushBehavior::Source::kTimeOfInputInMillis,
                          .source_value_range = {0, 250},
                      },
                      BrushBehavior::TargetNode{
                          .target = BrushBehavior::Target::kWidthMultiplier,
                          .target_modifier_range = {1.5, 2},
                      },
                  }},
                  BrushBehavior{{
                      BrushBehavior::SourceNode{
                          .source = BrushBehavior::Source::kNormalizedPressure,
                          .source_value_range = {0, 1},
                      },
                      BrushBehavior::TargetNode{
                          .target = BrushBehavior::Target::kPinchOffset,
                          .target_modifier_range = {0, 1},
                      },
                  }},
              },
      }),
      "BrushTip{scale=<1.25, 0.75>, corner_rounding=0, opacity_multiplier=0.7, "
      "behaviors={BrushBehavior{nodes={SourceNode{source=kTimeOfInputInMillis, "
      "source_value_range={0, 250}}, TargetNode{target=kWidthMultiplier, "
      "target_modifier_range={1.5, 2}}}}, "
      "BrushBehavior{nodes={SourceNode{source=kNormalizedPressure, "
      "source_value_range={0, 1}}, TargetNode{target=kPinchOffset, "
      "target_modifier_range={0, 1}}}}}}");
}

TEST(BrushTipTest, EqualAndNotEqual) {
  BrushTip brush_tip{
      .scale = {1.25f, 0.75f},
      .corner_rounding = 0.25f,
      .slant = Angle::Degrees(45),
      .pinch = 0.75f,
      .rotation = Angle::Degrees(90),
      .opacity_multiplier = 0.7f,
      .particle_gap_distance_scale = 0.5f,
      .particle_gap_duration = Duration32::Seconds(0.5),
      .behaviors = {
          BrushBehavior{{
              BrushBehavior::SourceNode{
                  .source = BrushBehavior::Source::kTimeOfInputInMillis,
                  .source_value_range = {0, 250},
              },
              BrushBehavior::TargetNode{
                  .target = BrushBehavior::Target::kWidthMultiplier,
                  .target_modifier_range = {1.5, 2},
              },
          }},
      }};

  EXPECT_EQ(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip({.scale = {7.77f, 8.88f},  // Modified
                .corner_rounding = 0.25f,
                .slant = Angle::Degrees(45),
                .pinch = 0.75f,
                .rotation = Angle::Degrees(90),
                .opacity_multiplier = 0.7f,
                .particle_gap_distance_scale = 0.5f,
                .particle_gap_duration = Duration32::Seconds(0.5),
                .behaviors = {BrushBehavior{{
                    BrushBehavior::SourceNode{
                        .source = BrushBehavior::Source::kTimeOfInputInMillis,
                        .source_value_range = {0, 250},
                    },
                    BrushBehavior::TargetNode{
                        .target = BrushBehavior::Target::kWidthMultiplier,
                        .target_modifier_range = {1.5, 2},
                    },
                }}}}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.99f,  // Modified
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(33),  // Modified
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.88f,  // Modified
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(22),  // Modified
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.9f,  // Modified
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0,  // Modified
           .particle_gap_duration = Duration32::Seconds(0.5),
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Zero(),  // Modified
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
           }}));
  EXPECT_NE(
      brush_tip,
      BrushTip(
          {.scale = {1.25f, 0.75f},
           .corner_rounding = 0.25f,
           .slant = Angle::Degrees(45),
           .pinch = 0.75f,
           .rotation = Angle::Degrees(90),
           .opacity_multiplier = 0.7f,
           .particle_gap_distance_scale = 0.5f,
           .particle_gap_duration = Duration32::Seconds(0.5),
           // Modified:
           .behaviors = {
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kTimeOfInputInMillis,
                       .source_value_range = {0, 250},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kWidthMultiplier,
                       .target_modifier_range = {1.5, 2},
                   },
               }},
               BrushBehavior{{
                   BrushBehavior::SourceNode{
                       .source = BrushBehavior::Source::kNormalizedPressure,
                       .source_value_range = {22, 77},
                   },
                   BrushBehavior::TargetNode{
                       .target = BrushBehavior::Target::kSlantOffsetInRadians,
                       .target_modifier_range = {1.44, 1.66},
                   },
               }},
           }}));
}

}  // namespace
}  // namespace ink
