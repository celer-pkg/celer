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

#include "ink/geometry/vec.h"

#include <cmath>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::FieldsAre;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::IsNan;
using ::testing::Le;
using ::testing::Not;

constexpr float kInfinity = std::numeric_limits<float>::infinity();
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();

TEST(VecTest, Stringify) {
  EXPECT_EQ(absl::StrCat(Vec()), "<0, 0>");
  EXPECT_EQ(absl::StrCat(Vec{-3, 7}), "<-3, 7>");
  EXPECT_EQ(absl::StrCat(Vec{1.125, -3.75}), "<1.125, -3.75>");
  EXPECT_EQ(absl::StrCat(Vec{kNan, kInfinity}), "<nan, inf>");
}

TEST(VecTest, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Vec(),
      Vec{0, 1},
      Vec{1, 0},
      Vec{1, 2},
      Vec{2, 1},
      Vec{-2, 1},
      Vec{kInfinity, kInfinity},
      Vec{kInfinity, -kInfinity},
  }));
}

TEST(VecTest, Equality) {
  EXPECT_EQ((Vec{1, 2}), (Vec{1, 2}));
  EXPECT_EQ((Vec{-.4, 17}), (Vec{-.4, 17}));

  EXPECT_NE((Vec{1, 2}), (Vec{1, 5}));
  EXPECT_NE((Vec{3, 2}), (Vec{.7, 2}));
  EXPECT_NE((Vec{-4, .3}), (Vec{.5, 12}));
}

TEST(VecTest, EqMatcher) {
  EXPECT_THAT((Vec{1, 2}), VecEq({1, 2}));
  EXPECT_THAT((Vec{3, 4}), Not(VecEq({3, 5})));
  EXPECT_THAT((Vec{5, 6}), Not(VecEq({4, 6})));

  // VecEq delegates to FloatEq, which uses a tolerance of 4 ULP.
  constexpr float kEps = std::numeric_limits<float>::epsilon();
  EXPECT_THAT((Vec{1, 1}), VecEq({1 + kEps, 1 - kEps}));
}

TEST(VecTest, NearMatcher) {
  EXPECT_THAT((Vec{1, 2}), VecNear({1.05, 1.95}, .1));
  EXPECT_THAT((Vec{3, 4}), Not(VecNear({3, 5}, .5)));
  EXPECT_THAT((Vec{5, 6}), Not(VecNear({4, 6}, .5)));
}

TEST(VecTest, FromDirectionAndMagnitude) {
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(0), 5),
              VecEq({5, 0}));
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(90), 5),
              VecNear({0, 5}, .0001));
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(180), 5),
              VecNear({-5, 0}, .0001));
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(90), -5),
              VecNear({0, -5}, .0001));
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(360), 5),
              VecNear({5, 0}, .0001));
  EXPECT_THAT(Vec::FromDirectionAndMagnitude(Angle::Degrees(45), std::sqrt(50)),
              VecEq({5, 5}));
  EXPECT_THAT(
      Vec::FromDirectionAndMagnitude(Angle::Degrees(135), std::sqrt(50)),
      VecEq({-5, 5}));
  EXPECT_THAT(
      Vec::FromDirectionAndMagnitude(Angle::Degrees(225), std::sqrt(50)),
      VecEq({-5, -5}));
  EXPECT_THAT(
      Vec::FromDirectionAndMagnitude(Angle::Degrees(315), std::sqrt(50)),
      VecEq({5, -5}));
}

TEST(VecTest, Magnitude) {
  EXPECT_FLOAT_EQ((Vec{1, 1}).Magnitude(), std::sqrt(2));
  EXPECT_FLOAT_EQ((Vec{-3, 4}).Magnitude(), 5);
  EXPECT_FLOAT_EQ((Vec{0, 0}).Magnitude(), 0);
  EXPECT_FLOAT_EQ((Vec{0, 17}).Magnitude(), 17);
}

TEST(VecTest, MagnitudeSquared) {
  EXPECT_FLOAT_EQ((Vec{1, 1}).MagnitudeSquared(), 2);
  EXPECT_FLOAT_EQ((Vec{3, -4}).MagnitudeSquared(), 25);
  EXPECT_FLOAT_EQ((Vec{0, 0}).MagnitudeSquared(), 0);
  EXPECT_FLOAT_EQ((Vec{15, 0}).MagnitudeSquared(), 225);
}

