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

#include "ink/geometry/affine_transform.h"

#include <array>
#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

using ::testing::Optional;

TEST(AffineTransformTest, Stringify) {
  EXPECT_EQ(absl::StrCat(AffineTransform::Identity()),
            "AffineTransform(1, 0, 0, 0, 1, 0)");
  EXPECT_EQ(absl::StrCat(AffineTransform::Scale(12, -345)),
            "AffineTransform(12, 0, 0, 0, -345, 0)");
  EXPECT_EQ(absl::StrCat(AffineTransform::Translate({1.5, -7})),
            "AffineTransform(1, 0, 1.5, 0, 1, -7)");
  EXPECT_EQ(absl::StrCat(AffineTransform(1, 23, 4, 56, 7, 89)),
            "AffineTransform(1, 23, 4, 56, 7, 89)");
}

TEST(AffineTransformTest, DefaultConstructorCreatesIdentityMatrix) {
  AffineTransform default_affine_transform;
  EXPECT_FLOAT_EQ(default_affine_transform.A(), 1);
  EXPECT_FLOAT_EQ(default_affine_transform.B(), 0);
  EXPECT_FLOAT_EQ(default_affine_transform.C(), 0);
  EXPECT_FLOAT_EQ(default_affine_transform.D(), 0);
  EXPECT_FLOAT_EQ(default_affine_transform.E(), 1);
  EXPECT_FLOAT_EQ(default_affine_transform.F(), 0);
}

TEST(AffineTransformTest, Getters) {
  AffineTransform test_affine_transform(1.0f, 12.4f, 0.0f, -4.3f, 9999,
                                        0.0002f);
  EXPECT_FLOAT_EQ(test_affine_transform.A(), 1.0f);
  EXPECT_FLOAT_EQ(test_affine_transform.B(), 12.4f);
  EXPECT_FLOAT_EQ(test_affine_transform.C(), 0.0f);
  EXPECT_FLOAT_EQ(test_affine_transform.D(), -4.3f);
  EXPECT_FLOAT_EQ(test_affine_transform.E(), 9999);
  EXPECT_FLOAT_EQ(test_affine_transform.F(), 0.0002f);
}

TEST(AffineTransformTest, AffineTransformEq) {
  AffineTransform test_affine_transform(1.0f, 12.4f, 0.0f, -4.3f, 9999,
                                        0.0002f);
  EXPECT_THAT(test_affine_transform,
              AffineTransformEq(
                  AffineTransform(1.0f, 12.4f, 0.0f, -4.3f, 9999, 0.0002f)));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.1f, 12.4f, 0.0f, -4.3f, 9999, 0.0002f))));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.0f, -12.4f, 0.0f, -4.3f, 9999, 0.0002f))));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.0f, 12.4f, 0.001f, -4.3f, 9999, 0.0002f))));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.0f, 12.4f, 0.0f, -43.0f, 9999, 0.0002f))));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.0f, 12.4f, 0.0f, -4.3f, 10000, 0.0002f))));
  EXPECT_THAT(test_affine_transform,
              Not(AffineTransformEq(
                  AffineTransform(1.0f, 12.4f, 0.0f, -4.3f, 9999, 0.0f))));
  EXPECT_THAT(AffineTransform(), AffineTransformEq(AffineTransform(
                                     1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
}

TEST(AffineTransformTest, IdentityTransform) {
  EXPECT_THAT(
      AffineTransform::Identity(),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
}

TEST(AffineTransformTest, Translate) {
  EXPECT_THAT(AffineTransform::Translate(Vec{0, 0}),
              AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::Translate(Vec{3, 4}),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 3.0f, 0.0f, 1.0f, 4.0f)));
}

TEST(AffineTransformTest, ScaleBothDimenisonsByOneValue) {
  EXPECT_THAT(
      AffineTransform::Scale(0.0f),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Scale(-43.2f),
              AffineTransformEq(
                  AffineTransform(-43.2f, 0.0f, 0.0f, 0.0f, -43.2f, 0.0f)));
}

TEST(AffineTransformTest, ScaleEachDimenisonIndependently) {
  EXPECT_THAT(
      AffineTransform::Scale(0.0f, 0.0f),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Scale(-21.1f, 7.89f),
              AffineTransformEq(
                  AffineTransform(-21.1f, 0.0f, 0.0f, 0.0f, 7.89f, 0.0f)));
}

TEST(AffineTransformTest, ScaleX) {
  EXPECT_THAT(
      AffineTransform::ScaleX(0.0f),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleX(0.077f),
      AffineTransformEq(AffineTransform(0.077f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
}

TEST(AffineTransformTest, ScaleY) {
  EXPECT_THAT(
      AffineTransform::ScaleY(0.0f),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleY(600.6f),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 0.0f, 600.6f, 0.0f)));
}

TEST(AffineTransformTest, Rotate) {
  EXPECT_THAT(AffineTransform::Rotate(Angle()),
              AffineTransformEq(AffineTransform::Identity()));
  Angle test_angle = Angle::Radians(2);
  float sin = Sin(test_angle);
  float cos = Cos(test_angle);
  EXPECT_THAT(
      AffineTransform::Rotate(test_angle),
      AffineTransformEq(AffineTransform(cos, -sin, 0.0f, sin, cos, 0.0f)));
}

TEST(AffineTransformTest, ShearX) {
  EXPECT_THAT(AffineTransform::ShearX(0.0f),
              AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::ShearX(2.2f),
      AffineTransformEq(AffineTransform(1.0f, 2.2f, 0.0f, 0.0f, 1.0f, 0.0f)));
}

TEST(AffineTransformTest, ShearY) {
  EXPECT_THAT(AffineTransform::ShearY(0.0f),
              AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::ShearY(7.0f),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 7.0f, 1.0f, 0.0f)));
}

