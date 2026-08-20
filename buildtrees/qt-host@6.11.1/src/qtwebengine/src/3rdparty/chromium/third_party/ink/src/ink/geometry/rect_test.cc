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

#include "ink/geometry/rect.h"

#include <array>
#include <cmath>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/point.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

using ::testing::NanSensitiveFloatEq;

TEST(RectTest, Stringify) {
  EXPECT_EQ(absl::StrCat(Rect()), "Rect[0 by 0 from (0, 0) to (0, 0)]");
  EXPECT_EQ(absl::StrCat(Rect::FromTwoPoints({-1, 2}, {3, 4})),
            "Rect[4 by 2 from (-1, 2) to (3, 4)]");
  EXPECT_EQ(absl::StrCat(Rect::FromCenterAndDimensions({5.5, -7.5}, 8, 6)),
            "Rect[8 by 6 from (1.5, -10.5) to (9.5, -4.5)]");
  EXPECT_EQ(absl::StrCat(Rect::FromTwoPoints(
                {-std::numeric_limits<float>::infinity(), 0},
                {0, std::numeric_limits<float>::infinity()})),
            "Rect[inf by inf from (-inf, 0) to (0, inf)]");
}

TEST(RectTest, DefaultConstructor) {
  Rect default_rect;
  EXPECT_FLOAT_EQ(default_rect.XMin(), 0);
  EXPECT_FLOAT_EQ(default_rect.YMin(), 0);
  EXPECT_FLOAT_EQ(default_rect.XMax(), 0);
  EXPECT_FLOAT_EQ(default_rect.YMax(), 0);
}

TEST(RectTest, FromTwoPoints) {
  // Input Points are the bottom left corner and the top right corner.
  Rect test_rect_1 = Rect::FromTwoPoints({1, 3}, {4, 6});
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), 1);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), 3);
  EXPECT_FLOAT_EQ(test_rect_1.XMax(), 4);
  EXPECT_FLOAT_EQ(test_rect_1.YMax(), 6);

  // Input Points are the top left corner and the bottom right corner.
  Rect test_rect_2 = Rect::FromTwoPoints({-23, 47}, {10, -1});
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), -23);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -1);
  EXPECT_FLOAT_EQ(test_rect_2.XMax(), 10);
  EXPECT_FLOAT_EQ(test_rect_2.YMax(), 47);

  // Input Points are the same.
  Rect test_rect_3 = Rect::FromTwoPoints({4.7, 8}, {4.7, 8});
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 4.7);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), 8);
  EXPECT_FLOAT_EQ(test_rect_3.XMax(), 4.7);
  EXPECT_FLOAT_EQ(test_rect_3.YMax(), 8);
}

TEST(RectTest, FromCenterAndDimensions) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({1, 2.5}, 4, 8);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), -1);
  EXPECT_FLOAT_EQ(test_rect_1.XMax(), 3);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -1.5);
  EXPECT_FLOAT_EQ(test_rect_1.YMax(), 6.5);

  // Dimensions are a float and 0
  Rect test_rect_2 = Rect::FromCenterAndDimensions({-3.3, 8}, 0, 4.24);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), -3.3);
  EXPECT_FLOAT_EQ(test_rect_2.XMax(), -3.3);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), 5.88);
  EXPECT_FLOAT_EQ(test_rect_2.YMax(), 10.12);
}

TEST(RectDeathTest, FromCenterAndDimensions) {
  EXPECT_DEATH_IF_SUPPORTED(
      (Rect::FromCenterAndDimensions({3, 3}, -4, 4)),
      "Cannot construct a rectangle with negative width or height");
  EXPECT_DEATH_IF_SUPPORTED(
      (Rect::FromCenterAndDimensions({-3, -3}, 4, -.0004)),
      "Cannot construct a rectangle with negative width or height");
  EXPECT_DEATH_IF_SUPPORTED(
      (Rect::FromCenterAndDimensions({0, 0}, -0, -4)),
      "Cannot construct a rectangle with negative width or height");
}

TEST(RectTest, Getters) {
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).XMin(), .35);
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).YMin(), -2.8);
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).XMax(), 1);
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).YMax(), 588);
}

TEST(RectTest, EqMatcher) {
  EXPECT_THAT((Rect::FromTwoPoints({1, -2.8}, {.35, 588})),
              RectEq(.35, -2.8, 1, 588));
  EXPECT_THAT((Rect::FromTwoPoints({4.7, 8}, {4.7, 8})),
              RectEq(4.7, 8, 4.7, 8));

  // RectEq delegates to FloatEq, which uses a tolerance of 4 ULP.
  constexpr float kEps = std::numeric_limits<float>::epsilon();

  EXPECT_THAT((Rect::FromTwoPoints({1, 1}, {1, 1})),
              RectEq(1 + 4 * kEps, 1 - 2 * kEps, 1 - kEps, 1 + kEps));
}

