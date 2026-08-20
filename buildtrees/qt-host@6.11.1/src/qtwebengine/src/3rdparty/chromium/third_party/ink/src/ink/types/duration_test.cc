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

#include "ink/types/duration.h"

#include <cmath>
#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "ink/types/fuzz_domains.h"
#include "ink/types/type_matchers.h"

namespace ink {
namespace {

using ::testing::FloatEq;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Not;

TEST(Duration32Test, Stringify) {
  // Duration32's AbslStringify implementation delegates to absl::Duration's
  // implementation, and we don't want to write a brittle test that will break
  // if they ever change their output format.  But we do want to exercise our
  // own code, and make sure that it's doing _something_ plausible, so here's a
  // pretty minimal test.
  EXPECT_THAT(absl::StrCat(Duration32::Seconds(4)), HasSubstr("4"));
  EXPECT_THAT(absl::StrCat(Duration32::Seconds(7)), HasSubstr("7"));

  // For duration values outside the range of absl::Duration, we fall back to
  // just printing the number of seconds (so that they don't print as "inf").
  EXPECT_EQ(
      absl::StrCat(Duration32::Seconds(std::numeric_limits<float>::max())),
      "3.40282e+38s");
  EXPECT_EQ(
      absl::StrCat(Duration32::Seconds(-std::numeric_limits<float>::max())),
      "-3.40282e+38s");

  // Only actually-infinite Duration32 values should print as "inf".
  EXPECT_EQ(absl::StrCat(Duration32::Infinite()), "inf");
  EXPECT_EQ(absl::StrCat(-Duration32::Infinite()), "-inf");
}

void CanStringifyAnyDuration32WithoutCrashing(Duration32 duration) {
  EXPECT_THAT(absl::StrCat(duration), Not(IsEmpty()));
}
FUZZ_TEST(Duration32Test, CanStringifyAnyDuration32WithoutCrashing)
    .WithDomains(ArbitraryDuration32());

TEST(Duration32Test, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Duration32::Zero(),
      Duration32::Infinite(),
      -Duration32::Infinite(),
      Duration32::Seconds(1),
      Duration32::Seconds(-1),
      Duration32::Seconds(2),
      Duration32::Seconds(std::numeric_limits<float>::max()),
      Duration32::Seconds(std::numeric_limits<float>::lowest()),
  }));
}

TEST(Duration32Test, DefaultConstructedIsZero) {
  Duration32 d;
  EXPECT_EQ(d.ToSeconds(), 0);
  EXPECT_EQ(d.ToMillis(), 0);
  EXPECT_EQ(d.ToAbseil(), absl::ZeroDuration());
  EXPECT_EQ(d, Duration32::Zero());
}

TEST(Duration32Test, FromFloatSeconds) {
  Duration32 duration = Duration32::Seconds(0);
  EXPECT_EQ(duration.ToSeconds(), 0);
  EXPECT_EQ(duration.ToMillis(), 0);
  EXPECT_EQ(duration.ToAbseil(), absl::ZeroDuration());

  duration = Duration32::Seconds(17.3f);
  EXPECT_EQ(duration.ToSeconds(), 17.3f);
  EXPECT_EQ(duration.ToMillis(), 17300.f);
  EXPECT_FLOAT_EQ(absl::ToDoubleSeconds(duration.ToAbseil()), 17.3f);

  duration = Duration32::Seconds(-5.4f);
  EXPECT_EQ(duration.ToSeconds(), -5.4f);
  EXPECT_EQ(duration.ToMillis(), -5400.f);
  EXPECT_FLOAT_EQ(absl::ToDoubleSeconds(duration.ToAbseil()), -5.4f);

  duration = Duration32::Seconds(std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToSeconds(), std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToMillis(), std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToAbseil(), absl::InfiniteDuration());
}

TEST(Duration32Test, FromFloatMillis) {
  Duration32 duration = Duration32::Millis(0);
  EXPECT_EQ(duration.ToSeconds(), 0);
  EXPECT_EQ(duration.ToMillis(), 0);
  EXPECT_EQ(duration.ToAbseil(), absl::ZeroDuration());

  duration = Duration32::Millis(42.5f);
  EXPECT_EQ(duration.ToSeconds(), 0.0425f);
  EXPECT_EQ(duration.ToMillis(), 42.5f);
  EXPECT_FLOAT_EQ(absl::ToDoubleSeconds(duration.ToAbseil()), 0.0425f);

  duration = Duration32::Millis(-0.25f);
  EXPECT_EQ(duration.ToSeconds(), -0.00025f);
  EXPECT_EQ(duration.ToMillis(), -0.25f);
  EXPECT_FLOAT_EQ(absl::ToDoubleSeconds(duration.ToAbseil()), -0.00025f);

  duration = Duration32::Millis(std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToSeconds(), std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToMillis(), std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToAbseil(), absl::InfiniteDuration());
}

