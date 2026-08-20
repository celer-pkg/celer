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

#include "ink/geometry/mesh_test_helpers.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/geometry/vec.h"

namespace ink {
namespace {

using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::ExplainMatchResult;
using ::testing::Matches;

using AttrType = ::ink::MeshFormat::AttributeType;
using AttrId = ::ink::MeshFormat::AttributeId;

MATCHER(MutableMeshNonPositionalAttributesAreZero,
        absl::StrCat(negation ? "doesn't have" : "has",
                     " non-zero values for all non-positional attributes")) {
  uint32_t n_attrs = arg.Format().Attributes().size();
  uint32_t n_verts = arg.VertexCount();
  for (uint32_t a_idx = 0; a_idx < n_attrs; ++a_idx) {
    if (a_idx == arg.VertexPositionAttributeIndex()) continue;

    for (uint32_t v_idx = 0; v_idx < n_verts; ++v_idx) {
      if (!Matches(Each(0))(arg.FloatVertexAttribute(v_idx, a_idx).Values())) {
        *result_listener << absl::Substitute(
            "Attribute $0 on vertex $1 has non-zero values: [$2]", a_idx, v_idx,
            absl::StrJoin(arg.FloatVertexAttribute(v_idx, a_idx).Values(),
                          ", "));
        return false;
      }
    }
  }
  return true;
}

MATCHER_P2(PointEqPolarCoordinates, radius, angle,
           absl::StrCat(negation ? "doesn't equal" : "equals",
                        absl::Substitute(
                            " the point $0, $1 units from the origin, "
                            "rotated $2 from the x-axis",
                            Point{0, 0} +
                                Vec::FromDirectionAndMagnitude(angle, radius),
                            radius, angle))) {
  return ExplainMatchResult(
      PointEq(Point{0, 0} + Vec::FromDirectionAndMagnitude(angle, radius)), arg,
      result_listener);
}
MATCHER_P3(PointNearPolarCoordinates, radius, angle, tol,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        absl::Substitute(
                            " the point $0, $1 units from the origin, "
                            "rotated $2 from the x-axis, tolerance $3",
                            Point{0, 0} +
                                Vec::FromDirectionAndMagnitude(angle, radius),
                            radius, angle, tol))) {
  return ExplainMatchResult(
      PointNear(Point{0, 0} + Vec::FromDirectionAndMagnitude(angle, radius),
                tol),
      arg, result_listener);
}

TEST(MeshTestHelpersTest, MakeStraightLineMutableMeshWithTwoTriangles) {
  MutableMesh m = MakeStraightLineMutableMesh(2);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 2u);
  ASSERT_EQ(m.VertexCount(), 4u);

  EXPECT_THAT(m.VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT(m.VertexPosition(1), PointEq({1, -1}));
  EXPECT_THAT(m.VertexPosition(2), PointEq({2, 0}));
  EXPECT_THAT(m.VertexPosition(3), PointEq({3, -1}));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
}

TEST(MeshTestHelpersTest, MakeStraightLineMutableMeshWithFourTriangles) {
  MutableMesh m = MakeStraightLineMutableMesh(4);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 4u);
  ASSERT_EQ(m.VertexCount(), 6u);

  EXPECT_THAT(m.VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT(m.VertexPosition(1), PointEq({1, -1}));
  EXPECT_THAT(m.VertexPosition(2), PointEq({2, 0}));
  EXPECT_THAT(m.VertexPosition(3), PointEq({3, -1}));
  EXPECT_THAT(m.VertexPosition(4), PointEq({4, 0}));
  EXPECT_THAT(m.VertexPosition(5), PointEq({5, -1}));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(3, 5, 4));
}

TEST(MeshTestHelpersTest, MakeStraightLineMutableMeshWithAlternateFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kOpacityShift},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  MutableMesh m = MakeStraightLineMutableMesh(2, *format);

  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
  ASSERT_EQ(m.TriangleCount(), 2u);
  ASSERT_EQ(m.VertexCount(), 4u);
  EXPECT_THAT(m, MutableMeshNonPositionalAttributesAreZero());
}