TEST(RectTest, NearMatcher) {
  EXPECT_THAT((Rect::FromTwoPoints({1, -2.8}, {.35, 588})),
              RectNear(.35, -2.8, 1, 588, .5));
  EXPECT_THAT((Rect::FromTwoPoints({1, -2.8}, {.35, 588})),
              RectNear(.85, -2.3, .5, 587.5, .5));
}

TEST(RectTest, EqMatcherRectExpected) {
  EXPECT_THAT((Rect::FromTwoPoints({1, -2.8}, {.4, 2.8})),
              RectEq(Rect::FromCenterAndDimensions({.7, 0}, 0.6, 5.6)));
  EXPECT_THAT((Rect::FromTwoPoints({4.7, 8}, {4.7, 8})),
              RectEq(Rect::FromCenterAndDimensions({4.7, 8}, 0, 0)));
}

TEST(RectTest, NearMatcherRectExpected) {
  EXPECT_THAT(
      (Rect::FromTwoPoints({1, -2.8}, {.35, 588})),
      RectNear(Rect::FromCenterAndDimensions({0.525, 292.6}, .65, 590.8), .5));
  EXPECT_THAT(
      (Rect::FromTwoPoints({1, -2.8}, {.35, 588})),
      RectNear(Rect::FromCenterAndDimensions({0.675, 292.4}, .65, 590.8), .5));
}

TEST(RectTest, Center) {
  EXPECT_THAT(Rect::FromTwoPoints({1, -2.8}, {8, 10}).Center(),
              PointEq({4.5, 3.6}));
  EXPECT_THAT(Rect::FromTwoPoints({-8.9992, 0}, {8.9992, 0}).Center(),
              PointEq({0, 0}));
  EXPECT_THAT(Rect::FromCenterAndDimensions({1.45, -900}, 20, 5.896).Center(),
              PointEq({1.45, -900}));

  constexpr float inf = std::numeric_limits<float>::infinity();
  EXPECT_THAT(Rect::FromTwoPoints({0, 0}, {inf, inf}).Center(),
              PointEq({inf, inf}));
  EXPECT_THAT(Rect::FromTwoPoints({-inf, -inf}, {0, 0}).Center(),
              PointEq({-inf, -inf}));
  EXPECT_THAT(Rect::FromTwoPoints({-inf, -inf}, {inf, inf}).Center(),
              PointEq({0, 0}));
}

void RectContainsItsCenter(Rect rect) {
  EXPECT_TRUE(rect.Contains(rect.Center()))
      << "Where rect is: " << testing::PrintToString(rect)
      << "\nAnd rect.Center() is: " << testing::PrintToString(rect.Center());
}
FUZZ_TEST(RectTest, RectContainsItsCenter).WithDomains(NotNanRect());

TEST(RectTest, SetCenter) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({1, 2}, 20, 8);
  test_rect_1.SetCenter({5, 35.89});
  EXPECT_THAT(test_rect_1.Center(), PointEq({5, 35.89}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 20);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 8);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({0, -8.5}, 0, 2.5);
  test_rect_2.SetCenter({-3.5, 77});
  EXPECT_THAT(test_rect_2.Center(), PointEq({-3.5, 77}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 2.5);
}

TEST(RectTest, Width) {
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).Width(), .65);
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({0, 10}, {-8.9, 20.2}).Width(), 8.9);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 0, 2.5).Width(), 0);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 1, 2.5).Width(), 1);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 3.89, 2.5).Width(),
                  3.89);
}

TEST(RectTest, SetWidth) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({1, 2}, 20, 8);
  test_rect_1.SetWidth(35.89);
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 35.89);
  EXPECT_THAT(test_rect_1.Center(), PointEq({1, 2}));

  Rect test_rect_2 = Rect::FromCenterAndDimensions({0, -8.5}, 0, 2.5);
  test_rect_2.SetWidth(.25);
  EXPECT_FLOAT_EQ(test_rect_2.Width(), .25);
  EXPECT_THAT(test_rect_2.Center(), PointEq({0, -8.5}));

  Rect test_rect_3 = Rect::FromCenterAndDimensions({30, -8.5}, 99, 2.5);
  test_rect_3.SetWidth(0);
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 0);
  EXPECT_THAT(test_rect_3.Center(), PointEq({30, -8.5}));
}

TEST(RectDeathTest, SetWidth) {
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({1, 2}, 20, 8).SetWidth(-.001),
      "Cannot set a width less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({1, 2}, 20, 8).SetWidth(-1685),
      "Cannot set a width less than 0");
}

