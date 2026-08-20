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

#include "ink/geometry/mesh.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/geometry/internal/mesh_packing.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/type_matchers.h"

namespace ink {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::HasSubstr;
using ::testing::Pointwise;

using AttrType = MeshFormat::AttributeType;
using AttrId = MeshFormat::AttributeId;

constexpr int kMax6Bits = mesh_internal::MaxValueForBits(6);
constexpr int kMax12Bits = mesh_internal::MaxValueForBits(12);
constexpr int kMax16Bits = mesh_internal::MaxValueForBits(16);

TEST(MeshTest, DefaultCtor) {
  Mesh m;
  EXPECT_EQ(m.VertexCount(), 0);
  EXPECT_THAT(m.Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(m.VertexPositionAttributeIndex(), 0);
  // The default `MeshFormat` has a packed vertex stride of 8 bytes.
  EXPECT_EQ(m.VertexStride(), 8);
  EXPECT_EQ(m.IndexStride(), 2);
  EXPECT_THAT(m.VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));
  EXPECT_TRUE(m.RawVertexData().empty());
  EXPECT_TRUE(m.RawIndexData().empty());
  EXPECT_TRUE(m.Bounds().IsEmpty());
  EXPECT_THAT(m.AttributeBounds(0), Eq(std::nullopt));
}

TEST(MeshTest, CreateWithDefaultFormat) {
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {5, 10, 20},
                                         {50, -30, 12}},
                                        // Triangles
                                        {0, 1, 2});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_EQ(m->VertexCount(), 3);
  EXPECT_EQ(m->TriangleCount(), 1);
  EXPECT_THAT(m->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(m->VertexPositionAttributeIndex(), 0);
  // The default `MeshFormat` has a packed vertex stride of 8 bytes.
  EXPECT_EQ(m->VertexStride(), 8);
  EXPECT_EQ(m->IndexStride(), 2);
  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));
  EXPECT_EQ(m->VertexPosition(0), (Point{5, 50}));
  EXPECT_EQ(m->VertexPosition(1), (Point{10, -30}));
  EXPECT_EQ(m->VertexPosition(2), (Point{20, 12}));
  EXPECT_THAT(m->Bounds(), EnvelopeEq(Rect::FromTwoPoints({5, -30}, {20, 50})));
  EXPECT_THAT(m->AttributeBounds(0),
              Optional(MeshAttributeBoundsEq(
                  {.minimum = {5, -30}, .maximum = {20, 50}})));
  EXPECT_THAT(m->TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m->GetTriangle(0), TriangleEq({{5, 50}, {10, -30}, {20, 12}}));
}

