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

#include "ink/geometry/mutable_mesh.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "fuzztest/fuzztest.h"
#include "absl/container/inlined_vector.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "absl/types/span.h"
#include "ink/geometry/fuzz_domains.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/type_matchers.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Pointwise;
using ::testing::SizeIs;

MATCHER(MeshTrianglesHaveNonNegativeArea,
        absl::StrCat("the mesh contains ", negation ? "one or more" : "no",
                     " triangles with negative area")) {
  for (uint32_t i = 0; i < arg.TriangleCount(); ++i) {
    Triangle t = arg.GetTriangle(i);
    if (t.SignedArea() < 0) {
      *result_listener << absl::Substitute(
          "Triangle {$0, $1, $2} at index $3 has negative area", t.p0, t.p1,
          t.p2, i);
      return false;
    }
  }
  return true;
}

TEST(MutableMeshTest, DefaultCtor) {
  MutableMesh m;
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_EQ(m.TriangleCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(m.VertexPositionAttributeIndex(), 0);
  // The default `MeshFormat` has an unpacked vertex stride of 8 bytes and an
  // unpacked index stride of 4 bytes.
  EXPECT_EQ(m.VertexStride(), 8);
  EXPECT_EQ(m.IndexStride(), 4);
}

TEST(MutableMeshTest, ConstructWithFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_EQ(m.TriangleCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
  EXPECT_EQ(m.VertexPositionAttributeIndex(), 1);
  EXPECT_EQ(m.VertexStride(), 28);
  EXPECT_EQ(m.IndexStride(), 2);
}

TEST(MutableMeshTest, AppendVertex) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({3, 4});

  EXPECT_EQ(m.VertexCount(), 1);
  EXPECT_EQ(m.VertexPosition(0), (Point{3, 4}));
  EXPECT_THAT(m.FloatVertexAttribute(0, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(3, 4));
  EXPECT_THAT(m.FloatVertexAttribute(0, 2).Values(), ElementsAre(0));
}

TEST(MutableMeshTest, AppendMultipleVertices) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});
  m.AppendVertex({10, 100});

  EXPECT_EQ(m.VertexCount(), 4);
  EXPECT_EQ(m.VertexPosition(0), (Point{3, 4}));
  EXPECT_EQ(m.VertexPosition(1), (Point{1, 0}));
  EXPECT_EQ(m.VertexPosition(2), (Point{-4, 2}));
  EXPECT_EQ(m.VertexPosition(3), (Point{10, 100}));
  EXPECT_THAT(m.FloatVertexAttribute(0, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(3, 4));
  EXPECT_THAT(m.FloatVertexAttribute(0, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m.FloatVertexAttribute(1, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(1, 1).Values(), ElementsAre(1, 0));
  EXPECT_THAT(m.FloatVertexAttribute(1, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m.FloatVertexAttribute(2, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(2, 1).Values(), ElementsAre(-4, 2));
  EXPECT_THAT(m.FloatVertexAttribute(2, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m.FloatVertexAttribute(3, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(3, 1).Values(), ElementsAre(10, 100));
  EXPECT_THAT(m.FloatVertexAttribute(3, 2).Values(), ElementsAre(0));
}

void CanStorePositionsInAnyMeshFormat(MeshFormat format,
                                      const std::vector<Point>& vertices) {
  MutableMesh mesh(format);
  for (Point vertex : vertices) {
    mesh.AppendVertex(vertex);
  }
  ASSERT_EQ(mesh.VertexCount(), vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    EXPECT_THAT(mesh.VertexPosition(i), NanSensitivePointEq(vertices[i]));
  }
}
FUZZ_TEST(MutableMeshTest, CanStorePositionsInAnyMeshFormat)
    .WithDomains(ArbitraryMeshFormat(), fuzztest::VectorOf(ArbitraryPoint()));

TEST(MutableMeshTest, CloneEmptyMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  MutableMesh m2 = m.Clone();
  EXPECT_EQ(m2.VertexCount(), 0);
  EXPECT_EQ(m2.TriangleCount(), 0);
  EXPECT_THAT(m2.Format(), MeshFormatEq(*format));
  EXPECT_EQ(m2.VertexPositionAttributeIndex(), 1);
  EXPECT_EQ(m2.VertexStride(), 28);
  EXPECT_EQ(m2.IndexStride(), 2);
}

TEST(MutableMeshTest, ClonedEmptyMeshModificationsAreUnique) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  MutableMesh m2 = m.Clone();

  m.AppendVertex({3, 4});

  EXPECT_EQ(m.VertexCount(), 1);
  EXPECT_EQ(m.VertexPosition(0), (Point{3, 4}));
  EXPECT_EQ(m2.VertexCount(), 0);

  m2.AppendVertex({1, 0});
  m2.AppendVertex({-4, 2});

  EXPECT_EQ(m.VertexCount(), 1);
  EXPECT_EQ(m.VertexPosition(0), (Point{3, 4}));
  EXPECT_EQ(m2.VertexCount(), 2);
  EXPECT_EQ(m2.VertexPosition(0), (Point{1, 0}));
  EXPECT_EQ(m2.VertexPosition(1), (Point{-4, 2}));
}

TEST(MutableMeshTest, CloneNonEmptyMesh) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});
  m.AppendVertex({10, 100});

  MutableMesh m2 = m.Clone();
  EXPECT_EQ(m2.VertexCount(), 4);
  EXPECT_EQ(m2.VertexPosition(0), (Point{3, 4}));
  EXPECT_EQ(m2.VertexPosition(1), (Point{1, 0}));
  EXPECT_EQ(m2.VertexPosition(2), (Point{-4, 2}));
  EXPECT_EQ(m2.VertexPosition(3), (Point{10, 100}));
  EXPECT_THAT(m2.FloatVertexAttribute(0, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m2.FloatVertexAttribute(0, 1).Values(), ElementsAre(3, 4));
  EXPECT_THAT(m2.FloatVertexAttribute(0, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m2.FloatVertexAttribute(1, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m2.FloatVertexAttribute(1, 1).Values(), ElementsAre(1, 0));
  EXPECT_THAT(m2.FloatVertexAttribute(1, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m2.FloatVertexAttribute(2, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m2.FloatVertexAttribute(2, 1).Values(), ElementsAre(-4, 2));
  EXPECT_THAT(m2.FloatVertexAttribute(2, 2).Values(), ElementsAre(0));
  EXPECT_THAT(m2.FloatVertexAttribute(3, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m2.FloatVertexAttribute(3, 1).Values(), ElementsAre(10, 100));
  EXPECT_THAT(m2.FloatVertexAttribute(3, 2).Values(), ElementsAre(0));
}

TEST(MutableMeshTest, ClonedNonEmptyMeshModificationsAreUnique) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({10, 100});
  MutableMesh m2 = m.Clone();

  m.AppendVertex({3, 4});

  EXPECT_EQ(m.VertexCount(), 2);
  EXPECT_EQ(m.VertexPosition(0), (Point{10, 100}));
  EXPECT_EQ(m.VertexPosition(1), (Point{3, 4}));
  EXPECT_EQ(m2.VertexCount(), 1);
  EXPECT_EQ(m2.VertexPosition(0), (Point{10, 100}));

  m2.AppendVertex({1, 0});
  m2.AppendVertex({-4, 2});

  EXPECT_EQ(m.VertexCount(), 2);
  EXPECT_EQ(m.VertexPosition(0), (Point{10, 100}));
  EXPECT_EQ(m.VertexPosition(1), (Point{3, 4}));
  EXPECT_EQ(m2.VertexCount(), 3);
  EXPECT_EQ(m2.VertexPosition(0), (Point{10, 100}));
  EXPECT_EQ(m2.VertexPosition(1), (Point{1, 0}));
  EXPECT_EQ(m2.VertexPosition(2), (Point{-4, 2}));
}

TEST(MutableMeshTest, ClearEmptyMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  EXPECT_EQ(m.VertexCount(), 0);
  m.Clear();
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
}

TEST(MutableMeshTest, ClearNonEmptyMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);

  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});
  m.AppendVertex({10, 100});
  EXPECT_EQ(m.VertexCount(), 4);

  m.Clear();
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
}

TEST(MutableMeshTest, ResetEmptyMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m;
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), Not(MeshFormatEq(*format)));

  m.Reset(*format);
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
}

TEST(MutableMeshTest, ResetNonEmptyMesh) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m;

  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});
  m.AppendVertex({10, 100});
  EXPECT_EQ(m.VertexCount(), 4);
  EXPECT_THAT(m.Format(), Not(MeshFormatEq(*format)));

  m.Reset(*format);
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(*format));
}

