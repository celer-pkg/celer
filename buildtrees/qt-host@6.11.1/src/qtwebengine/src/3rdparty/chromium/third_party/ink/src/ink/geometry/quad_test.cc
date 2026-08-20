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

#include "ink/geometry/quad.h"

#include <array>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

TEST(QuadTest, ConstructorsZeroValues) {
  Quad quad_1 = Quad::FromCenterAndDimensions({0, 0}, 0, 0);
  EXPECT_THAT(quad_1.Center(), PointEq({0, 0}));
  EXPECT_EQ(quad_1.Width(), 0);
  EXPECT_EQ(quad_1.Height(), 0);
  EXPECT_EQ(quad_1.Rotation(), Angle());
  EXPECT_EQ(quad_1.ShearFactor(), 0);

  Quad quad_2 =
      Quad::FromCenterDimensionsAndRotation({0, 0}, 0, 0, Angle::Radians(0));
  EXPECT_THAT(quad_2.Center(), PointEq({0, 0}));
  EXPECT_EQ(quad_2.Width(), 0);
  EXPECT_EQ(quad_2.Height(), 0);
  EXPECT_EQ(quad_2.Rotation(), Angle());
  EXPECT_EQ(quad_2.ShearFactor(), 0);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {0, 0}, 0, 0, Angle::Radians(0), 0);
  EXPECT_THAT(quad_3.Center(), PointEq({0, 0}));
  EXPECT_EQ(quad_3.Width(), 0);
  EXPECT_EQ(quad_3.Height(), 0);
  EXPECT_EQ(quad_3.Rotation(), Angle());
  EXPECT_EQ(quad_3.ShearFactor(), 0);

  Quad quad_4 = Quad::FromRect(Rect::FromTwoPoints({0, 0}, {0, 0}));
  EXPECT_THAT(quad_4.Center(), PointEq({0, 0}));
  EXPECT_EQ(quad_4.Width(), 0);
  EXPECT_EQ(quad_4.Height(), 0);
  EXPECT_EQ(quad_4.Rotation(), Angle());
  EXPECT_EQ(quad_4.ShearFactor(), 0);
}

TEST(QuadTest, ConstructorsVariousValues) {
  Quad quad_1 = Quad::FromCenterAndDimensions({-4.5f, 0.2f}, 11.6f, -5);
  EXPECT_THAT(quad_1.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_1.Width(), 11.6f);
  EXPECT_EQ(quad_1.Height(), -5);
  EXPECT_EQ(quad_1.Rotation(), Angle());
  EXPECT_EQ(quad_1.ShearFactor(), 0);

  Quad quad_2 = Quad::FromCenterDimensionsAndRotation({-4.5f, 0.2f}, 11.6f, -5,
                                                      Angle::Radians(-.5f));
  EXPECT_THAT(quad_2.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_2.Width(), 11.6f);
  EXPECT_EQ(quad_2.Height(), -5);
  EXPECT_EQ(quad_2.Rotation(), Angle::Radians(-.5f) + kFullTurn);
  EXPECT_EQ(quad_2.ShearFactor(), 0);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-4.5f, 0.2f}, 11.6f, -5, Angle::Radians(-.5f), -8.4f);
  EXPECT_THAT(quad_3.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_3.Width(), 11.6f);
  EXPECT_EQ(quad_3.Height(), -5);
  EXPECT_EQ(quad_3.Rotation(), Angle::Radians(-.5f) + kFullTurn);
  EXPECT_EQ(quad_3.ShearFactor(), -8.4f);

  Quad quad_4 =
      Quad::FromRect(Rect::FromCenterAndDimensions({-4.5f, 0.2f}, 11.6f, 5));
  EXPECT_THAT(quad_4.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_4.Width(), 11.6f);
  EXPECT_EQ(quad_4.Height(), 5);
  EXPECT_EQ(quad_4.Rotation(), Angle());
  EXPECT_EQ(quad_4.ShearFactor(), 0);
}

