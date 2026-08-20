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

#include "ink/geometry/envelope.h"

#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

using ::testing::Optional;

TEST(EnvelopeTest, DefaultConstructor) {
  Envelope envelope;
  EXPECT_TRUE(envelope.IsEmpty());
  EXPECT_EQ(envelope.AsRect(), std::nullopt);
}

TEST(EnvelopeTest, PointConstructor) {
  Envelope envelope(Point{3, 4});
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromCenterAndDimensions(Point{3, 4}, 0, 0))));
}

TEST(EnvelopeTest, SegmentConstructor) {
  Envelope envelope(Segment{Point{3, 4}, Point{-2.5, 0}});
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{3, 4}, Point{-2.5, 0}))));
}

TEST(EnvelopeTest, TriangleConstructor) {
  Envelope envelope(Triangle{Point{1, 2}, Point{0, 0}, Point{-4, 3}});
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-4, 0}, Point{1, 3}))));
}

TEST(EnvelopeTest, RectConstructor) {
  Envelope envelope(Rect::FromCenterAndDimensions(Point{-1.5, 1.5}, 5, 3));
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-4, 0}, Point{1, 3}))));
}

TEST(EnvelopeTest, QuadConstructor) {
  Envelope envelope(Quad::FromCenterDimensionsRotationAndShear(
      Point{0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f));
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{-2, -14}, Point{2, 14}))));
}

TEST(EnvelopeTest, IteratorConstructor) {
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  Envelope envelope(points.begin(), points.end());
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 4}))));
}

TEST(EnvelopeTest, ContainerConstructor) {
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  Envelope envelope(points);
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 4}))));
}

TEST(EnvelopeTest, AddPointToEmptyEnvelope) {
  Envelope envelope_1;
  envelope_1.Add(Point{3, 4});
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 3, 4)));

  Envelope envelope_2;
  envelope_2.Add(Point{-.05, 34.22});
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(-.05, 34.22, -.05, 34.22)));
}

TEST(EnvelopeTest, AddPointToNonEmptyEnvelope) {
  Envelope envelope_1;
  envelope_1.Add(Point{3, 4});
  EXPECT_FALSE(envelope_1.IsEmpty());
  envelope_1.Add(Point{15, 20});
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 15, 20)));

  Envelope envelope_2;
  envelope_2.Add(Point{-.05, 34.22});
  EXPECT_FALSE(envelope_2.IsEmpty());
  envelope_2.Add(Point{-14, 0});
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(-14, 0, -.05, 34.22)));
}

TEST(EnvelopeTest, AddSegmentToEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Segment{Point{3, 4}, Point{-2.5, 0}});
  EXPECT_FALSE(envelope.IsEmpty());
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{3, 4}, Point{-2.5, 0}))));
}

TEST(EnvelopeTest, AddSegmentToNonEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Segment{Point{3, 4}, Point{-2.5, 0}});
  EXPECT_FALSE(envelope.IsEmpty());
  envelope.Add(Segment{Point{9, 1}, Point{-1, 6}});
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{9, 6}, Point{-2.5, 0}))));
}

TEST(EnvelopeTest, AddTriangleToEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Triangle{Point{1, 2}, Point{0, 0}, Point{-4, 3}});
  EXPECT_FALSE(envelope.IsEmpty());
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-4, 0}, Point{1, 3}))));
}

TEST(EnvelopeTest, AddTriangleToNonEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Triangle{Point{1, 2}, Point{0, 0}, Point{-4, 3}});
  EXPECT_FALSE(envelope.IsEmpty());
  envelope.Add(Triangle{Point{-1, -3}, Point{3, -3}, Point{-3, -1}});
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{-4, -3}, Point{3, 3}))));
}

TEST(EnvelopeTest, AddRectToEmptyEnvelope) {
  Envelope envelope_1;
  envelope_1.Add(Rect::FromTwoPoints(Point{1, 2}, Point{4, 5}));
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(1, 2, 4, 5)));

  Envelope envelope_2;
  envelope_2.Add(Rect::FromTwoPoints(Point{-32.2, -4}, Point{6.4, 7.5}));
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(-32.2, -4, 6.4, 7.5)));
}

TEST(EnvelopeTest, AddRectToNonEmptyEnvelope) {
  Envelope envelope_1;
  envelope_1.Add(Rect::FromTwoPoints(Point{1, 2}, Point{4, 5}));
  EXPECT_FALSE(envelope_1.IsEmpty());
  envelope_1.Add(Rect::FromTwoPoints(Point{6, 7}, Point{10, 11}));
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(1, 2, 10, 11)));

  Envelope envelope_2;
  envelope_2.Add(Rect::FromTwoPoints(Point{-32.2, -4}, Point{6.4, 7.5}));
  EXPECT_FALSE(envelope_2.IsEmpty());
  envelope_2.Add(Rect::FromTwoPoints(Point{10.2, 13.2}, Point{25.1, 33.7}));
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(-32.2, -4, 25.1, 33.7)));
}

TEST(EnvelopeTest, AddQuadToEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Quad::FromCenterDimensionsRotationAndShear(
      Point{0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f));
  EXPECT_FALSE(envelope.IsEmpty());
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{-2, -14}, Point{2, 14}))));
}