TEST(MeshTestHelpersTest, MakeStraightLineMutableMeshWithTransform) {
  MutableMesh m =
      MakeStraightLineMutableMesh(2, {}, AffineTransform::Translate({1, -1}));

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 2u);
  ASSERT_EQ(m.VertexCount(), 4u);

  EXPECT_THAT(m.VertexPosition(0), PointEq({1, -1}));
  EXPECT_THAT(m.VertexPosition(1), PointEq({2, -2}));
  EXPECT_THAT(m.VertexPosition(2), PointEq({3, -1}));
  EXPECT_THAT(m.VertexPosition(3), PointEq({4, -2}));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
}

TEST(MeshTestHelpersTest, MakeCoiledRingMutableMeshArc) {
  MutableMesh m = MakeCoiledRingMutableMesh(8, 12);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 8u);
  ASSERT_EQ(m.VertexCount(), 10u);

  EXPECT_THAT(m.VertexPosition(0),
              PointEqPolarCoordinates(0.75, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(1),
              PointEqPolarCoordinates(1, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(2),
              PointEqPolarCoordinates(0.75, Angle::Degrees(30)));
  EXPECT_THAT(m.VertexPosition(3),
              PointEqPolarCoordinates(1, Angle::Degrees(30)));
  EXPECT_THAT(m.VertexPosition(4),
              PointEqPolarCoordinates(0.75, Angle::Degrees(60)));
  EXPECT_THAT(m.VertexPosition(5),
              PointEqPolarCoordinates(1, Angle::Degrees(60)));
  EXPECT_THAT(m.VertexPosition(6),
              PointEqPolarCoordinates(0.75, Angle::Degrees(90)));
  EXPECT_THAT(m.VertexPosition(7),
              PointEqPolarCoordinates(1, Angle::Degrees(90)));
  EXPECT_THAT(m.VertexPosition(8),
              PointEqPolarCoordinates(0.75, Angle::Degrees(120)));
  EXPECT_THAT(m.VertexPosition(9),
              PointEqPolarCoordinates(1, Angle::Degrees(120)));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(3, 5, 4));
  EXPECT_THAT(m.TriangleIndices(4), ElementsAre(4, 5, 6));
  EXPECT_THAT(m.TriangleIndices(5), ElementsAre(5, 7, 6));
  EXPECT_THAT(m.TriangleIndices(6), ElementsAre(6, 7, 8));
  EXPECT_THAT(m.TriangleIndices(7), ElementsAre(7, 9, 8));
}