TEST(Duration32Test, FromFloatNanIsInfinite) {
  EXPECT_EQ(Duration32::Seconds(std::nanf("")), Duration32::Infinite());
  EXPECT_EQ(Duration32::Millis(std::nanf("")), Duration32::Infinite());
}

TEST(Duration32Test, FromAbseilDuration) {
  Duration32 duration(absl::ZeroDuration());
  EXPECT_EQ(duration.ToSeconds(), 0);
  EXPECT_EQ(duration.ToMillis(), 0);

  duration = Duration32(absl::Seconds(3));
  EXPECT_EQ(duration.ToSeconds(), 3.f);
  EXPECT_EQ(duration.ToMillis(), 3000.f);

  duration = Duration32(absl::Seconds(-28.5f));
  EXPECT_EQ(duration.ToSeconds(), -28.5f);
  EXPECT_EQ(duration.ToMillis(), -28500.f);

  duration = Duration32(absl::InfiniteDuration());
  EXPECT_EQ(duration.ToSeconds(), std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToMillis(), std::numeric_limits<float>::infinity());

  duration = Duration32(-absl::InfiniteDuration());
  EXPECT_EQ(duration.ToSeconds(), -std::numeric_limits<float>::infinity());
  EXPECT_EQ(duration.ToMillis(), -std::numeric_limits<float>::infinity());
}

TEST(Duration32Test, Negation) {
  EXPECT_EQ((-Duration32::Seconds(5)).ToSeconds(), -5.f);
  EXPECT_EQ((-Duration32::Seconds(-1)).ToSeconds(), 1.f);
  EXPECT_EQ((-Duration32::Infinite()).ToSeconds(),
            -std::numeric_limits<float>::infinity());
}

TEST(Duration32Test, IsFinite) {
  EXPECT_FALSE(Duration32::Infinite().IsFinite());
  EXPECT_FALSE((-Duration32::Infinite()).IsFinite());
  EXPECT_TRUE(Duration32::Zero().IsFinite());
  EXPECT_TRUE(Duration32::Seconds(5).IsFinite());
  EXPECT_TRUE(Duration32::Seconds(-500).IsFinite());
  EXPECT_TRUE(Duration32::Millis(37).IsFinite());
  EXPECT_TRUE(Duration32::Millis(-0.001f).IsFinite());
}

TEST(Duration32Test, CompareEqual) {
  EXPECT_EQ(Duration32::Seconds(15), Duration32::Seconds(15));
  EXPECT_FALSE(Duration32::Seconds(15) == Duration32::Seconds(16));  // NOLINT

  EXPECT_EQ(Duration32::Infinite(), Duration32::Infinite());
  EXPECT_FALSE(Duration32::Seconds(1000) == Duration32::Infinite());  // NOLINT
}

TEST(Duration32Test, CompareUnequal) {
  EXPECT_NE(Duration32::Millis(32), Duration32::Millis(40));
  EXPECT_FALSE(Duration32::Seconds(32) != Duration32::Seconds(32));  // NOLINT

  EXPECT_FALSE(Duration32::Infinite() != Duration32::Infinite());  // NOLINT
  EXPECT_LT(Duration32::Seconds(1000), Duration32::Infinite());
}

TEST(Duration32Test, CompareLessThan) {
  EXPECT_LT(Duration32::Seconds(2), Duration32::Seconds(3));
  EXPECT_FALSE(Duration32::Seconds(3) < Duration32::Seconds(3));  // NOLINT
  EXPECT_FALSE(Duration32::Seconds(4) < Duration32::Seconds(3));  // NOLINT

  EXPECT_LT(Duration32::Seconds(20), Duration32::Infinite());
  EXPECT_LT(-Duration32::Infinite(), Duration32::Seconds(20));

  EXPECT_LT(-Duration32::Infinite(), Duration32::Infinite());
  EXPECT_FALSE(Duration32::Infinite() < -Duration32::Infinite());   // NOLINT
  EXPECT_FALSE(Duration32::Infinite() < Duration32::Infinite());    // NOLINT
  EXPECT_FALSE(-Duration32::Infinite() < -Duration32::Infinite());  // NOLINT
}