TEST(MutableMeshTest, SetVertexPosition) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});

  m.SetVertexPosition(1, {10, 20});
  m.SetVertexPosition(2, {20, 30});

  EXPECT_EQ(m.VertexCount(), 3);
  EXPECT_EQ(m.VertexPosition(1), (Point{10, 20}));
  EXPECT_EQ(m.VertexPosition(2), (Point{20, 30}));
}

TEST(MutableMeshTest, SetFloatVertexAttribute) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({3, 4});
  m.AppendVertex({1, 0});
  m.AppendVertex({-4, 2});

  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(2, 0, {3, 4, 5, 6});

  EXPECT_THAT(m.FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT(m.FloatVertexAttribute(2, 0).Values(), ElementsAre(3, 4, 5, 6));
}

TEST(MutableMeshTest, SetFloatVertexAttributeBytePacked) {
  MutableMesh m(*MeshFormat::Create(
      {{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
        MeshFormat::AttributeId::kPosition},
       {MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
        MeshFormat::AttributeId::kCustom0}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({-1, 3});

  m.SetFloatVertexAttribute(0, 1, {9});

  EXPECT_EQ(m.VertexPosition(0), (Point{-1, 3}));
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(9));
}

TEST(MutableMeshTest, VertexAccessorsAlternateFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                           MeshFormat::AttributeId::kTexture}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);

  m.AppendVertex({5, 10});
  m.SetFloatVertexAttribute(0, 1, {2, 4, 6});

  EXPECT_EQ(m.VertexPosition(0), (Point{5, 10}));
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(2, 4, 6));
}

TEST(MutableMeshTest, AppendTriangleIndices) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendTriangleIndices({0, 1, 2});

  EXPECT_EQ(m.TriangleCount(), 1);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
}

TEST(MutableMeshTest, AppendMultipleTriangles) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});
  m.AppendTriangleIndices({2, 3, 1});
  m.AppendTriangleIndices({4, 5, 7});

  EXPECT_EQ(m.TriangleCount(), 4);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(3, 5, 7));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(2, 3, 1));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(4, 5, 7));
}

TEST(MutableMeshTest, SetTriangleIndices) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});
  m.AppendTriangleIndices({2, 3, 1});

  m.SetTriangleIndices(1, {7, 8, 9});

  EXPECT_EQ(m.TriangleCount(), 3);
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(7, 8, 9));
}

TEST(MutableMeshTest, InsertTriangleIndices) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});
  m.AppendTriangleIndices({2, 3, 1});

  m.InsertTriangleIndices(1, {7, 8, 9});

  EXPECT_EQ(m.TriangleCount(), 4);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(7, 8, 9));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(3, 5, 7));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(2, 3, 1));
}

TEST(MutableMeshTest, InsertTriangleIndicesAtEnd) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});

  m.InsertTriangleIndices(2, {7, 8, 9});

  EXPECT_EQ(m.TriangleCount(), 3);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(3, 5, 7));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(7, 8, 9));
}

TEST(MutableMeshTest, ResizeEnlargingMesh) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({3, 4});
  m.SetFloatVertexAttribute(0, 0, {3, 4, 5, 6});
  m.SetFloatVertexAttribute(0, 2, {5});

  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});

  ASSERT_EQ(m.VertexCount(), 1);
  ASSERT_EQ(m.TriangleCount(), 2);
  m.Resize(2, 4);

  EXPECT_EQ(m.VertexCount(), 2);
  EXPECT_THAT(m.FloatVertexAttribute(0, 0).Values(), ElementsAre(3, 4, 5, 6));
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(3, 4));
  EXPECT_THAT(m.FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT(m.FloatVertexAttribute(1, 0).Values(), ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(1, 1).Values(), ElementsAre(0, 0));
  EXPECT_THAT(m.FloatVertexAttribute(1, 2).Values(), ElementsAre(0));

  EXPECT_EQ(m.TriangleCount(), 4);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(3, 5, 7));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(0, 0, 0));
  EXPECT_THAT(m.TriangleIndices(3), ElementsAre(0, 0, 0));
}

TEST(MutableMeshTest, ResizeShrinkingMesh) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));

  m.AppendVertex({3, 4});
  m.AppendVertex({1, 5});
  m.AppendVertex({-4, 2});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {8});
  m.SetFloatVertexAttribute(2, 0, {3, 4, 5, 6});

  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 5, 7});

  ASSERT_EQ(m.VertexCount(), 3);
  ASSERT_EQ(m.TriangleCount(), 2);
  m.Resize(2, 1);

  EXPECT_EQ(m.VertexCount(), 2);
  EXPECT_THAT(m.FloatVertexAttribute(0, 1).Values(), ElementsAre(3, 4));
  EXPECT_THAT(m.FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT(m.FloatVertexAttribute(1, 1).Values(), ElementsAre(1, 5));
  EXPECT_THAT(m.FloatVertexAttribute(1, 2).Values(), ElementsAre(8));

  EXPECT_EQ(m.TriangleCount(), 1);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 2));
}

TEST(MutableMeshTest, ValidateTrianglesValidCase) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendVertex({2, 1});
  m.AppendTriangleIndices({0, 4, 3});
  m.AppendTriangleIndices({0, 1, 4});
  m.AppendTriangleIndices({1, 5, 4});
  m.AppendTriangleIndices({1, 2, 5});

  EXPECT_EQ(absl::OkStatus(), m.ValidateTriangles());
}

TEST(MutableMeshTest, ValidateTrianglesNonExistentVertex) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendVertex({2, 1});
  m.AppendTriangleIndices({0, 4, 3});
  m.AppendTriangleIndices({0, 1, 4});
  m.AppendTriangleIndices({1, 6, 4});
  m.AppendTriangleIndices({1, 2, 5});

  absl::Status missing_vertex = m.ValidateTriangles();
  EXPECT_EQ(missing_vertex.code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(missing_vertex.message(), HasSubstr("non-existent vertex"));
}

TEST(MutableMeshTest, ValidateTrianglesAllowsTrianglesWithNegativeArea) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendVertex({2, 1});
  m.AppendTriangleIndices({0, 4, 3});
  m.AppendTriangleIndices({0, 1, 4});
  m.AppendTriangleIndices({1, 4, 5});
  m.AppendTriangleIndices({1, 2, 5});

  EXPECT_EQ(absl::OkStatus(), m.ValidateTriangles());
}

