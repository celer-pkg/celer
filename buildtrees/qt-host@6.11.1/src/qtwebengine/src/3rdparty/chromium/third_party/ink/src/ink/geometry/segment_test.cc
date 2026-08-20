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

#include "ink/geometry/segment.h"

#include <cmath>
#include <limits>
#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

using ::testing::FloatEq;
using ::testing::Optional;

TEST(SegmentTest, Stringify) {
  EXPECT_EQ(absl::StrCat(Segment()), "Segment[(0, 0) -> (0, 0)]");
  EXPECT_EQ(absl::StrCat(Segment{{-1, 2}, {3, 4}}),
            "Segment[(-1, 2) -> (3, 4)]");
  EXPECT_EQ(absl::StrCat(Segment{{-std::numeric_limits<float>::infinity(), 0},
                                 {0, std::numeric_limits<float>::infinity()}}),
            "Segment[(-inf, 0) -> (0, inf)]");
}

TEST(SegmentTest, Equality) {
  EXPECT_EQ((Segment{{1, 2}, {0, 0}}), (Segment{{1, 2}, {0, 0}}));
  EXPECT_EQ((Segment{{-.4, 17}, {-9, 6}}), (Segment{{-.4, 17}, {-9, 6}}));

  // A difference in any part of the segments result in inequality.
  EXPECT_NE((Segment{{1, 2}, {0, 0}}), (Segment{{1, 2}, {0, 1}}));
  EXPECT_NE((Segment{{1, 2}, {0, 0}}), (Segment{{1, 2}, {17, 0}}));
  EXPECT_NE((Segment{{1, 2}, {0, 0}}), (Segment{{1, -2}, {0, 0}}));
  EXPECT_NE((Segment{{1, 2}, {0, 0}}), (Segment{{.5, 2}, {0, 0}}));

  // Segments with flipped endpoints are not considered equal.
  EXPECT_NE((Segment{{1, 2}, {0, 0}}), (Segment{{0, 0}, {1, 2}}));
}

TEST(SegmentTest, SegmentEq) {
  EXPECT_THAT((Segment{{1, 2}, {0, 0}}), SegmentEq(Segment{{1, 2}, {0, 0}}));
  EXPECT_THAT((Segment{{-.4, 17}, {-9, 6}}),
              SegmentEq(Segment{{-.4, 17}, {-9, 6}}));

  EXPECT_THAT((Segment{{1, 2}, {0, 0}}),
              Not(SegmentEq(Segment{{5, 12}, {2, 90}})));
  EXPECT_THAT((Segment{{1, 2}, {0, 0}}),
              Not(SegmentEq(Segment{{0, 0}, {1, 2}})));
}

TEST(SegmentTest, Vector) {
  // Typical cases.
  EXPECT_THAT((Segment{{0, 0}, {1, 1}}).Vector(), VecEq({1, 1}));
  EXPECT_THAT((Segment{{-4, 2}, {0, 5}}).Vector(), VecEq({4, 3}));
  EXPECT_THAT((Segment{{0, 1}, {-1, 3}}).Vector(), VecEq({-1, 2}));
  EXPECT_THAT((Segment{{3, 4}, {-1, -1}}).Vector(), VecEq({-4, -5}));
  EXPECT_THAT((Segment{{0.6, 1.9}, {-1.2, 3.3}}).Vector(), VecEq({-1.8, 1.4}));

  // // Horizontal segments.
  EXPECT_THAT((Segment{{1, 1}, {1, -3}}).Vector(), VecEq({0, -4}));
  EXPECT_THAT((Segment{{3, -2}, {3, 4}}).Vector(), VecEq({0, 6}));

  // // Vertical segments.
  EXPECT_THAT((Segment{{4, 1}, {5, 1}}).Vector(), VecEq({1, 0}));
  EXPECT_THAT((Segment{{-1, -5}, {-3, -5}}).Vector(), VecEq({-2, 0}));

  // // Degenerate segments.
  EXPECT_THAT((Segment{{1, -5}, {1, -5}}).Vector(), VecEq({0, 0}));
}

TEST(SegmentTest, Length) {
  // Typical cases.
  EXPECT_FLOAT_EQ(std::sqrt(2), (Segment{{0, 0}, {1, 1}}).Length());
  EXPECT_FLOAT_EQ(5, (Segment{{-4, 2}, {0, 5}}).Length());
  EXPECT_FLOAT_EQ(std::sqrt(5), (Segment{{0, 1}, {-1, 3}}).Length());
  EXPECT_FLOAT_EQ(std::sqrt(41), (Segment{{3, 4}, {-1, -1}}).Length());

  // // Horizontal segments.
  EXPECT_FLOAT_EQ(4, (Segment{{1, 1}, {1, -3}}).Length());
  EXPECT_FLOAT_EQ(6, (Segment{{3, -2}, {3, 4}}).Length());

  // // Vertical segments.
  EXPECT_FLOAT_EQ(1, (Segment{{4, 1}, {5, 1}}).Length());
  EXPECT_FLOAT_EQ(2, (Segment{{-1, -5}, {-3, -5}}).Length());

  // // Degenerate segments.
  EXPECT_FLOAT_EQ(0, (Segment{{1, -5}, {1, -5}}).Length());
}