TEST(MeshTest, CreateWithCustomFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                           MeshFormat::AttributeId::kCustom0},
                          {MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  absl::StatusOr<Mesh> m = Mesh::Create(*format,
                                        {// Custom attribute
                                         {-200, 100, 500},
                                         {4, 5, 6},
                                         {.1, 25, -5},
                                         // Color
                                         {0, .5, 1},
                                         {.9, .5, .1},
                                         {.5, 1, .5},
                                         {1, 1, 1},
                                         // Position
                                         {17, -12, 5},
                                         {123, 456, 789}},
                                        // Triangles
                                        {0, 1, 2});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_EQ(m->VertexCount(), 3);
  EXPECT_EQ(m->TriangleCount(), 1);
  EXPECT_THAT(m->Format(), MeshFormatEq(*format));
  EXPECT_EQ(m->VertexPositionAttributeIndex(), 2);
  EXPECT_EQ(m->VertexStride(), 16);
  EXPECT_EQ(m->IndexStride(), 2);

  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = -200, .scale = 700. / kMax16Bits},
                     {.offset = 4, .scale = 2. / kMax16Bits},
                     {.offset = -5, .scale = 30. / kMax16Bits}}}}));
  EXPECT_THAT(
      m->VertexAttributeUnpackingParams(1),
      MeshAttributeCodingParamsEq({{{{.offset = 0, .scale = 1. / kMax6Bits},
                                     {.offset = 0.1, .scale = 0.8 / kMax6Bits},
                                     {.offset = 0.5, .scale = 0.5 / kMax6Bits},
                                     {.offset = 1, .scale = 1}}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = -12, .scale = 29. / kMax12Bits},
                     {.offset = 123, .scale = 666. / kMax12Bits}}}}));

  EXPECT_THAT(m->TriangleIndices(0), ElementsAre(0, 1, 2));

  // The maximum error values for each component of the custom attribute are
  // approximately 5.34e-3, 1.53e-3, and 2.29e-4.
  EXPECT_THAT(m->FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{-200, 4, .1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{100, 5, 25}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{500, 6, -5}));
  EXPECT_THAT(m->AttributeBounds(0),
              Optional(MeshAttributeBoundsNear(
                  {.minimum = {-200, 4, -5}, .maximum = {500, 6, 25}}, 6e-3)));

  // The maximum error values for each color component are 7.94e-3, 6.35e-3,
  // 3.97e-3, and 0.
  EXPECT_THAT(m->FloatVertexAttribute(0, 1).Values(),
              Pointwise(FloatNear(.008), std::vector<float>{0, .9, .5, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 1).Values(),
              Pointwise(FloatNear(.008), std::vector<float>{.5, .5, 1, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 1).Values(),
              Pointwise(FloatNear(.008), std::vector<float>{1, .1, .5, 1}));
  EXPECT_THAT(
      m->AttributeBounds(1),
      Optional(MeshAttributeBoundsNear(
          {.minimum = {0, .1, .5, 1}, .maximum = {1, 0.9, 1, 1}}, 0.008)));

  // The maximum error values for each position component are approximately
  // 3.54e-3 and 8.13e-2.
  EXPECT_THAT(m->VertexPosition(0), PointNear({17, 123}, 0.0036, 0.082));
  EXPECT_THAT(m->VertexPosition(1), PointNear({-12, 456}, 0.0036, 0.082));
  EXPECT_THAT(m->VertexPosition(2), PointNear({5, 789}, 0.0036, 0.082));
  EXPECT_THAT(m->Bounds(),
              EnvelopeNear(Rect::FromTwoPoints({-12, 123}, {17, 789}), 0.082));
  EXPECT_THAT(m->AttributeBounds(2),
              Optional(MeshAttributeBoundsNear(
                  {.minimum = {-12, 123}, .maximum = {17, 789}}, 0.082)));
  EXPECT_THAT(m->GetTriangle(0),
              TriangleNear({{17, 123}, {-12, 456}, {5, 789}}, 0.082));
}

TEST(MeshTest, CreateEmptyMeshWithDefaultFormat) {
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(), {{}, {}}, {});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_EQ(m->VertexCount(), 0);
  EXPECT_EQ(m->TriangleCount(), 0);
  EXPECT_THAT(m->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(m->VertexPositionAttributeIndex(), 0);
  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));
  EXPECT_TRUE(m->Bounds().IsEmpty());
  EXPECT_THAT(m->AttributeBounds(0), Eq(std::nullopt));
}