TEST(QuadTest, ConstructorsNegativeWidth) {
  Quad quad_1 = Quad::FromCenterAndDimensions({-4.5f, 0.2f}, -11.6f, -5);
  EXPECT_THAT(quad_1.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_1.Width(), 11.6f);
  EXPECT_EQ(quad_1.Height(), 5);
  EXPECT_EQ(quad_1.Rotation(), kHalfTurn);
  EXPECT_EQ(quad_1.ShearFactor(), 0);

  Quad quad_2 = Quad::FromCenterDimensionsAndRotation({-4.5f, 0.2f}, -11.6f, -5,
                                                      Angle::Radians(1.0f));
  EXPECT_THAT(quad_2.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_2.Width(), 11.6f);
  EXPECT_EQ(quad_2.Height(), 5);
  EXPECT_EQ(quad_2.Rotation(), Angle::Radians(1.0f) + kHalfTurn);
  EXPECT_EQ(quad_2.ShearFactor(), 0);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-4.5f, 0.2f}, -11.6f, -5, Angle::Radians(1.0f), -8.4f);
  EXPECT_THAT(quad_3.Center(), PointEq({-4.5f, 0.2f}));
  EXPECT_EQ(quad_3.Width(), 11.6f);
  EXPECT_EQ(quad_3.Height(), 5);
  EXPECT_EQ(quad_3.Rotation(), Angle::Radians(1.0f) + kHalfTurn);
  EXPECT_EQ(quad_3.ShearFactor(), -8.4f);
}

TEST(QuadTest, GettersGetVariousValues) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.0f);
  EXPECT_THAT(quad_1.Center(), PointEq({0, 0}));
  EXPECT_EQ(quad_1.Width(), 0);
  EXPECT_EQ(quad_1.Height(), 0);
  EXPECT_EQ(quad_1.Rotation(), Angle());
  EXPECT_EQ(quad_1.ShearFactor(), 0);

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {2.5f, 4.5f}, 2.4f, 3.6f, Angle::Radians(1.2f), 4.8f);
  EXPECT_THAT(quad_2.Center(), PointEq({2.5f, 4.5f}));
  EXPECT_EQ(quad_2.Width(), 2.4f);
  EXPECT_EQ(quad_2.Height(), 3.6f);
  EXPECT_EQ(quad_2.Rotation(), Angle::Radians(1.2f));
  EXPECT_EQ(quad_2.ShearFactor(), 4.8f);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-2.5f, -4.5f}, 2.4f, -3.6f, Angle::Radians(-1.2f), -4.8f);
  EXPECT_THAT(quad_3.Center(), PointEq({-2.5f, -4.5f}));
  EXPECT_EQ(quad_3.Width(), 2.4f);
  EXPECT_EQ(quad_3.Height(), -3.6f);
  EXPECT_EQ(quad_3.Rotation(), Angle::Radians(-1.2f) + kFullTurn);
  EXPECT_EQ(quad_3.ShearFactor(), -4.8f);
}

TEST(QuadTest, SettersSetLargerValue) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_1.SetCenter({45.0f, 55.0f});
  EXPECT_THAT(quad_1.Center(), PointEq({45.0f, 55.0f}));

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_2.SetWidth(6.0f);
  EXPECT_FLOAT_EQ(quad_2.Width(), 6.0f);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_3.SetHeight(7.0f);
  EXPECT_FLOAT_EQ(quad_3.Height(), 7.0f);

  Quad quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_4.SetRotation(Angle::Radians(1.0f));
  EXPECT_EQ(quad_4.Rotation(), Angle::Radians(1.0f));

  Quad quad_5 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_5.SetShearFactor(17.0f);
  EXPECT_FLOAT_EQ(quad_5.ShearFactor(), 17.0f);
}

TEST(QuadTest, SettersSetSmallerValue) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_1.SetCenter({1.2f, 2.1f});
  EXPECT_THAT(quad_1.Center(), PointEq({1.2f, 2.1f}));

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_2.SetWidth(0.3f);
  EXPECT_FLOAT_EQ(quad_2.Width(), 0.3f);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_3.SetHeight(1.8f);
  EXPECT_FLOAT_EQ(quad_3.Height(), 1.8f);

  Quad quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_4.SetRotation(Angle::Radians(0.3f));
  EXPECT_EQ(quad_4.Rotation(), Angle::Radians(0.3f));

  Quad quad_5 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_5.SetShearFactor(0.9f);
  EXPECT_FLOAT_EQ(quad_5.ShearFactor(), 0.9f);
}

TEST(QuadTest, SettersSetZeroValue) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_1.SetCenter({0.0f, 0.0f});
  EXPECT_THAT(quad_1.Center(), PointEq({0.0f, 0.0f}));

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_2.SetWidth(0.0f);
  EXPECT_FLOAT_EQ(quad_2.Width(), 0.0f);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_3.SetHeight(0.0f);
  EXPECT_FLOAT_EQ(quad_3.Height(), 0.0f);

  Quad quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_4.SetRotation(Angle::Radians(0.0f));
  EXPECT_EQ(quad_4.Rotation(), Angle::Radians(0.0f));

  Quad quad_5 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_5.SetShearFactor(0.0f);
  EXPECT_FLOAT_EQ(quad_5.ShearFactor(), 0.0f);
}

