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

#include "ink/geometry/angle.h"

#include <cmath>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/types/numbers.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Le;
using ::testing::Lt;

TEST(AngleTest, Stringify) {
  EXPECT_EQ(absl::StrCat(Angle()), "0π");
  EXPECT_EQ(absl::StrCat(kFullTurn), "2π");
  EXPECT_EQ(absl::StrCat(-kQuarterTurn), "-0.5π");
  EXPECT_EQ(absl::StrCat(Angle::Degrees(57.2958)), "0.31831π");
  EXPECT_EQ(absl::StrCat(Angle::Radians(1.0)), "0.31831π");
  EXPECT_EQ(
      absl::StrCat(Angle::Radians(-std::numeric_limits<float>::infinity())),
      "-infπ");
  EXPECT_EQ(absl::StrCat(Angle::Radians(std::nanf(""))), "nanπ");
}

TEST(AngleTest, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Angle(),
      kQuarterTurn,
      -kQuarterTurn,
      kHalfTurn,
      -kHalfTurn,
      kFullTurn,
      -kFullTurn,
      Angle::Radians(std::numeric_limits<float>::infinity()),
      Angle::Radians(-std::numeric_limits<float>::infinity()),
  }));
}

TEST(AngleTest, Equality) {
  EXPECT_EQ(Angle::Radians(1), Angle::Radians(1));
  EXPECT_EQ(Angle::Radians(0), Angle::Radians(0));
  EXPECT_EQ(Angle::Radians(.5), Angle::Radians(.5));
  EXPECT_EQ(Angle::Radians(-400), Angle::Radians(-400));

  EXPECT_NE(Angle::Radians(1), Angle::Radians(-1));
  EXPECT_NE(Angle::Radians(1), Angle::Radians(0));
  EXPECT_NE(Angle::Radians(1), Angle::Radians(.5));
  EXPECT_NE(Angle::Radians(1), Angle::Radians(-400));
  EXPECT_NE(Angle::Radians(-1), Angle::Radians(0));
  EXPECT_NE(Angle::Radians(-1), Angle::Radians(.5));
  EXPECT_NE(Angle::Radians(-1), Angle::Radians(-400));
  EXPECT_NE(Angle::Radians(0), Angle::Radians(.5));
  EXPECT_NE(Angle::Radians(0), Angle::Radians(-400));
  EXPECT_NE(Angle::Radians(.5), Angle::Radians(-400));
}