TEST(MeshTest, CreateEmptyMeshWithCustomFormat) {
  absl::StatusOr<MeshFormat> format =
      MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                           MeshFormat::AttributeId::kCustom0},
                          {MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                           MeshFormat::AttributeId::kColorShiftHsl},
                          {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                           MeshFormat::AttributeId::kPosition}},
                         MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());

  absl::StatusOr<Mesh> m =
      Mesh::Create(*format, {{}, {}, {}, {}, {}, {}, {}, {}, {}}, {});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_EQ(m->VertexCount(), 0);
  EXPECT_EQ(m->TriangleCount(), 0);
  EXPECT_THAT(m->Format(), MeshFormatEq(*format));
  EXPECT_EQ(m->VertexPositionAttributeIndex(), 2);
  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq({{{{.offset = 0, .scale = 1},
                                             {.offset = 0, .scale = 1},
                                             {.offset = 0, .scale = 1}}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(1),
              MeshAttributeCodingParamsEq({{{{.offset = 0, .scale = 1},
                                             {.offset = 0, .scale = 1},
                                             {.offset = 0, .scale = 1},
                                             {.offset = 0, .scale = 1}}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));
  EXPECT_TRUE(m->Bounds().IsEmpty());
  EXPECT_THAT(m->AttributeBounds(0), Eq(std::nullopt));
  EXPECT_THAT(m->AttributeBounds(1), Eq(std::nullopt));
  EXPECT_THAT(m->AttributeBounds(2), Eq(std::nullopt));
}

TEST(MeshTest, CreateWithMultipleTriangles) {
  //   3---2
  //  / \ / \
  // 4---0---1
  //  \ / \ /
  //   5---6
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {0, 2, 1, -1, -2, -1, 1},
                                         {0, 0, 2, 2, 0, -2, -2}},
                                        // Triangles
                                        {0, 1, 2,  //
                                         0, 2, 3,  //
                                         0, 3, 4,  //
                                         0, 4, 5,  //
                                         0, 5, 6,  //
                                         0, 6, 1});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_EQ(m->VertexCount(), 7);
  EXPECT_EQ(m->TriangleCount(), 6);
  EXPECT_THAT(m->Format(), MeshFormatEq(MeshFormat()));
  EXPECT_EQ(m->VertexPositionAttributeIndex(), 0);
  // The default `MeshFormat` has a packed vertex stride of 8 bytes.
  EXPECT_EQ(m->VertexStride(), 8);
  EXPECT_EQ(m->IndexStride(), 2);
  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}}));
  EXPECT_EQ(m->VertexPosition(0), (Point{0, 0}));
  EXPECT_EQ(m->VertexPosition(1), (Point{2, 0}));
  EXPECT_EQ(m->VertexPosition(2), (Point{1, 2}));
  EXPECT_EQ(m->VertexPosition(3), (Point{-1, 2}));
  EXPECT_EQ(m->VertexPosition(4), (Point{-2, 0}));
  EXPECT_EQ(m->VertexPosition(5), (Point{-1, -2}));
  EXPECT_EQ(m->VertexPosition(6), (Point{1, -2}));

  EXPECT_THAT(m->TriangleIndices(0), ElementsAre(0, 1, 2));
  EXPECT_THAT(m->TriangleIndices(1), ElementsAre(0, 2, 3));
  EXPECT_THAT(m->TriangleIndices(2), ElementsAre(0, 3, 4));
  EXPECT_THAT(m->TriangleIndices(3), ElementsAre(0, 4, 5));
  EXPECT_THAT(m->TriangleIndices(4), ElementsAre(0, 5, 6));
  EXPECT_THAT(m->TriangleIndices(5), ElementsAre(0, 6, 1));

  EXPECT_THAT(m->GetTriangle(0), TriangleEq({{0, 0}, {2, 0}, {1, 2}}));
  EXPECT_THAT(m->GetTriangle(1), TriangleEq({{0, 0}, {1, 2}, {-1, 2}}));
  EXPECT_THAT(m->GetTriangle(2), TriangleEq({{0, 0}, {-1, 2}, {-2, 0}}));
  EXPECT_THAT(m->GetTriangle(3), TriangleEq({{0, 0}, {-2, 0}, {-1, -2}}));
  EXPECT_THAT(m->GetTriangle(4), TriangleEq({{0, 0}, {-1, -2}, {1, -2}}));
  EXPECT_THAT(m->GetTriangle(5), TriangleEq({{0, 0}, {1, -2}, {2, 0}}));
}