TEST(QuadTest, SettersSetNegativeValue) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_1.SetCenter({-45.0f, -55.0f});
  EXPECT_THAT(quad_1.Center(), PointEq({-45.0f, -55.0f}));

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_2.SetWidth(-6.0f);
  EXPECT_FLOAT_EQ(quad_2.Width(), 6.0f);
  EXPECT_FLOAT_EQ(quad_2.Height(), -4.0f);
  EXPECT_EQ(quad_2.Rotation(), Angle::Radians(2.0f) + kHalfTurn);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_3.SetHeight(-7.0f);
  EXPECT_FLOAT_EQ(quad_3.Height(), -7.0f);

  Quad quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_4.SetRotation(Angle::Radians(-1.0f));
  EXPECT_EQ(quad_4.Rotation(), Angle::Radians(-1.0f) + kFullTurn);

  Quad quad_5 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  quad_5.SetShearFactor(-17.0f);
  EXPECT_FLOAT_EQ(quad_5.ShearFactor(), -17.0f);
}

TEST(QuadTest, OperatorsEquivalence) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), 3.0f);
  EXPECT_EQ(quad_1, quad_2);

  Quad quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.0f);
  Quad quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.0f);
  EXPECT_EQ(quad_3, quad_4);

  Quad quad_5 = Quad::FromCenterDimensionsRotationAndShear(
      {-4.5f, -0.2f}, 11.6f, -5, Angle::Radians(-.5f), -8.4f);
  Quad quad_6 = Quad::FromCenterDimensionsRotationAndShear(
      {-4.5f, -0.2f}, 11.6f, -5, Angle::Radians(-.5f), -8.4f);
  EXPECT_EQ(quad_5, quad_6);

  EXPECT_NE(quad_1, quad_3);
  EXPECT_NE(quad_1, quad_5);
  EXPECT_NE(quad_3, quad_5);
}

TEST(QuadTest, SemiAxesOnVariousQuads) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(1.0f), 3.0f);
  std::pair<Vec, Vec> semi_axes_1 = quad_1.SemiAxes();
  EXPECT_THAT(semi_axes_1.first, VecEq(Vec{0.540302305868f, 0.841470984808f}));
  EXPECT_THAT(semi_axes_1.second, VecEq(Vec{1.55887186559f, 6.12943052058f}));
}

TEST(QuadTest, SemiAxesOnSegmentLikeQuad) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 0.0f, 4.0f, Angle::Radians(1.0f), 3.0f);
  std::pair<Vec, Vec> semi_axes_1 = quad_1.SemiAxes();
  EXPECT_THAT(semi_axes_1.first, VecEq(Vec{0.0f, 0.0f}));
  EXPECT_THAT(semi_axes_1.second, VecEq(Vec{1.55887186559f, 6.12943052058f}));

  Quad quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 0.0f, Angle::Radians(1.0f), 3.0f);
  std::pair<Vec, Vec> semi_axes_2 = quad_2.SemiAxes();
  EXPECT_THAT(semi_axes_2.first, VecEq(Vec{0.540302305868f, 0.841470984808f}));
  EXPECT_THAT(semi_axes_2.second, VecEq(Vec{0.0f, 0.0f}));
}

TEST(QuadTest, SemiAxesOnPointLikeQuad) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 0.0f, 0.0f, Angle::Radians(1.0f), 3.0f);
  std::pair<Vec, Vec> semi_axes_1 = quad_1.SemiAxes();
  EXPECT_THAT(semi_axes_1.first, VecEq(Vec{0.0f, 0.0f}));
  EXPECT_THAT(semi_axes_1.second, VecEq(Vec{0.0f, 0.0f}));
}

TEST(QuadTest, SemiAxesRecomputesWhenQuadIsChanged) {
  Quad quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 5.0f}, 2.0f, 4.0f, Angle::Radians(1.0f), 3.0f);
  quad_1.SetRotation(Angle::Radians(3.0f));
  std::pair<Vec, Vec> semi_axes_1 = quad_1.SemiAxes();
  EXPECT_THAT(semi_axes_1.first, VecEq(Vec{-0.9899924966f, 0.14112000806f}));
  EXPECT_THAT(semi_axes_1.second, VecEq(Vec{-6.22219499572f, -1.13326494484f}));
}