TEST(Duration32Test, CompareGreaterThan) {
  EXPECT_GT(Duration32::Seconds(5), Duration32::Seconds(3));
  EXPECT_FALSE(Duration32::Seconds(5) > Duration32::Seconds(5));  // NOLINT
  EXPECT_FALSE(Duration32::Seconds(2) > Duration32::Seconds(5));  // NOLINT

  EXPECT_GT(Duration32::Infinite(), Duration32::Seconds(20));
  EXPECT_GT(Duration32::Seconds(20), -Duration32::Infinite());

  EXPECT_GT(Duration32::Infinite(), -Duration32::Infinite());
  EXPECT_FALSE(-Duration32::Infinite() > Duration32::Infinite());   // NOLINT
  EXPECT_FALSE(Duration32::Infinite() > Duration32::Infinite());    // NOLINT
  EXPECT_FALSE(-Duration32::Infinite() > -Duration32::Infinite());  // NOLINT
}

TEST(Duration32Test, CompareLessThanOrEqualTo) {
  EXPECT_LE(Duration32::Seconds(2), Duration32::Seconds(3));
  EXPECT_LE(Duration32::Seconds(3), Duration32::Seconds(3));
  EXPECT_FALSE(Duration32::Seconds(4) <= Duration32::Seconds(3));  // NOLINT

  EXPECT_LE(Duration32::Seconds(20), Duration32::Infinite());
  EXPECT_LE(-Duration32::Infinite(), Duration32::Seconds(20));

  EXPECT_LE(-Duration32::Infinite(), Duration32::Infinite());
  EXPECT_LE(Duration32::Infinite(), Duration32::Infinite());
  EXPECT_LE(-Duration32::Infinite(), -Duration32::Infinite());
  EXPECT_FALSE(Duration32::Infinite() <= -Duration32::Infinite());  // NOLINT
}

TEST(Duration32Test, CompareGreaterThanOrEqualTo) {
  EXPECT_GE(Duration32::Seconds(5), Duration32::Seconds(3));
  EXPECT_GE(Duration32::Seconds(5), Duration32::Seconds(5));
  EXPECT_FALSE(Duration32::Seconds(2) >= Duration32::Seconds(5));  // NOLINT

  EXPECT_GE(Duration32::Seconds(20), -Duration32::Infinite());
  EXPECT_GE(Duration32::Infinite(), Duration32::Seconds(20));

  EXPECT_GE(Duration32::Infinite(), -Duration32::Infinite());
  EXPECT_GE(Duration32::Infinite(), Duration32::Infinite());
  EXPECT_GE(-Duration32::Infinite(), -Duration32::Infinite());
  EXPECT_FALSE(-Duration32::Infinite() >= Duration32::Infinite());  // NOLINT
}

TEST(Duration32Test, Add) {
  Duration32 pos_inf = Duration32::Infinite();
  Duration32 neg_inf = -Duration32::Infinite();
  EXPECT_THAT(Duration32::Seconds(1) + Duration32::Seconds(2),
              Duration32Seconds(FloatEq(3)));
  EXPECT_EQ(Duration32::Seconds(1) + pos_inf, pos_inf);
  EXPECT_EQ(pos_inf + Duration32::Seconds(1), pos_inf);
  EXPECT_EQ(Duration32::Seconds(1) + neg_inf, neg_inf);
  EXPECT_EQ(neg_inf + Duration32::Seconds(1), neg_inf);
  EXPECT_EQ(pos_inf + pos_inf, pos_inf);
  EXPECT_EQ(pos_inf + neg_inf, pos_inf);
  EXPECT_EQ(neg_inf + pos_inf, neg_inf);
  EXPECT_EQ(neg_inf + neg_inf, neg_inf);
}

void PositiveInfiniteDurationPlusAnythingIsUnchanged(Duration32 duration) {
  EXPECT_EQ(Duration32::Infinite() + duration, Duration32::Infinite())
      << "Where duration is: " << testing::PrintToString(duration);
}
FUZZ_TEST(Duration32Test, PositiveInfiniteDurationPlusAnythingIsUnchanged)
    .WithDomains(ArbitraryDuration32());