TEST(AngleTest, Radians) {
  EXPECT_FLOAT_EQ(Angle::Radians(1).ValueInRadians(), 1.f);
  EXPECT_FLOAT_EQ(Angle::Radians(0).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ(Angle::Radians(.5).ValueInRadians(), 0.5f);
  EXPECT_FLOAT_EQ(Angle::Radians(-400).ValueInRadians(), -400.f);
}

TEST(AngleTest, Degrees) {
  EXPECT_FLOAT_EQ(Angle::Degrees(1).ValueInDegrees(), 1.f);
  EXPECT_FLOAT_EQ(Angle::Degrees(0).ValueInDegrees(), 0.f);
  EXPECT_FLOAT_EQ(Angle::Degrees(.5).ValueInDegrees(), 0.5f);
  EXPECT_FLOAT_EQ(Angle::Degrees(-400).ValueInDegrees(), -400.f);
}

TEST(AngleTest, ValueInRadians) {
  EXPECT_FLOAT_EQ(Angle::Degrees(0).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ(Angle::Degrees(180).ValueInRadians(), numbers::kPi);
  EXPECT_FLOAT_EQ(Angle::Degrees(-360).ValueInRadians(), -2 * numbers::kPi);
  EXPECT_FLOAT_EQ(Angle::Degrees(22.5).ValueInRadians(), numbers::kPi / 8);
}

TEST(AngleTest, ValueInDegrees) {
  EXPECT_FLOAT_EQ(Angle::Radians(0).ValueInDegrees(), 0.f);
  EXPECT_FLOAT_EQ(Angle::Radians(numbers::kPi).ValueInDegrees(), 180.f);
  EXPECT_FLOAT_EQ(Angle::Radians(-2 * numbers::kPi).ValueInDegrees(), -360.f);
  EXPECT_FLOAT_EQ(Angle::Radians(numbers::kPi / 8).ValueInDegrees(), 22.5f);
}

TEST(AngleTest, Normalized) {
  EXPECT_FLOAT_EQ(
      0, Angle::Radians(2 * numbers::kPi).Normalized().ValueInRadians());
  EXPECT_FLOAT_EQ(0, Angle::Radians(0).Normalized().ValueInRadians());
  EXPECT_FLOAT_EQ(numbers::kPi + numbers::kPi / 6,
                  Angle::Radians(numbers::kPi + numbers::kPi / 6)
                      .Normalized()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(numbers::kPi / 6,
                  Angle::Radians(2 * numbers::kPi + numbers::kPi / 6)
                      .Normalized()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(5 * numbers::kPi / 6,
                  Angle::Radians(-numbers::kPi - numbers::kPi / 6)
                      .Normalized()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(numbers::kPi,
                  Angle::Radians(-numbers::kPi).Normalized().ValueInRadians());
  EXPECT_FLOAT_EQ(
      numbers::kPi / 6,
      Angle::Radians(numbers::kPi / 6).Normalized().ValueInRadians());
  EXPECT_FLOAT_EQ(
      numbers::kPi + 5 * numbers::kPi / 6,
      Angle::Radians(-numbers::kPi / 6).Normalized().ValueInRadians());

  // tolerance is larger here because the raw angle is a bigger number.
  float tolerance = .0001;
  EXPECT_NEAR(numbers::kPi / 6,
              Angle::Radians(8 * numbers::kPi + numbers::kPi / 6)
                  .Normalized()
                  .ValueInRadians(),
              tolerance);

  // Regression test for b/282556899.  A small negative angle would ordinarily
  // normalize to just below 2π, but this angle is so small that adding it to 2π
  // gives 2π exactly, and Angle::Normalized() is supposed to always return a
  // value strictly less than 2π.  Therefore, normalizing this angle should in
  // fact return the zero angle.
  EXPECT_EQ(Angle::Radians(-1.4e-45f).Normalized(), Angle());
}

void NormalizedAngleIsBetweenZeroInclusiveAndTwoPiExclusive(Angle angle) {
  EXPECT_THAT(angle.Normalized(), AllOf(Ge(Angle()), Lt(kFullTurn)))
      << "Where angle is: " << testing::PrintToString(angle);
}
FUZZ_TEST(AngleTest, NormalizedAngleIsBetweenZeroInclusiveAndTwoPiExclusive)
    .WithDomains(FiniteAngle());

TEST(AngleTest, NormalizedAboutZero) {
  EXPECT_FLOAT_EQ(
      0,
      Angle::Radians(2 * numbers::kPi).NormalizedAboutZero().ValueInRadians());
  EXPECT_FLOAT_EQ(-5 * numbers::kPi / 6,
                  Angle::Radians(numbers::kPi + numbers::kPi / 6)
                      .NormalizedAboutZero()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(numbers::kPi / 6,
                  Angle::Radians(2 * numbers::kPi + numbers::kPi / 6)
                      .NormalizedAboutZero()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(5 * numbers::kPi / 6,
                  Angle::Radians(-numbers::kPi - numbers::kPi / 6)
                      .NormalizedAboutZero()
                      .ValueInRadians());
  EXPECT_FLOAT_EQ(
      numbers::kPi,
      Angle::Radians(-numbers::kPi).NormalizedAboutZero().ValueInRadians());
  EXPECT_FLOAT_EQ(
      numbers::kPi,
      Angle::Radians(numbers::kPi).NormalizedAboutZero().ValueInRadians());
  EXPECT_FLOAT_EQ(
      numbers::kPi / 6,
      Angle::Radians(numbers::kPi / 6).NormalizedAboutZero().ValueInRadians());
  EXPECT_FLOAT_EQ(
      -numbers::kPi / 6,
      Angle::Radians(-numbers::kPi / 6).NormalizedAboutZero().ValueInRadians());

  // tolerance is larger here because the raw angle is a bigger number.
  float tolerance = .0001;
  EXPECT_NEAR(numbers::kPi / 6,
              Angle::Radians(8 * numbers::kPi + numbers::kPi / 6)
                  .NormalizedAboutZero()
                  .ValueInRadians(),
              tolerance);
}

void NormalizedAboutZeroIsBetweenMinusPiExclusiveAndPiInclusive(Angle angle) {
  EXPECT_THAT(angle.NormalizedAboutZero(), AllOf(Gt(-kHalfTurn), Le(kHalfTurn)))
      << "Where angle is: " << testing::PrintToString(angle);
}
FUZZ_TEST(AngleTest, NormalizedAboutZeroIsBetweenMinusPiExclusiveAndPiInclusive)
    .WithDomains(FiniteAngle());

TEST(AngleTest, Sin) {
  EXPECT_FLOAT_EQ(Sin(Angle::Radians(1)), std::sin(1.f));
  EXPECT_FLOAT_EQ(Sin(Angle::Radians(0)), std::sin(0.f));
  EXPECT_FLOAT_EQ(Sin(Angle::Radians(.5)), std::sin(0.5f));
  EXPECT_FLOAT_EQ(Sin(Angle::Radians(-400)), std::sin(-400.f));
}

TEST(AngleTest, Cos) {
  EXPECT_FLOAT_EQ(Cos(Angle::Radians(1)), std::cos(1.f));
  EXPECT_FLOAT_EQ(Cos(Angle::Radians(0)), std::cos(0.f));
  EXPECT_FLOAT_EQ(Cos(Angle::Radians(.5)), std::cos(0.5f));
  EXPECT_FLOAT_EQ(Cos(Angle::Radians(-400)), std::cos(-400.f));
}

TEST(AngleTest, Tan) {
  EXPECT_FLOAT_EQ(Tan(Angle::Radians(1)), std::tan(1.f));
  EXPECT_FLOAT_EQ(Tan(Angle::Radians(0)), std::tan(0.f));
  EXPECT_FLOAT_EQ(Tan(Angle::Radians(.5)), std::tan(0.5f));
  EXPECT_FLOAT_EQ(Tan(Angle::Radians(-400)), std::tan(-400.f));
}

TEST(AngleTest, Asin) {
  EXPECT_FLOAT_EQ(Asin(1).ValueInRadians(), std::asin(1.f));
  EXPECT_FLOAT_EQ(Asin(0).ValueInRadians(), std::asin(0.f));
  EXPECT_FLOAT_EQ(Asin(.5).ValueInRadians(), std::asin(0.5f));
  EXPECT_FLOAT_EQ(Asin(-.8).ValueInRadians(), std::asin(-.8f));
}

TEST(AngleTest, Acos) {
  EXPECT_FLOAT_EQ(Acos(1).ValueInRadians(), std::acos(1.f));
  EXPECT_FLOAT_EQ(Acos(0).ValueInRadians(), std::acos(0.f));
  EXPECT_FLOAT_EQ(Acos(.5).ValueInRadians(), std::acos(0.5f));
  EXPECT_FLOAT_EQ(Acos(-.8).ValueInRadians(), std::acos(-.8f));
}

TEST(AngleTest, Atan) {
  EXPECT_FLOAT_EQ(Atan(1).ValueInRadians(), std::atan(1.f));
  EXPECT_FLOAT_EQ(Atan(0).ValueInRadians(), std::atan(0.f));
  EXPECT_FLOAT_EQ(Atan(.5).ValueInRadians(), std::atan(0.5f));
  EXPECT_FLOAT_EQ(Atan(-.8).ValueInRadians(), std::atan(-.8f));
}

TEST(AngleTest, Abs) {
  EXPECT_FLOAT_EQ((Abs(Angle::Radians(1))).ValueInRadians(), 1.f);
  EXPECT_FLOAT_EQ((Abs(Angle::Radians(0))).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Abs(Angle::Radians(-1))).ValueInRadians(), 1.f);
  EXPECT_FLOAT_EQ((Abs(Angle::Radians(-.5))).ValueInRadians(), 0.5f);
  EXPECT_FLOAT_EQ((Abs(Angle::Radians(-400))).ValueInRadians(), 400.f);
}

TEST(AngleTest, Mod) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(13.3);
  Angle C = Angle::Radians(6.2);
  Angle D = Angle::Radians(-.5);

  EXPECT_FLOAT_EQ((Mod(A, B)).ValueInRadians(), std::fmod(1.f, 13.3f));
  EXPECT_FLOAT_EQ((Mod(B, A)).ValueInRadians(), std::fmod(13.3f, 1.f));
  EXPECT_FLOAT_EQ((Mod(B, C)).ValueInRadians(), std::fmod(13.3f, 6.2f));
  EXPECT_FLOAT_EQ((Mod(B, D)).ValueInRadians(), std::fmod(13.3f, -.5f));
  EXPECT_FLOAT_EQ((Mod(C, A)).ValueInRadians(), std::fmod(6.2f, 1.f));
  EXPECT_FLOAT_EQ((Mod(C, D)).ValueInRadians(), std::fmod(6.2f, -.5f));
  EXPECT_FLOAT_EQ((Mod(D, A)).ValueInRadians(), std::fmod(-.5f, 1.f));
}

TEST(AngleTest, Min) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_EQ((Min(A, B)).ValueInRadians(), B.ValueInRadians());
  EXPECT_EQ((Min(A, C)).ValueInRadians(), C.ValueInRadians());
  EXPECT_EQ((Min(A, D)).ValueInRadians(), D.ValueInRadians());
  EXPECT_EQ((Min(B, C)).ValueInRadians(), B.ValueInRadians());
  EXPECT_EQ((Min(B, D)).ValueInRadians(), D.ValueInRadians());
  EXPECT_EQ((Min(C, D)).ValueInRadians(), D.ValueInRadians());
}

TEST(AngleTest, Max) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_EQ((Max(A, B)).ValueInRadians(), A.ValueInRadians());
  EXPECT_EQ((Max(A, C)).ValueInRadians(), A.ValueInRadians());
  EXPECT_EQ((Max(A, D)).ValueInRadians(), A.ValueInRadians());
  EXPECT_EQ((Max(B, C)).ValueInRadians(), C.ValueInRadians());
  EXPECT_EQ((Max(B, D)).ValueInRadians(), B.ValueInRadians());
  EXPECT_EQ((Max(C, D)).ValueInRadians(), C.ValueInRadians());
}

TEST(AngleTest, LessThan) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_THAT(A < B, testing::IsFalse());
  EXPECT_THAT(A < C, testing::IsFalse());
  EXPECT_THAT(A < D, testing::IsFalse());
  EXPECT_THAT(B < C, testing::IsTrue());
  EXPECT_THAT(B < D, testing::IsFalse());
  EXPECT_THAT(C < D, testing::IsFalse());
}

TEST(AngleTest, GreaterThan) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_THAT(A > B, testing::IsTrue());
  EXPECT_THAT(A > C, testing::IsTrue());
  EXPECT_THAT(A > D, testing::IsTrue());
  EXPECT_THAT(B > C, testing::IsFalse());
  EXPECT_THAT(B > D, testing::IsTrue());
  EXPECT_THAT(C > D, testing::IsTrue());
}

TEST(AngleTest, LessThanOrEqualTo) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_THAT(A <= B, testing::IsFalse());
  EXPECT_THAT(A <= C, testing::IsFalse());
  EXPECT_THAT(A <= D, testing::IsFalse());
  EXPECT_THAT(B <= C, testing::IsTrue());
  EXPECT_THAT(B <= D, testing::IsFalse());
  EXPECT_THAT(C <= D, testing::IsFalse());

  EXPECT_THAT(A <= A, testing::IsTrue());
  EXPECT_THAT(B <= B, testing::IsTrue());
  EXPECT_THAT(C <= C, testing::IsTrue());
  EXPECT_THAT(D <= D, testing::IsTrue());
}