TEST(QuadTest, IsRectangularOnVariousQuads) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {4.0f, 25.0f}, 10.0f, 15.0f, Angle::Radians(2.0f), 3.0f)
                   .IsRectangular());
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, -25.0f}, 2.0f, 4.0f, Angle::Radians(2.0f), -0.05f)
                   .IsRectangular());
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {40.0f, 25.0f}, -2.0f, 10.0f, Angle::Radians(2.0f), 0.0f)
                  .IsRectangular());
}

TEST(QuadTest, IsRectangularOnLineLikeQuads) {
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 25.0f, 0.0f, Angle::Radians(0.0f), 0.0f)
                  .IsRectangular());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {40.0f, -25.5f}, 25.0f, 0.0f, Angle::Radians(0.0f), 0)
                  .IsRectangular());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, -25.5f}, 25.0f, 0.0f, Angle::Radians(0.0f), 0.5f)
                   .IsRectangular());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 25.0f, Angle::Radians(0.0f), 0.0f)
                  .IsRectangular());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {40.0f, -25.5f}, 0.0f, 25.0f, Angle::Radians(0.0f), 0)
                  .IsRectangular());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, -25.5f}, 0.0f, 25.0f, Angle::Radians(0.0f), 0.5f)
                   .IsRectangular());
}

TEST(QuadTest, IsRectangularOnPointLikeQuads) {
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.0f)
                  .IsRectangular());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {0.0f, 0.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 2.0f)
                   .IsRectangular());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {40.0f, -25.5f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0)
                  .IsRectangular());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, -25.5f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.5f)
                   .IsRectangular());
}

TEST(QuadTest, IsAxisAlignedOnNonRectangularQuads) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, 25.0f}, -20.0f, 4.0f, Angle::Radians(0.0f), 3.0f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 5.0f, -3.0f, Angle::Radians(0.0f), -.05f)
                   .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedOnNonRightAngleRotatedQuads) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {40.0f, 25.0f}, -20.0f, 4.0f, Angle::Radians(0.1f), 0.0f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 5.0f, -3.0f, Angle::Radians(-5.0f), 0.0f)
                   .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedOnRectangularRightAngleRotatedQuads) {
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {40.0f, 25.0f}, -20.0f, 4.0f, Angle::Radians(0.0f), 0.0f)
                  .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 20.0f, 4.0f, kFullTurn, 0.0f)
                  .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 20.0f, 4.0f, kQuarterTurn, 0.0f)
                  .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 20.0f, 4.0f, kQuarterTurn * 30, 0.0f)
                  .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedOnLineLikeQuads) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 30.0f, 0.0f, Angle::Radians(0.1f), -.05f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 30.0f, 0.0f, Angle::Radians(0.1f), 0.0f)
                   .IsAxisAligned());

  // This is considered non-rectangular because it has a non-zero shear factor.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 30.0f, 0.0f, Angle::Radians(0.0f), 0.1f)
                   .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 30.0f, 0.0f, Angle::Radians(0.0f), 0.0f)
                  .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 30.0f, Angle::Radians(0.1f), -.05f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 30.0f, Angle::Radians(0.1f), 0.0f)
                   .IsAxisAligned());

  // This is considered non-rectangular because it has a non-zero shear factor.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 30.0f, Angle::Radians(0.0f), 0.1f)
                   .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 0.0f, 30.0f, Angle::Radians(0.0f), 0.0f)
                  .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedOnPointLikeQuads) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 0.0f, Angle::Radians(0.1f), -.05f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 0.0f, Angle::Radians(0.1f), 0.0f)
                   .IsAxisAligned());

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.1f)
                   .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 0.0f, 0.0f, Angle::Radians(0.0f), 0.0f)
                  .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedToleranceDefault) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(11e-6), 0.0f)
                   .IsAxisAligned());

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(10e-6), 0.0f)
                  .IsAxisAligned());
}