TEST(AffineTransformTest, ScaleAboutPointByOneValue) {
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 6.0f}),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 6.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(13.0f, {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(13.0f, 0.0f, 0.0f, 0.0f, 13.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleAboutPoint(13.0f, {4.0f, 6.0f}),
              AffineTransformEq(
                  AffineTransform(13.0f, 0.0f, -48.0f, 0.0f, 13.0f, -72.0f)));
}

void ScaleAboutPointByOneValueEquivalence(float scale_factor, Point center) {
  EXPECT_THAT(AffineTransform::ScaleAboutPoint(scale_factor, center),
              AffineTransformEq(AffineTransform::Translate(center - kOrigin) *
                                AffineTransform::Scale(scale_factor) *
                                AffineTransform::Translate(kOrigin - center)));
}
FUZZ_TEST(AffineTransformTest, ScaleAboutPointByOneValueEquivalence)
    .WithDomains(fuzztest::Finite<float>(), FinitePoint());

TEST(AffineTransformTest, ScaleAboutPointByTwoValues) {
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, 0.0f, {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, 0.0f, {4.0f, 6.0f}),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 6.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(8.0f, 13.0f, {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(8.0f, 0.0f, 0.0f, 0.0f, 13.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleAboutPoint(8.0f, 13.0f, {4.0f, 6.0f}),
              AffineTransformEq(
                  AffineTransform(8.0f, 0.0f, -28.0f, 0.0f, 13.0f, -72.0f)));
}

void ScaleAboutPointByTwoValuesEquivalence(float x_scale_factor,
                                           float y_scale_factor, Point center) {
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(x_scale_factor, y_scale_factor, center),
      AffineTransformEq(AffineTransform::Translate(center - kOrigin) *
                        AffineTransform::Scale(x_scale_factor, y_scale_factor) *
                        AffineTransform::Translate(kOrigin - center)));
}
FUZZ_TEST(AffineTransformTest, ScaleAboutPointByTwoValuesEquivalence)
    .WithDomains(fuzztest::Finite<float>(), fuzztest::Finite<float>(),
                 FinitePoint());

TEST(AffineTransformTest, RotateAboutPoint) {
  Angle test_angle = Angle::Radians(2);
  float sin = Sin(test_angle);
  float cos = Cos(test_angle);
  EXPECT_THAT(
      AffineTransform::RotateAboutPoint(Angle::Radians(0.0f), {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::RotateAboutPoint(Angle::Radians(0.0f), {4.0f, 6.0f}),
      AffineTransformEq(AffineTransform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::RotateAboutPoint(test_angle, {0.0f, 0.0f}),
      AffineTransformEq(AffineTransform(cos, -sin, 0.0f, sin, cos, 0.0f)));
  EXPECT_THAT(
      AffineTransform::RotateAboutPoint(test_angle, {4.0f, 6.0f}),
      AffineTransformEq(AffineTransform(cos, -sin, -4 * cos + 6 * sin + 4.0f,
                                        sin, cos, -4 * sin - 6 * cos + 6.0f)));
}

TEST(AffineTransformTest, Inverse) {
  EXPECT_THAT(AffineTransform::Identity().Inverse(),
              Optional(AffineTransformEq(AffineTransform::Identity())));
  EXPECT_THAT(AffineTransform::Scale(4, 10).Inverse(),
              Optional(AffineTransformEq(AffineTransform::Scale(0.25, 0.1))));
  EXPECT_THAT(AffineTransform::Rotate(Angle::Degrees(30)).Inverse(),
              Optional(AffineTransformEq(
                  AffineTransform::Rotate(Angle::Degrees(-30)))));
  EXPECT_THAT(
      AffineTransform::Translate({5, 10}).Inverse(),
      Optional(AffineTransformEq(AffineTransform::Translate({-5, -10}))));
  EXPECT_THAT(AffineTransform::ShearX(5).Inverse(),
              Optional(AffineTransformEq(AffineTransform::ShearX(-5))));
}

TEST(AffineTransformTest, CannotFindInverse) {
  EXPECT_EQ(AffineTransform(0, 0, 0, 0, 0, 0).Inverse(), std::nullopt);
  EXPECT_EQ(AffineTransform::ScaleX(0).Inverse(), std::nullopt);
  EXPECT_EQ(AffineTransform::ScaleY(0).Inverse(), std::nullopt);
}

TEST(AffineTransformTest, ApplyPoint) {
  Point test_point = {4.0f, 6.0f};

  EXPECT_THAT(AffineTransform::Identity().Apply(test_point),
              PointEq({4.0f, 6.0f}));
  EXPECT_THAT(AffineTransform::Translate(Vec{3.0f, -20.0f}).Apply(test_point),
              PointEq({7.0f, -14.0f}));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_point),
              PointEq({10.0f, 6.0f}));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_point),
              PointEq({4.0f, 15.0f}));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_point),
              PointEq({10.0f, 15.0f}));
  EXPECT_THAT(AffineTransform::Scale(2.5f, -.5f).Apply(test_point),
              PointEq({10.0f, -3.0f}));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_point),
              PointEq({19.0f, 6.0f}));
  EXPECT_THAT(AffineTransform::ShearY(2.5f).Apply(test_point),
              PointEq({4.0f, 16.0f}));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_point),
              PointEq({-4.0f, -6.0f}));
}

TEST(AffineTransformTest, ApplySegment) {
  Segment test_segment = Segment{{4.0f, 6.0f}, {40.0f, 60.0f}};

  EXPECT_THAT(AffineTransform::Identity().Apply(test_segment),
              SegmentEq(Segment{{4.0f, 6.0f}, {40.0f, 60.0f}}));
  EXPECT_THAT(AffineTransform::Translate(Vec{3.0f, -20.0f}).Apply(test_segment),
              SegmentEq(Segment{{7.0f, -14.0f}, {43.0f, 40.0f}}));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_segment),
              SegmentEq(Segment{{10.0f, 6.0f}, {100.0f, 60.0f}}));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_segment),
              SegmentEq(Segment{{4.0f, 15.0f}, {40.0f, 150.0f}}));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_segment),
              SegmentEq(Segment{{10.0f, 15.0f}, {100.0f, 150.0f}}));
  EXPECT_THAT(AffineTransform::Scale(2.5f, -.5f).Apply(test_segment),
              SegmentEq(Segment{{10.0f, -3.0f}, {100.0f, -30.0f}}));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_segment),
              SegmentEq(Segment{{19.0f, 6.0f}, {190.0f, 60.0f}}));
  EXPECT_THAT(AffineTransform::ShearY(2.5f).Apply(test_segment),
              SegmentEq(Segment{{4.0f, 16.0f}, {40.0f, 160.0f}}));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_segment),
              SegmentEq(Segment{{-4.0f, -6.0f}, {-40.0f, -60.0f}}));
  EXPECT_THAT(AffineTransform::ScaleAboutPoint(2.0f, {22.0f, 33.0f})
                  .Apply(test_segment),
              SegmentEq(Segment{{-14.0f, -21.0f}, {58.0f, 87.0f}}));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {22.0f, 33.0f})
                  .Apply(test_segment),
              SegmentEq(Segment{{49.0f, 15.0f}, {-5.0f, 51.0f}}));
}