TEST(MutableMeshTest, ValidateTrianglesRepeatedVertex) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendVertex({2, 1});
  m.AppendTriangleIndices({0, 4, 3});
  m.AppendTriangleIndices({0, 1, 4});
  m.AppendTriangleIndices({1, 4, 4});
  m.AppendTriangleIndices({1, 2, 5});

  absl::Status repeated_vertex = m.ValidateTriangles();
  EXPECT_EQ(repeated_vertex.code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(repeated_vertex.message(),
              HasSubstr("does not refer to three distinct vertices"));
}

TEST(MutableMeshTest, ValidateTrianglesDoesNotDetectExtraVertices) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendVertex({2, 1});
  m.AppendVertex({3, 3});
  m.AppendTriangleIndices({0, 4, 3});
  m.AppendTriangleIndices({0, 1, 4});
  m.AppendTriangleIndices({1, 5, 4});
  m.AppendTriangleIndices({1, 2, 5});

  EXPECT_EQ(absl::OkStatus(), m.ValidateTriangles());
}

TEST(MutableMeshTest, GetTriangle) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});

  EXPECT_EQ(m.GetTriangle(0),
            (Triangle{.p0 = {0, 0}, .p1 = {1, 0}, .p2 = {0, 1}}));
  EXPECT_EQ(m.GetTriangle(1),
            (Triangle{.p0 = {1, 0}, .p1 = {1, 1}, .p2 = {0, 1}}));
}

TEST(MutableMeshTest, TriangleAccessorsAlternateFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({4, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 1});

  m.AppendTriangleIndices({0, 1, 3});
  m.AppendTriangleIndices({1, 2, 5});
  m.InsertTriangleIndices(1, {1, 4, 3});
  m.SetTriangleIndices(2, {1, 2, 4});

  EXPECT_EQ(m.TriangleCount(), 3);
  EXPECT_THAT(m.TriangleIndices(0), ElementsAre(0, 1, 3));
  EXPECT_THAT(m.TriangleIndices(1), ElementsAre(1, 4, 3));
  EXPECT_THAT(m.TriangleIndices(2), ElementsAre(1, 2, 4));
  EXPECT_EQ(m.GetTriangle(0),
            (Triangle{.p0 = {0, 0}, .p1 = {2, 0}, .p2 = {1, 1}}));
  EXPECT_EQ(m.GetTriangle(1),
            (Triangle{.p0 = {2, 0}, .p1 = {3, 1}, .p2 = {1, 1}}));
  EXPECT_EQ(m.GetTriangle(2),
            (Triangle{.p0 = {2, 0}, .p1 = {4, 0}, .p2 = {3, 1}}));
}

template <typename T>
std::vector<std::byte> CopyToBytes(absl::Span<const T> t_data) {
  std::vector<std::byte> byte_data(sizeof(T) * t_data.size());
  std::memcpy(byte_data.data(), t_data.data(), byte_data.size());
  return byte_data;
}

TEST(MutableMeshTest, RawVertexDataWhenEmpty) {
  MutableMesh m;
  EXPECT_TRUE(m.RawVertexData().empty());
}

TEST(MutableMeshTest, RawVertexDataWhenNonEmpty) {
  MutableMesh m;
  m.AppendVertex({4, 9});
  std::vector<std::byte> vertex_byte_data_1 = CopyToBytes<float>({4, 9});
  EXPECT_THAT(m.RawVertexData(), ElementsAreArray(vertex_byte_data_1));

  m.AppendVertex({3, 5});
  std::vector<std::byte> vertex_byte_data_2 = CopyToBytes<float>({4, 9, 3, 5});
  EXPECT_THAT(m.RawVertexData(), ElementsAreArray(vertex_byte_data_2));
}

TEST(MutableMeshTest, RawVertexDataWhenNonEmptyWithDifferentFormat) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({4, 9});
  m.AppendVertex({3, 5});

  std::vector<std::byte> vertex_byte_data_1 =
      CopyToBytes<float>({0, 0, 0, 0, 4, 9, 0, 0, 0, 0, 0, 3, 5, 0});
  EXPECT_THAT(m.RawVertexData(), ElementsAreArray(vertex_byte_data_1));

  m.SetFloatVertexAttribute(0, 0, {3, 4, 5, 6});
  m.SetFloatVertexAttribute(1, 2, {2});

  std::vector<std::byte> vertex_byte_data_2 =
      CopyToBytes<float>({3, 4, 5, 6, 4, 9, 0, 0, 0, 0, 0, 3, 5, 2});
  EXPECT_THAT(m.RawVertexData(), ElementsAreArray(vertex_byte_data_2));
}

TEST(MutableMeshTest, RawIndexDataWhenEmpty) {
  MutableMesh m;
  EXPECT_TRUE(m.RawIndexData().empty());
}

TEST(MutableMeshTest, RawIndexDataWhenNonEmpty) {
  MutableMesh m;
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});

  m.AppendTriangleIndices({0, 1, 2});
  std::vector<std::byte> index_byte_data_1 = CopyToBytes<uint32_t>({0, 1, 2});
  EXPECT_THAT(m.RawIndexData(), ElementsAreArray(index_byte_data_1));

  m.AppendTriangleIndices({1, 3, 2});
  std::vector<std::byte> index_byte_data_2 =
      CopyToBytes<uint32_t>({0, 1, 2, 1, 3, 2});
  EXPECT_THAT(m.RawIndexData(), ElementsAreArray(index_byte_data_2));
}

TEST(MutableMeshTest, RawIndexDataWhenNonEmptyWithDifferentFormat) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({1, 0});
  m.AppendVertex({0, 1});
  m.AppendVertex({1, 1});

  m.AppendTriangleIndices({0, 1, 2});
  std::vector<std::byte> index_byte_data_1 = CopyToBytes<uint16_t>({0, 1, 2});
  EXPECT_THAT(m.RawIndexData(), ElementsAreArray(index_byte_data_1));

  m.AppendTriangleIndices({1, 3, 2});
  std::vector<std::byte> index_byte_data_2 =
      CopyToBytes<uint16_t>({0, 1, 2, 1, 3, 2});
  EXPECT_THAT(m.RawIndexData(), ElementsAreArray(index_byte_data_2));
}