void MagnitudeSquaredIsSquareOfMagnitude(Vec vec) {
  EXPECT_FLOAT_EQ(vec.MagnitudeSquared(), vec.Magnitude() * vec.Magnitude())
      << "Where vec.Magnitude() is: " << vec.Magnitude();
}
FUZZ_TEST(VecTest, MagnitudeSquaredIsSquareOfMagnitude)
    .WithDomains(NotNanVec());

TEST(VecTest, Direction) {
  EXPECT_FLOAT_EQ((Vec{5, 0}).Direction().ValueInDegrees(), 0);
  EXPECT_FLOAT_EQ((Vec{0, 5}).Direction().ValueInDegrees(), 90);
  EXPECT_FLOAT_EQ((Vec{-5, 0}).Direction().ValueInDegrees(), 180);
  EXPECT_FLOAT_EQ((Vec{0, -5}).Direction().ValueInDegrees(), -90);
  EXPECT_FLOAT_EQ((Vec{5, 5}).Direction().ValueInDegrees(), 45);
  EXPECT_FLOAT_EQ((Vec{-5, 5}).Direction().ValueInDegrees(), 135);
  EXPECT_FLOAT_EQ((Vec{-5, -5}).Direction().ValueInDegrees(), -135);
  EXPECT_FLOAT_EQ((Vec{5, -5}).Direction().ValueInDegrees(), -45);

  // Handle the zero vector the same way that std::atan2 does:
  EXPECT_THAT((Vec{+0.f, +0.f}).Direction(), AngleEq(Angle()));
  EXPECT_THAT((Vec{+0.f, -0.f}).Direction(), AngleEq(Angle()));
  EXPECT_THAT((Vec{-0.f, +0.f}).Direction(), AngleEq(kHalfTurn));
  EXPECT_THAT((Vec{-0.f, -0.f}).Direction(), AngleEq(-kHalfTurn));
}

void DirectionIsNanIfEitherComponentIsNan(Vec vec) {
  EXPECT_THAT((Vec{kNan, vec.y}).Direction(), IsNanAngle());
  EXPECT_THAT((Vec{vec.x, kNan}).Direction(), IsNanAngle());
}
FUZZ_TEST(VecTest, DirectionIsNanIfEitherComponentIsNan)
    .WithDomains(ArbitraryVec());

void DirectionIsBetweenMinusPiAndPiInclusive(Vec vec) {
  EXPECT_THAT(vec.Direction(), AllOf(Ge(-kHalfTurn), Le(kHalfTurn)))
      << "Where vec is: " << testing::PrintToString(vec);
}
FUZZ_TEST(VecTest, DirectionIsBetweenMinusPiAndPiInclusive)
    .WithDomains(NotNanVec());

void DirectionSignIsYSign(Vec vec) {
  EXPECT_EQ(std::copysign(1.f, vec.Direction().ValueInRadians()),
            std::copysign(1.f, vec.y))
      << "Where vec is: " << testing::PrintToString(vec)
      << "\nAnd vec.Direction() is: "
      << testing::PrintToString(vec.Direction());
}
FUZZ_TEST(VecTest, DirectionSignIsYSign).WithDomains(NotNanVec());

TEST(VecTest, Orthogonal) {
  EXPECT_THAT((Vec{3, 1}).Orthogonal(), VecEq({-1, 3}));
  EXPECT_THAT((Vec{-3, 1}).Orthogonal(), VecEq({-1, -3}));
  EXPECT_THAT((Vec{-3, -1}).Orthogonal(), VecEq({1, -3}));
  EXPECT_THAT((Vec{3, -1}).Orthogonal(), VecEq({1, 3}));
}