void RectWidthIsTwiceSemiWidth(Rect rect) {
  // Note that it is NOT guaranteed that SemiWidth() == Width() * 0.5f, since
  // Width() can overflow to infinity for large finite Rects.
  EXPECT_THAT(rect.SemiWidth() * 2.0f, NanSensitiveFloatEq(rect.Width()))
      << "Where rect is: " << testing::PrintToString(rect);
}
FUZZ_TEST(RectTest, RectWidthIsTwiceSemiWidth).WithDomains(NotNanRect());

void FiniteRectSemiWidthIsFinite(Rect rect) {
  EXPECT_TRUE(std::isfinite(rect.SemiWidth()));
}
FUZZ_TEST(RectTest, FiniteRectSemiWidthIsFinite).WithDomains(FiniteRect());

TEST(RectTest, Height) {
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({1, -2.8}, {.35, 588}).Height(), 590.8);
  EXPECT_FLOAT_EQ(Rect::FromTwoPoints({0, 10}, {-8.9, 20.2}).Height(), 10.2);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 2.5, 0).Height(), 0);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 2.5, 1).Height(), 1);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({0, -8.5}, 2.5, 3.89).Height(),
                  3.89);
}

TEST(RectTest, SetHeight) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({1, 2}, 20, 8);
  test_rect_1.SetHeight(35.89);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 35.89);
  EXPECT_THAT(test_rect_1.Center(), PointEq({1, 2}));

  Rect test_rect_2 = Rect::FromCenterAndDimensions({0, -8.5}, 0, 2.5);
  test_rect_2.SetHeight(.25);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), .25);
  EXPECT_THAT(test_rect_2.Center(), PointEq({0, -8.5}));

  Rect test_rect_3 = Rect::FromCenterAndDimensions({30, -8.5}, 99, 2.5);
  test_rect_3.SetHeight(0);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 0);
  EXPECT_THAT(test_rect_3.Center(), PointEq({30, -8.5}));
}

TEST(RectDeathTest, SetHeight) {
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({1, 2}, 20, 8).SetHeight(-.001),
      "Cannot set a height less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({1, 2}, 20, 8).SetHeight(-1685),
      "Cannot set a height less than 0");
}

void RectHeightIsTwiceSemiHeight(Rect rect) {
  // Note that it is NOT guaranteed that SemiHeight() == Height() * 0.5f, since
  // Height() can overflow to infinity for large finite Rects.
  EXPECT_THAT(rect.SemiHeight() * 2.0f, NanSensitiveFloatEq(rect.Height()))
      << "Where rect is: " << testing::PrintToString(rect);
}
FUZZ_TEST(RectTest, RectHeightIsTwiceSemiHeight).WithDomains(NotNanRect());

void FiniteRectSemiHeightIsFinite(Rect rect) {
  EXPECT_TRUE(std::isfinite(rect.SemiHeight()));
}
FUZZ_TEST(RectTest, FiniteRectSemiHeightIsFinite).WithDomains(FiniteRect());

TEST(RectTest, AspectRatio) {
  EXPECT_FLOAT_EQ(
      Rect::FromCenterAndDimensions({100, -8.55}, 2.5, 1).AspectRatio(), 2.5);
  EXPECT_FLOAT_EQ(
      Rect::FromCenterAndDimensions({100, -8.55}, 80, 20).AspectRatio(), 4);
  EXPECT_FLOAT_EQ(
      Rect::FromCenterAndDimensions({100, -8.55}, 0, 850).AspectRatio(), 0);
}

TEST(RectDeathTest, AspectRatio) {
  EXPECT_DEATH_IF_SUPPORTED(
      (Rect::FromCenterAndDimensions({3, 3}, 4, 0).AspectRatio()),
      "Cannot determine the Aspect Ratio when the height is 0");
}

TEST(RectTest, Area) {
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({100, -8.55}, 2.5, 1).Area(),
                  2.5);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({100, -8.55}, 80, 20).Area(),
                  1600);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({100, -8.55}, 0, 850).Area(),
                  0);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({100, -8.55}, 4000, 0).Area(),
                  0);
  EXPECT_FLOAT_EQ(Rect::FromCenterAndDimensions({100, -8.55}, 0, 0).Area(), 0);
}

TEST(RectTest, Corners) {
  std::array<Point, 4> test_corners_1 =
      Rect::FromCenterAndDimensions({5, 5}, 2, 15).Corners();

  EXPECT_THAT(test_corners_1[0], PointEq({4, -2.5}));
  EXPECT_THAT(test_corners_1[1], PointEq({6, -2.5}));
  EXPECT_THAT(test_corners_1[2], PointEq({6, 12.5}));
  EXPECT_THAT(test_corners_1[3], PointEq({4, 12.5}));

  std::array<Point, 4> test_corners_2 =
      Rect::FromTwoPoints({8.89, -2.8}, {1, 1000}).Corners();

  EXPECT_THAT(test_corners_2[0], PointEq({1, -2.8}));
  EXPECT_THAT(test_corners_2[1], PointEq({8.89, -2.8}));
  EXPECT_THAT(test_corners_2[2], PointEq({8.89, 1000}));
  EXPECT_THAT(test_corners_2[3], PointEq({1, 1000}));
}