TEST(AffineTransformTest, ApplyTriangle) {
  Triangle test_triangle =
      Triangle{{1.0f, 2.0f}, {6.0f, -3.0f}, {-4.0f, -6.0f}};

  EXPECT_THAT(
      AffineTransform::Identity().Apply(test_triangle),
      TriangleEq(Triangle{{1.0f, 2.0f}, {6.0f, -3.0f}, {-4.0f, -6.0f}}));
  EXPECT_THAT(
      AffineTransform::Translate(Vec{3.0f, -20.0f}).Apply(test_triangle),
      TriangleEq(Triangle{{4.0f, -18.0f}, {9.0f, -23.0f}, {-1.0f, -26.0f}}));
  EXPECT_THAT(
      AffineTransform::ScaleX(2.5f).Apply(test_triangle),
      TriangleEq(Triangle{{2.5f, 2.0f}, {15.0f, -3.0f}, {-10.0f, -6.0f}}));
  EXPECT_THAT(
      AffineTransform::ScaleY(2.5f).Apply(test_triangle),
      TriangleEq(Triangle{{1.0f, 5.0f}, {6.0f, -7.5f}, {-4.0f, -15.0f}}));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f).Apply(test_triangle),
      TriangleEq(Triangle{{2.5f, 5.0f}, {15.0f, -7.5f}, {-10.0f, -15.0f}}));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_triangle),
      TriangleEq(Triangle{{2.5f, -1.0f}, {15.0f, 1.5f}, {-10.0f, 3.0f}}));
  EXPECT_THAT(
      AffineTransform::ShearX(2.5f).Apply(test_triangle),
      TriangleEq(Triangle{{6.0f, 2.0f}, {-1.5f, -3.0f}, {-19.0f, -6.0f}}));
  EXPECT_THAT(
      AffineTransform::ShearY(2.5f).Apply(test_triangle),
      TriangleEq(Triangle{{1.0f, 4.5f}, {6.0f, 12.0f}, {-4.0f, -16.0f}}));
  EXPECT_THAT(
      AffineTransform::Rotate(kHalfTurn).Apply(test_triangle),
      TriangleEq(Triangle{{-1.0f, -2.0f}, {-6.0f, 3.0f}, {4.0f, 6.0f}}));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {0.0f, 0.0f}).Apply(test_triangle),
      TriangleEq(Triangle{{2.0f, 4.0f}, {12.0f, -6.0f}, {-8.0f, -12.0f}}));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {0.0f, 0.0f})
                  .Apply(test_triangle),
              TriangleEq(Triangle{{-2.0f, 1.0f}, {3.0f, 6.0f}, {6.0f, -4.0f}}));
}

TEST(AffineTransformTest, ApplyRect) {
  Rect test_rect = Rect::FromCenterAndDimensions({4.0f, 1.0f}, 6.0f, 8.0f);
  EXPECT_THAT(AffineTransform::Identity().Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({4.0f, 1.0f}, 6.0f, 8.0f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({0.0f, 0.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({5.0f, 4.0f}, 6.0f, 8.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleX(2.5f).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({10.0f, 1.0f}, 15.0f, 8.0f)));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({4.0f, 2.5f}, 6.0f, 20.0f)));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({10.0f, 2.5f}, 15.0f, 20.0f)));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({10.0f, -0.5f}, 15.0f, -4.0f)));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {6.5f, 1.0f}, 6.0f, 8.0f, Angle(), 2.5f)));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation({-4.0f, -1.0f}, 6.0f,
                                                           8.0f, kHalfTurn)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {4.0f, 1.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({4.0f, 1.0f}, 12.0f, 16.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 1.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({4.0f, 1.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {4.0f, 1.0f})
                  .Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation(
                  {4.0f, 1.0f}, 6.0f, 8.0f, kQuarterTurn)));
  std::array<Point, 4> test_corners =
      AffineTransform::ShearY(2.5f).Apply(test_rect).Corners();
  EXPECT_THAT(test_corners[0], PointEq({1.0f, -0.5f}));
  EXPECT_THAT(test_corners[1], PointEq({7.0f, 14.5f}));
  EXPECT_THAT(test_corners[2], PointEq({7.0f, 22.5f}));
  EXPECT_THAT(test_corners[3], PointEq({1.0f, 7.5f}));
}

TEST(AffineTransformTest, ApplyZeroWidthRect) {
  Rect test_rect = Rect::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 8.0f);

  EXPECT_THAT(AffineTransform::Identity().Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 8.0f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({0.0f, 0.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.0f, 5.0f}, 0.0f, 8.0f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 2.0f}, 0.0f, 8.0f)));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 5.0f}, 0.0f, 20.0f)));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 5.0f}, 0.0f, 20.0f)));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({2.5f, -1.0f}, 0.0f, -4.0f)));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {6.0f, 2.0f}, 0.0f, 8.0f, Angle(), 2.5f)));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation({-1.0f, -2.0f}, 0.0f,
                                                           8.0f, kHalfTurn)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 16.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {1.0f, 2.0f})
                  .Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation(
                  {1.0f, 2.0f}, 0.0f, 8.0f, kQuarterTurn)));
  std::array<Point, 4> test_corners =
      AffineTransform::ShearY(2.5f).Apply(test_rect).Corners();
  EXPECT_THAT(test_corners[0], PointEq({1.0f, 0.5f}));
  EXPECT_THAT(test_corners[1], PointEq({1.0f, 0.5f}));
  EXPECT_THAT(test_corners[2], PointEq({1.0f, 8.5f}));
  EXPECT_THAT(test_corners[3], PointEq({1.0f, 8.5f}));
}

TEST(AffineTransformTest, ApplyZeroHeightRect) {
  Rect test_rect = Rect::FromCenterAndDimensions({1.0f, 2.0f}, 8.0f, 0.0f);
  EXPECT_THAT(AffineTransform::Identity().Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 8.0f, 0.0f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({0.0f, 0.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.0f, 5.0f}, 8.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 2.0f}, 20.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 5.0f}, 8.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 5.0f}, 20.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({2.5f, -1.0f}, 20.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {6.0f, 2.0f}, 8.0f, 0.0f, Angle(), 2.5f)));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation({-1.0f, -2.0f}, 8.0f,
                                                           0.0f, kHalfTurn)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 16.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {1.0f, 2.0f})
                  .Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation(
                  {1.0f, 2.0f}, 8.0f, 0.0f, kQuarterTurn)));
  std::array<Point, 4> test_corners =
      AffineTransform::ShearY(2.5f).Apply(test_rect).Corners();
  EXPECT_THAT(test_corners[0], PointEq({-3.0f, -5.5f}));
  EXPECT_THAT(test_corners[1], PointEq({5.0f, 14.5f}));
  EXPECT_THAT(test_corners[2], PointEq({5.0f, 14.5f}));
  EXPECT_THAT(test_corners[3], PointEq({-3.0f, -5.5f}));
}