TEST(QuadTest, IsAxisAlignedToleranceSet) {
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(9e-5), 0.0f)
                  .IsAxisAligned(Angle::Radians(1e-4)));

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(2e-4), 0.0f)
                   .IsAxisAligned(Angle::Radians(1e-4)));

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(1e-9), 0.0f)
                   .IsAxisAligned(Angle::Radians(0.0f)));

  // A tolerance >= 1/8 turn makes the rotation check always pass.
  for (int i = 0; i < 100; i++) {
    EXPECT_TRUE(
        Quad::FromCenterDimensionsRotationAndShear(
            {-40.0f, -25.0f}, 3.0f, 4.0f, Angle::Radians(0.05f * i), 0.0f)
            .IsAxisAligned(kFullTurn / 8));
  }
}

TEST(QuadTest, SignedAreaOnVariousQuads) {
  EXPECT_FLOAT_EQ(
      Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 20.0f, 10.0f,
                                                 Angle::Radians(2.0f), 2.0f)
          .SignedArea(),
      200.0f);

  EXPECT_FLOAT_EQ(
      Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 2.0f, -50.0f,
                                                 Angle::Radians(2.0f), 2.0f)
          .SignedArea(),
      -100.0f);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, -4.0f, 3.0f, Angle::Radians(2.0f), 2.0f)
                      .SignedArea(),
                  -12.0f);

  EXPECT_FLOAT_EQ(
      Quad::FromCenterDimensionsRotationAndShear(
          {-40.0f, -25.0f}, -5.0f, -30.0f, Angle::Radians(2.0f), 2.0f)
          .SignedArea(),
      150.0f);
}

TEST(QuadTest, SignedAreaOnFlatQuads) {
  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 20.0f, 0.0f, Angle::Radians(2.0f), 2.0f)
                      .SignedArea(),
                  0);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 0.0f, 20.0f, Angle::Radians(2.0f), 2.0f)
                      .SignedArea(),
                  0);
}

TEST(QuadTest, SignedAreaOnPointLikeQuad) {
  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 0.0f, 0.0f, Angle::Radians(2.0f), 2.0f)
                      .SignedArea(),
                  0);
}

TEST(QuadTest, AspectRationOnVariousQuads) {
  EXPECT_FLOAT_EQ(
      Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 20.0f, 10.0f,
                                                 Angle::Radians(2.0f), 2.0f)
          .AspectRatio(),
      2.0f);

  EXPECT_FLOAT_EQ(
      Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 10.0f, 10.0f,
                                                 Angle::Radians(2.0f), 2.0f)
          .AspectRatio(),
      1.0f);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 5.0f, 10.0f, Angle::Radians(2.0f), 2.0f)
                      .AspectRatio(),
                  0.5f);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 0.0f, 10.0f, Angle::Radians(2.0f), 2.0f)
                      .AspectRatio(),
                  0.0f);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 2.0f, 0.1f, Angle::Radians(2.0f), 2.0f)
                      .AspectRatio(),
                  20.0f);

  EXPECT_FLOAT_EQ(Quad::FromCenterDimensionsRotationAndShear(
                      {-40.0f, -25.0f}, 2.0f, -0.1f, Angle::Radians(2.0f), 2.0f)
                      .AspectRatio(),
                  -20.0f);
}

TEST(QuadDeathTest, AspectRatioFailsWithZeroHeight) {
  EXPECT_DEATH_IF_SUPPORTED(
      (Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 15.0f, 0.0f,
                                                  Angle::Radians(2.0f), 2.0f)
           .AspectRatio()),
      "height");

  EXPECT_DEATH_IF_SUPPORTED(
      (Quad::FromCenterDimensionsRotationAndShear({0.0f, 0.0f}, 0.0f, 0.0f,
                                                  Angle::Radians(0.0f), 0.0f)
           .AspectRatio()),
      "height");
}

TEST(QuadTest, Corners) {
  std::array<Point, 4> test_corners_1 =
      Quad::FromCenterDimensionsRotationAndShear({-40.0f, -25.0f}, 10.0f, 16.0f,
                                                 kFullTurn, 1.0f)
          .Corners();

  EXPECT_THAT(test_corners_1[0], PointEq({-53.0f, -33.0f}));
  EXPECT_THAT(test_corners_1[1], PointEq({-43.0f, -33.0f}));
  EXPECT_THAT(test_corners_1[2], PointEq({-27.0f, -17.0f}));
  EXPECT_THAT(test_corners_1[3], PointEq({-37.0f, -17.0f}));

  std::array<Point, 4> test_corners_2 =
      Quad::FromCenterDimensionsRotationAndShear({0.0f, 0.0f}, 20.0f, 4.0f,
                                                 kQuarterTurn, 2.0f)
          .Corners();

  EXPECT_THAT(test_corners_2[0], PointEq({2.0f, -14.0f}));
  EXPECT_THAT(test_corners_2[1], PointEq({2.0f, 6.0f}));
  EXPECT_THAT(test_corners_2[2], PointEq({-2.0f, 14.0f}));
  EXPECT_THAT(test_corners_2[3], PointEq({-2.0f, -6.0f}));
}