void RectContainsItsCorners(Rect rect) {
  for (Point corner : rect.Corners()) {
    EXPECT_TRUE(rect.Contains(corner))
        << "Where rect is: " << testing::PrintToString(rect)
        << "\nAnd corner is: " << testing::PrintToString(corner);
  }
}
FUZZ_TEST(RectTest, RectContainsItsCorners).WithDomains(NotNanRect());

TEST(RectTest, GetEdge) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({5, 5}, 2, 15);

  EXPECT_EQ(test_rect_1.GetEdge(0), (Segment{{4, -2.5}, {6, -2.5}}));
  EXPECT_EQ(test_rect_1.GetEdge(1), (Segment{{6, -2.5}, {6, 12.5}}));
  EXPECT_EQ(test_rect_1.GetEdge(2), (Segment{{6, 12.5}, {4, 12.5}}));
  EXPECT_EQ(test_rect_1.GetEdge(3), (Segment{{4, 12.5}, {4, -2.5}}));

  Rect test_rect_2 = Rect::FromTwoPoints({8.89, -2.8}, {1, 1000});

  EXPECT_EQ(test_rect_2.GetEdge(0), (Segment{{1, -2.8}, {8.89, -2.8}}));
  EXPECT_EQ(test_rect_2.GetEdge(1), (Segment{{8.89, -2.8}, {8.89, 1000}}));
  EXPECT_EQ(test_rect_2.GetEdge(2), (Segment{{8.89, 1000}, {1, 1000}}));
  EXPECT_EQ(test_rect_2.GetEdge(3), (Segment{{1, 1000}, {1, -2.8}}));
}

TEST(RectDeathTest, GetEdge) {
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromTwoPoints({8.89, -2.8}, {1, 1000}).GetEdge(4),
      "Index 4 out of bounds");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromTwoPoints({8.89, -2.8}, {1, 1000}).GetEdge(12),
      "Index 12 out of bounds");
  // -1 becomes interpreted as 2^64 - 1 due to the value being cast to a size_t
  // object
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromTwoPoints({8.89, -2.8}, {1, 1000}).GetEdge(-1),
      "out of bounds");
}

TEST(RectTest, ContainsPoint) {
  // Centered Points are contained.
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5, 5}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5.5, 5.5}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({4.5, 4.5}));

  // Sides are contained.
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({4, 5}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({6, 5}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5, 4}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5, 6}));

  // Corners are contained.
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({4, 4}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({6, 4}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({4, 6}));
  EXPECT_TRUE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({6, 6}));

  // 8 outer areas are excluded.
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({3, 3}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({3, 5}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({3, 7}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5, 3}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({5, 7}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({7, 3}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({7, 5}));
  EXPECT_FALSE(Rect::FromCenterAndDimensions({5, 5}, 2, 2).Contains({7, 7}));

  // Contains requires more than 2 Units of Least Precision difference to
  // be considered outside the Rect.
  EXPECT_TRUE(
      Rect::FromCenterAndDimensions({5, 5}, 2, 2)
          .Contains({5, 6.0f + 2.0f * std::numeric_limits<float>::epsilon()}));
  EXPECT_FALSE(
      Rect::FromCenterAndDimensions({5, 5}, 2, 2)
          .Contains({5, 6.0f + 2.1f * std::numeric_limits<float>::epsilon()}));
}

TEST(RectTest, ContainsRect) {
  Rect r1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  Rect r2 = Rect::FromTwoPoints({1, 1}, {9, 7});
  Rect r3 = Rect::FromTwoPoints({-1, 1}, {12, 7});

  EXPECT_TRUE(r1.Contains(r1));
  EXPECT_TRUE(r1.Contains(r2));
  EXPECT_FALSE(r1.Contains(r3));

  EXPECT_FALSE(r2.Contains(r1));
  EXPECT_TRUE(r2.Contains(r2));
  EXPECT_FALSE(r2.Contains(r3));

  EXPECT_FALSE(r3.Contains(r1));
  EXPECT_TRUE(r3.Contains(r2));
  EXPECT_TRUE(r3.Contains(r3));

  // Contains requires more than 2 Units of Least Precision difference to
  // be considered outside the Rect.
  EXPECT_TRUE(
      Rect::FromTwoPoints({4, 4}, {6, 6})
          .Contains(Rect::FromTwoPoints(
              {4, 4},
              {6, 6.0f + 2.0f * std::numeric_limits<float>::epsilon()})));
  EXPECT_FALSE(
      Rect::FromTwoPoints({4, 4}, {6, 6})
          .Contains(Rect::FromTwoPoints(
              {4, 4},
              {6, 6.0f + 2.1f * std::numeric_limits<float>::epsilon()})));
}

TEST(RectTest, OffsetBySameValue) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({10, -10}, 6, 8);
  test_rect_1.Offset(15);
  EXPECT_THAT(test_rect_1.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 36);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 38);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), -8);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -29);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({10, -10}, 4, 30);
  test_rect_2.Offset(-10);
  EXPECT_THAT(test_rect_2.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 10);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), 10);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -15);

  Rect test_rect_3 = Rect::FromCenterAndDimensions({10, -10}, 30, 4);
  test_rect_3.Offset(-10);
  EXPECT_THAT(test_rect_3.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 10);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 5);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), -10);

  Rect test_rect_4 = Rect::FromCenterAndDimensions({10, -10}, 4, 6);
  test_rect_4.Offset(-10);
  EXPECT_THAT(test_rect_4.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_4.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.XMin(), 10);
  EXPECT_FLOAT_EQ(test_rect_4.YMin(), -10);
}