TEST(VecTest, AsUnitVec) {
  EXPECT_THAT((Vec{4, 0}).AsUnitVec(), VecEq({1, 0}));
  EXPECT_THAT((Vec{0, 4}).AsUnitVec(), VecEq({0, 1}));
  EXPECT_THAT((Vec{-4, 0}).AsUnitVec(), VecEq({-1, 0}));
  EXPECT_THAT((Vec{0, -4}).AsUnitVec(), VecEq({0, -1}));
  EXPECT_THAT((Vec{4, 4}).AsUnitVec(), VecEq({std::sqrt(.5f), std::sqrt(.5f)}));
  EXPECT_THAT((Vec{-4, 4}).AsUnitVec(),
              VecEq({-std::sqrt(.5f), std::sqrt(.5f)}));
  EXPECT_THAT((Vec{-4, -4}).AsUnitVec(),
              VecEq({-std::sqrt(.5f), -std::sqrt(.5f)}));
  EXPECT_THAT((Vec{4, -4}).AsUnitVec(),
              VecEq({std::sqrt(.5f), -std::sqrt(.5f)}));

  // Handle NaN vectors:
  EXPECT_THAT((Vec{0, kNan}).AsUnitVec(), FieldsAre(IsNan(), IsNan()));
  EXPECT_THAT((Vec{kNan, 0}).AsUnitVec(), FieldsAre(IsNan(), IsNan()));
  EXPECT_THAT((Vec{kNan, kNan}).AsUnitVec(), FieldsAre(IsNan(), IsNan()));

  // Do the right thing for infinite vectors:
  EXPECT_THAT((Vec{4, kInfinity}).AsUnitVec(), VecEq({0, 1}));
  EXPECT_THAT((Vec{-4, kInfinity}).AsUnitVec(), VecEq({0, 1}));
  EXPECT_THAT((Vec{4, -kInfinity}).AsUnitVec(), VecEq({0, -1}));
  EXPECT_THAT((Vec{-4, -kInfinity}).AsUnitVec(), VecEq({0, -1}));
  EXPECT_THAT((Vec{kInfinity, 4}).AsUnitVec(), VecEq({1, 0}));
  EXPECT_THAT((Vec{kInfinity, -4}).AsUnitVec(), VecEq({1, 0}));
  EXPECT_THAT((Vec{-kInfinity, 4}).AsUnitVec(), VecEq({-1, 0}));
  EXPECT_THAT((Vec{-kInfinity, -4}).AsUnitVec(), VecEq({-1, 0}));
  EXPECT_THAT((Vec{kInfinity, kInfinity}).AsUnitVec(),
              VecEq(Vec::FromDirectionAndMagnitude(Angle::Degrees(45), 1)));
  EXPECT_THAT((Vec{-kInfinity, kInfinity}).AsUnitVec(),
              VecEq(Vec::FromDirectionAndMagnitude(Angle::Degrees(135), 1)));
  EXPECT_THAT((Vec{-kInfinity, -kInfinity}).AsUnitVec(),
              VecEq(Vec::FromDirectionAndMagnitude(Angle::Degrees(-135), 1)));
  EXPECT_THAT((Vec{kInfinity, -kInfinity}).AsUnitVec(),
              VecEq(Vec::FromDirectionAndMagnitude(Angle::Degrees(-45), 1)));

  // Special cases for the zero vector, to maintain equivalence with
  // Vec::FromDirectionAndMagnitude(v.Direction(), 1):
  EXPECT_THAT((Vec{+0.f, 0.f}).AsUnitVec(), VecEq({1, 0}));
  EXPECT_THAT((Vec{-0.f, 0.f}).AsUnitVec(), VecEq({-1, 0}));
}

void UnitVecMagnitudeIsOne(Vec vec) {
  EXPECT_FLOAT_EQ(vec.AsUnitVec().Magnitude(), 1.f)
      << "Where vec is: " << testing::PrintToString(vec)
      << "\nAnd vec.AsUnitVec() is: "
      << testing::PrintToString(vec.AsUnitVec());
}
FUZZ_TEST(VecTest, UnitVecMagnitudeIsOne).WithDomains(NotNanVec());

void UnitVecDirectionIsUnchanged(Vec vec) {
  EXPECT_THAT(vec.AsUnitVec().Direction(), AngleEq(vec.Direction()))
      << "Where vec is: " << testing::PrintToString(vec)
      << "\nAnd vec.AsUnitVec() is: "
      << testing::PrintToString(vec.AsUnitVec());
}
FUZZ_TEST(VecTest, UnitVecDirectionIsUnchanged).WithDomains(NotNanVec());

TEST(VecTest, DotProduct) {
  Vec a{3, 0};
  Vec b{-1, 4};
  Vec c{2, .5};
  Vec d{6, 6};

  EXPECT_FLOAT_EQ(Vec::DotProduct(a, b), -3);
  EXPECT_FLOAT_EQ(Vec::DotProduct(a, c), 6);
  EXPECT_FLOAT_EQ(Vec::DotProduct(a, d), 18);
  EXPECT_FLOAT_EQ(Vec::DotProduct(b, c), 0);
  EXPECT_FLOAT_EQ(Vec::DotProduct(b, d), 18);
  EXPECT_FLOAT_EQ(Vec::DotProduct(c, d), 15);
}