TEST(MeshTestHelpersTest, MakeCoiledRingMutableMeshWrapsAroundAndOverlaps) {
  MutableMesh m = MakeCoiledRingMutableMesh(10, 4);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 10u);
  ASSERT_EQ(m.VertexCount(), 12u);

  EXPECT_THAT(m.VertexPosition(0), PointNear({0.75, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(1), PointNear({1, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(2), PointNear({0, 0.75}, 1e-5));
  EXPECT_THAT(m.VertexPosition(3), PointNear({0, 1}, 1e-5));
  EXPECT_THAT(m.VertexPosition(4), PointNear({-0.75, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(5), PointNear({-1, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(6), PointNear({0, -0.75}, 1e-5));
  EXPECT_THAT(m.VertexPosition(7), PointNear({0, -1}, 1e-5));
  EXPECT_THAT(m.VertexPosition(8), PointNear({0.75, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(9), PointNear({1, 0}, 1e-5));
  EXPECT_THAT(m.VertexPosition(10), PointNear({0, 0.75}, 1e-5));
  EXPECT_THAT(m.VertexPosition(11), PointNear({0, 1}, 1e-5));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(3, 5, 4));
  EXPECT_THAT(m.TriangleIndices(4), ElementsAre(4, 5, 6));
  EXPECT_THAT(m.TriangleIndices(5), ElementsAre(5, 7, 6));
  EXPECT_THAT(m.TriangleIndices(6), ElementsAre(6, 7, 8));
  EXPECT_THAT(m.TriangleIndices(7), ElementsAre(7, 9, 8));
  EXPECT_THAT(m.TriangleIndices(8), ElementsAre(8, 9, 10));
  EXPECT_THAT(m.TriangleIndices(9), ElementsAre(9, 11, 10));
}

TEST(MeshTestHelpersTest, MakeCoiledRingMutableMeshWithAlternateFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kOpacityShift},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  MutableMesh m = MakeCoiledRingMutableMesh(12, 3, *format);

  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
  ASSERT_EQ(m.TriangleCount(), 12u);
  ASSERT_EQ(m.VertexCount(), 14u);
  EXPECT_THAT(m, MutableMeshNonPositionalAttributesAreZero());
}

TEST(MeshTestHelpersTest, MakeCoiledRingMutableMeshWithTransform) {
  MutableMesh m =
      MakeCoiledRingMutableMesh(8, 12, {}, AffineTransform::Scale(2));

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 8u);
  ASSERT_EQ(m.VertexCount(), 10u);

  EXPECT_THAT(m.VertexPosition(0),
              PointEqPolarCoordinates(1.5, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(1),
              PointEqPolarCoordinates(2, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(2),
              PointEqPolarCoordinates(1.5, Angle::Degrees(30)));
  EXPECT_THAT(m.VertexPosition(3),
              PointEqPolarCoordinates(2, Angle::Degrees(30)));
  EXPECT_THAT(m.VertexPosition(4),
              PointEqPolarCoordinates(1.5, Angle::Degrees(60)));
  EXPECT_THAT(m.VertexPosition(5),
              PointEqPolarCoordinates(2, Angle::Degrees(60)));
  EXPECT_THAT(m.VertexPosition(6),
              PointEqPolarCoordinates(1.5, Angle::Degrees(90)));
  EXPECT_THAT(m.VertexPosition(7),
              PointEqPolarCoordinates(2, Angle::Degrees(90)));
  EXPECT_THAT(m.VertexPosition(8),
              PointEqPolarCoordinates(1.5, Angle::Degrees(120)));
  EXPECT_THAT(m.VertexPosition(9),
              PointEqPolarCoordinates(2, Angle::Degrees(120)));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 3, 2));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(3, 5, 4));
  EXPECT_THAT(m.TriangleIndices(4), ElementsAre(4, 5, 6));
  EXPECT_THAT(m.TriangleIndices(5), ElementsAre(5, 7, 6));
  EXPECT_THAT(m.TriangleIndices(6), ElementsAre(6, 7, 8));
  EXPECT_THAT(m.TriangleIndices(7), ElementsAre(7, 9, 8));
}

TEST(MeshTestHelpersTest, MakeStarMutableMeshWithThreePoints) {
  MutableMesh m = MakeStarMutableMesh(3);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 3u);
  ASSERT_EQ(m.VertexCount(), 7u);

  EXPECT_THAT(m.VertexPosition(0),
              PointEqPolarCoordinates(0.25, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(1),
              PointEqPolarCoordinates(1, Angle::Degrees(60)));
  EXPECT_THAT(m.VertexPosition(2),
              PointEqPolarCoordinates(0.25, Angle::Degrees(120)));
  EXPECT_THAT(m.VertexPosition(3),
              PointEqPolarCoordinates(1, Angle::Degrees(180)));
  EXPECT_THAT(m.VertexPosition(4),
              PointEqPolarCoordinates(0.25, Angle::Degrees(240)));
  EXPECT_THAT(m.VertexPosition(5),
              PointEqPolarCoordinates(1, Angle::Degrees(300)));
  EXPECT_THAT(m.VertexPosition(6),
              PointEqPolarCoordinates(0.25, Angle::Degrees(360)));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(4, 5, 6));
}

TEST(MeshTestHelpersTest, MakeStarMutableMeshWithFivePoints) {
  MutableMesh m = MakeStarMutableMesh(5);

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 5u);
  ASSERT_EQ(m.VertexCount(), 11u);

  EXPECT_THAT(m.VertexPosition(0),
              PointEqPolarCoordinates(0.25, Angle::Degrees(0)));
  EXPECT_THAT(m.VertexPosition(1),
              PointEqPolarCoordinates(1, Angle::Degrees(36)));
  EXPECT_THAT(m.VertexPosition(2),
              PointEqPolarCoordinates(0.25, Angle::Degrees(72)));
  EXPECT_THAT(m.VertexPosition(3),
              PointEqPolarCoordinates(1, Angle::Degrees(108)));
  EXPECT_THAT(m.VertexPosition(4),
              PointEqPolarCoordinates(0.25, Angle::Degrees(144)));
  EXPECT_THAT(m.VertexPosition(5),
              PointEqPolarCoordinates(1, Angle::Degrees(180)));
  EXPECT_THAT(m.VertexPosition(6),
              PointEqPolarCoordinates(0.25, Angle::Degrees(216)));
  EXPECT_THAT(m.VertexPosition(7),
              PointEqPolarCoordinates(1, Angle::Degrees(252)));
  EXPECT_THAT(m.VertexPosition(8),
              PointEqPolarCoordinates(0.25, Angle::Degrees(288)));
  EXPECT_THAT(m.VertexPosition(9),
              PointNear(Point{0, 0} + Vec::FromDirectionAndMagnitude(
                                          Angle::Degrees(324), 1),
                        1e-5));
  EXPECT_THAT(m.VertexPosition(10),
              PointEqPolarCoordinates(0.25, Angle::Degrees(360)));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(4, 5, 6));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(6, 7, 8));
  EXPECT_THAT(m.TriangleIndices(4), ElementsAre(8, 9, 10));
}

TEST(MeshTestHelpersTest, MakeStarMutableMeshWithAlternateFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kOpacityShift},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat3PackedInTwoFloats, AttrId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  MutableMesh m = MakeStarMutableMesh(10, *format);

  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
  ASSERT_EQ(m.TriangleCount(), 10u);
  ASSERT_EQ(m.VertexCount(), 21u);
  EXPECT_THAT(m, MutableMeshNonPositionalAttributesAreZero());
}