TEST(MeshTest, CreateWithAllPackingParams) {
  absl::StatusOr<Mesh> m = Mesh::Create(
      *MeshFormat::Create(
          {{AttrType::kFloat3PackedInTwoFloats,
            MeshFormat::AttributeId::kCustom0},
           {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
           {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
      // Custom attribute
      {{-200, 100, 500},
       {4, 5, 6},
       {.1, 25, -5},
       // Color
       {0, .5, 1},
       {.9, .5, .1},
       {.5, 1, .5},
       {1, 1, 1},
       // Position
       {17, -12, 5},
       {123, 456, 789}},
      // Triangles
      {0, 1, 2},
      {MeshAttributeCodingParams{{{.offset = -1000, .scale = 1},
                                  {.offset = 3, .scale = 0.1},
                                  {.offset = -10, .scale = 0.01}}},
       MeshAttributeCodingParams{{{.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1}}},
       MeshAttributeCodingParams{
           {{.offset = -20, .scale = 0.5}, {.offset = 100, .scale = .2}}}});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq({{{.offset = -1000, .scale = 1},
                                            {.offset = 3, .scale = 0.1},
                                            {.offset = -10, .scale = 0.01}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(1),
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq({{{.offset = -20, .scale = 0.5},
                                            {.offset = 100, .scale = .2}}}));

  // The chosen packing transform can represent these values nearly exactly
  EXPECT_THAT(m->FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(1e-6), std::vector<float>{-200, 4, .1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatEq(), std::vector<float>{100, 5, 25}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatEq(), std::vector<float>{500, 6, -5}));

  // The chosen packing transform can represent these values exactly.
  EXPECT_THAT(m->FloatVertexAttribute(0, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{0, .9, .5, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{.5, .5, 1, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{1, .1, .5, 1}));

  // The chosen packing transform can represent these values exactly.
  EXPECT_THAT(m->VertexPosition(0), PointEq({17, 123}));
  EXPECT_THAT(m->VertexPosition(1), PointEq({-12, 456}));
  EXPECT_THAT(m->VertexPosition(2), PointEq({5, 789}));
}

TEST(MeshTest, CreateWithSomePackingParams) {
  absl::StatusOr<Mesh> m = Mesh::Create(
      *MeshFormat::Create(
          {{AttrType::kFloat3PackedInTwoFloats,
            MeshFormat::AttributeId::kCustom0},
           {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl},
           {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
      // Custom Attribute
      {{-200, 100, 500},
       {4, 5, 6},
       {.1, 25, -5},
       // Color
       {0, .5, 1},
       {.9, .5, .1},
       {.5, 1, .5},
       {1, 1, 1},
       // Position
       {17, -12, 5},
       {123, 456, 789}},
      // Triangles
      {0, 1, 2},
      {std::nullopt,
       MeshAttributeCodingParams{{{.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1},
                                  {.offset = 0, .scale = 0.1}}},
       std::nullopt});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_THAT(m->VertexAttributeUnpackingParams(0),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = -200, .scale = 700. / kMax16Bits},
                     {.offset = 4, .scale = 2. / kMax16Bits},
                     {.offset = -5, .scale = 30. / kMax16Bits}}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(1),
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1},
                                            {.offset = 0, .scale = 0.1}}}));
  EXPECT_THAT(m->VertexAttributeUnpackingParams(2),
              MeshAttributeCodingParamsEq(
                  {{{{.offset = -12, .scale = 29. / kMax12Bits},
                     {.offset = 123, .scale = 666. / kMax12Bits}}}}));

  EXPECT_THAT(m->TriangleIndices(0), ElementsAre(0, 1, 2));

  // This attribute uses the default unpacking transform; the maximum error
  // values for each component of the custom attribute are approximately
  // 5.34e-3, 1.53e-3, and 2.29e-4, so we check values with a tolerance just
  // higher than the greatest of these.
  EXPECT_THAT(m->FloatVertexAttribute(0, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{-200, 4, .1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{100, 5, 25}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 0).Values(),
              Pointwise(FloatNear(6e-3), std::vector<float>{500, 6, -5}));
  EXPECT_THAT(m->AttributeBounds(0),
              Optional(MeshAttributeBoundsNear(
                  {.minimum = {-200, 4, -5}, .maximum = {500, 6, 25}}, 6e-3)));

  // The chosen unpacking transform can represent these with no error.
  EXPECT_THAT(m->FloatVertexAttribute(0, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{0, .9, .5, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(1, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{.5, .5, 1, 1}));
  EXPECT_THAT(m->FloatVertexAttribute(2, 1).Values(),
              Pointwise(FloatEq(), std::vector<float>{1, .1, .5, 1}));

  // This attribute uses the default unpacking transform; the maximum error
  // values for each position component are approximately 3.54e-3 and 8.13e-2.
  EXPECT_THAT(m->VertexPosition(0), PointNear({17, 123}, 0.0036, 0.082));
  EXPECT_THAT(m->VertexPosition(1), PointNear({-12, 456}, 0.0036, 0.082));
  EXPECT_THAT(m->VertexPosition(2), PointNear({5, 789}, 0.0036, 0.082));
}

template <typename T>
std::vector<std::byte> AsByteVector(absl::Span<const T> t_data) {
  std::vector<std::byte> byte_data(sizeof(T) * t_data.size());
  std::memcpy(byte_data.data(), t_data.data(), byte_data.size());
  return byte_data;
}

TEST(MeshTest, RawVertexDataDefaultFormat) {
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {5, 10},
                                         {15, 20}},
                                        // Triangles
                                        {});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_THAT(m->RawVertexData(),
              ElementsAreArray(AsByteVector<float>({5, 15, 10, 20})));
}

TEST(MeshTest, RawVertexDataCustomFormat) {
  absl::StatusOr<Mesh> m = Mesh::Create(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition}},
                          MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
      {// Color
       {0, .5, 1},
       {.9, .5, .1},
       {.5, 1, .5},
       {1, 1, 1},
       // Position
       {17, -12, 5},
       {123, 456, 789}},
      // Triangles
      {});
  ASSERT_EQ(m.status(), absl::OkStatus());
  EXPECT_THAT(m->RawVertexData(),
              ElementsAreArray(AsByteVector<float>(
                  {258048, 16773120, 8261568, 2047, 16515072, 9838591})));
}

TEST(MeshTest, RawIndexData) {
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {0, 0, 0, 0, 0},
                                         {0, 0, 0, 0, 0}},
                                        // Triangles
                                        {0, 1, 2,  //
                                         0, 2, 3,  //
                                         0, 3, 4,  //
                                         0, 4, 1});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_THAT(m->RawIndexData(),
              ElementsAreArray(AsByteVector<uint16_t>({0, 1, 2,  //
                                                       0, 2, 3,  //
                                                       0, 3, 4,  //
                                                       0, 4, 1})));
}

TEST(MeshTest, CreateErrorWhenPositionMissingComponent) {
  absl::Status position_missing_component =
      Mesh::Create(MeshFormat(),
                   {// Position -- missing a component
                    {5, 10, 20}},
                   // Triangles
                   {})
          .status();
  EXPECT_EQ(position_missing_component.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(position_missing_component.message(),
              HasSubstr("Wrong number of vertex attributes"));
}

TEST(MeshTest, CreateErrorWhenPositionHasExtraComponent) {
  absl::Status position_has_extra_component =
      Mesh::Create(MeshFormat(),
                   {// Position -- extra component
                    {5, 10, 20},
                    {50, -30, 12},
                    {1, 2, 3}},
                   // Triangles
                   {})
          .status();
  EXPECT_EQ(position_has_extra_component.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(position_has_extra_component.message(),
              HasSubstr("Wrong number of vertex attributes"));
}

TEST(MeshTest, CreationErrorTooManyVerticesForIndex) {
  constexpr int kMaxVerticesForIndex = 1 << 16;
  std::vector<float> position_x;
  std::vector<float> position_y;
  position_x.resize(kMaxVerticesForIndex);
  position_y.resize(kMaxVerticesForIndex);
  EXPECT_EQ(absl::OkStatus(),
            Mesh::Create(MeshFormat(), {position_x, position_y}, {}).status());

  position_x.resize(kMaxVerticesForIndex + 1);
  position_y.resize(kMaxVerticesForIndex + 1);
  absl::Status too_many_vertices =
      Mesh::Create(MeshFormat(), {position_x, position_y}, {}).status();
  EXPECT_EQ(too_many_vertices.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(too_many_vertices.message(),
              HasSubstr("more vertices than can be represented by the index"));
}

TEST(MeshTest, CreationErrorAttributesHaveDifferentSizes) {
  absl::Status different_attr_sizes = Mesh::Create(MeshFormat(),
                                                   {// Position
                                                    {1, 2, 3, 4},
                                                    {5, 6, 7, 8, 9}},
                                                   // Triangles
                                                   {})
                                          .status();
  EXPECT_EQ(different_attr_sizes.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(different_attr_sizes.message(), HasSubstr("unequal lengths"));
}

TEST(MeshTest, CreatonErrorInfValue) {
  constexpr float kInf = std::numeric_limits<float>::infinity();
  absl::Status inf_value =
      Mesh::Create(MeshFormat(), {{1, 2, 3}, {1, 2, kInf}}, {}).status();
  EXPECT_EQ(inf_value.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(inf_value.message(), HasSubstr("Non-finite value"));
}

TEST(MeshTest, CreatonErrorNegInfValue) {
  constexpr float kInf = std::numeric_limits<float>::infinity();
  absl::Status neg_inf_value =
      Mesh::Create(MeshFormat(), {{1, 2, 3}, {1, 2, -kInf}}, {}).status();
  EXPECT_EQ(neg_inf_value.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(neg_inf_value.message(), HasSubstr("Non-finite value"));
}

TEST(MeshTest, CreatonErrorNanValue) {
  absl::Status nan_value =
      Mesh::Create(MeshFormat(), {{1, 2, 3}, {1, 2, std::nanf("")}}, {})
          .status();
  EXPECT_EQ(nan_value.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(nan_value.message(), HasSubstr("Non-finite value"));
}

TEST(MeshTest, CreationErrorRangeLargerThanFloatMax) {
  absl::Status range_too_large =
      Mesh::Create(*MeshFormat::Create(
                       {{MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                         MeshFormat::AttributeId::kPosition}},
                       MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
                   {// Position
                    {1, 3e38, -3e38},
                    {1, 2, 3}},
                   // Triangles
                   {})
          .status();
  EXPECT_EQ(range_too_large.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(range_too_large.message(), HasSubstr("exceeds float precision"));
}

TEST(MeshTest, CreationErrorTriangleIndicesTwoNotDivisibleByThree) {
  absl::Status wrong_triangle_indices = Mesh::Create(MeshFormat(),
                                                     {// Position
                                                      {1, 2, 3, 4},
                                                      {5, 6, 7, 9}},
                                                     // Triangles
                                                     {0, 1})
                                            .status();
  EXPECT_EQ(wrong_triangle_indices.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(wrong_triangle_indices.message(), HasSubstr("divisible by 3"));
}

TEST(MeshTest, CreationErrorTriangleIndicesFourNotDivisibleByThree) {
  absl::Status wrong_triangle_indices = Mesh::Create(MeshFormat(),
                                                     {// Position
                                                      {1, 2, 3, 4},
                                                      {5, 6, 7, 9}},
                                                     // Triangles
                                                     {0, 1, 2, 3})
                                            .status();
  EXPECT_EQ(wrong_triangle_indices.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(wrong_triangle_indices.message(), HasSubstr("divisible by 3"));
}

TEST(MeshTest, CreationErrorTriangleRefersToNonExistentVertex) {
  absl::Status non_existent_vertex = Mesh::Create(MeshFormat(),
                                                  {// Position
                                                   {1, 2, 3, 4},
                                                   {5, 6, 7, 9}},
                                                  // Triangles;
                                                  {0, 1, 4})
                                         .status();
  EXPECT_EQ(non_existent_vertex.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(non_existent_vertex.message(), HasSubstr("non-existent vertex"));
}

TEST(MeshTest, CreationErrorWrongNumberOfPackingParams) {
  absl::Status wrong_number_of_packaing_params =
      Mesh::Create(
          // The default format has one attribute (position) with two components
          MeshFormat(),
          // Position
          {{1, 2, 3}, {4, 5, 6}},
          // Triangles
          {0, 1, 2},
          {MeshAttributeCodingParams{
               {{.offset = -20, .scale = 0.25}, {.offset = 1, .scale = 0.5}}},
           MeshAttributeCodingParams{
               {{.offset = -20, .scale = 0.25}, {.offset = 1, .scale = 0.5}}}})
          .status();
  EXPECT_EQ(wrong_number_of_packaing_params.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(wrong_number_of_packaing_params.message(),
              HasSubstr("Wrong number of coding params"));
}

TEST(MeshTest, CreationErrorPackingParamsForUnpackedType) {
  absl::Status packing_params_for_unpacked =
      Mesh::Create(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, MeshFormat::AttributeId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {// Custom attribute
           {-200, 100, 500},
           // Position
           {17, -12, 5},
           {123, 456, 789}},
          // Triangles
          {0, 1, 2},
          {MeshAttributeCodingParams{{{.offset = 1234, .scale = .1234}}},
           MeshAttributeCodingParams{{{.offset = -20, .scale = 0.25},
                                      {.offset = 100, .scale = 0.5}}}})
          .status();
  EXPECT_EQ(packing_params_for_unpacked.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(packing_params_for_unpacked.message(),
              HasSubstr("but the attribute is unpacked"));
}

TEST(MeshTest, CreationErrorInvalidPackingParams) {
  {
    absl::Status invalid_packing_params =
        Mesh::Create(
            MakeSinglePackedPositionFormat(),
            // Position
            {{1, 2, 3}, {4, 5, 6}},
            // Triangles
            {0, 1, 2},
            {MeshAttributeCodingParams{{{.offset = -20, .scale = 0.25}}}})
            .status();
    EXPECT_EQ(invalid_packing_params.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(invalid_packing_params.message(),
                HasSubstr("is not valid for format"));
  }

  {
    absl::Status invalid_packing_params =
        Mesh::Create(
            MakeSinglePackedPositionFormat(),
            // Position
            {{1, 2, 3}, {4, 5, 6}},
            // Triangles
            {0, 1, 2},
            {MeshAttributeCodingParams{{{.offset = -20, .scale = std::nanf("")},
                                        {.offset = 1, .scale = 0.5}}}})
            .status();
    EXPECT_EQ(invalid_packing_params.code(),
              absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(invalid_packing_params.message(),
                HasSubstr("is not valid for format"));
  }
}

TEST(MeshTest, CreationErrorPackingParamsCanNotRepresentAllValues) {
  absl::Status can_not_represent_all_values =
      Mesh::Create(
          MakeSinglePackedPositionFormat(),
          // Position
          {{1, 2, 3}, {4, 5, 6}},
          // Triangles
          {0, 1, 2},
          // Can't represent maximum value.
          {MeshAttributeCodingParams{
              {// This can represent value in [0, 1023.75].
               {.offset = 0, .scale = 0.25},
               // This can represent value in [-2047, 0.5], which does not
               // cover the range of values needed for the y-coordinate.
               {.offset = -2047, .scale = 0.5}}}})
          .status();
  EXPECT_EQ(can_not_represent_all_values.code(),
            absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(can_not_represent_all_values.message(),
              HasSubstr("cannot represent all values"));
}

TEST(MeshTest, CloneDefaultConstructedMesh) {
  Mesh original;

  Mesh clone = original;

  EXPECT_EQ(clone.VertexCount(), original.VertexCount());
  EXPECT_THAT(clone.Format(), MeshFormatEq(original.Format()));
  EXPECT_EQ(clone.VertexPositionAttributeIndex(),
            original.VertexPositionAttributeIndex());
  EXPECT_EQ(clone.VertexStride(), original.VertexStride());
  EXPECT_EQ(clone.IndexStride(), original.IndexStride());
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(0),
      MeshAttributeCodingParamsEq(original.VertexAttributeUnpackingParams(0)));
  EXPECT_THAT(clone.RawVertexData(),
              ElementsAreArray(original.RawVertexData()));
  EXPECT_THAT(clone.RawIndexData(), ElementsAreArray(original.RawIndexData()));
  EXPECT_THAT(clone.Bounds(), EnvelopeEq(original.Bounds()));
  EXPECT_THAT(clone.AttributeBounds(0), Eq(std::nullopt));
}

TEST(MeshTest, CloneNonDefaultEmptyMesh) {
  absl::StatusOr<Mesh> original = Mesh::Create(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                            MeshFormat::AttributeId::kCustom0},
                           {MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition}},
                          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
      {{}, {}, {}, {}, {}, {}, {}, {}, {}}, {});
  ASSERT_EQ(original.status(), absl::OkStatus());

  Mesh clone = *original;

  EXPECT_EQ(clone.VertexCount(), original->VertexCount());
  EXPECT_THAT(clone.Format(), MeshFormatEq(original->Format()));
  EXPECT_EQ(clone.VertexPositionAttributeIndex(),
            original->VertexPositionAttributeIndex());
  EXPECT_EQ(clone.VertexStride(), original->VertexStride());
  EXPECT_EQ(clone.IndexStride(), original->IndexStride());
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(0),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(0)));
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(1),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(1)));
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(2),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(2)));
  EXPECT_THAT(clone.RawVertexData(),
              ElementsAreArray(original->RawVertexData()));
  EXPECT_THAT(clone.RawIndexData(), ElementsAreArray(original->RawIndexData()));
  EXPECT_THAT(clone.Bounds(), EnvelopeEq(original->Bounds()));
  EXPECT_THAT(clone.AttributeBounds(0), Eq(std::nullopt));
  EXPECT_THAT(clone.AttributeBounds(1), Eq(std::nullopt));
  EXPECT_THAT(clone.AttributeBounds(2), Eq(std::nullopt));
}

TEST(MeshTest, CloneNonEmptyMesh) {
  absl::StatusOr<Mesh> original = Mesh::Create(
      *MeshFormat::Create({{MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
                            MeshFormat::AttributeId::kCustom0},
                           {MeshFormat::AttributeType::kFloat4PackedInOneFloat,
                            MeshFormat::AttributeId::kColorShiftHsl},
                           {MeshFormat::AttributeType::kFloat2PackedInOneFloat,
                            MeshFormat::AttributeId::kPosition}},
                          MeshFormat::IndexFormat::k32BitUnpacked16BitPacked),
      {// Custom attribute
       {-200, 100, 500},
       {4, 5, 6},
       {.1, 25, -5},
       // Color
       {0, .5, 1},
       {.9, .5, .1},
       {.5, 1, .5},
       {1, 1, 1},
       // Position
       {17, -12, 5},
       {123, 456, 789}},
      // Triangles
      {0, 1, 2});
  ASSERT_EQ(original.status(), absl::OkStatus());

  Mesh clone = *original;

  EXPECT_EQ(clone.VertexCount(), original->VertexCount());
  EXPECT_THAT(clone.Format(), MeshFormatEq(original->Format()));
  EXPECT_EQ(clone.VertexPositionAttributeIndex(),
            original->VertexPositionAttributeIndex());
  EXPECT_EQ(clone.VertexStride(), original->VertexStride());
  EXPECT_EQ(clone.IndexStride(), original->IndexStride());
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(0),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(0)));
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(1),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(1)));
  EXPECT_THAT(
      clone.VertexAttributeUnpackingParams(2),
      MeshAttributeCodingParamsEq(original->VertexAttributeUnpackingParams(2)));
  EXPECT_THAT(clone.RawVertexData(),
              ElementsAreArray(original->RawVertexData()));
  EXPECT_THAT(clone.RawIndexData(), ElementsAreArray(original->RawIndexData()));
  EXPECT_THAT(clone.Bounds(), EnvelopeEq(original->Bounds()));
  EXPECT_THAT(clone.AttributeBounds(0),
              Optional(MeshAttributeBoundsEq(*original->AttributeBounds(0))));
  EXPECT_THAT(clone.AttributeBounds(1),
              Optional(MeshAttributeBoundsEq(*original->AttributeBounds(1))));
  EXPECT_THAT(clone.AttributeBounds(2),
              Optional(MeshAttributeBoundsEq(*original->AttributeBounds(2))));
  EXPECT_THAT(clone.VertexPosition(0), PointEq(original->VertexPosition(0)));
  EXPECT_THAT(clone.FloatVertexAttribute(0, 0).Values(),
              ElementsAreArray(original->FloatVertexAttribute(0, 0).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(0, 1).Values(),
              ElementsAreArray(original->FloatVertexAttribute(0, 1).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(0, 2).Values(),
              ElementsAreArray(original->FloatVertexAttribute(0, 2).Values()));
  EXPECT_THAT(clone.VertexPosition(1), PointEq(original->VertexPosition(1)));
  EXPECT_THAT(clone.FloatVertexAttribute(1, 0).Values(),
              ElementsAreArray(original->FloatVertexAttribute(1, 0).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(1, 1).Values(),
              ElementsAreArray(original->FloatVertexAttribute(1, 1).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(1, 2).Values(),
              ElementsAreArray(original->FloatVertexAttribute(1, 2).Values()));
  EXPECT_THAT(clone.VertexPosition(2), PointEq(original->VertexPosition(2)));
  EXPECT_THAT(clone.FloatVertexAttribute(2, 0).Values(),
              ElementsAreArray(original->FloatVertexAttribute(2, 0).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(2, 1).Values(),
              ElementsAreArray(original->FloatVertexAttribute(2, 1).Values()));
  EXPECT_THAT(clone.FloatVertexAttribute(2, 2).Values(),
              ElementsAreArray(original->FloatVertexAttribute(2, 2).Values()));
  EXPECT_THAT(clone.TriangleIndices(0),
              ElementsAreArray(original->TriangleIndices(0)));
}

TEST(MeshDeathTest, VertexIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {1, 2},
                                         {3, 4}},
                                        // Triangles
                                        {});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_DEATH_IF_SUPPORTED(m->FloatVertexAttribute(3, 0), "");
  EXPECT_DEATH_IF_SUPPORTED(m->VertexPosition(3), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshDeathTest, AttributeIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {1, 2},
                                         {3, 4}},
                                        // Triangles
                                        {});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_DEATH_IF_SUPPORTED(m->FloatVertexAttribute(0, 1), "");
  EXPECT_DEATH_IF_SUPPORTED(m->VertexAttributeUnpackingParams(1), "");
  EXPECT_DEATH_IF_SUPPORTED(m->AttributeBounds(1), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshDeathTest, TriangleIndexOutOfBounds) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  absl::StatusOr<Mesh> m = Mesh::Create(MeshFormat(),
                                        {// Position
                                         {1, 2, 3, 4},
                                         {5, 6, 7, 8}},
                                        // Triangles
                                        {0, 1, 2,  //
                                         0, 2, 3});
  ASSERT_EQ(m.status(), absl::OkStatus());

  EXPECT_DEATH_IF_SUPPORTED(m->TriangleIndices(3), "");
  EXPECT_DEATH_IF_SUPPORTED(m->GetTriangle(3), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

}  // namespace
}  // namespace ink