TEST(VecTest, Determinant) {
  Vec a{3, 0};
  Vec b{-1, 4};
  Vec c{2, .5};

  EXPECT_FLOAT_EQ(Vec::Determinant(a, b), 12);
  EXPECT_FLOAT_EQ(Vec::Determinant(a, c), 1.5);
  EXPECT_FLOAT_EQ(Vec::Determinant(b, a), -12);
  EXPECT_FLOAT_EQ(Vec::Determinant(b, c), -8.5);
  EXPECT_FLOAT_EQ(Vec::Determinant(c, a), -1.5);
  EXPECT_FLOAT_EQ(Vec::Determinant(c, b), 8.5);
}

TEST(VecTest, AbsoluteAngleBetween) {
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{5, 0}, Vec{5, 0}).ValueInDegrees(), 0);
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{5, 0}, Vec{0, 5}).ValueInDegrees(), 90);
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{-5, 0}, Vec{5, 0}).ValueInDegrees(), 180);
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{5, 0}, Vec{5, 5}).ValueInDegrees(), 45);
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{5, 0}, Vec{-5, 5}).ValueInDegrees(), 135);
  EXPECT_FLOAT_EQ(
      Vec::AbsoluteAngleBetween(Vec{5, 0}, Vec{-5, -5}).ValueInDegrees(), 135);
}

void AbsoluteAngleIsBetweenZeroAndPiInclusive(Vec a, Vec b) {
  EXPECT_THAT(Vec::AbsoluteAngleBetween(a, b),
              AllOf(Ge(Angle()), Le(kHalfTurn)))
      << "Where a is: " << testing::PrintToString(a)
      << "\n  And b is: " << testing::PrintToString(b);
}
FUZZ_TEST(AngleTest, AbsoluteAngleIsBetweenZeroAndPiInclusive)
    .WithDomains(NotNanVec(), NotNanVec());

void AbsoluteAngleBetweenEquivalence(Vec a, Vec b) {
  // Test that the claimed equivalence in the doc comment for
  // AbsoluteAngleBetween is valid.
  EXPECT_THAT(
      Vec::AbsoluteAngleBetween(a, b),
      AngleNear(Abs((b.Direction() - a.Direction()).NormalizedAboutZero()),
                0.001f))
      << "Where a is: " << testing::PrintToString(a)
      << "\n  And b is: " << testing::PrintToString(b);
}
FUZZ_TEST(VecTest, AbsoluteAngleBetweenEquivalence)
    .WithDomains(NotNanVec(), NotNanVec());

void AbsoluteAngleBetweenIsNanIfAnyInputIsNan(Vec a, Vec b) {
  EXPECT_THAT(Vec::AbsoluteAngleBetween({kNan, a.y}, b), IsNanAngle());
  EXPECT_THAT(Vec::AbsoluteAngleBetween({a.x, kNan}, b), IsNanAngle());
  EXPECT_THAT(Vec::AbsoluteAngleBetween(a, {kNan, b.y}), IsNanAngle());
  EXPECT_THAT(Vec::AbsoluteAngleBetween(a, {b.x, kNan}), IsNanAngle());
}
FUZZ_TEST(VecTest, AbsoluteAngleBetweenIsNanIfAnyInputIsNan)
    .WithDomains(ArbitraryVec(), ArbitraryVec());

TEST(VecTest, SignedAngleBetween) {
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{5, 0}, Vec{5, 0}).ValueInDegrees(), 0);
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{5, 0}, Vec{0, 5}).ValueInDegrees(), 90);
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{-5, 0}, Vec{5, 0}).ValueInDegrees(), 180);
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{5, 0}, Vec{5, 5}).ValueInDegrees(), 45);
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{5, 0}, Vec{-5, 5}).ValueInDegrees(), 135);
  EXPECT_FLOAT_EQ(
      Vec::SignedAngleBetween(Vec{5, 0}, Vec{-5, -5}).ValueInDegrees(), -135);
}

void SignedAngleIsBetweenMinusPiExclusiveAndPiInclusive(Vec a, Vec b) {
  EXPECT_THAT(Vec::SignedAngleBetween(a, b),
              AllOf(Gt(-kHalfTurn), Le(kHalfTurn)))
      << "Where a is: " << testing::PrintToString(a)
      << "\nAnd b is: " << testing::PrintToString(b);
}
FUZZ_TEST(AngleTest, SignedAngleIsBetweenMinusPiExclusiveAndPiInclusive)
    .WithDomains(NotNanVec(), NotNanVec());