TEST(MutableMeshTest, FromMeshDefaultFormat) {
  absl::StatusOr<Mesh> mesh = Mesh::Create(MeshFormat(),
                                           {// Position
                                            {1, 2, 3, 4},
                                            {5, 6, 7, 8}},
                                           // Triangles
                                           {0, 1, 2,  //
                                            0, 2, 3});
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  MutableMesh mutable_mesh = MutableMesh::FromMesh(*mesh);

  EXPECT_THAT(mutable_mesh.Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(mutable_mesh.VertexCount(), 4);
  EXPECT_EQ(mutable_mesh.TriangleCount(), 2);
  EXPECT_EQ(mutable_mesh.VertexPosition(0), (Point{1, 5}));
  EXPECT_EQ(mutable_mesh.VertexPosition(1), (Point{2, 6}));
  EXPECT_EQ(mutable_mesh.VertexPosition(2), (Point{3, 7}));
  EXPECT_EQ(mutable_mesh.VertexPosition(3), (Point{4, 8}));
  EXPECT_THAT(mutable_mesh.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(mutable_mesh.TriangleIndices(1), ElementsAre(0, 2, 3));
}

TEST(MutableMeshTest, FromMeshCustomFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  absl::StatusOr<Mesh> mesh = Mesh::Create(*format,
                                           {// Color
                                            {.1, .3, .5, .9},
                                            {1, 0, .75, .125},
                                            {.3, .7, .4, .5},
                                            {1, 0, 1, 0},
                                            // Position
                                            {1, 2, 3, 4},
                                            {5, 6, 7, 8},
                                            // Custom attribute
                                            {100, 0, 200, 400}},
                                           // Triangles
                                           {0, 1, 2,  //
                                            0, 2, 3});
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  MutableMesh mutable_mesh = MutableMesh::FromMesh(*mesh);

  EXPECT_THAT(mutable_mesh.Format(), MeshFormatEq(*format));
  EXPECT_EQ(mutable_mesh.VertexCount(), 4);
  EXPECT_EQ(mutable_mesh.TriangleCount(), 2);
  EXPECT_EQ(mutable_mesh.VertexPosition(0), (Point{1, 5}));
  EXPECT_EQ(mutable_mesh.VertexPosition(1), (Point{2, 6}));
  EXPECT_EQ(mutable_mesh.VertexPosition(2), (Point{3, 7}));
  EXPECT_EQ(mutable_mesh.VertexPosition(3), (Point{4, 8}));
  EXPECT_THAT(mutable_mesh.TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(mutable_mesh.TriangleIndices(1), ElementsAre(0, 2, 3));

  // The maximum error values for each color are 6.35e-3, 7.94e-3, 3.17e-3, and
  // 7.94e-3.
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(0.008), std::vector<float>{.1, 1, .3, 1}));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(0.008), std::vector<float>{.3, 0, .7, 0}));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatNear(0.008), std::vector<float>{.5, .75, .4, 1}));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(3, 0).Values(),
              Pointwise(FloatNear(0.008), std::vector<float>{.9, .125, .5, 0}));

  // The custom attribute is stored unpacked, so there is no error.
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(0, 2).Values(),
              ElementsAre(100));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(1, 2).Values(), ElementsAre(0));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(2, 2).Values(),
              ElementsAre(200));
  EXPECT_THAT(mutable_mesh.FloatVertexAttribute(3, 2).Values(),
              ElementsAre(400));
}

TEST(MutableMeshTest, FromMeshCopiesAllIndexBytes) {
  // In order to test that all bytes are copied from the index, we a vertex
  // whose index has a least one `1` in each byte, the smallest of which is
  // 0x0101 (257 in decimal).
  std::vector<float> position_placeholder(258, 0);
  absl::StatusOr<Mesh> mesh = Mesh::Create(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat2Unpacked,
                            MeshFormat::AttributeId::kPosition}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
      {// Position
       position_placeholder, position_placeholder},
      // Triangles
      {0, 1, 0x0101});
  ASSERT_EQ(mesh.status(), absl::OkStatus());

  MutableMesh mutable_mesh = MutableMesh::FromMesh(*mesh);

  EXPECT_THAT(mutable_mesh.TriangleIndices(0), ElementsAre(0, 1, 0x0101));
}

TEST(MutableMeshTest, AsMeshesEmpty) {
  auto empty_meshes = MutableMesh().AsMeshes();
  ASSERT_EQ(empty_meshes.status(), absl::OkStatus());
  EXPECT_THAT(*empty_meshes, IsEmpty());
}

TEST(MutableMeshtest, AsMeshesEmptyWithFormat) {
  auto empty_meshes =
      MutableMesh(*MeshFormat::Create(
                      {{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                        MeshFormat::AttributeId::kColorShiftHsl},
                       {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                        MeshFormat::AttributeId::kPosition},
                       {MeshFormat::AttributeType::kFloat1Unpacked,
                        MeshFormat::AttributeId::kCustom0}},
                      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked))
          .AsMeshes();
  ASSERT_EQ(empty_meshes.status(), absl::OkStatus());
  EXPECT_THAT(*empty_meshes, IsEmpty());
}

TEST(MutableMeshTest, AsMeshesDefaultFormat) {
  MutableMesh m;
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  EXPECT_THAT((*meshes)[0].Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ((*meshes)[0].VertexCount(), 4);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);

  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointEq({2, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointEq({1, 1}));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointEq({3, 2}));
  EXPECT_THAT((*meshes)[0].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({0, 0}, {3, 2})));
  EXPECT_THAT(
      (*meshes)[0].AttributeBounds(0),
      Optional(MeshAttributeBoundsEq({.minimum = {0, 0}, .maximum = {3, 2}})));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(1, 3, 2));
}

TEST(MutableMeshTest, AsMeshesCustomFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  EXPECT_THAT((*meshes)[0].Format(), MeshFormatEq(*format));
  ASSERT_EQ((*meshes)[0].VertexCount(), 4);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);
  // The maximum error for position is 0.5 * 3 / 4095 ~= 3.67e-4.
  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointNear({0, 0}, 3.67e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointNear({2, 0}, 3.67e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointNear({1, 1}, 3.67e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointNear({3, 2}, 3.67e-4));
  EXPECT_THAT((*meshes)[0].Bounds(),
              EnvelopeNear(Rect::FromTwoPoints({0, 0}, {3, 2}), 3.67e-4));
  EXPECT_THAT((*meshes)[0].AttributeBounds(1),
              Optional(MeshAttributeBoundsNear(
                  {.minimum = {0, 0}, .maximum = {3, 2}}, 3.67e-4)));
  // The maximum error for color is 0.5 * 1 / 63 ~= 7.94e-3.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(7.94e-3), std::vector{1, 0, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(7.94e-3), std::vector{0, 1, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatNear(7.94e-3), std::vector{0, 0, 1, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 0).Values(),
              Pointwise(FloatNear(7.94e-3), std::vector{0.5, 0.5, 0.5, 0.5}));
  EXPECT_THAT(
      (*meshes)[0].AttributeBounds(0),
      Optional(MeshAttributeBoundsNear(
          {.minimum = {0, 0, 0, 0.5}, .maximum = {1, 1, 1, 1}}, 7.94e-3)));
  // The custom attribute is stored unpacked, an so has no error.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 2).Values(),
              ElementsAre(15));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 2).Values(),
              ElementsAre(-5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 2).Values(),
              ElementsAre(25));
  EXPECT_THAT(
      (*meshes)[0].AttributeBounds(2),
      Optional(MeshAttributeBoundsEq({.minimum = {-5}, .maximum = {25}})));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(1, 3, 2));
}

TEST(MutableMeshTest, AsMeshesExcludesUnusedVertices) {
  MutableMesh m;
  m.AppendVertex({0, 0});
  m.AppendVertex({1, -1});
  m.AppendVertex({2, 0});
  m.AppendVertex({3, -1});
  m.AppendVertex({4, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 1});
  m.AppendTriangleIndices({0, 2, 5});
  m.AppendTriangleIndices({2, 6, 5});
  m.AppendTriangleIndices({2, 4, 6});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  EXPECT_THAT((*meshes)[0].Format(), MeshFormatEq(MeshFormat()));
  ASSERT_EQ((*meshes)[0].VertexCount(), 5);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 3);
  // Vertices at indices 1 and 3 in the original mesh we skipped, so the
  // remaining vertices get remapped to new indices:
  // 0 -> 0
  // 2 -> 1
  // 4 -> 4
  // 5 -> 2
  // 6 -> 3
  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointEq({2, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointEq({1, 1}));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointEq({3, 1}));
  EXPECT_THAT((*meshes)[0].VertexPosition(4), PointEq({4, 0}));
  EXPECT_THAT((*meshes)[0].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({0, 0}, {4, 1})));
  EXPECT_THAT(
      (*meshes)[0].AttributeBounds(0),
      Optional(MeshAttributeBoundsEq({.minimum = {0, 0}, .maximum = {4, 1}})));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(1, 3, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(2), ElementsAre(1, 4, 3));
}