TEST(AngleTest, GreaterThanOrEqualTo) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_THAT(A > B, testing::IsTrue());
  EXPECT_THAT(A > B, testing::IsTrue());
  EXPECT_THAT(A > C, testing::IsTrue());
  EXPECT_THAT(A > D, testing::IsTrue());
  EXPECT_THAT(B > C, testing::IsFalse());
  EXPECT_THAT(B > D, testing::IsTrue());
  EXPECT_THAT(C > D, testing::IsTrue());

  EXPECT_THAT(A <= A, testing::IsTrue());
  EXPECT_THAT(B <= B, testing::IsTrue());
  EXPECT_THAT(C <= C, testing::IsTrue());
  EXPECT_THAT(D <= D, testing::IsTrue());
}

TEST(AngleTest, Addition) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_EQ((A + B).ValueInRadians(), 1.f);
  EXPECT_EQ((A + C).ValueInRadians(), 1.5f);
  EXPECT_EQ((A + D).ValueInRadians(), -399.f);
  EXPECT_EQ((B + C).ValueInRadians(), .5f);
  EXPECT_EQ((B + D).ValueInRadians(), -400.f);
  EXPECT_EQ((C + D).ValueInRadians(), -399.5f);
}

TEST(AngleTest, Subtraction) {
  Angle A = Angle::Radians(1);
  Angle B = Angle::Radians(0);
  Angle C = Angle::Radians(.5);
  Angle D = Angle::Radians(-400);

  EXPECT_EQ((A - B).ValueInRadians(), 1.f);
  EXPECT_EQ((A - C).ValueInRadians(), .5f);
  EXPECT_EQ((A - D).ValueInRadians(), 401.f);
  EXPECT_EQ((B - A).ValueInRadians(), -1.f);
  EXPECT_EQ((B - C).ValueInRadians(), -.5f);
  EXPECT_EQ((B - D).ValueInRadians(), 400.f);
  EXPECT_EQ((C - A).ValueInRadians(), -.5f);
  EXPECT_EQ((C - B).ValueInRadians(), .5f);
  EXPECT_EQ((C - D).ValueInRadians(), 400.5f);
  EXPECT_EQ((D - A).ValueInRadians(), -401.f);
  EXPECT_EQ((D - B).ValueInRadians(), -400.f);
  EXPECT_EQ((D - C).ValueInRadians(), -400.5f);
}