TEST(SegmentTest, Midpoint) {
  // Typical cases.
  EXPECT_THAT((Segment{{0, 0}, {1, 1}}).Midpoint(), PointEq({.5, .5}));
  EXPECT_THAT((Segment{{-4, 2}, {0, 5}}).Midpoint(), PointEq({-2, 3.5}));
  EXPECT_THAT((Segment{{0, 1}, {-1, 3}}).Midpoint(), PointEq({-.5, 2}));
  EXPECT_THAT((Segment{{3, 4}, {-1, -1}}).Midpoint(), PointEq({1, 1.5}));
  EXPECT_THAT((Segment{{0.6, 1.9}, {-1.2, 3.3}}).Midpoint(),
              PointEq({-.3, 2.6}));

  // // Horizontal segments.
  EXPECT_THAT((Segment{{1, 1}, {1, -3}}).Midpoint(), PointEq({1, -1}));
  EXPECT_THAT((Segment{{3, -2}, {3, 4}}).Midpoint(), PointEq({3, 1}));

  // // Vertical segments.
  EXPECT_THAT((Segment{{4, 1}, {5, 1}}).Midpoint(), PointEq({4.5, 1}));
  EXPECT_THAT((Segment{{-1, -5}, {-3, -5}}).Midpoint(), PointEq({-2, -5}));

  // // Degenerate segments.
  EXPECT_THAT((Segment{{1, -5}, {1, -5}}).Midpoint(), PointEq({1, -5}));
}

// Tests that Segment::Midpoint() will not result in float overflow.
void MidpointIsFinite(Segment segment) {
  EXPECT_THAT(segment.Midpoint(), IsFinitePoint());
}
FUZZ_TEST(SegmentTest, MidpointIsFinite).WithDomains(FiniteSegment());

TEST(SegmentTest, Lerp) {
  Segment test_segment = Segment{{6, 3}, {8, -5}};

  EXPECT_THAT(test_segment.Lerp(0), PointEq({6, 3}));
  EXPECT_THAT(test_segment.Lerp(1), PointEq({8, -5}));

  EXPECT_THAT(test_segment.Lerp(.2), PointEq({6.4, 1.4}));
  EXPECT_THAT(test_segment.Lerp(.5), PointEq({7, -1}));
  EXPECT_THAT(test_segment.Lerp(.9), PointEq({7.8, -4.2}));

  EXPECT_THAT(test_segment.Lerp(-1), PointEq({4, 11}));
  EXPECT_THAT(test_segment.Lerp(1.3), PointEq({8.6, -7.4}));
}

void LerpZeroIsStart(Segment segment) {
  EXPECT_THAT(segment.Lerp(0.f), PointEq(segment.start));
}
FUZZ_TEST(SegmentTest, LerpZeroIsStart).WithDomains(FiniteSegment());

void LerpOneIsEnd(Segment segment) {
  EXPECT_THAT(segment.Lerp(1.f), PointEq(segment.end));
}
FUZZ_TEST(SegmentTest, LerpOneIsEnd).WithDomains(FiniteSegment());

// Tests that Segment::Lerp() will not result in float overflow for [0, 1].
void LerpZeroToOneIsFinite(Segment segment, float ratio) {
  EXPECT_THAT(segment.Lerp(ratio), IsFinitePoint());
}
FUZZ_TEST(SegmentTest, LerpZeroToOneIsFinite)
    .WithDomains(FiniteSegment(), fuzztest::InRange(0.f, 1.f));

TEST(SegmentTest, Project) {
  Segment test_segment = Segment{{0, 0}, {1, 1}};

  // On the endpoints.
  EXPECT_THAT(test_segment.Project({0, 0}), Optional(FloatEq(0.0f)));
  EXPECT_THAT(test_segment.Project({1, 1}), Optional(FloatEq(1.0f)));

  // On the segment.
  EXPECT_THAT(test_segment.Project({0.1, 0.1}), Optional(FloatEq(0.1f)));
  EXPECT_THAT(test_segment.Project({0.6, 0.6}), Optional(FloatEq(0.6f)));

  // On the line, but past the ends of the segment.
  EXPECT_THAT(test_segment.Project({-1, -1}), Optional(FloatEq(-1.0f)));
  EXPECT_THAT(test_segment.Project({2, 2}), Optional(FloatEq(2.0f)));
  EXPECT_THAT(test_segment.Project({-10, -10}), Optional(FloatEq(-10.0f)));
  EXPECT_THAT(test_segment.Project({50, 50}), Optional(FloatEq(50.0f)));

  // Off to the side of the line.
  EXPECT_THAT(test_segment.Project({0, 1}), Optional(FloatEq(0.5f)));
  EXPECT_THAT(test_segment.Project({1, 0}), Optional(FloatEq(0.5f)));
  EXPECT_THAT(test_segment.Project({0.7, 0.2}), Optional(FloatEq(0.45f)));
}

TEST(SegmentTest, ProjectToDegenerateSegment) {
  // Degenerate segment.
  EXPECT_EQ(std::nullopt, (Segment{{2, 3}, {2, 3}}).Project({1, 1}));

  // This segment is technically not degenerate, as the endpoints are different.
  // However, it's so small that it's squared length underflows to zero.
  EXPECT_EQ(std::nullopt, (Segment{{0, 0}, {1e-23, 1e-23}}).Project({1, 1}));

  // This returns nullopt for degenerate segments even if the point equal to one
  // of the endpoints.
  EXPECT_EQ(std::nullopt, (Segment{{2, 3}, {2, 3}}).Project({2, 3}));
  EXPECT_EQ(std::nullopt, (Segment{{0, 0}, {1e-23, 1e-23}}).Project({0, 0}));
  EXPECT_EQ(std::nullopt,
            (Segment{{0, 0}, {1e-23, 1e-23}}).Project({1e-23, 1e-23}));
}

}  // namespace
}  // namespace ink