TEST(MutableMeshTest, AsMeshes16BitReturnsMultiplePartitionsForLargeMesh) {
  // The maximum number of vertices for a 16-bit index is 65536, so 1e5
  // triangles will give us two meshes.
  MutableMesh m = MakeStraightLineMutableMesh(1e5);

  // The resulting meshes will still be triangle strips, and so will have two
  // more vertices than triangles.
  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 2);
  EXPECT_THAT((*meshes)[0].Format(), MeshFormatEq(MeshFormat()));
  EXPECT_THAT((*meshes)[1].Format(), MeshFormatEq(MeshFormat()));
  // The first mesh has the maximum number of vertices (2^16).
  EXPECT_EQ((*meshes)[0].VertexCount(), 65536);
  EXPECT_EQ((*meshes)[0].TriangleCount(), 65534);
  EXPECT_THAT((*meshes)[0].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({0, -1}, {65535, 0})));
  // The second mesh has the remainder, plus two that overlap
  // ((1e5 + 2) - 2^16 + 2).
  EXPECT_EQ((*meshes)[1].VertexCount(), 34468);
  EXPECT_EQ((*meshes)[1].TriangleCount(), 34466);
  EXPECT_THAT((*meshes)[1].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({65534, -1}, {100001, 0})));
}

TEST(MutableMeshTest, AsMeshesPartitionsUseSameUnpackingParams) {
  MutableMesh m =
      MakeStraightLineMutableMesh(1e5, MakeSinglePackedPositionFormat());
  // The `MutableMesh`'s bounds are (0, -1) -> (99999, 0), and we have 12 bits
  // of precision per component.
  MeshAttributeCodingParams expected_unpacking_params{
      .components = {{.offset = 0, .scale = 100001. / 4095},
                     {.offset = -1, .scale = 1. / 4095}}};

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 2);
  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(expected_unpacking_params));
  EXPECT_THAT((*meshes)[1].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(expected_unpacking_params));
}

TEST(MutableMeshTest, AsMeshes16BitMultiplePartitionsPreservesMinimumBounds) {
  MutableMesh m =
      MakeStraightLineMutableMesh(1e5, MakeSinglePackedPositionFormat());
  // Add one more triangle to ensure that the minimum value is not in the first
  // partition.
  m.AppendVertex({-100, -100});
  m.AppendTriangleIndices({m.VertexCount() - 1, 1, 0});
  // The `MutableMesh`'s bounds are (-100, -100) -> (99999, 0), and we have
  // 12 bits of precision per component.
  MeshAttributeCodingParams expected_unpacking_params{
      .components = {{.offset = -100, .scale = 100101. / 4095},
                     {.offset = -100, .scale = 100. / 4095}}};

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 2);
  EXPECT_THAT((*meshes)[0].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({0, -1}, {65535, 0})));
  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(expected_unpacking_params));
  EXPECT_THAT((*meshes)[1].Bounds(),
              EnvelopeEq(Rect::FromTwoPoints({-100, -100}, {100001, 0})));
  EXPECT_THAT((*meshes)[1].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(expected_unpacking_params));
}

TEST(MutableMeshTest, AsMeshesWithCustomPackingParams) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  MutableMesh m(*format);
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                             {.offset = -1.2, .scale = .1},
                                             {.offset = -1.3, .scale = .1},
                                             {.offset = -1.4, .scale = .1}}},
                  MeshAttributeCodingParams{{{.offset = -5, .scale = .01},
                                             {.offset = -4, .scale = .05}}},
                  std::nullopt});
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_THAT((*meshes)[0].Format(), MeshFormatEq(*format));
  ASSERT_EQ((*meshes)[0].VertexCount(), 4);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);

  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq({{{.offset = -1.1, .scale = .1},
                                            {.offset = -1.2, .scale = .1},
                                            {.offset = -1.3, .scale = .1},
                                            {.offset = -1.4, .scale = .1}}}));
  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(1),
              MeshAttributeCodingParamsEq({{{.offset = -5, .scale = .01},
                                            {.offset = -4, .scale = .05}}}));
  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1}}}));

  // The chosen packing transforms can represent these values nearly exactly.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(1e-6), std::vector{1, 0, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(1e-6), std::vector{0, 1, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatEq(), std::vector{0, 0, 1, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 0).Values(),
              Pointwise(FloatEq(), std::vector{0.5, 0.5, 0.5, 0.5}));

  // The chosen packing transforms can represent these values exactly.
  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointEq({0, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointEq({2, 0}));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointEq({1, 1}));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointEq({3, 2}));

  // The custom attribute is stored unpacked, an so has no error.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 2).Values(),
              ElementsAre(15));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 2).Values(),
              ElementsAre(-5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 2).Values(),
              ElementsAre(25));
}

TEST(MutableMeshTest,
     AsMeshesWithCustomPackingParamsUsingDefaultForPackedType) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition},
                          {MeshFormat::AttributeType::kFloat1Unpacked,
                           MeshFormat::AttributeId::kCustom0}},
                         MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  MutableMesh m(*format);
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                             {.offset = -1.2, .scale = .1},
                                             {.offset = -1.3, .scale = .1},
                                             {.offset = -1.4, .scale = .1}}},
                  std::nullopt, std::nullopt});
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_THAT((*meshes)[0].Format(), MeshFormatEq(*format));
  ASSERT_EQ((*meshes)[0].VertexCount(), 4);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);

  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq({{{.offset = -1.1, .scale = .1},
                                            {.offset = -1.2, .scale = .1},
                                            {.offset = -1.3, .scale = .1},
                                            {.offset = -1.4, .scale = .1}}}));
  constexpr int k12BitMax = mesh_internal::MaxValueForBits(12);
  EXPECT_THAT(
      (*meshes)[0].VertexAttributeUnpackingParams(1),
      MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 3.f / k12BitMax},
                                    {.offset = 0, .scale = 2.f / k12BitMax}}}));
  EXPECT_THAT((*meshes)[0].VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1}}}));

  // The chosen packing transforms can represent these values nearly exactly.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(1e-6), std::vector{1, 0, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(1e-6), std::vector{0, 1, 0, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatEq(), std::vector{0, 0, 1, 1}));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 0).Values(),
              Pointwise(FloatEq(), std::vector{0.5, 0.5, 0.5, 0.5}));

  // The default packing transform was used; the maximum error is ~3.66e-4 for
  // the x-component, and ~2.44e-4 for the y-component.
  EXPECT_THAT((*meshes)[0].VertexPosition(0),
              PointNear({0, 0}, 3.67e-4, 2.45e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(1),
              PointNear({2, 0}, 3.67e-4, 2.45e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(2),
              PointNear({1, 1}, 3.67e-4, 2.45e-4));
  EXPECT_THAT((*meshes)[0].VertexPosition(3),
              PointNear({3, 2}, 3.67e-4, 2.45e-4));

  // The custom attribute is stored unpacked, an so has no error.
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(0, 2).Values(), ElementsAre(5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(1, 2).Values(),
              ElementsAre(15));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(2, 2).Values(),
              ElementsAre(-5));
  EXPECT_THAT((*meshes)[0].FloatVertexAttribute(3, 2).Values(),
              ElementsAre(25));
}