TEST(AngleTest, Negation) {
  EXPECT_EQ((-Angle::Radians(1)).ValueInRadians(), -1.f);
  EXPECT_EQ((-Angle::Radians(0)).ValueInRadians(), 0.f);
  EXPECT_EQ((-Angle::Radians(.5)).ValueInRadians(), -.5f);
  EXPECT_EQ((-Angle::Radians(-400)).ValueInRadians(), 400.f);
}

TEST(AngleTest, Multiplication) {
  EXPECT_FLOAT_EQ((Angle::Radians(5) * 2).ValueInRadians(), 10.f);
  EXPECT_FLOAT_EQ((Angle::Radians(5) * 0).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Angle::Radians(5) * .2).ValueInRadians(), 1.f);
  EXPECT_FLOAT_EQ((Angle::Radians(5) * -3).ValueInRadians(), -15.f);
  EXPECT_FLOAT_EQ((Angle::Radians(0) * 30).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) * 2).ValueInRadians(), 1.6f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) * 0).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) * .9).ValueInRadians(), .72f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) * -9).ValueInRadians(), -7.2f);
  EXPECT_FLOAT_EQ((Angle::Radians(-3) * 2).ValueInRadians(), -6.f);
  EXPECT_FLOAT_EQ((Angle::Radians(-3) * 0).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Angle::Radians(-3) * .2).ValueInRadians(), -.6f);
  EXPECT_FLOAT_EQ((Angle::Radians(-3) * -3).ValueInRadians(), 9.f);
}