TEST(RectTest, OffsetBySpecificValues) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({10, -10}, 6, 8);
  test_rect_1.Offset(15, 10);
  EXPECT_THAT(test_rect_1.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 36);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 28);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), -8);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -24);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({10, -10}, 20, 30);
  test_rect_2.Offset(-5, -20);
  EXPECT_THAT(test_rect_2.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 10);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), 5);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -10);

  Rect test_rect_3 = Rect::FromCenterAndDimensions({10, -10}, 30, 4);
  test_rect_3.Offset(-10, -5);
  EXPECT_THAT(test_rect_3.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 10);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 5);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), -10);

  Rect test_rect_4 = Rect::FromCenterAndDimensions({10, -10}, 4, 6);
  test_rect_4.Offset(-10, -25);
  EXPECT_THAT(test_rect_4.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_4.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.XMin(), 10);
  EXPECT_FLOAT_EQ(test_rect_4.YMin(), -10);
}

TEST(RectTest, ScaleBySameValue) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({10, -10}, 6, 8);
  test_rect_1.Scale(1);
  EXPECT_THAT(test_rect_1.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 6);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 8);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), 7);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -14);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({10, -10}, 4, 30);
  test_rect_2.Scale(.5);
  EXPECT_THAT(test_rect_2.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 2);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 15);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), 9);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -17.5);

  Rect test_rect_3 = Rect::FromCenterAndDimensions({10, -10}, 6, 4);
  test_rect_3.Scale(2.5);
  EXPECT_THAT(test_rect_3.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 15);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 10);
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 2.5);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), -15);

  Rect test_rect_4 = Rect::FromCenterAndDimensions({10, -10}, 4, 6);
  test_rect_4.Scale(0);
  EXPECT_THAT(test_rect_4.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_4.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.XMin(), 10);
  EXPECT_FLOAT_EQ(test_rect_4.YMin(), -10);
}

TEST(RectDeathTest, ScaleBySameValue) {
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-1),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({0, 0}, 0, 0).Scale(-1),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-.001),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-5),
      "Cannot scale a rectangle by a value less than 0");
}

TEST(RectTest, ScaleBySpecificValues) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({10, -10}, 6, 8);
  test_rect_1.Scale(1, 1);
  EXPECT_THAT(test_rect_1.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 6);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 8);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), 7);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -14);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({10, -10}, 4, 30);
  test_rect_2.Scale(.5, 2.5);
  EXPECT_THAT(test_rect_2.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 2);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 75);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), 9);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -47.5);

  Rect test_rect_3 = Rect::FromCenterAndDimensions({10, -10}, 6, 4);
  test_rect_3.Scale(2.5, .5);
  EXPECT_THAT(test_rect_3.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 15);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 2);
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 2.5);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), -11);

  Rect test_rect_4 = Rect::FromCenterAndDimensions({10, -10}, 4, 6);
  test_rect_4.Scale(0, 0);
  EXPECT_THAT(test_rect_4.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_4.Width(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.Height(), 0);
  EXPECT_FLOAT_EQ(test_rect_4.XMin(), 10);
  EXPECT_FLOAT_EQ(test_rect_4.YMin(), -10);
}

TEST(RectDeathTest, ScaleBySpecificValues) {
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-1, -1),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({0, 0}, 0, 0).Scale(-1, -1),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-.005, -.001),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(-5, 14),
      "Cannot scale a rectangle by a value less than 0");
  EXPECT_DEATH_IF_SUPPORTED(
      Rect::FromCenterAndDimensions({10, -10}, 6, 8).Scale(12, -2),
      "Cannot scale a rectangle by a value less than 0");
}