TEST(MutableMeshTest, AsMeshesFailsIfNonExistentVertexIsReferenced) {
  MutableMesh m;
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});

  absl::Status missing_vertex = m.AsMeshes().status();
  EXPECT_EQ(missing_vertex.code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(missing_vertex.message(), HasSubstr("non-existent vertex"));
}

TEST(MutableMeshTest, AsMeshesAllowsTrianglesWithNegativeArea) {
  MutableMesh m;
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 2, 3});

  EXPECT_EQ(absl::OkStatus(), m.AsMeshes().status());
}

TEST(MutableMeshTest, AsMeshesRejectsNonFiniteValues) {
  MutableMesh baseline(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  baseline.AppendVertex({0, 0});
  baseline.SetFloatVertexAttribute(0, 0, {.25, .5, .75, 1});
  baseline.SetFloatVertexAttribute(0, 2, {5000});
  baseline.AppendVertex({2, 0});
  baseline.SetFloatVertexAttribute(1, 0, {.1, .2, .3, .4});
  baseline.SetFloatVertexAttribute(1, 2, {-777});
  baseline.AppendVertex({1, 1});
  baseline.SetFloatVertexAttribute(2, 0, {.9, .7, .5, 1});
  baseline.SetFloatVertexAttribute(2, 2, {0});
  baseline.AppendTriangleIndices({0, 1, 2});

  MutableMesh with_infinite_attr = baseline.Clone();
  with_infinite_attr.SetFloatVertexAttribute(
      2, 0, {std::numeric_limits<float>::infinity(), 0, 0, 1});

  MutableMesh with_nan_attr = baseline.Clone();
  with_nan_attr.SetVertexPosition(0, {std::nanf(""), 0});

  MutableMesh with_non_finite_unpacked_value = baseline.Clone();
  with_non_finite_unpacked_value.SetFloatVertexAttribute(0, 2, {std::nanf("")});

  MutableMesh with_finite_values_but_infinite_bounds = baseline.Clone();
  with_finite_values_but_infinite_bounds.SetVertexPosition(1, {0, 3e38});
  with_finite_values_but_infinite_bounds.SetVertexPosition(2, {0, -3e38});

  EXPECT_EQ(absl::OkStatus(), baseline.AsMeshes().status());
  {
    absl::Status status = with_infinite_attr.AsMeshes().status();
    EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition);
    EXPECT_THAT(status.message(), HasSubstr("non-finite value"));
  }
  {
    absl::Status status = with_nan_attr.AsMeshes().status();
    EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition);
    EXPECT_THAT(status.message(), HasSubstr("non-finite value"));
  }
  {
    absl::Status status = with_non_finite_unpacked_value.AsMeshes().status();
    EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition);
    EXPECT_THAT(status.message(), HasSubstr("non-finite value"));
  }
  {
    absl::Status status =
        with_finite_values_but_infinite_bounds.AsMeshes().status();
    EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(status.message(), HasSubstr("exceeds float precision"));
  }
}

TEST(MutableMeshTest, AsMeshesFailsWrongNumberOfPackingParams) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  absl::Status wrong_num_params =
      m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                             {.offset = -1.2, .scale = .1},
                                             {.offset = -1.3, .scale = .1},
                                             {.offset = -1.4, .scale = .1}}},
                  MeshAttributeCodingParams{{{.offset = -5, .scale = .01},
                                             {.offset = -4, .scale = .05}}},
                  std::nullopt, std::nullopt})
          .status();
  EXPECT_EQ(wrong_num_params.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(wrong_num_params.message(),
              HasSubstr("Wrong number of coding params"));
}

TEST(MutableMeshTest, AsMeshesFailsPackingParamsForUnpackedType) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  absl::Status packing_for_unpacked_attr =
      m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                             {.offset = -1.2, .scale = .1},
                                             {.offset = -1.3, .scale = .1},
                                             {.offset = -1.4, .scale = .1}}},
                  MeshAttributeCodingParams{{{.offset = -5, .scale = .01},
                                             {.offset = -4, .scale = .05}}},
                  MeshAttributeCodingParams{{{.offset = 10, .scale = 10}}}})
          .status();
  EXPECT_EQ(packing_for_unpacked_attr.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(packing_for_unpacked_attr.message(),
              HasSubstr("but the attribute is unpacked"));
}

TEST(MutableMeshTest, AsMeshesFailsInvalidPackingParams) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  {
    absl::Status invalid_packing_params =
        m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                               {.offset = -1.2, .scale = .1},
                                               {.offset = -1.3, .scale = .1},
                                               {.offset = -1.4, .scale = .1}}},
                    // Wrong number of components.
                    MeshAttributeCodingParams{{{.offset = -5, .scale = .01},
                                               {.offset = -4, .scale = .05},
                                               {.offset = 3, .scale = 17}}},
                    std::nullopt})
            .status();
    EXPECT_EQ(invalid_packing_params.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(invalid_packing_params.message(),
                HasSubstr("is not valid for format"));
  }

  {
    absl::Status invalid_packing_params =
        m.AsMeshes(
             // Non-finite values.
             {MeshAttributeCodingParams{
                  {{.offset = -1.1, .scale = .1},
                   {.offset = -1.2, .scale = .1},
                   {.offset = -1.3, .scale = std::nanf("")},
                   {.offset = -1.4, .scale = .1}}},
              MeshAttributeCodingParams{
                  {{.offset = -5, .scale = .01}, {.offset = -4, .scale = .05}}},
              std::nullopt})
            .status();
    EXPECT_EQ(invalid_packing_params.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(invalid_packing_params.message(),
                HasSubstr("is not valid for format"));
  }
}

TEST(MutableMeshTest,
     AsMeshesFailsPackingParamsCannotRepresentAttributeValues) {
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({0, 0});
  m.AppendVertex({2, 0});
  m.AppendVertex({1, 1});
  m.AppendVertex({3, 2});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 3, 2});
  m.SetFloatVertexAttribute(0, 0, {1, 0, 0, 1});
  m.SetFloatVertexAttribute(1, 0, {0, 1, 0, 1});
  m.SetFloatVertexAttribute(2, 0, {0, 0, 1, 1});
  m.SetFloatVertexAttribute(3, 0, {0.5, 0.5, 0.5, 0.5});
  m.SetFloatVertexAttribute(0, 2, {5});
  m.SetFloatVertexAttribute(1, 2, {15});
  m.SetFloatVertexAttribute(2, 2, {-5});
  m.SetFloatVertexAttribute(3, 2, {25});

  {
    absl::Status insufficient_range =
        m.AsMeshes(
             // Can't represent minimum value.
             {MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                         {.offset = -1.2, .scale = .1},
                                         {.offset = .5, .scale = .1},
                                         {.offset = -1.4, .scale = .1}}},
              MeshAttributeCodingParams{
                  {{.offset = -5, .scale = .01}, {.offset = -4, .scale = .05}}},
              std::nullopt})
            .status();
    EXPECT_EQ(insufficient_range.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(insufficient_range.message(),
                HasSubstr("cannot represent all values"));
  }
  {
    absl::Status insufficient_range =
        m.AsMeshes({MeshAttributeCodingParams{{{.offset = -1.1, .scale = .1},
                                               {.offset = -1.2, .scale = .1},
                                               {.offset = -5, .scale = .1},
                                               {.offset = -1.4, .scale = .1}}},
                    // Can't represent maximum value.
                    MeshAttributeCodingParams{{{.offset = 0, .scale = .00005},
                                               {.offset = -4, .scale = .05}}},
                    std::nullopt})
            .status();
    EXPECT_EQ(insufficient_range.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(insufficient_range.message(),
                HasSubstr("cannot represent all values"));
  }
}