TEST(AngleTest, Division) {
  EXPECT_FLOAT_EQ((Angle::Radians(6) / 2).ValueInRadians(), 3.f);
  EXPECT_FLOAT_EQ((Angle::Radians(6) / .5).ValueInRadians(), 12.f);
  EXPECT_FLOAT_EQ((Angle::Radians(6) / -3).ValueInRadians(), -2.f);
  EXPECT_FLOAT_EQ((Angle::Radians(0) / 30).ValueInRadians(), 0.f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) / 2).ValueInRadians(), .4f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) / .4).ValueInRadians(), 2.f);
  EXPECT_FLOAT_EQ((Angle::Radians(.8) / -8).ValueInRadians(), -.1f);
  EXPECT_FLOAT_EQ((Angle::Radians(-4) / 2).ValueInRadians(), -2.f);
  EXPECT_FLOAT_EQ((Angle::Radians(-4) / .5).ValueInRadians(), -8.f);
  EXPECT_FLOAT_EQ((Angle::Radians(-4) / -8).ValueInRadians(), .5f);
  EXPECT_FLOAT_EQ(Angle::Radians(4) / Angle::Radians(2), 2.f);
  EXPECT_FLOAT_EQ(Angle::Radians(-.8) / Angle::Radians(8), -.1f);
  EXPECT_FLOAT_EQ(Angle::Radians(0) / Angle::Radians(32), 0.f);
}

TEST(AngleTest, AdditionAssignment) {
  Angle angle = Angle::Radians(1);
  EXPECT_FLOAT_EQ((angle += Angle::Radians(2)).ValueInRadians(), 3);
  EXPECT_FLOAT_EQ((angle += Angle::Radians(0.01)).ValueInRadians(), 3.01);
}

TEST(AngleTest, SubtractionAssignment) {
  Angle angle = Angle::Radians(3);
  EXPECT_FLOAT_EQ((angle -= Angle::Radians(2)).ValueInRadians(), 1);
  EXPECT_FLOAT_EQ((angle -= Angle::Radians(-5)).ValueInRadians(), 6);
}

TEST(AngleTest, MultiplicationAssignment) {
  Angle angle = Angle::Radians(0.2);
  EXPECT_FLOAT_EQ((angle *= 5).ValueInRadians(), 1);
  EXPECT_FLOAT_EQ((angle *= 0.5).ValueInRadians(), 0.5);
}

TEST(AngleTest, DivisionAssignment) {
  Angle angle = Angle::Radians(7);
  EXPECT_FLOAT_EQ((angle /= 2).ValueInRadians(), 3.5);
  EXPECT_FLOAT_EQ((angle /= 0.1).ValueInRadians(), 35);
}

}  // namespace
}  // namespace ink