TEST(RectTest, Translate) {
  Rect test_rect_1 = Rect::FromCenterAndDimensions({10, -10}, 6, 8);
  test_rect_1.Translate(Vec{2, 5});
  EXPECT_THAT(test_rect_1.Center(), PointEq({12, -5}));
  EXPECT_FLOAT_EQ(test_rect_1.Width(), 6);
  EXPECT_FLOAT_EQ(test_rect_1.Height(), 8);
  EXPECT_FLOAT_EQ(test_rect_1.XMin(), 9);
  EXPECT_FLOAT_EQ(test_rect_1.YMin(), -9);

  Rect test_rect_2 = Rect::FromCenterAndDimensions({10, -10}, 4, 30);
  test_rect_2.Translate(Vec{-8, -12.5});
  EXPECT_THAT(test_rect_2.Center(), PointEq({2, -22.5}));
  EXPECT_FLOAT_EQ(test_rect_2.Width(), 4);
  EXPECT_FLOAT_EQ(test_rect_2.Height(), 30);
  EXPECT_FLOAT_EQ(test_rect_2.XMin(), 0);
  EXPECT_FLOAT_EQ(test_rect_2.YMin(), -37.5);

  Rect test_rect_3 = Rect::FromCenterAndDimensions({10, -10}, 6, 4);
  test_rect_3.Translate(Vec{0, 0});
  EXPECT_THAT(test_rect_3.Center(), PointEq({10, -10}));
  EXPECT_FLOAT_EQ(test_rect_3.Width(), 6);
  EXPECT_FLOAT_EQ(test_rect_3.Height(), 4);
  EXPECT_FLOAT_EQ(test_rect_3.XMin(), 7);
  EXPECT_FLOAT_EQ(test_rect_3.YMin(), -12);
}