void NegitiveInfiniteDurationPlusAnythingIsUnchanged(Duration32 duration) {
  EXPECT_EQ(-Duration32::Infinite() + duration, -Duration32::Infinite())
      << "Where duration is: " << testing::PrintToString(duration);
}
FUZZ_TEST(Duration32Test, NegitiveInfiniteDurationPlusAnythingIsUnchanged)
    .WithDomains(ArbitraryDuration32());

TEST(Duration32Test, Subtract) {
  Duration32 pos_inf = Duration32::Infinite();
  Duration32 neg_inf = -Duration32::Infinite();
  EXPECT_THAT(Duration32::Seconds(1) - Duration32::Seconds(2),
              Duration32Seconds(FloatEq(-1.f)));
  EXPECT_EQ(Duration32::Seconds(1) - pos_inf, neg_inf);
  EXPECT_EQ(pos_inf - Duration32::Seconds(1), pos_inf);
  EXPECT_EQ(Duration32::Seconds(1) - neg_inf, pos_inf);
  EXPECT_EQ(neg_inf - Duration32::Seconds(1), neg_inf);
  EXPECT_EQ(pos_inf - pos_inf, pos_inf);
  EXPECT_EQ(pos_inf - neg_inf, pos_inf);
  EXPECT_EQ(neg_inf - pos_inf, neg_inf);
  EXPECT_EQ(neg_inf - neg_inf, neg_inf);
}

void PositiveInfiniteDurationMinusAnythingIsUnchanged(Duration32 duration) {
  EXPECT_EQ(Duration32::Infinite() - duration, Duration32::Infinite())
      << "Where duration is: " << testing::PrintToString(duration);
}
FUZZ_TEST(Duration32Test, PositiveInfiniteDurationMinusAnythingIsUnchanged)
    .WithDomains(ArbitraryDuration32());

void NegitiveInfiniteDurationMinusAnythingIsUnchanged(Duration32 duration) {
  EXPECT_EQ(-Duration32::Infinite() - duration, -Duration32::Infinite())
      << "Where duration is: " << testing::PrintToString(duration);
}
FUZZ_TEST(Duration32Test, NegitiveInfiniteDurationMinusAnythingIsUnchanged)
    .WithDomains(ArbitraryDuration32());

TEST(Duration32Test, Multiply) {
  EXPECT_THAT(5 * Duration32::Seconds(1), Duration32Seconds(FloatEq(5)));
  EXPECT_THAT(Duration32::Seconds(1) * 7, Duration32Seconds(FloatEq(7)));
  EXPECT_EQ(-3 * Duration32::Infinite(), -Duration32::Infinite());
  EXPECT_EQ(Duration32::Infinite() * 0, Duration32::Infinite());

  EXPECT_EQ(std::nanf("") * Duration32::Seconds(5), Duration32::Infinite());
  EXPECT_EQ(std::nanf("") * Duration32::Seconds(0), Duration32::Infinite());
  EXPECT_EQ(std::nanf("") * Duration32::Seconds(-5), -Duration32::Infinite());
  EXPECT_EQ(Duration32::Seconds(2) * std::nanf(""), Duration32::Infinite());
  EXPECT_EQ(Duration32::Seconds(0) * std::nanf(""), Duration32::Infinite());
  EXPECT_EQ(Duration32::Seconds(-2) * std::nanf(""), -Duration32::Infinite());
}

TEST(Duration32Test, DivideDurationByFloat) {
  EXPECT_THAT(Duration32::Seconds(1) / 4, Duration32Seconds(FloatEq(0.25f)));
  EXPECT_EQ(Duration32::Infinite() / 2, Duration32::Infinite());

  EXPECT_EQ(Duration32::Infinite() / std::numeric_limits<float>::infinity(),
            Duration32::Infinite());
  EXPECT_EQ(-Duration32::Infinite() / std::numeric_limits<float>::infinity(),
            -Duration32::Infinite());
  EXPECT_EQ(Duration32::Infinite() / -std::numeric_limits<float>::infinity(),
            -Duration32::Infinite());
  EXPECT_EQ(-Duration32::Infinite() / -std::numeric_limits<float>::infinity(),
            Duration32::Infinite());

  EXPECT_EQ(Duration32::Seconds(2) / std::nanf(""), Duration32::Infinite());
  EXPECT_EQ(Duration32::Seconds(0) / std::nanf(""), Duration32::Infinite());
  EXPECT_EQ(Duration32::Seconds(-2) / std::nanf(""), -Duration32::Infinite());
}

