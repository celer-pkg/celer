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

#include "ink/types/physical_distance.h"

#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_cat.h"
#include "ink/types/fuzz_domains.h"
#include "ink/types/type_matchers.h"

namespace ink {
namespace {

using ::testing::FloatEq;
using ::testing::IsEmpty;
using ::testing::Not;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();

TEST(PhysicalDistanceTest, HasMicronPrecisionUpToTwentyMeters) {
  // Validate the claim made in the class documentation that `PhysicalDistance`
  // has micron precision up to twenty meters.
  PhysicalDistance micrometer = PhysicalDistance::Meters(1e-6);
  PhysicalDistance twenty_meters = PhysicalDistance::Meters(20);
  EXPECT_NE(twenty_meters + micrometer, twenty_meters);

  // At twenty-one meters, however, that precision begins to break down.
  PhysicalDistance twenty_one_meters = PhysicalDistance::Meters(21);
  EXPECT_EQ(twenty_one_meters + micrometer, twenty_one_meters);
}

TEST(PhysicalDistanceTest, Stringify) {
  EXPECT_EQ(absl::StrCat(PhysicalDistance::Zero()), "0cm");
  EXPECT_EQ(absl::StrCat(PhysicalDistance::Centimeters(1)), "1cm");
  EXPECT_EQ(absl::StrCat(PhysicalDistance::Centimeters(-1.5)), "-1.5cm");
  EXPECT_EQ(absl::StrCat(PhysicalDistance::Inches(1)), "2.54cm");
  EXPECT_EQ(absl::StrCat(PhysicalDistance::Meters(1)), "100cm");
}

void CanStringifyAnyPhysicalDistanceWithoutCrashing(PhysicalDistance distance) {
  EXPECT_THAT(absl::StrCat(distance), Not(IsEmpty()));
}
FUZZ_TEST(PhysicalDistanceTest, CanStringifyAnyPhysicalDistanceWithoutCrashing)
    .WithDomains(ArbitraryPhysicalDistance());

TEST(PhysicalDistanceTest, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      PhysicalDistance::Zero(),
      PhysicalDistance::Centimeters(1),
      PhysicalDistance::Centimeters(-1),
      PhysicalDistance::Centimeters(2),
      PhysicalDistance::Centimeters(std::numeric_limits<float>::max()),
      PhysicalDistance::Centimeters(std::numeric_limits<float>::lowest()),
      PhysicalDistance::Centimeters(kInfinity),
      PhysicalDistance::Centimeters(-kInfinity),
  }));
}

TEST(PhysicalDistanceTest, DefaultConstructedIsZero) {
  PhysicalDistance distance;
  EXPECT_EQ(distance.ToCentimeters(), 0);
  EXPECT_EQ(distance.ToInches(), 0);
  EXPECT_EQ(distance.ToMeters(), 0);
  EXPECT_EQ(distance, PhysicalDistance::Zero());
}

TEST(PhysicalDistanceTest, UnitConversion) {
  EXPECT_FLOAT_EQ(PhysicalDistance::Inches(1).ToCentimeters(), 2.54);
  EXPECT_FLOAT_EQ(PhysicalDistance::Inches(1).ToInches(), 1);
  EXPECT_FLOAT_EQ(PhysicalDistance::Inches(1).ToMeters(), 0.0254);

  EXPECT_FLOAT_EQ(PhysicalDistance::Centimeters(254).ToCentimeters(), 254);
  EXPECT_FLOAT_EQ(PhysicalDistance::Centimeters(254).ToInches(), 100);
  EXPECT_FLOAT_EQ(PhysicalDistance::Centimeters(254).ToMeters(), 2.54);

  EXPECT_FLOAT_EQ(PhysicalDistance::Meters(2.54).ToCentimeters(), 254);
  EXPECT_FLOAT_EQ(PhysicalDistance::Meters(2.54).ToInches(), 100);
  EXPECT_FLOAT_EQ(PhysicalDistance::Meters(2.54).ToMeters(), 2.54);
}

TEST(PhysicalDistanceTest, IsFinite) {
  EXPECT_FALSE(PhysicalDistance::Centimeters(kNan).IsFinite());
  EXPECT_FALSE(PhysicalDistance::Meters(kInfinity).IsFinite());
  EXPECT_FALSE(PhysicalDistance::Inches(-kInfinity).IsFinite());
  EXPECT_TRUE(PhysicalDistance::Zero().IsFinite());
  EXPECT_TRUE(PhysicalDistance::Centimeters(5).IsFinite());
  EXPECT_TRUE(PhysicalDistance::Centimeters(-500).IsFinite());
  EXPECT_TRUE(PhysicalDistance::Inches(37).IsFinite());
  EXPECT_TRUE(PhysicalDistance::Inches(-0.001f).IsFinite());
}