TEST(AffineTransformTest, ApplyPointLikeRect) {
  Rect test_rect = Rect::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f);
  EXPECT_THAT(AffineTransform::Identity().Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({0.0f, 0.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.0f, 5.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({1.0f, 5.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, 5.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::Scale(2.5f, -.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterAndDimensions({2.5f, -1.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::ShearX(2.5f).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {6.0f, 2.0f}, 0.0f, 0.0f, Angle(), 2.5f)));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation({-1.0f, -2.0f}, 0.0f,
                                                           0.0f, kHalfTurn)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {1.0f, 2.0f}).Apply(test_rect),
      QuadEq(Quad::FromCenterAndDimensions({1.0f, 2.0f}, 0.0f, 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {1.0f, 2.0f})
                  .Apply(test_rect),
              QuadEq(Quad::FromCenterDimensionsAndRotation(
                  {1.0f, 2.0f}, 0.0f, 0.0f, kQuarterTurn)));
  std::array<Point, 4> test_corners =
      AffineTransform::ShearY(2.5f).Apply(test_rect).Corners();
  EXPECT_THAT(test_corners[0], PointEq({1.0f, 4.5f}));
  EXPECT_THAT(test_corners[1], PointEq({1.0f, 4.5f}));
  EXPECT_THAT(test_corners[2], PointEq({1.0f, 4.5f}));
  EXPECT_THAT(test_corners[3], PointEq({1.0f, 4.5f}));
}

TEST(AffineTransformTest, ApplyQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 1.0f}, 6.0f, 8.0f, kQuarterTurn, 0.5f);
  std::array<Point, 4> starting_corners =
      AffineTransform::Identity().Apply(test_quad).Corners();
  EXPECT_THAT(starting_corners[0], PointNear({8.0f, -4.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[1], PointNear({8.0f, 2.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[2], PointNear({0.0f, 6.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[3], PointNear({0.0f, 0.0f}, 0.0001f));
  EXPECT_THAT(AffineTransform::Identity().Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 6.0f, 8.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {5.0f, 4.0f}, 6.0f, 8.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 1.0f}, 6.0f, 20.0f, kQuarterTurn, 0.2f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {4.0f, 2.5f}, 15.0f, 8.0f, kQuarterTurn, 1.25f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 2.5f}, 15.0f, 20.0f, kQuarterTurn, 0.5f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f, -.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, -0.5f}, 3.0f, -20.0f,
                           kQuarterTurn + kHalfTurn, -0.1f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-4.0f, -1.0f}, 6.0f, 8.0f, kQuarterTurn + kHalfTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear(
          {4.0f, 1.0f}, 12.0f, 16.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear({4.0f, 1.0f}, 0.0f,
                                                        0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {4.0f, 1.0f})
                  .Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 6.0f, 8.0f, kHalfTurn, 0.5f)));
  std::array<Point, 4> shearx_corners =
      AffineTransform::ShearX(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(shearx_corners[0], PointNear({-2.0f, -4.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[1], PointNear({13.0f, 2.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[2], PointNear({15.0f, 6.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[3], PointNear({0.0f, 0.0f}, 0.0001f));
  std::array<Point, 4> sheary_corners =
      AffineTransform::ShearY(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(sheary_corners[0], PointNear({8.0f, 16.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[1], PointNear({8.0f, 22.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[2], PointNear({0.0f, 6.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[3], PointNear({0.0f, 0.0f}, 0.0001f));
}

TEST(AffineTransformTest, ApplyZeroWidthQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 1.0f}, 0.0f, 8.0f, kQuarterTurn, 0.5f);
  std::array<Point, 4> starting_corners =
      AffineTransform::Identity().Apply(test_quad).Corners();
  EXPECT_THAT(starting_corners[0], PointNear({8.0f, -1.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[1], PointNear({8.0f, -1.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[2], PointNear({0.0f, 3.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[3], PointNear({0.0f, 3.0f}, 0.0001f));
  EXPECT_THAT(AffineTransform::Identity().Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 0.0f, 8.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {5.0f, 4.0f}, 0.0f, 8.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 1.0f}, 0.0f, 20.0f, kQuarterTurn, 0.2f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {4.0f, 2.5f}, 0.0f, 8.0f, kQuarterTurn, 1.25f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 2.5f}, 0.0f, 20.0f, kQuarterTurn, 0.5f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f, -.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, -0.5f}, 0.0f, -20.0f,
                           kQuarterTurn + kHalfTurn, -0.1f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-4.0f, -1.0f}, 0.0f, 8.0f, kQuarterTurn + kHalfTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear(
          {4.0f, 1.0f}, 0.0f, 16.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear({4.0f, 1.0f}, 0.0f,
                                                        0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {4.0f, 1.0f})
                  .Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 0.0f, 8.0f, kHalfTurn, 0.5f)));
  std::array<Point, 4> shearx_corners =
      AffineTransform::ShearX(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(shearx_corners[0], PointNear({5.5f, -1.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[1], PointNear({5.5f, -1.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[2], PointNear({7.5f, 3.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[3], PointNear({7.5f, 3.0f}, 0.0001f));
  std::array<Point, 4> sheary_corners =
      AffineTransform::ShearY(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(sheary_corners[0], PointNear({8.0f, 19.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[1], PointNear({8.0f, 19.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[2], PointNear({0.0f, 3.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[3], PointNear({0.0f, 3.0f}, 0.0001f));
}

TEST(AffineTransformTest, ApplyZeroHeightQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 1.0f}, 6.0f, 0.0f, kQuarterTurn, 0.5f);
  std::array<Point, 4> starting_corners =
      AffineTransform::Identity().Apply(test_quad).Corners();
  EXPECT_THAT(starting_corners[0], PointNear({4.0f, -2.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[1], PointNear({4.0f, 4.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[2], PointNear({4.0f, 4.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[3], PointNear({4.0f, -2.0f}, 0.0001f));
  EXPECT_THAT(AffineTransform::Identity().Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 6.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {5.0f, 4.0f}, 6.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 1.0f}, 6.0f, 0.0f, kQuarterTurn, 0.2f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {4.0f, 2.5f}, 15.0f, 0.0f, kQuarterTurn, 1.25f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 2.5f}, 15.0f, 0.0f, kQuarterTurn, 0.5f),
                       0.0001f));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_quad),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {10.0f, -0.5f}, 3.0f, 0.0f, kQuarterTurn + kHalfTurn, -0.1f),
               0.0001f));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-4.0f, -1.0f}, 6.0f, 0.0f, kQuarterTurn + kHalfTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear(
          {4.0f, 1.0f}, 12.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear({4.0f, 1.0f}, 0.0f,
                                                        0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {4.0f, 1.0f})
                  .Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 6.0f, 0.0f, kHalfTurn, 0.5f)));
  std::array<Point, 4> shearx_corners =
      AffineTransform::ShearX(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(shearx_corners[0], PointNear({-1.0f, -2.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[1], PointNear({14.0f, 4.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[2], PointNear({14.0f, 4.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[3], PointNear({-1.0f, -2.0f}, 0.0001f));
  std::array<Point, 4> sheary_corners =
      AffineTransform::ShearY(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(sheary_corners[0], PointNear({4.0f, 8.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[1], PointNear({4.0f, 14.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[2], PointNear({4.0f, 14.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[3], PointNear({4.0f, 8.0f}, 0.0001f));
}

TEST(AffineTransformTest, ApplyPointLikeQuad) {
  Quad test_quad = Quad::FromCenterDimensionsRotationAndShear(
      {4.0f, 1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.5f);
  std::array<Point, 4> starting_corners =
      AffineTransform::Identity().Apply(test_quad).Corners();
  EXPECT_THAT(starting_corners[0], PointNear({4.0f, 1.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[1], PointNear({4.0f, 1.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[2], PointNear({4.0f, 1.0f}, 0.0001f));
  EXPECT_THAT(starting_corners[3], PointNear({4.0f, 1.0f}, 0.0001f));
  EXPECT_THAT(AffineTransform::Identity().Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform(0, 0, 0, 0, 0, 0).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {0.0f, 0.0f}, 0.0f, 0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::Translate(Vec{1.0f, 3.0f}).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {5.0f, 4.0f}, 0.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(AffineTransform::ScaleX(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.2f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::ScaleY(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {4.0f, 2.5f}, 0.0f, 0.0f, kQuarterTurn, 1.25f),
                       0.0001f));
  EXPECT_THAT(AffineTransform::Scale(2.5f).Apply(test_quad),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {10.0f, 2.5f}, 0.0f, 0.0f, kQuarterTurn, 0.5f),
                       0.0001f));
  EXPECT_THAT(
      AffineTransform::Scale(2.5f, -.5f).Apply(test_quad),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {10.0f, -0.5f}, 0.0f, 0.0f, kQuarterTurn + kHalfTurn, -0.1f),
               0.0001f));
  EXPECT_THAT(AffineTransform::Rotate(kHalfTurn).Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {-4.0f, -1.0f}, 0.0f, 0.0f, kQuarterTurn + kHalfTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(2.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear(
          {4.0f, 1.0f}, 0.0f, 0.0f, kQuarterTurn, 0.5f)));
  EXPECT_THAT(
      AffineTransform::ScaleAboutPoint(0.0f, {4.0f, 1.0f}).Apply(test_quad),
      QuadEq(Quad::FromCenterDimensionsRotationAndShear({4.0f, 1.0f}, 0.0f,
                                                        0.0f, Angle(), 0.0f)));
  EXPECT_THAT(AffineTransform::RotateAboutPoint(kQuarterTurn, {4.0f, 1.0f})
                  .Apply(test_quad),
              QuadEq(Quad::FromCenterDimensionsRotationAndShear(
                  {4.0f, 1.0f}, 0.0f, 0.0f, kHalfTurn, 0.5f)));
  std::array<Point, 4> shearx_corners =
      AffineTransform::ShearX(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(shearx_corners[0], PointNear({6.5f, 1.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[1], PointNear({6.5f, 1.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[2], PointNear({6.5f, 1.0f}, 0.0001f));
  EXPECT_THAT(shearx_corners[3], PointNear({6.5f, 1.0f}, 0.0001f));
  std::array<Point, 4> sheary_corners =
      AffineTransform::ShearY(2.5f).Apply(test_quad).Corners();
  EXPECT_THAT(sheary_corners[0], PointNear({4.0f, 11.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[1], PointNear({4.0f, 11.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[2], PointNear({4.0f, 11.0f}, 0.0001f));
  EXPECT_THAT(sheary_corners[3], PointNear({4.0f, 11.0f}, 0.0001f));
}

TEST(AffineTransformTest, FindSegmentSimpleTransforms) {
  // Unmoved Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{0, 0}, {2, 3}})
          .value(),
      AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{0, 0}, {2, 3}})
          .value()
          .Apply(Segment{{0, 0}, {2, 3}}),
      SegmentNear(Segment{{0, 0}, {2, 3}}, 0.0001f));

  // Translate Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{3, -12}, {5, -9}})
          .value(),
      AffineTransformEq(AffineTransform::Translate(Vec{3, -12})));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{3, -12}, {5, -9}})
          .value()
          .Apply(Segment{{0, 0}, {2, 3}}),
      SegmentNear(Segment{{3, -12}, {5, -9}}, 0.0001f));

  // Rotate Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{2, 3}, {0, 0}})
          .value(),
      AffineTransformNear(
          AffineTransform::RotateAboutPoint(kHalfTurn, {1, 1.5}), 0.0001f));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{2, 3}, {0, 0}})
          .value()
          .Apply(Segment{{0, 0}, {2, 3}}),
      SegmentNear(Segment{{2, 3}, {0, 0}}, 0.0001f));

  // Scale Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{3, 4}, {7, 6}}, Segment{{1, 3}, {9, 7}})
          .value(),
      AffineTransformNear(AffineTransform::ScaleAboutPoint(2.0f, {5, 5}),
                          0.01f));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{3, 4}, {7, 6}}, Segment{{1, 3}, {9, 7}})
          .value()
          .Apply(Segment{{3, 4}, {7, 6}}),
      SegmentNear(Segment{{1, 3}, {9, 7}}, 0.01f));
}

TEST(AffineTransformTest, FindSegmentComplexTransforms) {
  // Rotate and Translate Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{3, 4}, {1, 1}})
          .value(),
      AffineTransformNear(
          AffineTransform::RotateAboutPoint(kHalfTurn, {2, 2.5}) *
              AffineTransform::Translate(Vec{1, 1}),
          0.0001f));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}}, Segment{{3, 4}, {1, 1}})
          .value()
          .Apply(Segment{{0, 0}, {2, 3}}),
      SegmentNear(Segment{{3, 4}, {1, 1}}, 0.0001f));

  // Rotate and Scale Segment
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{4, -0.5}, {-2, 3.5}})
                  .value(),
              AffineTransformNear(
                  AffineTransform::RotateAboutPoint(kQuarterTurn, {1, 1.5}) *
                      AffineTransform::ScaleAboutPoint(2, {1, 1.5}),
                  0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{4, -0.5}, {-2, 3.5}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{4, -0.5}, {-2, 3.5}}, 0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{-2, 3.5}, {4, -0.5}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{-2, 3.5}, {4, -0.5}}, 0.0001f));

  // Due to the way the transforms are calculated, the transform found for a
  // segment that has had it's direction flipped will contain a rotation
  // transform of π, rather than using a scaling transform with a negative
  // scaling factor. The scaling factor used in the scaling transform can never
  // be negative.
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{3, 4.5}, {-1, -1.5}})
                  .value(),
              AffineTransformNear(
                  AffineTransform::RotateAboutPoint(kHalfTurn, {1, 1.5}) *
                      AffineTransform::ScaleAboutPoint(2, {1, 1.5}),
                  0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{3, 4.5}, {-1, -1.5}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{3, 4.5}, {-1, -1.5}}, 0.0001f));

  // Translate and Scale Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                            Segment{{-3, -30.5}, {4, -20}})
          .value(),
      AffineTransformNear(AffineTransform::ScaleAboutPoint(3.5, {0.5, -25.25}) *
                              AffineTransform::Translate(Vec{-.5, -26.75}),
                          0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{-3, -30.5}, {4, -20}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{-3, -30.5}, {4, -20}}, 0.0001f));

  // Rotate, Scale, and translate Segment
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                            Segment{{16.5, -9.5}, {10.5, -5.5}})
          .value(),
      AffineTransformNear(
          AffineTransform::RotateAboutPoint(kQuarterTurn, {13.5, -7.5}) *
              AffineTransform::ScaleAboutPoint(2, {13.5, -7.5}) *
              AffineTransform::Translate(Vec{12.5, -9}),
          0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{16.5, -9.5}, {10.5, -5.5}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{16.5, -9.5}, {10.5, -5.5}}, 0.0001f));
  EXPECT_THAT(AffineTransform::Find(Segment{{0, 0}, {2, 3}},
                                    Segment{{10.5, -5.5}, {16.5, -9.5}})
                  .value()
                  .Apply(Segment{{0, 0}, {2, 3}}),
              SegmentNear(Segment{{10.5, -5.5}, {16.5, -9.5}}, 0.0001f));
}

TEST(AffineTransformTest, FindZeroLengthSegment) {
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {0, 0}}, Segment{{0, 0}, {0, 0}})
          .value(),
      AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{0, 0}, {0, 0}}, Segment{{1, 1}, {1, 1}})
          .value(),
      AffineTransformEq(AffineTransform::Translate(Vec{1, 1})));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{1, 1}, {2, 2}}, Segment{{0, 0}, {0, 0}})
          .value(),
      AffineTransformEq(AffineTransform::ScaleAboutPoint(0, {0, 0})));
  EXPECT_THAT(
      AffineTransform::Find(Segment{{1, 1}, {2, 2}}, Segment{{5, 5}, {5, 5}})
          .value(),
      AffineTransformNear(AffineTransform::ScaleAboutPoint(0, {5, 5}) *
                              AffineTransform::Translate(Vec{3.5, 3.5}),
                          0.0001f));
  EXPECT_FALSE(
      AffineTransform::Find(Segment{{0, 0}, {0, 0}}, Segment{{1, 1}, {2, 2}})
          .has_value());
}

TEST(AffineTransformTest, FindTriangleComplexTransforms) {
  EXPECT_THAT(AffineTransform::Find(Triangle{{0, 0}, {4, 2}, {3, 5}},
                                    Triangle{{-12, 3}, {-2, -7}, {22, 50}})
                  .value()
                  .Apply(Triangle{{0, 0}, {4, 2}, {3, 5}}),
              TriangleNear(Triangle{{-12, 3}, {-2, -7}, {22, 50}}, 0.0001f));
  EXPECT_THAT(AffineTransform::Find(Triangle{{8, 9}, {4, 2}, {3, 5}},
                                    Triangle{{8, 9}, {-5, 53}, {64, -.5}})
                  .value()
                  .Apply(Triangle{{8, 9}, {4, 2}, {3, 5}}),
              TriangleNear(Triangle{{8, 9}, {-5, 53}, {64, -.5}}, 0.0001f));
  EXPECT_THAT(AffineTransform::Find(Triangle{{8, 9}, {4, 2}, {3, 5}},
                                    Triangle{{8, 9}, {4, 2}, {64, -.5}})
                  .value()
                  .Apply(Triangle{{8, 9}, {4, 2}, {3, 5}}),
              TriangleNear(Triangle{{8, 9}, {4, 2}, {64, -.5}}, 0.0001f));
  // You can go from non-degenerate triangles to degenerate triangles, not the
  // other way around.
  EXPECT_THAT(AffineTransform::Find(Triangle{{8, 9}, {4, 2}, {3, 5}},
                                    Triangle{{3, 3}, {3, 3}, {3, 3}})
                  .value()
                  .Apply(Triangle{{8, 9}, {4, 2}, {3, 5}}),
              TriangleNear(Triangle{{3, 3}, {3, 3}, {3, 3}}, 0.0001f));
  EXPECT_THAT(AffineTransform::Find(Triangle{{8, 9}, {4, 2}, {3, 5}},
                                    Triangle{{3, 3}, {3, 3}, {450, 229}})
                  .value()
                  .Apply(Triangle{{8, 9}, {4, 2}, {3, 5}}),
              TriangleNear(Triangle{{3, 3}, {3, 3}, {450, 229}}, 0.0001f));
}

TEST(AffineTransformTest, FindTriangleInvalid) {
  // It is not possible to find a transform when your starting triangle has an
  // area of 0.
  // These Triangles have an area of 0 because their points are collinear.
  EXPECT_FALSE(AffineTransform::Find(Triangle{{1, 1}, {1, 2}, {1, 3}},
                                     Triangle{{8, 9}, {4, 2}, {64, -.5}})
                   .has_value());
  EXPECT_FALSE(AffineTransform::Find(Triangle{{0, 0}, {1, 12}, {2, 24}},
                                     Triangle{{8, 9}, {4, 2}, {64, -.5}})
                   .has_value());
  // A Triangle with 2 identical points will always have an area of 0.
  EXPECT_FALSE(AffineTransform::Find(Triangle{{1, 1}, {1, 1}, {15, 23}},
                                     Triangle{{8, 9}, {4, 2}, {64, -.5}})
                   .has_value());
  EXPECT_FALSE(AffineTransform::Find(Triangle{{1, 1}, {1, 1}, {15, 23}},
                                     Triangle{{2, 2}, {2, 2}, {16, 24}})
                   .has_value());
  // A point like triangle will always fail to find a transform.
  EXPECT_FALSE(AffineTransform::Find(Triangle{{1, 1}, {1, 1}, {1, 1}},
                                     Triangle{{2, 2}, {2, 2}, {2, 2}})
                   .has_value());
  EXPECT_FALSE(AffineTransform::Find(Triangle{{1, 1}, {1, 1}, {1, 1}},
                                     Triangle{{1, 1}, {1, 1}, {1, 1}})
                   .has_value());
}

TEST(AffineTransformTest, FindTriangleSimpleTransforms) {
  // Identity
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{1, 1}, {4, 1}, {1, 5}}),
              Optional(AffineTransformEq(AffineTransform::Identity())));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{1, 1}, {4, 1}, {1, 5}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{1, 1}, {4, 1}, {1, 5}}, 0.0001f));
  // Translate
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{11, 3}, {14, 3}, {11, 7}}),
              Optional(AffineTransformNear(
                  AffineTransform::Translate(Vec{10, 2}), 0.0001f)));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{11, 3}, {14, 3}, {11, 7}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{11, 3}, {14, 3}, {11, 7}}, 0.0001f));
  // Rotate
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{-1, 1}, {-1, 4}, {-5, 1}}),
              Optional(AffineTransformNear(
                  AffineTransform::Rotate(kQuarterTurn), 0.0001f)));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{-1, 1}, {-1, 4}, {-5, 1}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{-1, 1}, {-1, 4}, {-5, 1}}, 0.0001f));
  // Scale
  EXPECT_THAT(
      AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                            Triangle{{3, 3}, {12, 3}, {3, 15}}),
      Optional(AffineTransformNear(AffineTransform::Scale(3), 0.0001f)));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{3, 3}, {12, 3}, {3, 15}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{3, 3}, {12, 3}, {3, 15}}, 0.0001f));
  // ShearX
  EXPECT_THAT(
      AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                            Triangle{{3, 1}, {6, 1}, {11, 5}}),
      Optional(AffineTransformNear(AffineTransform::ShearX(2), 0.0001f)));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{3, 1}, {6, 1}, {11, 5}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{3, 1}, {6, 1}, {11, 5}}, 0.0001f));
  // ShearY
  EXPECT_THAT(
      AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                            Triangle{{1, 3}, {4, 9}, {1, 7}}),
      Optional(AffineTransformNear(AffineTransform::ShearY(2), 0.0001f)));
  EXPECT_THAT(AffineTransform::Find(Triangle{{1, 1}, {4, 1}, {1, 5}},
                                    Triangle{{1, 3}, {4, 9}, {1, 7}})
                  .value()
                  .Apply(Triangle{{1, 1}, {4, 1}, {1, 5}}),
              TriangleNear(Triangle{{1, 3}, {4, 9}, {1, 7}}, 0.0001f));
}