TEST(MutableMeshTest, AsMeshesCorrectsSingleFlippedTriangle) {
  constexpr float kTol = 1.f;
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({0, 0});
  m.AppendVertex({4095, 0});
  m.AppendVertex({4095, 4095});
  m.AppendVertex({0.4, 0.6});
  m.AppendVertex({3967.4, 4094.6});
  m.AppendVertex({793.73, 819.47});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 4, 5});  // Flipped

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_EQ((*meshes)[0].VertexCount(), 6);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);

  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointNear({0, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointNear({4095, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointNear({4095, 4095}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointNear({0.4, 0.6}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(4),
              PointNear({3967.4, 4094.6}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(5),
              PointNear({793.73, 819.47}, kTol));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(3, 4, 5));

  EXPECT_THAT((*meshes)[0], MeshTrianglesHaveNonNegativeArea());
}

TEST(MutableMeshTest, AsMeshesCorrectsSingleFlippedTriangleWithNonUnitScale) {
  MutableMesh m(MakeSinglePackedPositionFormat());
  // This mesh is the same as the one in AsMeshesCorrectsSingleFlippedTriangle,
  // but scaled by 0.5 in the x-direction and 2 in the y-direction.
  m.AppendVertex({0, 0});
  m.AppendVertex({2047.5, 0});
  m.AppendVertex({2047.5, 8190});
  m.AppendVertex({0.2, 1.2});
  m.AppendVertex({1983.7, 8189.2});
  m.AppendVertex({396.865, 1638.94});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 4, 5});  // Flipped

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_EQ((*meshes)[0].VertexCount(), 6);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 2);

  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointNear({0, 0}, .5, 2));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointNear({2047.5, 0}, .5, 2));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointNear({2047.5, 8190}, .5, 2));
  EXPECT_THAT((*meshes)[0].VertexPosition(3), PointNear({0.2, 1.2}, .5, 2));
  EXPECT_THAT((*meshes)[0].VertexPosition(4),
              PointNear({1983.7, 8189.2}, .5, 2));
  EXPECT_THAT((*meshes)[0].VertexPosition(5),
              PointNear({396.865, 1638.94}, .5, 2));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(3, 4, 5));

  EXPECT_THAT((*meshes)[0], MeshTrianglesHaveNonNegativeArea());
}