void SignedAngleBetweenEquivalence(Vec a, Vec b) {
  EXPECT_THAT(
      Vec::SignedAngleBetween(a, b),
      NormalizedAngleNear((b.Direction() - a.Direction()).NormalizedAboutZero(),
                          0.001f))
      << "Where a is: " << testing::PrintToString(a)
      << "\nAnd b is: " << testing::PrintToString(b);
}
FUZZ_TEST(VecTest, SignedAngleBetweenEquivalence)
    .WithDomains(NotNanVec(), NotNanVec());

void SignedAngleBetweenIsNanIfAnyInputIsNan(Vec a, Vec b) {
  EXPECT_THAT(Vec::SignedAngleBetween({kNan, a.y}, b), IsNanAngle());
  EXPECT_THAT(Vec::SignedAngleBetween({a.x, kNan}, b), IsNanAngle());
  EXPECT_THAT(Vec::SignedAngleBetween(a, {kNan, b.y}), IsNanAngle());
  EXPECT_THAT(Vec::SignedAngleBetween(a, {b.x, kNan}), IsNanAngle());
}
FUZZ_TEST(VecTest, SignedAngleBetweenIsNanIfAnyInputIsNan)
    .WithDomains(ArbitraryVec(), ArbitraryVec());

TEST(VecTest, Addition) {
  Vec a{3, 0};
  Vec b{-1, .3};
  Vec c{2.7, 4};

  EXPECT_THAT(a + b, VecEq({2, .3}));
  EXPECT_THAT(a + c, VecEq({5.7, 4}));
  EXPECT_THAT(b + c, VecEq({1.7, 4.3}));
}

TEST(VecTest, Subtraction) {
  Vec a{0, -2};
  Vec b{.5, 19};
  Vec c{1.1, -3.4};

  EXPECT_THAT(a - b, VecEq({-.5, -21}));
  EXPECT_THAT(a - c, VecEq({-1.1, 1.4}));
  EXPECT_THAT(b - c, VecEq({-.6, 22.4}));
}

TEST(VecTest, Negation) {
  Vec a{0, -2};
  Vec b{.5, 19};
  Vec c{1.1, -3.4};

  EXPECT_THAT(-a, VecEq({0, 2}));
  EXPECT_THAT(-b, VecEq({-.5, -19}));
  EXPECT_THAT(-c, VecEq({-1.1, 3.4}));
}

TEST(VecTest, ScalarMultiplication) {
  Vec a{.7, -3};
  Vec b{3, 5};

  EXPECT_THAT(a * 2, VecEq({1.4, -6}));
  EXPECT_THAT(.1 * a, VecEq({.07, -.3}));
  EXPECT_THAT(b * -.3, VecEq({-.9, -1.5}));
  EXPECT_THAT(4 * b, Vec({12, 20}));
}

TEST(VecTest, ScalarDivision) {
  Vec a{7, .9};
  Vec b{-4.5, -2};

  EXPECT_THAT(a / 2, VecEq({3.5, .45}));
  EXPECT_THAT(a / -.1, VecEq({-70, -9}));
  EXPECT_THAT(b / 5, VecEq({-.9, -.4}));
  EXPECT_THAT(b / .2, VecEq({-22.5, -10}));
}

TEST(VecTest, AddAssign) {
  Vec a{1, 2};
  a += {.x = 3, .y = -1};
  EXPECT_THAT(a, VecEq({4, 1}));
  a += {.x = -.5, .y = 2};
  EXPECT_THAT(a, VecEq({3.5, 3}));
}

TEST(VecTest, SubractAssign) {
  Vec a{1, 2};
  a -= {.x = 3, .y = -1};
  EXPECT_THAT(a, VecEq({-2, 3}));
  a -= {.x = -.5, .y = 2};
  EXPECT_THAT(a, VecEq({-1.5, 1}));
}

TEST(VecTest, ScalarMultiplyAssign) {
  Vec a{1, 2};
  a *= 2;
  EXPECT_THAT(a, VecEq({2, 4}));
  a *= -.4;
  EXPECT_THAT(a, VecEq({-.8, -1.6}));
}

TEST(VecTest, ScalarDivideAssign) {
  Vec a{1, 2};
  a /= 2;
  EXPECT_THAT(a, VecEq({.5, 1}));
  a /= -.4;
  EXPECT_THAT(a, VecEq({-1.25, -2.5}));
}

}  // namespace
}  // namespace ink