TEST(EnvelopeTest, AddQuadNotContainingOriginToEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Quad::FromCenterAndDimensions(Point{10.0f, 20.0f}, 4.0f, 6.0f));
  EXPECT_FALSE(envelope.IsEmpty());
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{8, 17}, Point{12, 23}))));
}

TEST(EnvelopeTest, AddQuadToNonEmptyEnvelope) {
  Envelope envelope;
  envelope.Add(Quad::FromCenterDimensionsRotationAndShear(
      Point{0.0f, 0.0f}, 20.0f, 4.0f, kQuarterTurn, 2.0f));
  EXPECT_FALSE(envelope.IsEmpty());
  envelope.Add(Quad::FromCenterDimensionsRotationAndShear(
      Point{-40.0f, -25.0f}, 10.0f, 16.0f, kFullTurn, 1.0f));
  EXPECT_THAT(
      envelope.AsRect(),
      Optional(RectEq(Rect::FromTwoPoints(Point{-53, -33}, Point{2, 14}))));
}

TEST(EnvelopeTest, AddIteratorToEmptyEnvelope) {
  Envelope envelope;
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  envelope.Add(points.begin(), points.end());
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 4}))));
}

TEST(EnvelopeTest, AddIteratorToNonEmptyEnvelope) {
  Envelope envelope(Point{3, 9});
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  envelope.Add(points.begin(), points.end());
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 9}))));
}

TEST(EnvelopeTest, AddContainerToEmptyEnvelope) {
  Envelope envelope;
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  envelope.Add(points);
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 4}))));
}

TEST(EnvelopeTest, AddContainerToNonEmptyEnvelope) {
  Envelope envelope(Point{3, 9});
  std::vector<Point> points = {Point{0, 0}, Point{-2, 4}, Point{6, 3}};
  envelope.Add(points);
  EXPECT_THAT(envelope.AsRect(),
              Optional(RectEq(Rect::FromTwoPoints(Point{-2, 0}, Point{6, 9}))));
}

TEST(EnvelopeTest, AddEmptyEnvelopeToEmptyEnvelope) {
  Envelope envelope_1;
  Envelope envelope_2;

  envelope_1.Add(envelope_2);
  EXPECT_TRUE(envelope_1.IsEmpty());
  EXPECT_EQ(envelope_1.AsRect(), std::nullopt);
}

TEST(EnvelopeTest, AddNonEmptyEnvelopeToEmptyEnvelope) {
  Envelope envelope_1;
  Envelope envelope_2;

  envelope_2.Add(Point{3, 4});
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(3, 4, 3, 4)));

  envelope_1.Add(envelope_2);
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 3, 4)));
}

TEST(EnvelopeTest, AddEmptyEnvelopeToNonEmptyEnvelope) {
  Envelope envelope_1;
  Envelope envelope_2;

  envelope_2.Add(Point{3, 4});
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(3, 4, 3, 4)));

  envelope_2.Add(envelope_1);
  EXPECT_TRUE(envelope_1.IsEmpty());
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(3, 4, 3, 4)));
}

TEST(EnvelopeTest, AddNonEmptyEnvelopeToNonEmptyEnvelope) {
  Envelope envelope_1;
  envelope_1.Add(Point{3, 4});
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 3, 4)));

  Envelope envelope_2;
  envelope_2.Add(Point{7, 8});
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(7, 8, 7, 8)));

  envelope_1.Add(envelope_2);
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 7, 8)));
}

TEST(EnvelopeTest, ResetEmptyEnvelopeRemainsEmptyEnvelope) {
  Envelope envelope;

  envelope.Reset();
  EXPECT_TRUE(envelope.IsEmpty());
  EXPECT_EQ(envelope.AsRect(), std::nullopt);
}

TEST(EnvelopeTest, ResetNonEmptyEnvelopeBecomesEmptyEnvelope) {
  Envelope envelope_1;

  envelope_1.Add(Point{3, 4});
  EXPECT_FALSE(envelope_1.IsEmpty());
  EXPECT_THAT(envelope_1.AsRect(), Optional(RectEq(3, 4, 3, 4)));

  envelope_1.Reset();
  EXPECT_TRUE(envelope_1.IsEmpty());
  EXPECT_EQ(envelope_1.AsRect(), std::nullopt);

  Envelope envelope_2;

  envelope_2.Add(Rect::FromTwoPoints(Point{2, 3}, Point{7, 8}));
  EXPECT_FALSE(envelope_2.IsEmpty());
  EXPECT_THAT(envelope_2.AsRect(), Optional(RectEq(2, 3, 7, 8)));

  envelope_2.Reset();
  EXPECT_TRUE(envelope_2.IsEmpty());
  EXPECT_EQ(envelope_2.AsRect(), std::nullopt);
}

TEST(EnvelopeTest, Stringify) {
  EXPECT_EQ(absl::StrCat(Envelope()), "Envelope[<empty>]");
  EXPECT_EQ(absl::StrCat(Envelope(Rect::FromTwoPoints({-5, 10}, {-8, 15}))),
            "Envelope[*Bounds() = Rect[3 by 5 from (-8, 10) to (-5, 15)]]");
}

}  // namespace
}  // namespace ink
