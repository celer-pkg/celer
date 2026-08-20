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

#include "ink/strokes/internal/constrain_brush_tip_extrusion.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/angle.h"
#include "ink/strokes/internal/brush_tip_extrusion.h"
#include "ink/strokes/internal/brush_tip_state.h"
#include "ink/strokes/internal/type_matchers.h"

namespace ink::strokes_internal {
namespace {

using ResultType =
    ::ink::strokes_internal::ConstrainedBrushTipExtrusion::ResultType;

using ::testing::FloatNear;

constexpr float kEpsilon = 1e-4;
constexpr float kTravelThreshold = kEpsilon * 0.1;
constexpr float kTol = 1e-3;
constexpr int kMaxIter = 20;

TEST(ConstrainBrushTipTest, LastExtrusionContainsProposedExtrusion) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 4, .height = 4},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {1, 0}, .width = 1, .height = 1},
                             kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type,
            ResultType::kLastExtrusionContainsProposedExtrusion);
  EXPECT_EQ(result.lerp_amount, -1);
  EXPECT_TRUE(result.extrusion.IsBreakPoint());
}

TEST(ConstrainBrushTipTest, ProposedExtrusionContainsLastExtrusion) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 1},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {1, 0}, .width = 4, .height = 4},
                             kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type,
            ResultType::kProposedExtrusionContainsLastExtrusion);
  EXPECT_EQ(result.lerp_amount, -1);
  EXPECT_TRUE(result.extrusion.IsBreakPoint());
}

TEST(ConstrainBrushTipTest, ProposedExtrusionIsValid) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 1},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {4, 4}, .width = 1, .height = 1},
                             kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type, ResultType::kProposedExtrusionIsValid);
  EXPECT_EQ(result.lerp_amount, -1);
  EXPECT_TRUE(result.extrusion.IsBreakPoint());
}

TEST(ConstrainBrushTipTest, CannotFindValidIntermediateExtrusion) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 2},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {0, 0}, .width = 2, .height = 1},
                             kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type,
            ResultType::kCannotFindValidIntermediateExtrusion);
  EXPECT_EQ(result.lerp_amount, -1);
  EXPECT_TRUE(result.extrusion.IsBreakPoint());
}

TEST(ConstrainBrushTipTest, SimpleConstrainedCase) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1.5, .height = 4},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {1, 0.75},
                              .width = 1.5,
                              .height = 4,
                              .rotation = Angle::Degrees(90)},
                             kEpsilon};
  float expected_lerp_amount = 0.3829;
  BrushTipExtrusion expected_extrusion{
      BrushTipState::LerpShapeAttributes(last.GetState(), proposed.GetState(),
                                         expected_lerp_amount),
      kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_THAT(result.lerp_amount, FloatNear(expected_lerp_amount, kTol));
  EXPECT_THAT(result.extrusion.GetState(),
              BrushTipStateNear(expected_extrusion.GetState(), kTol));
  EXPECT_THAT(result.extrusion.GetShape(),
              BrushTipShapeNear(expected_extrusion.GetShape(), kTol));
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(last, result.extrusion,
                                                      kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(ConstrainBrushTipTest, OptimalShapeIsAtLerpAmountZero) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 2},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {0, 0.5},
                              .width = 1,
                              .height = 2,
                              .slant = Angle::Degrees(30)},
                             kEpsilon};
  float expected_lerp_amount = 0;
  BrushTipExtrusion expected_extrusion{
      BrushTipState::LerpShapeAttributes(last.GetState(), proposed.GetState(),
                                         expected_lerp_amount),
      kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_NEAR(result.lerp_amount, expected_lerp_amount, kTol);
  EXPECT_THAT(result.extrusion.GetState(),
              BrushTipStateNear(expected_extrusion.GetState(), kTol));
  EXPECT_THAT(result.extrusion.GetShape(),
              BrushTipShapeNear(expected_extrusion.GetShape(), kTol));
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(last, result.extrusion,
                                                      kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(ConstrainBrushTipTest, ConstrainWhenTangentIndicesChangeWithLerpAmount) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 4},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {1, 0.25},
                              .width = 1,
                              .height = 4,
                              .rotation = Angle::Degrees(90)},
                             kEpsilon};
  float expected_lerp_amount = 0.3543;
  BrushTipExtrusion expected_extrusion{
      BrushTipState::LerpShapeAttributes(last.GetState(), proposed.GetState(),
                                         expected_lerp_amount),
      kEpsilon};

  // At t = 0, the tangent circle indices are {1, 1} on the left side and {3, 3}
  // on the right side, but left indices change to {1, 0} at t ~= 0.12, and the
  // right indices change to {3, 2} at t ~= 0.19
  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, kMaxIter);

  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_THAT(result.lerp_amount, FloatNear(expected_lerp_amount, kTol));
  EXPECT_THAT(result.extrusion.GetState(),
              BrushTipStateNear(expected_extrusion.GetState(), kTol));
  EXPECT_THAT(result.extrusion.GetShape(),
              BrushTipShapeNear(expected_extrusion.GetShape(), kTol));
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(last, result.extrusion,
                                                      kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(ConstrainBrushTipTest, MoreIterationsResultsInMorePrecision) {
  BrushTipExtrusion last{{.position = {0, 0}, .width = 1, .height = 6.1},
                         kEpsilon};
  BrushTipExtrusion proposed{{.position = {2.5, 0},
                              .width = 1,
                              .height = 6.1,
                              .rotation = Angle::Degrees(90)},
                             kEpsilon};

  ConstrainedBrushTipExtrusion result =
      ConstrainBrushTipExtrusion(last, proposed, kEpsilon, 2);
  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_FLOAT_EQ(result.lerp_amount, 0.5);

  result = ConstrainBrushTipExtrusion(last, proposed, kEpsilon, 3);
  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_FLOAT_EQ(result.lerp_amount, 0.625);

  result = ConstrainBrushTipExtrusion(last, proposed, kEpsilon, 4);
  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_FLOAT_EQ(result.lerp_amount, 0.6875);

  result = ConstrainBrushTipExtrusion(last, proposed, kEpsilon, 5);
  EXPECT_EQ(result.result_type, ResultType::kConstrainedExtrusionFound);
  EXPECT_FLOAT_EQ(result.lerp_amount, 0.71875);
}

}  // namespace
}  // namespace ink::strokes_internal