TEST(MeshTestHelpersTest, MakeStarMutableMeshWithTransform) {
  MutableMesh m =
      MakeStarMutableMesh(3, {}, AffineTransform::Rotate(Angle::Degrees(45)));

  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ(m.TriangleCount(), 3u);
  ASSERT_EQ(m.VertexCount(), 7u);

  EXPECT_THAT(m.VertexPosition(0),
              PointEqPolarCoordinates(0.25, Angle::Degrees(45)));
  EXPECT_THAT(m.VertexPosition(1),
              PointEqPolarCoordinates(1, Angle::Degrees(105)));
  EXPECT_THAT(m.VertexPosition(2),
              PointNearPolarCoordinates(0.25, Angle::Degrees(165), 1e-5));
  EXPECT_THAT(m.VertexPosition(3),
              PointEqPolarCoordinates(1, Angle::Degrees(225)));
  EXPECT_THAT(m.VertexPosition(4),
              PointEqPolarCoordinates(0.25, Angle::Degrees(285)));
  EXPECT_THAT(m.VertexPosition(5),
              PointNearPolarCoordinates(1, Angle::Degrees(345), 1e-5));
  EXPECT_THAT(m.VertexPosition(6),
              PointEqPolarCoordinates(0.25, Angle::Degrees(45)));

  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(2, 3, 4));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(4, 5, 6));
}

}  // namespace
}  // namespace ink