TEST(AffineTransformTest, FindRectComplexTransforms) {
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({-30, -5}, 6, 170))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({-30, -5}, 6, 170)),
               0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({0, 0}, 55.8, 3.55))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(
          Quad::FromRect(Rect::FromCenterAndDimensions({0, 0}, 55.8, 3.55)),
          0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({-30, -5}, 0, 0))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({-30, -5}, 0, 0)),
               0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({0, 0}, 0, 0))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({0, 0}, 0, 0)),
               0.001f));
}

TEST(AffineTransformTest, FindRectInvalid) {
  // A Transform cannot be found for a rect with a 0 for its height, a 0 for its
  // width, or a 0 for both dimensions.
  EXPECT_FALSE(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 0, 50),
                            Rect::FromCenterAndDimensions({40, 60}, 0, 50))
          .has_value());
  EXPECT_FALSE(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 0),
                            Rect::FromCenterAndDimensions({40, 60}, 30, 0))
          .has_value());
  EXPECT_FALSE(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 0, 0),
                            Rect::FromCenterAndDimensions({40, 60}, 0, 0))
          .has_value());
}

TEST(AffineTransformTest, FindRectSimpleTransforms) {
  // Identity
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({40, 60}, 30, 50))
          .value(),
      AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({40, 60}, 30, 50))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
               0.001f));
  // Translate
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({45, 65}, 30, 50))
          .value(),
      AffineTransformEq(AffineTransform::Translate(Vec{5, 5})));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({40, 60}, 30, 50),
                            Rect::FromCenterAndDimensions({45, 65}, 30, 50))
          .value()
          .Apply(Rect::FromCenterAndDimensions({40, 60}, 30, 50)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({45, 65}, 30, 50)),
               0.001f));
  // Scale by 1 value
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({30, 60}, 30, 90))
          .value(),
      AffineTransformEq(AffineTransform::Scale(3)));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({30, 60}, 30, 90))
          .value()
          .Apply(Rect::FromCenterAndDimensions({10, 20}, 10, 30)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({30, 60}, 30, 90)),
               0.001f));
  // Scale by 2 values
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({30, 40}, 30, 60))
          .value(),
      AffineTransformEq(AffineTransform::Scale(3, 2)));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({30, 40}, 30, 60))
          .value()
          .Apply(Rect::FromCenterAndDimensions({10, 20}, 10, 30)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({30, 40}, 30, 60)),
               0.001f));
  // ScaleAboutPoint by 1 value
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({10, 20}, 5, 15))
          .value(),
      AffineTransformEq(AffineTransform::ScaleAboutPoint(.5f, {10, 20})));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({10, 20}, 5, 15))
          .value()
          .Apply(Rect::FromCenterAndDimensions({10, 20}, 10, 30)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({10, 20}, 5, 15)),
               0.001f));
  // ScaleAboutPoint by 2 values
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({10, 20}, 40, 180))
          .value(),
      AffineTransformEq(AffineTransform::ScaleAboutPoint(4, 6, {10, 20})));
  EXPECT_THAT(
      AffineTransform::Find(Rect::FromCenterAndDimensions({10, 20}, 10, 30),
                            Rect::FromCenterAndDimensions({10, 20}, 40, 180))
          .value()
          .Apply(Rect::FromCenterAndDimensions({10, 20}, 10, 30)),
      QuadNear(Quad::FromRect(Rect::FromCenterAndDimensions({10, 20}, 40, 180)),
               0.001f));
}