TEST(QuadTest, GetEdge) {
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f);

  EXPECT_THAT(test_quad_1.GetEdge(0),
              SegmentEq(Segment{{-53.0f, -33.0f}, {-43.0f, -33.0f}}));
  EXPECT_THAT(test_quad_1.GetEdge(1),
              SegmentEq(Segment{{-43.0f, -33.0f}, {-27.0f, -17.0f}}));
  EXPECT_THAT(test_quad_1.GetEdge(2),
              SegmentEq(Segment{{-27.0f, -17.0f}, {-37.0f, -17.0f}}));
  EXPECT_THAT(test_quad_1.GetEdge(3),
              SegmentEq(Segment{{-37.0f, -17.0f}, {-53.0f, -33.0f}}));

  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f);

  EXPECT_THAT(test_quad_2.GetEdge(0),
              SegmentEq(Segment{{2.0f, -14.0f}, {2.0f, 6.0f}}));
  EXPECT_THAT(test_quad_2.GetEdge(1),
              SegmentEq(Segment{{2.0f, 6.0f}, {-2.0f, 14.0f}}));
  EXPECT_THAT(test_quad_2.GetEdge(2),
              SegmentEq(Segment{{-2.0f, 14.0f}, {-2.0f, -6.0f}}));
  EXPECT_THAT(test_quad_2.GetEdge(3),
              SegmentEq(Segment{{-2.0f, -6.0f}, {2.0f, -14.0f}}));
}

TEST(QuadDeathTest, GetEdge) {
  EXPECT_DEATH_IF_SUPPORTED(Quad::FromCenterDimensionsRotationAndShear(
                                {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)
                                .GetEdge(4),
                            "out of bounds");
  EXPECT_DEATH_IF_SUPPORTED(Quad::FromCenterDimensionsRotationAndShear(
                                {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)
                                .GetEdge(12),
                            "out of bounds");
  EXPECT_DEATH_IF_SUPPORTED(Quad::FromCenterDimensionsRotationAndShear(
                                {-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f)
                                .GetEdge(-1),
                            "out of bounds");
}

TEST(QuadTest, JoinForInteriorPointOnUprightQuad) {
  // Inside of the Quad.
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_1.Join({12.0f, 14.0f});
  EXPECT_THAT(test_quad_1, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f)));
  // On the edge of the Quad.
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_2.Join({5.0f, 14.0f});
  EXPECT_THAT(test_quad_2, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f)));
  // At the corner of the Quad.
  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_3.Join({15.0f, 18.0f});
  EXPECT_THAT(test_quad_3, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f)));
}

TEST(QuadTest, JoinForExteriorPointOnUprightQuad) {
  // Below (smaller y value) the Quad.
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_1.Join({7.0f, -2.0f});
  EXPECT_THAT(test_quad_1, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 8.0f}, 10.0f, 20.0f, Angle(), 0.0f)));
  // To the Left (smaller x value) of the Quad.
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_2.Join({0.0f, 14.0f});
  EXPECT_THAT(test_quad_2, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {7.5f, 10.0f}, 15.0f, 16.0f, Angle(), 0.0f)));
  // Above (larger y value) & to the right (larger x value) of the Quad.
  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 10.0f, 16.0f, Angle(), 0.0f);
  test_quad_3.Join({20.0f, 22.0f});
  EXPECT_THAT(test_quad_3, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {12.5f, 12.0f}, 15.0f, 20.0f, Angle(), 0.0f)));
}

TEST(QuadTest, JoinForInteriorPointOnSlantedRotatedQuad) {
  // Inside the Quad.
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_1.Join({-46.0f, -16.0f});
  EXPECT_THAT(test_quad_1,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)));
  // On the side of the Quad.
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_2.Join({-36.0f, -34.0f});
  EXPECT_THAT(test_quad_2,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)));
  // At the corner of the Quad.
  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_3.Join({-48.0f, -22.0f});
  EXPECT_THAT(test_quad_3,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)));
}