TEST(PhysicalDistanceTest, Comparisons) {
  EXPECT_TRUE(PhysicalDistance::Inches(1) == PhysicalDistance::Inches(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) == PhysicalDistance::Centimeters(1));

  EXPECT_TRUE(PhysicalDistance::Inches(1) != PhysicalDistance::Centimeters(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) != PhysicalDistance::Inches(1));

  EXPECT_TRUE(PhysicalDistance::Inches(1) > PhysicalDistance::Centimeters(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) > PhysicalDistance::Meters(1));

  EXPECT_TRUE(PhysicalDistance::Inches(1) < PhysicalDistance::Meters(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) < PhysicalDistance::Centimeters(1));

  EXPECT_TRUE(PhysicalDistance::Inches(1) >= PhysicalDistance::Inches(1));
  EXPECT_TRUE(PhysicalDistance::Inches(1) >= PhysicalDistance::Centimeters(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) >= PhysicalDistance::Meters(1));

  EXPECT_TRUE(PhysicalDistance::Inches(1) <= PhysicalDistance::Inches(1));
  EXPECT_TRUE(PhysicalDistance::Inches(1) <= PhysicalDistance::Meters(1));
  EXPECT_FALSE(PhysicalDistance::Inches(1) <= PhysicalDistance::Centimeters(1));
}

TEST(PhysicalDistanceTest, Negation) {
  EXPECT_EQ(-PhysicalDistance::Centimeters(5),
            PhysicalDistance::Centimeters(-5));
  EXPECT_EQ(-PhysicalDistance::Centimeters(-1),
            PhysicalDistance::Centimeters(1));
  EXPECT_EQ(-PhysicalDistance::Centimeters(kInfinity),
            PhysicalDistance::Centimeters(-kInfinity));
}

TEST(PhysicalDistanceTest, BinaryOperators) {
  EXPECT_THAT(PhysicalDistance::Inches(2) + PhysicalDistance::Inches(3),
              PhysicalDistanceEq(PhysicalDistance::Inches(5)));
  EXPECT_THAT(PhysicalDistance::Inches(2) - PhysicalDistance::Inches(3),
              PhysicalDistanceEq(PhysicalDistance::Inches(-1)));
  EXPECT_THAT(PhysicalDistance::Inches(2) * 3,
              PhysicalDistanceEq(PhysicalDistance::Inches(6)));
  EXPECT_THAT(2 * PhysicalDistance::Inches(3),
              PhysicalDistanceEq(PhysicalDistance::Inches(6)));
  EXPECT_THAT(2 * PhysicalDistance::Inches(3),
              PhysicalDistanceEq(PhysicalDistance::Inches(6)));
  EXPECT_THAT(PhysicalDistance::Inches(3) / 2,
              PhysicalDistanceEq(PhysicalDistance::Inches(1.5)));
  EXPECT_THAT(PhysicalDistance::Inches(3) / PhysicalDistance::Inches(2),
              FloatEq(1.5));
}

TEST(PhysicalDistanceTest, PlusEquals) {
  PhysicalDistance distance = PhysicalDistance::Inches(2);
  distance += PhysicalDistance::Inches(3);
  EXPECT_THAT(distance, PhysicalDistanceEq(PhysicalDistance::Inches(5)));
}

TEST(PhysicalDistanceTest, MinusEquals) {
  PhysicalDistance distance = PhysicalDistance::Inches(2);
  distance -= PhysicalDistance::Inches(3);
  EXPECT_THAT(distance, PhysicalDistanceEq(PhysicalDistance::Inches(-1)));
}

TEST(PhysicalDistanceTest, TimesEquals) {
  PhysicalDistance distance = PhysicalDistance::Inches(2);
  distance *= 3;
  EXPECT_THAT(distance, PhysicalDistanceEq(PhysicalDistance::Inches(6)));
}

TEST(PhysicalDistanceTest, DivideEquals) {
  PhysicalDistance distance = PhysicalDistance::Inches(3);
  distance /= 2;
  EXPECT_THAT(distance, PhysicalDistanceEq(PhysicalDistance::Inches(1.5)));
}

}  // namespace
}  // namespace ink