TEST(RectTest, JoinWithRect) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.Join(Rect::FromTwoPoints({0, 0}, {10, 8}));
  EXPECT_THAT(test_rect_1, RectEq(0, 0, 10, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.Join(Rect::FromTwoPoints({2, 3}, {4, 5}));
  EXPECT_THAT(test_rect_2, RectEq(0, 0, 10, 8));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.Join(Rect::FromTwoPoints({7, 7}, {9, 12}));
  EXPECT_THAT(test_rect_3, RectEq(0, 0, 10, 12));

  Rect test_rect_4 = Rect::FromTwoPoints({2, 3}, {4, 5});
  test_rect_4.Join(Rect::FromTwoPoints({0, 0}, {10, 8}));
  EXPECT_THAT(test_rect_4, RectEq(0, 0, 10, 8));

  Rect test_rect_5 = Rect::FromTwoPoints({2, 3}, {4, 5});
  test_rect_5.Join(Rect::FromTwoPoints({2, 3}, {4, 5}));
  EXPECT_THAT(test_rect_5, RectEq(2, 3, 4, 5));

  Rect test_rect_6 = Rect::FromTwoPoints({2, 3}, {4, 5});
  test_rect_6.Join(Rect::FromTwoPoints({7, 7}, {9, 12}));
  EXPECT_THAT(test_rect_6, RectEq(2, 3, 9, 12));

  Rect test_rect_7 = Rect::FromTwoPoints({7, 7}, {9, 12});
  test_rect_7.Join(Rect::FromTwoPoints({0, 0}, {10, 8}));
  EXPECT_THAT(test_rect_7, RectEq(0, 0, 10, 12));

  Rect test_rect_8 = Rect::FromTwoPoints({7, 7}, {9, 12});
  test_rect_8.Join(Rect::FromTwoPoints({2, 3}, {4, 5}));
  EXPECT_THAT(test_rect_8, RectEq(2, 3, 9, 12));

  Rect test_rect_9 = Rect::FromTwoPoints({7, 7}, {9, 12});
  test_rect_9.Join(Rect::FromTwoPoints({7, 7}, {9, 12}));
  EXPECT_THAT(test_rect_9, RectEq(7, 7, 9, 12));
}

void RectJoinWithRectContainsBoth(Rect rect1, Rect rect2) {
  Rect joined = rect1;
  joined.Join(rect2);
  EXPECT_TRUE(joined.Contains(rect1))
      << "Where rect1 is: " << testing::PrintToString(rect1)
      << "\nAnd rect2 is: " << testing::PrintToString(rect2)
      << "\nAnd joined is: " << testing::PrintToString(joined);
  EXPECT_TRUE(joined.Contains(rect2))
      << "Where rect1 is: " << testing::PrintToString(rect1)
      << "\nAnd rect2 is: " << testing::PrintToString(rect2)
      << "\nAnd joined is: " << testing::PrintToString(joined);
}
FUZZ_TEST(RectTest, RectJoinWithRectContainsBoth)
    .WithDomains(NotNanRect(), NotNanRect());

TEST(RectTest, JoinWithPoint) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.Join({3, 3});
  EXPECT_THAT(test_rect_1, RectEq(0, 0, 10, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.Join({20, 3});
  EXPECT_THAT(test_rect_2, RectEq(0, 0, 20, 8));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.Join({3, 20});
  EXPECT_THAT(test_rect_3, RectEq(0, 0, 10, 20));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.Join({-20, -20});
  EXPECT_THAT(test_rect_4, RectEq(-20, -20, 10, 8));
}

void RectJoinWithPointContainsBoth(Rect rect, Point point) {
  Rect joined = rect;
  joined.Join(point);
  EXPECT_TRUE(joined.Contains(rect))
      << "Where rect is: " << testing::PrintToString(rect)
      << "\nAnd point is: " << testing::PrintToString(point)
      << "\nAnd joined is: " << testing::PrintToString(joined);
  EXPECT_TRUE(joined.Contains(point))
      << "Where rect is: " << testing::PrintToString(rect)
      << "\nAnd point is: " << testing::PrintToString(point)
      << "\nAnd joined is: " << testing::PrintToString(joined);
}
FUZZ_TEST(RectTest, RectJoinWithPointContainsBoth)
    .WithDomains(NotNanRect(), NotNanPoint());

TEST(RectTest, ContainingRectWithAspectRatio) {
  // This Rect's center is {30, 40} and aspect ratio is .5
  Rect test_rect_1 = Rect::FromTwoPoints({-20, -60}, {80, 140});

  EXPECT_THAT(test_rect_1.ContainingRectWithAspectRatio(.25f),
              RectEq(-20, -160, 80, 240));
  EXPECT_THAT(test_rect_1.ContainingRectWithAspectRatio(.5f),
              RectEq(-20, -60, 80, 140));
  EXPECT_THAT(test_rect_1.ContainingRectWithAspectRatio(.8f),
              RectEq(-50, -60, 110, 140));
  EXPECT_THAT(test_rect_1.ContainingRectWithAspectRatio(2),
              RectEq(-170, -60, 230, 140));

  // This Rect's center is {40, 20} and aspect ratio is 2
  Rect test_rect_2 = Rect::FromTwoPoints({-80, -40}, {160, 80});

  EXPECT_THAT(test_rect_2.ContainingRectWithAspectRatio(.5f),
              RectEq(-80, -220, 160, 260));
  EXPECT_THAT(test_rect_2.ContainingRectWithAspectRatio(1.5f),
              RectEq(-80, -60, 160, 100));
  EXPECT_THAT(test_rect_2.ContainingRectWithAspectRatio(2),
              RectEq(-80, -40, 160, 80));
  EXPECT_THAT(test_rect_2.ContainingRectWithAspectRatio(4),
              RectEq(-200, -40, 280, 80));
}

TEST(RectDeathTest, ContainingRectWithAspectRatio) {
  // This Rect's center is {30, 40} and aspect ratio is .5
  Rect test_rect_1 = Rect::FromTwoPoints({-20, -60}, {80, 140});
  EXPECT_DEATH_IF_SUPPORTED(test_rect_1.ContainingRectWithAspectRatio(0),
                            "Aspect ratio cannot be <= 0");
  EXPECT_DEATH_IF_SUPPORTED(test_rect_1.ContainingRectWithAspectRatio(-1),
                            "Aspect ratio cannot be <= 0");
  EXPECT_DEATH_IF_SUPPORTED(test_rect_1.ContainingRectWithAspectRatio(-.00001f),
                            "Aspect ratio cannot be <= 0");
}

TEST(RectTest, InteriorRectWithAspectRatio) {
  // This Rect's center is {30, 40} and aspect ratio is .5
  Rect test_rect_1 = Rect::FromTwoPoints({-20, -60}, {80, 140});

  EXPECT_THAT(test_rect_1.InteriorRectWithAspectRatio(0),
              RectEq(30, -60, 30, 140));
  EXPECT_THAT(test_rect_1.InteriorRectWithAspectRatio(.25f),
              RectEq(5, -60, 55, 140));
  EXPECT_THAT(test_rect_1.InteriorRectWithAspectRatio(.5f),
              RectEq(-20, -60, 80, 140));
  EXPECT_THAT(test_rect_1.InteriorRectWithAspectRatio(.8f),
              RectEq(-20, -22.5, 80, 102.5));
  EXPECT_THAT(test_rect_1.InteriorRectWithAspectRatio(2),
              RectEq(-20, 15, 80, 65));

  // This Rect's center is {40, 20} and aspect ratio is 2
  Rect test_rect_2 = Rect::FromTwoPoints({-80, -40}, {160, 80});

  EXPECT_THAT(test_rect_2.InteriorRectWithAspectRatio(0),
              RectEq(40, -40, 40, 80));
  EXPECT_THAT(test_rect_2.InteriorRectWithAspectRatio(.5f),
              RectEq(10, -40, 70, 80));
  EXPECT_THAT(test_rect_2.InteriorRectWithAspectRatio(1.5f),
              RectEq(-50, -40, 130, 80));
  EXPECT_THAT(test_rect_2.InteriorRectWithAspectRatio(2),
              RectEq(-80, -40, 160, 80));
  EXPECT_THAT(test_rect_2.InteriorRectWithAspectRatio(4),
              RectEq(-80, -10, 160, 50));
}

TEST(RectDeathTest, InteriorRectWithAspectRatio) {
  // This Rect's center is {30, 40} and aspect ratio is .5
  Rect test_rect_1 = Rect::FromTwoPoints({-20, -60}, {80, 140});
  EXPECT_DEATH_IF_SUPPORTED(test_rect_1.InteriorRectWithAspectRatio(-1),
                            "Aspect ratio cannot be < 0");
  EXPECT_DEATH_IF_SUPPORTED(test_rect_1.InteriorRectWithAspectRatio(-.00001f),
                            "Aspect ratio cannot be < 0");
}

TEST(RectTest, ResizeSettingValueThatCreatesNonFlatRect) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.ResizeSettingXMinTo(5);
  EXPECT_THAT(test_rect_1, RectEq(5, 0, 10, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.ResizeSettingYMinTo(5);
  EXPECT_THAT(test_rect_2, RectEq(0, 5, 10, 8));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.ResizeSettingXMaxTo(5);
  EXPECT_THAT(test_rect_3, RectEq(0, 0, 5, 8));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.ResizeSettingYMaxTo(5);
  EXPECT_THAT(test_rect_4, RectEq(0, 0, 10, 5));
}

TEST(RectTest, ResizeSettingValueThatCreatesFlatRect) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.ResizeSettingXMinTo(15);
  EXPECT_THAT(test_rect_1, RectEq(15, 0, 15, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.ResizeSettingYMinTo(15);
  EXPECT_THAT(test_rect_2, RectEq(0, 15, 10, 15));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.ResizeSettingXMaxTo(-5);
  EXPECT_THAT(test_rect_3, RectEq(-5, 0, -5, 8));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.ResizeSettingYMaxTo(-5);
  EXPECT_THAT(test_rect_4, RectEq(0, -5, 10, -5));
}

TEST(RectTest, TranslateSettingValueLowerThanLowBound) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.TranslateSettingXMinTo(-5);
  EXPECT_THAT(test_rect_1, RectEq(-5, 0, 5, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.TranslateSettingYMinTo(-5);
  EXPECT_THAT(test_rect_2, RectEq(0, -5, 10, 3));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.TranslateSettingXMaxTo(-5);
  EXPECT_THAT(test_rect_3, RectEq(-15, 0, -5, 8));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.TranslateSettingYMaxTo(-5);
  EXPECT_THAT(test_rect_4, RectEq(0, -13, 10, -5));
}

TEST(RectTest, TranslateSettingValueHigherThanHighBound) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.TranslateSettingXMinTo(15);
  EXPECT_THAT(test_rect_1, RectEq(15, 0, 25, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.TranslateSettingYMinTo(15);
  EXPECT_THAT(test_rect_2, RectEq(0, 15, 10, 23));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.TranslateSettingXMaxTo(15);
  EXPECT_THAT(test_rect_3, RectEq(5, 0, 15, 8));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.TranslateSettingYMaxTo(15);
  EXPECT_THAT(test_rect_4, RectEq(0, 7, 10, 15));
}

TEST(RectTest, TranslateSettingValueBetweenBounds) {
  Rect test_rect_1 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_1.TranslateSettingXMinTo(5);
  EXPECT_THAT(test_rect_1, RectEq(5, 0, 15, 8));

  Rect test_rect_2 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_2.TranslateSettingYMinTo(5);
  EXPECT_THAT(test_rect_2, RectEq(0, 5, 10, 13));

  Rect test_rect_3 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_3.TranslateSettingXMaxTo(5);
  EXPECT_THAT(test_rect_3, RectEq(-5, 0, 5, 8));

  Rect test_rect_4 = Rect::FromTwoPoints({0, 0}, {10, 8});
  test_rect_4.TranslateSettingYMaxTo(5);
  EXPECT_THAT(test_rect_4, RectEq(0, -3, 10, 5));
}

}  // namespace
}  // namespace ink