TEST(QuadTest, JoinForExteriorPointOnSlantedRotatedQuad) {
  // Below relative to the center and rotation of the Quad.
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_1.Join({-28.0f, -38.0f});
  EXPECT_THAT(test_quad_1,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-38.0f, -27.0f}, 10.0f, 20.0f, kQuarterTurn, 1.0f)));
  // To the Left relative to the center and rotation of the Quad.
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_2.Join({-44.0f, -36.0f});
  EXPECT_THAT(test_quad_2,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -30.0f}, 20.0f, 16.0f, kQuarterTurn, 1.0f)));
  // Above & to the right relative to the center and rotation of the Quad.
  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f);
  test_quad_3.Join({-50.0f, -6.0f});
  EXPECT_THAT(test_quad_3,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-41.0f, -22.0f}, 14.0f, 18.0f, kQuarterTurn, 1.0f)));
  // Quad with 2 points in Q1, center in Q2, two points in Q3, joining a point
  // in Q4.
  Quad test_quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {-10.0f, 2.0f}, 10.0f, 25.0f, kQuarterTurn + kHalfTurn, -2.0f);
  test_quad_4.Join({20.0f, -20.0f});
  EXPECT_THAT(test_quad_4, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {-1.25f, -19.0f}, 87.0f, 42.5f,
                               kQuarterTurn + kHalfTurn, -2.0f)));
}

TEST(QuadTest, JoinForInteriorPointOnLineLikeQuad) {
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 8.0f, 0.0f, Angle(), 0.0f);
  test_quad_1.Join({12.0f, 10.0f});
  EXPECT_THAT(test_quad_1, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 10.0f}, 8.0f, 0.0f, Angle(), 0.0f)));

  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 0.0f, 16.0f, Angle(), 0.0f);
  test_quad_2.Join({10.0f, 14.0f});
  EXPECT_THAT(test_quad_2, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 10.0f}, 0.0f, 16.0f, Angle(), 0.0f)));

  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 8.0f, 0.0f, kQuarterTurn, 0.0f);
  test_quad_3.Join({10.0f, 14.0f});
  EXPECT_THAT(test_quad_3,
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 10.0f}, 8.0f, 0.0f, kQuarterTurn, 0.0f),
                       0.001));

  Quad test_quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 0.0f, 16.0f, kQuarterTurn, 0.0f);
  test_quad_4.Join({18.0f, 10.0f});
  EXPECT_THAT(test_quad_4,
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 10.0f}, 0.0f, 16.0f, kQuarterTurn, 0.0f),
                       0.001));
}

TEST(QuadTest, JoinForExteriorPointOnLineLikeQuad) {
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 8.0f, 0.0f, Angle(), 0.0f);
  test_quad_1.Join({12.0f, 14.0f});
  EXPECT_THAT(test_quad_1, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {10.0f, 12.0f}, 8.0f, 4.0f, Angle(), 0.0f)));

  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 0.0f, 16.0f, Angle(), 0.0f);
  test_quad_2.Join({12.0f, 14.0f});
  EXPECT_THAT(test_quad_2, QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                               {11.0f, 10.0f}, 2.0f, 16.0f, Angle(), 0.0f)));

  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 8.0f, 0.0f, kQuarterTurn, 0.0f);
  test_quad_3.Join({15.0f, 14.0f});
  EXPECT_THAT(test_quad_3,
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {12.5f, 10.0f}, 8.0f, 5.0f, kQuarterTurn, 0.0f),
                       0.001));

  Quad test_quad_4 = Quad::FromCenterDimensionsRotationAndShear(
      {10.0f, 10.0f}, 0.0f, 16.0f, kQuarterTurn, 0.0f);
  test_quad_4.Join({18.0f, 4.0f});
  EXPECT_THAT(test_quad_4,
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 7.0f}, 6.0f, 16.0f, kQuarterTurn, 0.0f),
                       0.001));
}

TEST(QuadTest, JoinPointOnPointLikeQuad) {
  // Contained Point.
  Quad test_quad_1 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f);
  test_quad_1.Join({-40.0f, -25.0f});
  EXPECT_THAT(test_quad_1,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f)));
  // To the Bottom Left relative to the center and rotation of the Quad.
  Quad test_quad_2 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f);
  test_quad_2.Join({-32.0f, -38.0f});
  EXPECT_THAT(test_quad_2,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-36.0f, -31.5f}, 5.0f, 8.0f, kQuarterTurn, 1.0f)));
  // To the Upper Right relative to the center and rotation of the Quad.
  Quad test_quad_3 = Quad::FromCenterDimensionsRotationAndShear(
      {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f);
  test_quad_3.Join({-48.0f, -12.0f});
  EXPECT_THAT(test_quad_3,
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-44.0f, -18.5f}, 5.0f, 8.0f, kQuarterTurn, 1.0f)));
}