TEST(Duration32Test, DivideDurationByDuration) {
  EXPECT_FLOAT_EQ(Duration32::Seconds(3) / Duration32::Seconds(4), 0.75f);

  EXPECT_EQ(Duration32::Infinite() / Duration32::Infinite(),
            std::numeric_limits<float>::infinity());
  EXPECT_EQ(-Duration32::Infinite() / Duration32::Infinite(),
            -std::numeric_limits<float>::infinity());
  EXPECT_EQ(Duration32::Infinite() / -Duration32::Infinite(),
            -std::numeric_limits<float>::infinity());
  EXPECT_EQ(-Duration32::Infinite() / -Duration32::Infinite(),
            std::numeric_limits<float>::infinity());

  EXPECT_EQ(Duration32::Zero() / Duration32::Zero(),
            std::numeric_limits<float>::infinity());
}

TEST(Duration32Test, AddAssign) {
  Duration32 duration = Duration32::Seconds(5);
  EXPECT_EQ(&(duration += -Duration32::Seconds(3)), &duration);
  EXPECT_THAT(duration, Duration32Seconds(FloatEq(2)));

  duration = Duration32::Seconds(2);
  EXPECT_EQ(duration += Duration32::Infinite(), Duration32::Infinite());

  duration = Duration32::Seconds(7);
  EXPECT_EQ(duration += -Duration32::Infinite(), -Duration32::Infinite());
}

TEST(Duration32Test, SubtractAssign) {
  Duration32 duration = Duration32::Seconds(5);
  EXPECT_EQ(&(duration -= Duration32::Seconds(7)), &duration);
  EXPECT_THAT(duration, Duration32Seconds(FloatEq(-2)));

  duration = Duration32::Seconds(2);
  EXPECT_EQ(duration -= Duration32::Infinite(), -Duration32::Infinite());
  duration = Duration32::Seconds(7);
  EXPECT_EQ(duration -= -Duration32::Infinite(), Duration32::Infinite());

  duration = Duration32::Infinite();
  EXPECT_EQ(duration -= Duration32::Infinite(), Duration32::Infinite());
  duration = -Duration32::Infinite();
  EXPECT_EQ(duration -= -Duration32::Infinite(), -Duration32::Infinite());
}

TEST(Duration32Test, MultiplyAssign) {
  Duration32 duration = Duration32::Seconds(5);
  EXPECT_EQ(&(duration *= 0.5), &duration);
  EXPECT_THAT(duration, Duration32Seconds(FloatEq(2.5f)));

  duration = Duration32::Seconds(2);
  EXPECT_EQ(duration *= std::nanf(""), Duration32::Infinite());
  duration = Duration32::Seconds(0);
  EXPECT_EQ(duration *= std::nanf(""), Duration32::Infinite());
  duration = Duration32::Seconds(-7);
  EXPECT_EQ(duration *= std::nanf(""), -Duration32::Infinite());
}

TEST(Duration32Test, DivideAssign) {
  Duration32 duration = Duration32::Seconds(5);
  EXPECT_EQ(&(duration /= 20), &duration);
  EXPECT_THAT(duration, Duration32Seconds(FloatEq(0.25f)));

  duration = Duration32::Seconds(2);
  EXPECT_EQ(duration /= std::nanf(""), Duration32::Infinite());
  duration = Duration32::Seconds(0);
  EXPECT_EQ(duration /= std::nanf(""), Duration32::Infinite());
  duration = Duration32::Seconds(-5);
  EXPECT_EQ(duration /= std::nanf(""), -Duration32::Infinite());
}

TEST(Duration32Test, DocumentedPrecisionDurationsAreDistinct) {
  Duration32 long_duration = 28 * 60 * Duration32::Seconds(1);
  Duration32 short_duration = Duration32::Millis(0.1);
  EXPECT_LT(long_duration, long_duration + short_duration);
  EXPECT_NE(long_duration, long_duration + short_duration);
  EXPECT_GT(long_duration, long_duration - short_duration);
  EXPECT_NE(long_duration, long_duration - short_duration);
}

}  // namespace
}  // namespace ink
