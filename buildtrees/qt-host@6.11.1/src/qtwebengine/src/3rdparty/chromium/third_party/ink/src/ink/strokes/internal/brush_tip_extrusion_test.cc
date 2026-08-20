#include "ink/strokes/internal/brush_tip_extrusion.h"

#include "gtest/gtest.h"
#include "ink/geometry/angle.h"

namespace ink::strokes_internal {
namespace {

constexpr float kEpsilon = 1e-5;
constexpr float kTravelThreshold = kEpsilon * 0.1;

TEST(BrushTipExtrusionTest, EvaluateTangentQualityTwoNonOverlappingCircles) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality(
          {{.position = {0, 0}, .width = 1, .height = 1, .percent_radius = 1},
           kEpsilon},
          {{.position = {2, 1}, .width = 1, .height = 1, .percent_radius = 1},
           kEpsilon},
          kTravelThreshold),
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest, EvaluateTangentQualityTwoNonOverlappingSquares) {
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(
                {{.position = {0, 0}, .width = 1, .height = 1}, kEpsilon},
                {{.position = {2, 1}, .width = 1, .height = 1}, kEpsilon},
                kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest, EvaluateTangentQualityTwoNonOverlappingStadiums) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality(
          {{.position = {0, 0}, .width = 1, .height = 1, .percent_radius = 0.5},
           kEpsilon},
          {{.position = {2, 1}, .width = 1, .height = 1, .percent_radius = 0.5},
           kEpsilon},
          kTravelThreshold),
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest,
     EvaluateTangentQualityTwoTrapezoidsPointingInOppositeDirections) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality({{.position = {-1, 0},
                                                  .width = 1,
                                                  .height = 1,
                                                  .rotation = -kQuarterTurn,
                                                  .pinch = 0.5},
                                                 kTravelThreshold},
                                                {{.position = {1, 0},
                                                  .width = 1,
                                                  .height = 1,
                                                  .rotation = kQuarterTurn,
                                                  .pinch = 0.5},
                                                 kEpsilon},
                                                kTravelThreshold),
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest,
     EvaluateTangentQualityTwoRoundedTrapezoidsPointingInOppositeDirections) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality({{.position = {-1, 0},
                                                  .width = 1,
                                                  .height = 1,
                                                  .percent_radius = 0.5,
                                                  .rotation = -kQuarterTurn,
                                                  .pinch = 0.5},
                                                 kTravelThreshold},
                                                {{.position = {1, 0},
                                                  .width = 1,
                                                  .height = 1,
                                                  .percent_radius = 0.5,
                                                  .rotation = kQuarterTurn,
                                                  .pinch = 0.5},
                                                 kEpsilon},
                                                kTravelThreshold),
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest, JoinedShapeTwoPillsAtRightAngles) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality(
          {{.position = {0, 0}, .width = 2, .height = 1, .percent_radius = 1},
           kEpsilon},
          {{.position = {2, 0},
            .width = 2,
            .height = 1,
            .percent_radius = 1,
            .rotation = kQuarterTurn},
           kEpsilon},
          kTravelThreshold),
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

TEST(BrushTipExtrusionTest,
     EvaluateTangentQualityCrossedRectanglesFirstShapeNotFullyCovered) {
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(
                {{.position = {0, 0}, .width = 3, .height = 1}, kEpsilon},
                {{.position = {0.5, 0}, .width = 1, .height = 2}, kEpsilon},
                kTravelThreshold),
            BrushTipExtrusion::TangentQuality::
                kBadTangentsJoinedShapeDoesNotCoverInputShapes);
}

TEST(BrushTipExtrusionTest,
     EvaluateTangentQualityCrossedRectanglesSecondShapeNotFullyCovered) {
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(
                {{.position = {0, 0}, .width = 1, .height = 2}, kEpsilon},
                {{.position = {0.5, 0}, .width = 3, .height = 1}, kEpsilon},
                kTravelThreshold),
            BrushTipExtrusion::TangentQuality::
                kBadTangentsJoinedShapeDoesNotCoverInputShapes);
}

TEST(BrushTipExtrusionTest, EvaluateTangentQualityFirstShapeContainsSecond) {
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(
                {{.position = {0, 0}, .width = 4, .height = 4}, kEpsilon},
                {{.position = {0.5, 0}, .width = 1, .height = 1}, kEpsilon},
                kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kNoTangentsFirstContainsSecond);
}

TEST(BrushTipExtrusionTest, EvaluateTangentQualitySecondShapeContainsFirst) {
  EXPECT_EQ(BrushTipExtrusion::EvaluateTangentQuality(
                {{.position = {0.5, 0}, .width = 1, .height = 1}, kEpsilon},
                {{.position = {0, 0}, .width = 4, .height = 4}, kEpsilon},
                kTravelThreshold),
            BrushTipExtrusion::TangentQuality::kNoTangentsSecondContainsFirst);
}

TEST(BrushTipExtrusionTest,
     EvaluateTangentQualityAccountsForFloatingPointPrecision) {
  EXPECT_EQ(
      BrushTipExtrusion::EvaluateTangentQuality(
          {{.position = {1, 2},
            .width = 5,
            .height = 7,
            .rotation = kFullTurn / 8},
           kEpsilon},
          {{.position = {1.1818182, 2.1818182},
            .width = 5,
            .height = 7,
            .rotation = kFullTurn / 8},
           kEpsilon},
          kTravelThreshold),
      // Without the offset to the `RoundedPolygon` in `EvaluateTangentQuality`,
      // this would be `kBadTangentsJoinedShapeDoesNotCoverInputShapes`, as one
      // of the corners of the second shape lies ~1e-7 units outside the joined
      // shape due to floating-point precision loss.
      BrushTipExtrusion::TangentQuality::kGoodTangents);
}

}  // namespace
}  // namespace ink::strokes_internal