TEST(QuadTest, ContainsForInteriorPointOnUprightQuad) {
  // Inside of the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                         16.0f, Angle(), 0.0f)
                  .Contains({12.0f, 14.0f}));

  // On the edge of the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                         16.0f, Angle(), 0.0f)
                  .Contains({5.0f, 14.0f}));

  // At the corner of the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                         16.0f, Angle(), 0.0f)
                  .Contains({15.0f, 18.0f}));
}

TEST(QuadTest, ContainsForExteriorPointOnUprightQuad) {
  // Below (smaller y value) the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                          16.0f, Angle(), 0.0f)
                   .Contains({7.0f, -2.0f}));

  // To the Left (smaller x value) of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                          16.0f, Angle(), 0.0f)
                   .Contains({0.0f, 14.0f}));

  // Above (larger y value) & to the right (larger x value) of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 10.0f,
                                                          16.0f, Angle(), 0.0f)
                   .Contains({20.0f, 22.0f}));
}

TEST(QuadTest, ContainsForInteriorPointOnSlantedRotatedQuad) {
  // Inside the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                  .Contains({-46.0f, -16.0f}));

  // On the side of the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                  .Contains({-36.01f, -33.98f}));

  // At the corner of the Quad.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                  .Contains({-48.0f, -22.0f}));
}

TEST(QuadTest, ContainsForExteriorPointOnSlantedRotatedQuad) {
  // Below relative to the center and rotation of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                   .Contains({-28.0f, -38.0f}));

  // To the Left relative to the center and rotation of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                   .Contains({-44.0f, -36.0f}));

  // Above & to the right relative to the center and rotation of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 10.0f, 16.0f, kQuarterTurn, 1.0f)
                   .Contains({-50.0f, -6.0f}));

  // A Quad with 2 points in Q1, center in Q2, 2 points in Q3, and checking
  // containment with a point in Q4.
  EXPECT_FALSE(
      Quad::FromCenterDimensionsRotationAndShear(
          {-10.0f, 2.0f}, 10.0f, 25.0f, kQuarterTurn + kHalfTurn, -2.0f)
          .Contains({20.0f, -20.0f}));
}

TEST(QuadTest, ContainsForInteriorPointOnLineLikeQuad) {
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 8.0f,
                                                         0.0f, Angle(), 0.0f)
                  .Contains({12.0f, 10.0f}));

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 0.0f,
                                                         16.0f, Angle(), 0.0f)
                  .Contains({10.0f, 14.0f}));

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {10.0f, 10.0f}, 8.0f, 0.0f, kQuarterTurn, 0.0f)
                  .Contains({10.0f, 10.0f}));

  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {10.0f, 10.0f}, 0.0f, 16.0f, kQuarterTurn, 0.0f)
                  .Contains({10.0f, 10.0f}));
}

TEST(QuadTest, ContainsForExteriorPointOnLineLikeQuad) {
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 8.0f,
                                                          0.0f, Angle(), 0.0f)
                   .Contains({12.0f, 14.0f}));

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear({10.0f, 10.0f}, 0.0f,
                                                          16.0f, Angle(), 0.0f)
                   .Contains({12.0f, 14.0f}));

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {10.0f, 10.0f}, 8.0f, 0.0f, kQuarterTurn, 0.0f)
                   .Contains({15.0f, 14.0f}));

  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {10.0f, 10.0f}, 0.0f, 16.0f, kQuarterTurn, 0.0f)
                   .Contains({18.0f, 4.0f}));
}

TEST(QuadTest, ContainsPointOnPointLikeQuad) {
  // Contained Point.
  EXPECT_TRUE(Quad::FromCenterDimensionsRotationAndShear(
                  {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f)
                  .Contains({-40.0f, -25.0f}));

  // To the Bottom Left relative to the center and rotation of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f)
                   .Contains({-32.0f, -38.0f}));

  // To the Upper Right relative to the center and rotation of the Quad.
  EXPECT_FALSE(Quad::FromCenterDimensionsRotationAndShear(
                   {-40.0f, -25.0f}, 0.0f, 0.0f, kQuarterTurn, 1.0f)
                   .Contains({-48.0f, -12.0f}));
}

}  // namespace
}  // namespace ink