TEST(MutableMeshTest, AsMeshesMultipleFlippedTrianglesWithSharedVertices) {
  constexpr float kTol = 1.f;
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({0, 0});
  m.AppendVertex({4095, 0});
  m.AppendVertex({4095, 4095});
  m.AppendVertex({2358.52, 3913.05});
  m.AppendVertex({255.948, 424.528});
  m.AppendVertex({1668.59, 2436.19});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 0, 4});  // Flipped
  m.AppendTriangleIndices({5, 4, 0});
  m.AppendTriangleIndices({5, 3, 0});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_EQ((*meshes)[0].VertexCount(), 6);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 4);

  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointNear({0, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointNear({4095, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointNear({4095, 4095}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(3),
              PointNear({2358.52, 3913.05}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(4),
              PointNear({255.948, 424.528}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(5),
              PointNear({1668.59, 2436.19}, kTol));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(3, 0, 4));
  EXPECT_THAT((*meshes)[0].TriangleIndices(2), ElementsAre(5, 4, 0));
  EXPECT_THAT((*meshes)[0].TriangleIndices(3), ElementsAre(5, 3, 0));

  EXPECT_THAT((*meshes)[0], MeshTrianglesHaveNonNegativeArea());
}

TEST(MutableMeshTest, AsMeshesCorrectsMultipleTrianglesWithOneNudge) {
  constexpr float kTol = 1.f;
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({0, 0});
  m.AppendVertex({4095, 0});
  m.AppendVertex({4095, 4095});
  m.AppendVertex({997.73, 678.97});
  m.AppendVertex({1424.83, 1273.84});
  m.AppendVertex({1696.56, 1652.68});
  m.AppendVertex({1747.14, 1133.63});
  m.AppendVertex({1767.93, 1146.35});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({3, 4, 5});  // Flipped
  m.AppendTriangleIndices({6, 7, 3});  // Flipped

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_EQ(meshes->size(), 1);
  ASSERT_EQ((*meshes)[0].VertexCount(), 8);
  ASSERT_EQ((*meshes)[0].TriangleCount(), 3);

  EXPECT_THAT((*meshes)[0].VertexPosition(0), PointNear({0, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(1), PointNear({4095, 0}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(2), PointNear({4095, 4095}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(3),
              PointNear({997.73, 678.97}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(4),
              PointNear({1424.83, 1273.84}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(5),
              PointNear({1696.56, 1652.68}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(6),
              PointNear({1747.14, 1133.63}, kTol));
  EXPECT_THAT((*meshes)[0].VertexPosition(7),
              PointNear({1767.93, 1146.35}, kTol));

  EXPECT_THAT((*meshes)[0].TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT((*meshes)[0].TriangleIndices(1), ElementsAre(3, 4, 5));
  EXPECT_THAT((*meshes)[0].TriangleIndices(2), ElementsAre(6, 7, 3));

  EXPECT_THAT((*meshes)[0], MeshTrianglesHaveNonNegativeArea());
}

TEST(MutableMeshTest,
     AsMeshesCannotCorrectTriangleThatHasFlippedDuplicateTriangle) {
  // This mesh has two triangles, each with zero area, using the same three
  // points but with opposite ordering. After quantization, one has positive
  // area and one has negative area -- but since correcting one flips the other,
  // there is no solution.
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({1, -1});
  m.AppendVertex({0, 0});
  m.AppendVertex({-1, 1});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({1, 0, 2});

  // `AsMeshes` will succeed, but the result still contain flipped triangles.
  auto meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  EXPECT_THAT(*meshes, Not(Each(MeshTrianglesHaveNonNegativeArea())));
}

TEST(MutableMeshTest, AsMeshesFlippedTrianglesFailureCase) {
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({0, 0});
  m.AppendVertex({4095, 0});
  m.AppendVertex({4095, 4095});
  m.AppendVertex({225.634, 2038.04});
  m.AppendVertex({233.615, 2038.78});
  m.AppendVertex({3928.94, 2247.34});
  m.AppendVertex({1978, 2231.02});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({6, 3, 4});  // Flipped
  m.AppendTriangleIndices({4, 3, 5});

  // TODO: b/283825926 - We can't correct the flipped triangles in this mesh
  // right now -- the algorithm's current search space does not contain a
  // solution.
  // We might fix this by expanding the search space (e.g. allowing correction
  // by more than one unit, or by allowing correction to move a vertex in the
  // same direction as quantization error), or by retrying with a different
  // scaling factor (which gives us a new search space).

  // `AsMeshes` will succeed, but the result still contain flipped triangles.
  auto meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  EXPECT_THAT(*meshes, Not(Each(MeshTrianglesHaveNonNegativeArea())));
}

TEST(MutableMeshTest,
     AsMeshesFlippedTrianglesFailureCaseWithCustomPackingParams) {
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({0, 0});
  m.AppendVertex({4095, 0});
  m.AppendVertex({4095, 4095});
  m.AppendVertex({225.634, 2038.04});
  m.AppendVertex({233.615, 2038.78});
  m.AppendVertex({3928.94, 2247.34});
  m.AppendVertex({1978, 2231.02});
  m.AppendTriangleIndices({0, 1, 2});
  m.AppendTriangleIndices({6, 3, 4});
  m.AppendTriangleIndices({4, 3, 5});

  // `AsMeshes` will succeed, but the result still contain flipped triangles.
  constexpr float kScale = 4095.f / 3974;
  auto meshes = m.AsMeshes({MeshAttributeCodingParams{
      {{.offset = 0, .scale = kScale}, {.offset = 0, .scale = kScale}}}});
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  EXPECT_THAT(*meshes, Not(Each(MeshTrianglesHaveNonNegativeArea())));
}

TEST(MutableMeshTest, AsMeshesOmitAttribute) {
  absl::StatusOr<MeshFormat> original_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(original_format.status(), absl::OkStatus());
  MutableMesh mutable_mesh(*original_format);
  mutable_mesh.AppendVertex({0, 0});
  mutable_mesh.AppendVertex({4, 0});
  mutable_mesh.AppendVertex({0, 3});
  mutable_mesh.AppendTriangleIndices({0, 1, 2});

  absl::StatusOr<absl::InlinedVector<Mesh, 1>> meshes =
      mutable_mesh.AsMeshes({}, {MeshFormat::AttributeId::kColorShiftHsl});
  ASSERT_EQ(meshes.status(), absl::OkStatus());

  ASSERT_THAT(*meshes, SizeIs(1));
  const Mesh& mesh = (*meshes)[0];

  absl::StatusOr<MeshFormat> expected_format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(expected_format.status(), absl::OkStatus());

  EXPECT_EQ(mesh.Format(), *expected_format);
  ASSERT_EQ(mesh.TriangleCount(), 1);
  EXPECT_EQ(mesh.GetTriangle(0), (Triangle{{0, 0}, {4, 0}, {0, 3}}));
}

TEST(MutableMeshTest, AddMeshesPositionOmittedIsError) {
  MutableMesh mutable_mesh;
  mutable_mesh.AppendVertex({0, 0});
  mutable_mesh.AppendVertex({4, 0});
  mutable_mesh.AppendVertex({0, 3});
  mutable_mesh.AppendTriangleIndices({0, 1, 2});

  absl::Status position_omitted =
      mutable_mesh.AsMeshes({}, {MeshFormat::AttributeId::kPosition}).status();
  EXPECT_EQ(position_omitted.code(), absl::StatusCode::kInvalidArgument);
}

TEST(MutableMeshTest, AsMeshesFuzzFailureb295270747) {
  // This is the reduced case from b/295270747, with unused vertices removed.
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({-1, 1});
  m.AppendVertex({0, -2435.0625});
  m.AppendVertex({0.98450989, 0});
  m.AppendVertex({0, 0.46548146});
  m.AppendTriangleIndices({2, 0, 3});
  m.AppendTriangleIndices({2, 0, 1});

  // This was previously CHECK-failing due to an error in bounds calculation.
  auto meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  EXPECT_THAT(*meshes, Each(MeshTrianglesHaveNonNegativeArea()));
}

TEST(MutableMeshTest, AsMeshesFuzzFailureb294848324) {
  // This is the reduced case from b/294848324, with unused vertices removed.
  MutableMesh m(MakeSinglePackedPositionFormat());
  m.AppendVertex({-1, 1});
  m.AppendVertex({0.031326283, 0});
  m.AppendVertex({-0, 0.31641644});
  m.AppendVertex({1, -1});
  m.AppendVertex({0, -10957.781});
  m.AppendVertex({0, 0.46548146});
  m.AppendTriangleIndices({1, 3, 5});
  m.AppendTriangleIndices({0, 4, 2});

  // This was previously failing, returning an OK status and a mesh that still
  // had flipped triangles, do an error in the scale of the nudge vectors.
  auto meshes = m.AsMeshes();
  ASSERT_EQ(meshes.status(), absl::OkStatus());
  EXPECT_THAT(*meshes, Each(MeshTrianglesHaveNonNegativeArea()));
}

void AsMeshesDoesNotFailForValidMutableMesh(const MutableMesh& mutable_mesh) {
  // `AsMeshes` will now always succeed on a valid arbitrary MutableMesh, but
  // the result may contain flipped triangles.
  EXPECT_EQ(absl::OkStatus(), mutable_mesh.AsMeshes().status());
}
FUZZ_TEST(MutableMeshTest, AsMeshesDoesNotFailForValidMutableMesh)
    .WithDomains(ValidPackableNonEmptyPositionOnlyMutableMesh(
        MeshFormat::AttributeType::kFloat2PackedInOneFloat));

TEST(MutableMeshDeathTest, VertexIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({1, 2});
  m.AppendVertex({3, 4});

  EXPECT_DEATH_IF_SUPPORTED(m.VertexPosition(3), "");
  EXPECT_DEATH_IF_SUPPORTED(m.SetVertexPosition(3, {5, 6}), "");
  EXPECT_DEATH_IF_SUPPORTED(m.FloatVertexAttribute(3, 0), "");
  EXPECT_DEATH_IF_SUPPORTED(
      m.SetFloatVertexAttribute(3, 0, SmallArray<float, 4>({1})), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MutableMeshDeathTest, AttributeIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({1, 2});
  m.AppendVertex({3, 4});

  EXPECT_DEATH_IF_SUPPORTED(m.FloatVertexAttribute(1, 3), "");
  EXPECT_DEATH_IF_SUPPORTED(
      m.SetFloatVertexAttribute(1, 3, SmallArray<float, 4>({1})), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MutableMeshDeathTest, WrongNumberOfComponentsForAttribute) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({1, 2});
  m.AppendVertex({3, 4});

  EXPECT_DEATH_IF_SUPPORTED(
      m.SetFloatVertexAttribute(1, 1, SmallArray<float, 4>({1, 2, 3})), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MutableMeshDeathTest, TriangleIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({1, 2});
  m.AppendVertex({3, 4});
  m.AppendVertex({5, 6});
  m.AppendVertex({0, 4});
  m.AppendTriangleIndices({0, 1, 3});
  m.AppendTriangleIndices({0, 2, 3});

  EXPECT_DEATH_IF_SUPPORTED(m.TriangleIndices(2), "");
  EXPECT_DEATH_IF_SUPPORTED(m.SetTriangleIndices(2, {1, 2, 3}), "");
  EXPECT_DEATH_IF_SUPPORTED(m.InsertTriangleIndices(3, {1, 2, 3}), "");
  EXPECT_DEATH_IF_SUPPORTED(m.GetTriangle(2), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MutableMeshDeathTest, TriangleVertexIndexNotRepresentable) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  std::array<uint32_t, 3> bad_indices{1, 65536, 2};
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendTriangleIndices({0, 1, 2});

  EXPECT_DEATH_IF_SUPPORTED(m.AppendTriangleIndices(bad_indices), "");
  EXPECT_DEATH_IF_SUPPORTED(m.SetTriangleIndices(0, bad_indices), "");
  EXPECT_DEATH_IF_SUPPORTED(m.InsertTriangleIndices(1, bad_indices), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MutableMeshDeathTest, TriangleVertexIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  MutableMesh m(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition},
                           {MeshFormat::AttributeType::kFloat1Unpacked,
                            MeshFormat::AttributeId::kCustom0}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked));
  m.AppendVertex({1, 2});
  m.AppendVertex({3, 4});
  m.AppendTriangleIndices({0, 1, 2});

  EXPECT_DEATH_IF_SUPPORTED(m.GetTriangle(0), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

}  // namespace
}  // namespace ink