TEST(AffineTransformTest, FindQuadComplexTransforms) {
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {-30, -5}, 6, 170, Angle::Radians(1.5f), 1.3f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {-30, -5}, 6, 170, Angle::Radians(1.5f), 1.3f),
               0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {17, -436}, .5, 224, kQuarterTurn, -0.7f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {17, -436}, .5, 224, kQuarterTurn, -0.7f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.01f));
}

TEST(AffineTransformTest, FindQuadInvalid) {
  // A Transform cannot be found for a Quad with a 0 for its height, a 0 for its
  // width, or a 0 for both dimensions.
  EXPECT_FALSE(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 0, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 0, 50, Angle::Radians(2.5f), 2.5f))
          .has_value());
  EXPECT_FALSE(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 0, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 0, Angle::Radians(2.5f), 2.5f))
          .has_value());
  EXPECT_FALSE(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 0, 0, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 0, 0, Angle::Radians(2.5f), 2.5f))
          .has_value());
}

TEST(AffineTransformTest, FindQuadSimpleTransforms) {
  // Identity
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value(),
      AffineTransformEq(AffineTransform::Identity()));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.001f));
  EXPECT_THAT(AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                        {40, 60}, 30, 50, Angle(), 2.5f),
                                    Quad::FromCenterDimensionsRotationAndShear(
                                        {40, 60}, 30, 50, kFullTurn, 2.5f))
                  .value(),
              AffineTransformEq(AffineTransform::Identity()));
  // Translate
  EXPECT_THAT(
      AffineTransform::Find(
          Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
          Quad::FromCenterDimensionsRotationAndShear(
              {-550, 2.3}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value(),
      AffineTransformNear(AffineTransform::Translate({-590, -57.7}), 0.001f));
  EXPECT_THAT(AffineTransform::Find(
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                  Quad::FromCenterDimensionsRotationAndShear(
                      {-550, 2.3}, 30, 50, Angle::Radians(2.5f), 2.5f))
                  .value()
                  .Apply(Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {-550, 2.3}, 30, 50, Angle::Radians(2.5f), 2.5f),
                       0.001f));
  // Scale by 1 value
  EXPECT_THAT(AffineTransform::Find(
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                  Quad::FromCenterDimensionsRotationAndShear(
                      {-20, -30}, -15, -25, Angle::Radians(2.5f), 2.5f))
                  .value(),
              AffineTransformNear(AffineTransform::Scale(-.5), 0.001f));
  EXPECT_THAT(AffineTransform::Find(
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                  Quad::FromCenterDimensionsRotationAndShear(
                      {-20, -30}, -15, -25, Angle::Radians(2.5f), 2.5f))
                  .value()
                  .Apply(Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {-20, -30}, -15, -25, Angle::Radians(2.5f), 2.5f),
                       0.001f));
  // Scale by 2 values
  EXPECT_THAT(AffineTransform::Find(
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                  Quad::FromCenterDimensionsRotationAndShear(
                      {12, -120}, 9, -100, Angle::Radians(2.5f), 2.5f))
                  .value()
                  .Apply(Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {12, -120}, 9, -100, Angle::Radians(2.5f), 2.5f),
                       0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, -60}, 30, 50, kQuarterTurn, 0.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {-600, -6}, -450, 5, kQuarterTurn, 0.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, -60}, 30, 50, kQuarterTurn, 0.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear({-600, -6}, -450, 5,
                                                          kQuarterTurn, 0.5f),
               0.001f));
  // Rotate
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle(), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value(),
      AffineTransformNear(
          AffineTransform::RotateAboutPoint(Angle::Radians(2.5f), {40, 60}),
          0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle(), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear({40, 60}, 30, 50,
                                                            Angle(), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(1.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(1.5f), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.001f));
  // Shear
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 0.0f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 0.0f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 1.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 1.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
               0.001f));
  // ScaleAboutPoint by 1 value
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, -3, -5, Angle::Radians(2.5f), 2.5f))
          .value(),
      AffineTransformNear(AffineTransform::ScaleAboutPoint(-.1f, {40, 60}),
                          0.001f));
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, -3, -5, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, -3, -5, Angle::Radians(2.5f), 2.5f),
               0.001f));
  // ScaleAboutPoint by 2 values
  EXPECT_THAT(
      AffineTransform::Find(Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                            Quad::FromCenterDimensionsRotationAndShear(
                                {40, 60}, -3, -5, Angle::Radians(2.5f), 2.5f))
          .value()
          .Apply(Quad::FromCenterDimensionsRotationAndShear(
              {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
      QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                   {40, 60}, -3, -5, Angle::Radians(2.5f), 2.5f),
               0.001f));
  EXPECT_THAT(AffineTransform::Find(
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f),
                  Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 532, -608, Angle::Radians(2.5f), 2.5f))
                  .value()
                  .Apply(Quad::FromCenterDimensionsRotationAndShear(
                      {40, 60}, 30, 50, Angle::Radians(2.5f), 2.5f)),
              QuadNear(Quad::FromCenterDimensionsRotationAndShear(
                           {40, 60}, 532, -608, Angle::Radians(2.5f), 2.5f),
                       0.001f));
}

TEST(AffineTransformTest, Multiplication) {
  EXPECT_THAT(
      AffineTransform(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 6.0f) *
          AffineTransform(13.0f, 0.0f, 0.0f, 0.0f, 13.0f, 0.0f),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 6.0f)));
  EXPECT_THAT(
      AffineTransform(13.0f, 0.0f, 0.0f, 0.0f, 13.0f, 0.0f) *
          AffineTransform(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 6.0f),
      AffineTransformEq(AffineTransform(0.0f, 0.0f, 52.0f, 0.0f, 0.0f, 78.0f)));
  EXPECT_THAT(AffineTransform(2.0f, -5.0f, 4.0f, 3.0f, 9.0f, -6.0f) *
                  AffineTransform(11.0f, 17.0f, -7.0f, -8.0f, 14.0f, 19.0f),
              AffineTransformEq(AffineTransform(62.0f, -36.0f, -105.0f, -39.0f,
                                                177.0f, 144.0f)));
}

}  // namespace
}  // namespace ink
