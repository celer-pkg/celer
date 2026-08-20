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

#include "ink/geometry/internal/mesh_packing.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/point.h"
#include "ink/geometry/type_matchers.h"
#include "ink/types/small_array.h"

namespace ink::mesh_internal {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::FieldsAre;
using ::testing::HasSubstr;
using ::testing::Property;

using AttrType = MeshFormat::AttributeType;
using AttrId = MeshFormat::AttributeId;

constexpr float kInf = std::numeric_limits<float>::infinity();
const float kNaN = std::nanf("");

constexpr uint32_t kMax6Bit = MaxValueForBits(6);
constexpr uint32_t kMax8Bit = MaxValueForBits(8);
constexpr uint32_t kMax10Bit = MaxValueForBits(10);
constexpr uint32_t kMax12Bit = MaxValueForBits(12);
constexpr uint32_t kMax16Bit = MaxValueForBits(16);
constexpr uint32_t kMax18Bit = MaxValueForBits(18);
constexpr uint32_t kMax20Bit = MaxValueForBits(20);
constexpr uint32_t kMax24Bit = MaxValueForBits(24);

// This is a helper function that packs and unpacks a mesh attribute value to
// make testing the effects of `PackAttribute` easier.
SmallArray<float, 4> PackAttributeAndGetAsFloatArray(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value) {
  // Create the byte vector.
  std::vector<std::byte> byte_vector(MeshFormat::PackedAttributeSize(type));

  // Pack the attribute and store it in the byte vector.
  PackAttribute(type, packing_params, unpacked_value,
                absl::MakeSpan(byte_vector));

  // Check that the packed attribute cleanly repacks into floats.
  ABSL_CHECK_EQ(MeshFormat::PackedAttributeSize(type) % sizeof(float), 0);

  // Create the output array.
  SmallArray<float, 4> output(MeshFormat::PackedAttributeSize(type) /
                              sizeof(float));

  // Unpack the attribute.
  std::memcpy(output.Values().data(), byte_vector.data(),
              MeshFormat::PackedAttributeSize(type));

  return output;
}

std::vector<std::byte> PackAttributeAndGetAsByteVector(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& unpacked_value) {
  // Create the byte vector.
  std::vector<std::byte> byte_vector(MeshFormat::PackedAttributeSize(type));

  // Pack the attribute and store it in the byte vector.
  PackAttribute(type, packing_params, unpacked_value,
                absl::MakeSpan(byte_vector));

  return byte_vector;
}

SmallArray<float, 4> UnpackAttributeFromFloatArray(
    MeshFormat::AttributeType type,
    const MeshAttributeCodingParams& packing_params,
    const SmallArray<float, 4>& packed_value) {
  // Create the byte vector.
  uint32_t byte_vector_size = packed_value.Values().size() * sizeof(float);
  std::vector<std::byte> byte_vector(byte_vector_size);

  // Copy the floats into bytes.
  std::memcpy(byte_vector.data(), packed_value.Values().data(),
              byte_vector_size);

  // Unpack the attribute and return it as a SmallArray of floats.
  return UnpackAttribute(type, packing_params, absl::MakeSpan(byte_vector));
}

TEST(MeshPackingTest, MaxValueForBits) {
  EXPECT_EQ(MaxValueForBits(0), 0);
  EXPECT_EQ(MaxValueForBits(1), 1);
  EXPECT_EQ(MaxValueForBits(4), 15);
  EXPECT_EQ(MaxValueForBits(6), 63);
  EXPECT_EQ(MaxValueForBits(10), 1023);
  EXPECT_EQ(MaxValueForBits(12), 4095);
  EXPECT_EQ(MaxValueForBits(32), 4294967295);
}

TEST(MeshPackingTest, Float1Unpacked) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat1Unpacked,
                                      {{{.offset = 0, .scale = 1}}}, {2.5})
          .Values(),
      ElementsAre(2.5));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat1Unpacked,
                                    {{{.offset = 0, .scale = 1}}}, {-300})
          .Values(),
      ElementsAre(-300));
}

TEST(MeshPackingTest, Float2Unpacked) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2Unpacked,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {-10, -0.2})
          .Values(),
      ElementsAre(-10, -0.2));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2Unpacked,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {20, 112})
          .Values(),
      ElementsAre(20, 112));
}

TEST(MeshPackingTest, Float2PackedInOneFloat) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(
                  AttrType::kFloat2PackedInOneFloat,
                  {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                  {200.1, 400.9})
                  .Values(),
              ElementsAre(819601));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {819601})
          .Values(),
      ElementsAre(200, 401));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat2PackedInOneFloat,
                                              {{{.offset = -500, .scale = 4},
                                                {.offset = -500, .scale = 4}}},
                                              {-498.1, 2001})
                  .Values(),
              ElementsAre(625));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = -500, .scale = 4}, {.offset = -500, .scale = 4}}}, {625})
          .Values(),
      ElementsAre(-500, 2000));
}

TEST(MeshPackingTest, Float2PackedInThreeUnsignedBytes_XY12) {
  std::vector<std::byte> byte_vector_1 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {555.1, 962.9});
  // 555: 0010 - 0010 - 1011: 2 - 2 - B
  // 963: 0011 - 1100 - 0011: 3 - C - 3
  EXPECT_THAT(byte_vector_1,
              ElementsAre(std::byte(0x22), std::byte(0xB3), std::byte(0xC3)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_1))
          .Values(),
      ElementsAre(555, 963));

  std::vector<std::byte> byte_vector_2 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
      {{{.offset = -500, .scale = 4}, {.offset = -500, .scale = 4}}},
      {-359.1, 3500.1});
  EXPECT_THAT(byte_vector_2,
              ElementsAre(std::byte(0x02), std::byte(0x33), std::byte(0xE8)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                              {{{.offset = -500, .scale = 4},
                                {.offset = -500, .scale = 4}}},
                              byte_vector_2)
                  .Values(),
              ElementsAre(-360, 3500));
}

TEST(MeshPackingTest, Float2PackedInFourUnsignedBytes_X12_Y20) {
  std::vector<std::byte> byte_vector_1 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
      {1385.1, 750749.9});
  // 1385:   0101 - 0110 - 1001              : 5 - 6 - 9
  // 750750: 1011 - 0111 - 0100 - 1001 - 1110: B - 7 - 4 - 9 - E
  EXPECT_THAT(byte_vector_1, ElementsAre(std::byte(0x56), std::byte(0x9B),
                                         std::byte(0x74), std::byte(0x9E)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_1))
          .Values(),
      ElementsAre(1385, 750750));

  std::vector<std::byte> byte_vector_2 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      {{{.offset = -500, .scale = 4}, {.offset = -500, .scale = 4}}},
      {-59.9, 4012012.1});
  EXPECT_THAT(byte_vector_2, ElementsAre(std::byte(0x06), std::byte(0xEF),
                                         std::byte(0x4E), std::byte(0x78)));
  EXPECT_THAT(
      UnpackAttribute(
          AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
          {{{.offset = -500, .scale = 4}, {.offset = -500, .scale = 4}}},
          byte_vector_2)
          .Values(),
      ElementsAre(-60, 4012012));
}

TEST(MeshPackingTest, Float1PackedInOneUnsignedByte) {
  std::vector<std::byte> byte_vector_1 =
      PackAttributeAndGetAsByteVector(AttrType::kFloat1PackedInOneUnsignedByte,
                                      {{{.offset = 0, .scale = 1}}}, {234.1});
  // 234.1 is rounded to become 234.
  // 234 translated into bits is 1110 1010.
  // 1110 1010 translated into hexadecimal is E A.

  EXPECT_THAT(byte_vector_1, ElementsAre(std::byte(0xEA)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                              {{{.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_1))
                  .Values(),
              ElementsAre(234));

  std::vector<std::byte> byte_vector_2 =
      PackAttributeAndGetAsByteVector(AttrType::kFloat1PackedInOneUnsignedByte,
                                      {{{.offset = -90, .scale = 3}}}, {509.9});
  EXPECT_THAT(byte_vector_2, ElementsAre(std::byte(0xC8)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                              {{{.offset = -90, .scale = 3}}}, byte_vector_2)
                  .Values(),
              ElementsAre(510));
}

TEST(MeshPackingTest, Float3Unpacked) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat3Unpacked,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {3, 5, 7})
                  .Values(),
              ElementsAre(3, 5, 7));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3Unpacked,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {6, 8, 10})
                  .Values(),
              ElementsAre(6, 8, 10));
}

TEST(MeshPackingTest, Float3PackedInOneFloat) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {49.9, 100.4, 150})
                  .Values(),
              ElementsAre(3302550));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {3302550})
                  .Values(),
              ElementsAre(50, 100, 150));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = -10, .scale = 0.125},
                                        {.offset = -10, .scale = 0.125},
                                        {.offset = -10, .scale = 0.125}}},
                                      {-9.2, 19.1, 0})
          .Values(),
      ElementsAre(452944));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                            {{{.offset = -10, .scale = 0.125},
                                              {.offset = -10, .scale = 0.125},
                                              {.offset = -10, .scale = 0.125}}},
                                            {452944})
                  .Values(),
              ElementsAre(-9.25, 19.125, 0));
}

TEST(MeshPackingTest, Float3PackedInTwoFloats) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {50.1, 10000, 300.6})
          .Values(),
      ElementsAre(12839, 1048877));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {12839, 1048877})
                  .Values(),
              ElementsAre(50, 10000, 301));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 2000, .scale = 4},
                                        {.offset = 2000, .scale = 4},
                                        {.offset = 2000, .scale = 4}}},
                                      {3000, 200000, 50000})
          .Values(),
      ElementsAre(64193, 6041312));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                            {{{.offset = 2000, .scale = 4},
                                              {.offset = 2000, .scale = 4},
                                              {.offset = 2000, .scale = 4}}},
                                            {64193, 6041312})
                  .Values(),
              ElementsAre(3000, 200000, 50000));
}

TEST(MeshPackingTest, Float3PackedInFourUnsignedBytes_XYZ10) {
  std::vector<std::byte> byte_vector_1 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      {{{.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1}}},
      {401.1, 274.9, 887.9});
  // rounded number - Number in Bits - Number in Hexadecimal
  //           401:   0110 0100 01:    6 - 4
  //           275:   01 0001 0011:    5 - 1 - 3
  //           888:   1101 1110 00:    D - E - 0
  EXPECT_THAT(byte_vector_1, ElementsAre(std::byte(0x64), std::byte(0x51),
                                         std::byte(0x3D), std::byte(0xE0)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                              {{{.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_1))
                  .Values(),
              ElementsAre(401, 275, 888));

  std::vector<std::byte> byte_vector_2 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      {{{.offset = -50, .scale = 2},
        {.offset = -50, .scale = 2},
        {.offset = -50, .scale = 2}}},
      {250, 950, 1750});
  // Formula to adjust for scale and offset:std::round((value - offset) / scale)
  // std::round((250 - (-50)) / 2) = 150
  // std::round((950 - (-50)) / 2) = 500
  // std::round((1750 - (-50)) / 2)= 900
  // adjusted number - Number in Bits - Number in Hexadecimal
  //            150:   0010 0101 10:    2 - 5
  //            500:   01 1111 0100:    9 - F - 4
  //            900:   1110 0001 00:    E - 1 - 0
  EXPECT_THAT(byte_vector_2, ElementsAre(std::byte(0x25), std::byte(0x9F),
                                         std::byte(0x4E), std::byte(0x10)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                              {{{.offset = -50, .scale = 2},
                                {.offset = -50, .scale = 2},
                                {.offset = -50, .scale = 2}}},
                              absl::MakeSpan(byte_vector_2))
                  .Values(),
              ElementsAre(250, 950, 1750));
}

TEST(MeshPackingTest, Float4Unpacked) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat4Unpacked,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {0, 1, 1, 2})
                  .Values(),
              ElementsAre(0, 1, 1, 2));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4Unpacked,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {-5, 10, -15, 20})
                  .Values(),
              ElementsAre(-5, 10, -15, 20));
}

TEST(MeshPackingTest, Float4PackedInOneFloat) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {1.1, 53, 12.7, 41})
                  .Values(),
              ElementsAre(480105));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {480105})
                  .Values(),
              ElementsAre(1, 53, 13, 41));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                              {{{.offset = -12, .scale = 0.5},
                                                {.offset = -12, .scale = 0.5},
                                                {.offset = -12, .scale = 0.5},
                                                {.offset = -12, .scale = 0.5}}},
                                              {-9.6, 4.8, 11.3, 19.1})
                  .Values(),
              ElementsAre(1453054));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                            {{{.offset = -12, .scale = 0.5},
                                              {.offset = -12, .scale = 0.5},
                                              {.offset = -12, .scale = 0.5},
                                              {.offset = -12, .scale = 0.5}}},
                                            {1453054})
                  .Values(),
              ElementsAre(-9.5, 5, 11.5, 19));
}

TEST(MeshPackingTest, Float4PackedInTwoFloats) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {250.1, 790.6, 500, 1023})
          .Values(),
      ElementsAre(1024791, 2049023));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {1024791, 2049023})
                  .Values(),
              ElementsAre(250, 791, 500, 1023));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 5000, .scale = 8},
                                        {.offset = 5000, .scale = 8},
                                        {.offset = 5000, .scale = 8},
                                        {.offset = 5000, .scale = 8}}},
                                      {6001, 11007, 20000.9, 16000})
          .Values(),
      ElementsAre(512751, 7681375));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                            {{{.offset = 5000, .scale = 8},
                                              {.offset = 5000, .scale = 8},
                                              {.offset = 5000, .scale = 8},
                                              {.offset = 5000, .scale = 8}}},
                                            {512751, 7681375})
                  .Values(),
              ElementsAre(6000, 11008, 20000, 16000));
}

TEST(MeshPackingTest, Float4PackedInThreeFloats) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {100, 1000, 10000, 100000})
          .Values(),
      ElementsAre(6400, 4096156, 4294304));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {6400, 4096156, 4294304})
          .Values(),
      ElementsAre(100, 1000, 10000, 100000));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 5, .scale = 1. / 32},
                                        {.offset = 5, .scale = 1. / 32},
                                        {.offset = 5, .scale = 1. / 32},
                                        {.offset = 5, .scale = 1. / 32}}},
                                      {100, 5000, 415.16, 1987})
          .Values(),
      ElementsAre(194599, 393421, 1374144));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = 5, .scale = 1. / 32},
                                      {.offset = 5, .scale = 1. / 32},
                                      {.offset = 5, .scale = 1. / 32},
                                      {.offset = 5, .scale = 1. / 32}}},
                                    {194599, 393421, 1374144})
          .Values(),
      ElementsAre(100, 5000, 415.15625, 1987));
}

TEST(MeshPackingTest, DifferentOffsetAndScalePerComponent) {
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat2PackedInOneFloat,
                                              {{{.offset = -100, .scale = 2},
                                                {.offset = 200, .scale = .5}}},
                                              {5000, 1000})
                  .Values(),
              ElementsAre(10446400));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat2PackedInOneFloat,
                                            {{{.offset = -100, .scale = 2},
                                              {.offset = 200, .scale = .5}}},
                                            {10446400})
                  .Values(),
              ElementsAre(5000, 1000));

  std::vector<std::byte> byte_vector_xy12 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
      {{{.offset = -200, .scale = 5}, {.offset = 50, .scale = .1}}},
      {2305, 170});
  EXPECT_THAT(byte_vector_xy12,
              ElementsAre(std::byte(0x1F), std::byte(0x54), std::byte(0xB0)));
  EXPECT_THAT(UnpackAttribute(
                  AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                  {{{.offset = -200, .scale = 5}, {.offset = 50, .scale = .1}}},
                  byte_vector_xy12)
                  .Values(),
              ElementsAre(2305, 170));

  std::vector<std::byte> byte_vector_x12_y20 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      {{{.offset = -200, .scale = 5}, {.offset = -50, .scale = 2}}},
      {2305, 500050});
  EXPECT_THAT(byte_vector_x12_y20,
              ElementsAre(std::byte(0x1F), std::byte(0x53), std::byte(0xD0),
                          std::byte(0xC2)));
  EXPECT_THAT(UnpackAttribute(
                  AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                  {{{.offset = -200, .scale = 5}, {.offset = -50, .scale = 2}}},
                  byte_vector_x12_y20)
                  .Values(),
              ElementsAre(2305, 500050));

  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = 100, .scale = 1},
                                        {.offset = -400, .scale = 4},
                                        {.offset = -30, .scale = 0.25}}},
                                      {300, 500, 15})
          .Values(),
      ElementsAre(13164980));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                            {{{.offset = 100, .scale = 1},
                                              {.offset = -400, .scale = 4},
                                              {.offset = -30, .scale = 0.25}}},
                                            {13164980})
                  .Values(),
              ElementsAre(300, 500, 15));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = -50000, .scale = 2},
                                        {.offset = 10000, .scale = .5},
                                        {.offset = -200, .scale = .125}}},
                                      {-30000, 20000, 0})
          .Values(),
      ElementsAre(2560078, 2098752));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                            {{{.offset = -50000, .scale = 2},
                                              {.offset = 10000, .scale = .5},
                                              {.offset = -200, .scale = .125}}},
                                            {2560078, 2098752})
                  .Values(),
              ElementsAre(-30000, 20000, 0));

  std::vector<std::byte> byte_vector_xyz10 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      {{{.offset = -50, .scale = 2},
        {.offset = 100, .scale = .1},
        {.offset = 400, .scale = 4}}},
      {250, 150, 4000});
  EXPECT_THAT(byte_vector_xyz10, ElementsAre(std::byte(0x25), std::byte(0x9F),
                                             std::byte(0x4E), std::byte(0x10)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                              {{{.offset = -50, .scale = 2},
                                {.offset = 100, .scale = .1},
                                {.offset = 400, .scale = 4}}},
                              absl::MakeSpan(byte_vector_xyz10))
                  .Values(),
              ElementsAre(250, 150, 4000));

  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                      {{{.offset = 10, .scale = 1},
                                        {.offset = -200, .scale = 8},
                                        {.offset = -10, .scale = 0.5},
                                        {.offset = 20, .scale = 0.125}}},
                                      {50, 72, 5, 25})
          .Values(),
      ElementsAre(10626984));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                            {{{.offset = 10, .scale = 1},
                                              {.offset = -200, .scale = 8},
                                              {.offset = -10, .scale = 0.5},
                                              {.offset = 20, .scale = 0.125}}},
                                            {10626984})
                  .Values(),
              ElementsAre(50, 72, 5, 25));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 100, .scale = 1},
                                        {.offset = 200, .scale = 0.5},
                                        {.offset = 300, .scale = 2},
                                        {.offset = 400, .scale = 0.25}}},
                                      {4000, 500, 1000, 700})
          .Values(),
      ElementsAre(15975000, 1434800));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                            {{{.offset = 100, .scale = 1},
                                              {.offset = 200, .scale = 0.5},
                                              {.offset = 300, .scale = 2},
                                              {.offset = 400, .scale = 0.25}}},
                                            {15975000, 1434800})
                  .Values(),
              ElementsAre(4000, 500, 1000, 700));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = -8000, .scale = 0.0625},
                                        {.offset = -10000, .scale = 0.125},
                                        {.offset = -20000, .scale = 0.25},
                                        {.offset = -50000, .scale = 0.5}}},
                                      {12, 123, 1234, 12345})
          .Values(),
      ElementsAre(8204307, 12944687, 2221842));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = -8000, .scale = 0.0625},
                                      {.offset = -10000, .scale = 0.125},
                                      {.offset = -20000, .scale = 0.25},
                                      {.offset = -50000, .scale = 0.5}}},
                                    {8204307, 12944687, 2221842})
          .Values(),
      ElementsAre(12, 123, 1234, 12345));
}

TEST(MeshPackingTest, UnpackedFormatsIgnoreOffsetAndScale) {
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat1Unpacked,
                                      {{{.offset = 5, .scale = 2}}}, {17})
          .Values(),
      ElementsAre(17));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat1Unpacked,
          {{{.offset = 0, .scale = 1.5}, {.offset = 2, .scale = 6}}}, {102030})
          .Values(),
      ElementsAre(102030));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat2Unpacked,
                                              {{{.offset = kNaN, .scale = -1}}},
                                              {73, 9876})
                  .Values(),
              ElementsAre(73, 9876));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat2Unpacked,
                                            {{{.offset = -400, .scale = .25},
                                              {.offset = -500, .scale = 1.25}}},
                                            {12, 34})
                  .Values(),
              ElementsAre(12, 34));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat3Unpacked,
                                              {{{.offset = 100, .scale = 4},
                                                {.offset = 200, .scale = 5},
                                                {.offset = 300, .scale = 6}}},
                                              {50, 60, 70})
                  .Values(),
              ElementsAre(50, 60, 70));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat3Unpacked,
                                    {{{.offset = -1000, .scale = kInf},
                                      {.offset = -1000, .scale = kInf}}},
                                    {9000, 9001, 9002})
          .Values(),
      ElementsAre(9000, 9001, 9002));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat4Unpacked,
                                              {{{.offset = 0, .scale = -kInf}}},
                                              {5, 6, 7, 8})
                  .Values(),
              ElementsAre(5, 6, 7, 8));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4Unpacked,
                                            {{{.offset = 24, .scale = 5},
                                              {.offset = 18, .scale = 10},
                                              {.offset = 12, .scale = 15},
                                              {.offset = 6, .scale = 20}}},
                                            {15, 30, 45, 60})
                  .Values(),
              ElementsAre(15, 30, 45, 60));
}

TEST(MeshPackingTest, MinimumRepresentableValues) {
  std::vector<std::byte> byte_vector_1 =
      PackAttributeAndGetAsByteVector(AttrType::kFloat1PackedInOneUnsignedByte,
                                      {{{.offset = 0, .scale = 1}}}, {0});
  EXPECT_THAT(byte_vector_1, ElementsAre(std::byte(0x00)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                              {{{.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_1))
                  .Values(),
              ElementsAre(0));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0})
          .Values(),
      ElementsAre(0));
  EXPECT_THAT(UnpackAttributeFromFloatArray(
                  AttrType::kFloat2PackedInOneFloat,
                  {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0})
                  .Values(),
              ElementsAre(0, 0));
  std::vector<std::byte> byte_vector_xy12 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0});
  EXPECT_THAT(byte_vector_xy12,
              ElementsAre(std::byte(0x00), std::byte(0x00), std::byte(0x00)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xy12))
          .Values(),
      ElementsAre(0, 0));
  std::vector<std::byte> byte_vector_x12_y20 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0});
  EXPECT_THAT(byte_vector_x12_y20,
              ElementsAre(std::byte(0x00), std::byte(0x00), std::byte(0x00),
                          std::byte(0x00)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_x12_y20))
          .Values(),
      ElementsAre(0, 0));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {0, 0, 0})
                  .Values(),
              ElementsAre(0));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {0})
                  .Values(),
              ElementsAre(0, 0, 0));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, 0})
          .Values(),
      ElementsAre(0, 0));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {0, 0})
                  .Values(),
              ElementsAre(0, 0, 0));
  std::vector<std::byte> byte_vector_xyz10 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      {{{.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1}}},
      {0, 0, 0});
  EXPECT_THAT(byte_vector_xyz10, ElementsAre(std::byte(0x00), std::byte(0x00),
                                             std::byte(0x00), std::byte(0x00)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                              {{{.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_xyz10))
                  .Values(),
              ElementsAre(0, 0, 0));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {0, 0, 0, 0})
                  .Values(),
              ElementsAre(0));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {0})
                  .Values(),
              ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, 0, 0})
          .Values(),
      ElementsAre(0, 0));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {0, 0})
                  .Values(),
              ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, 0, 0})
          .Values(),
      ElementsAre(0, 0, 0));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {0, 0, 0})
          .Values(),
      ElementsAre(0, 0, 0, 0));
}

TEST(MeshPackingTest, MaximumRepresentableValues) {
  std::vector<std::byte> byte_vector_1 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat1PackedInOneUnsignedByte, {{{.offset = 0, .scale = 1}}},
      {kMax8Bit});
  EXPECT_THAT(byte_vector_1, ElementsAre(std::byte(0xFF)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                              {{{.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_1))
                  .Values(),
              ElementsAre(kMax8Bit));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(
                  AttrType::kFloat2PackedInOneFloat,
                  {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                  {kMax12Bit, kMax12Bit})
                  .Values(),
              ElementsAre(kMax24Bit));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {kMax24Bit})
          .Values(),
      ElementsAre(kMax12Bit, kMax12Bit));
  std::vector<std::byte> byte_vector_xy12 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
      {kMax12Bit, kMax12Bit});
  EXPECT_THAT(byte_vector_xy12,
              ElementsAre(std::byte(0xFF), std::byte(0xFF), std::byte(0xFF)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xy12))
          .Values(),
      ElementsAre(kMax12Bit, kMax12Bit));
  std::vector<std::byte> byte_vector_x12_y20 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
      {kMax12Bit, kMax20Bit});
  EXPECT_THAT(byte_vector_x12_y20,
              ElementsAre(std::byte(0xFF), std::byte(0xFF), std::byte(0xFF),
                          std::byte(0xFF)));
  EXPECT_THAT(
      UnpackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_x12_y20))
          .Values(),
      ElementsAre(kMax12Bit, kMax20Bit));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                              {{{.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1},
                                                {.offset = 0, .scale = 1}}},
                                              {kMax8Bit, kMax8Bit, kMax8Bit})
                  .Values(),
              ElementsAre(kMax24Bit));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {kMax24Bit})
                  .Values(),
              ElementsAre(kMax8Bit, kMax8Bit, kMax8Bit));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {kMax16Bit, kMax16Bit, kMax16Bit})
          .Values(),
      ElementsAre(kMax24Bit, kMax24Bit));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {kMax24Bit, kMax24Bit})
                  .Values(),
              ElementsAre(kMax16Bit, kMax16Bit, kMax16Bit));
  std::vector<std::byte> byte_vector_xyz10 = PackAttributeAndGetAsByteVector(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      {{{.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1},
        {.offset = 0, .scale = 1}}},
      {kMax10Bit, kMax10Bit, kMax10Bit});
  EXPECT_THAT(byte_vector_xyz10, ElementsAre(std::byte(0xFF), std::byte(0xFF),
                                             std::byte(0xFF), std::byte(0xFC)));
  EXPECT_THAT(UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                              {{{.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1},
                                {.offset = 0, .scale = 1}}},
                              absl::MakeSpan(byte_vector_xyz10))
                  .Values(),
              ElementsAre(kMax10Bit, kMax10Bit, kMax10Bit));
  EXPECT_THAT(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {kMax6Bit, kMax6Bit, kMax6Bit, kMax6Bit})
          .Values(),
      ElementsAre(kMax24Bit));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {kMax24Bit})
                  .Values(),
              ElementsAre(kMax6Bit, kMax6Bit, kMax6Bit, kMax6Bit));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(
                  AttrType::kFloat4PackedInTwoFloats,
                  {{{.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1}}},
                  {kMax12Bit, kMax12Bit, kMax12Bit, kMax12Bit})
                  .Values(),
              ElementsAre(kMax24Bit, kMax24Bit));
  EXPECT_THAT(UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                            {{{.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1},
                                              {.offset = 0, .scale = 1}}},
                                            {kMax24Bit, kMax24Bit})
                  .Values(),
              ElementsAre(kMax12Bit, kMax12Bit, kMax12Bit, kMax12Bit));
  EXPECT_THAT(PackAttributeAndGetAsFloatArray(
                  AttrType::kFloat4PackedInThreeFloats,
                  {{{.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1},
                    {.offset = 0, .scale = 1}}},
                  {kMax18Bit, kMax18Bit, kMax18Bit, kMax18Bit})
                  .Values(),
              ElementsAre(kMax24Bit, kMax24Bit, kMax24Bit));
  EXPECT_THAT(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {kMax24Bit, kMax24Bit, kMax24Bit})
          .Values(),
      ElementsAre(kMax18Bit, kMax18Bit, kMax18Bit, kMax18Bit));
}

TEST(MeshPackingDeathTest, CannotPackWrongNumberOfComponents) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat1Unpacked,
                                      {{{.offset = 0, .scale = 1}}}, {1, 2}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(AttrType::kFloat1PackedInOneUnsignedByte,
                                      {{{.offset = 0, .scale = 1}}}, {1, 2}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2Unpacked,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3Unpacked,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
          {{{.offset = 0, .scale = 1},
            {.offset = 0, .scale = 1},
            {.offset = 0, .scale = 1}}},
          {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4Unpacked,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {1}),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotPackUnrepresentableValues) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> byte_vector_1(MeshFormat::PackedAttributeSize(
      AttrType::kFloat1PackedInOneUnsignedByte));
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                    {{{.offset = 0, .scale = 1}}}, {256},
                    absl::MakeSpan(byte_vector_1)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                    {{{.offset = 0, .scale = 1}}}, {-1},
                    absl::MakeSpan(byte_vector_1)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 4096}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {-1, 0}),
      "");
  std::vector<std::byte> byte_vector_xy12(MeshFormat::PackedAttributeSize(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12));
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                    {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                    {0, 4096}, absl::MakeSpan(byte_vector_xy12)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                    {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                    {-1, 0}, absl::MakeSpan(byte_vector_xy12)),
      "");
  std::vector<std::byte> byte_vector_x12_y20(MeshFormat::PackedAttributeSize(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20));
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                    {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                    {kMax12Bit + 1, 0}, absl::MakeSpan(byte_vector_x12_y20)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                    {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                    {0, kMax20Bit + 1}, absl::MakeSpan(byte_vector_x12_y20)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {256, 0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, -1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 65536, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {-1, 0, 0}),
      "");
  std::vector<std::byte> byte_vector_xyz10(MeshFormat::PackedAttributeSize(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10));
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                    {{{.offset = 0, .scale = 1},
                      {.offset = 0, .scale = 1},
                      {.offset = 0, .scale = 1}}},
                    {0, kMax10Bit + 1, 0}, absl::MakeSpan(byte_vector_xyz10)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                    {{{.offset = 0, .scale = 1},
                      {.offset = 0, .scale = 1},
                      {.offset = 0, .scale = 1}}},
                    {0, 0, -1}, absl::MakeSpan(byte_vector_xyz10)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, 0, 64}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, -1, 0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {4096, 0, 0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, -1, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 262144, 0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{{.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1},
                                        {.offset = 0, .scale = 1}}},
                                      {0, 0, -1, 0}),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotUnpackWrongNumberOfComponents) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat1Unpacked,
                                    {{{.offset = 0, .scale = 1}}}, {1, 2}),
      "");
  std::vector<std::byte> byte_vector_1(2);
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_1)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2Unpacked,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1, 2}),
      "");
  std::vector<std::byte> byte_vector_xy12(4);
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xy12)),
      "");
  std::vector<std::byte> byte_vector_x12_y20(5);
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_x12_y20)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat3Unpacked,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1, 2}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInTwoFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1}),
      "");
  std::vector<std::byte> byte_vector_xyz10(3);
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                      {{{.offset = 0, .scale = 1},
                        {.offset = 0, .scale = 1},
                        {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xyz10)),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat4Unpacked,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInOneFloat,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1, 2}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInTwoFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                    {{{.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1},
                                      {.offset = 0, .scale = 1}}},
                                    {1}),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotUnpackUnrepresentableValues) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {-1}),
      "Cannot unpack: Unrepresentable value found");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1e8}),
      "Cannot unpack: Unrepresentable value found");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotPackNonFiniteValues) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {-kInf, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {kNaN, 0}),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotUnpackNonFiniteValues) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {kInf}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {kNaN}),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotPackWrongNumberOfPackingParams) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat1PackedInOneUnsignedByte,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat2PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1}}}, {1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
          {{{.offset = 0, .scale = 1}}}, {1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
          {{{.offset = 0, .scale = 1}}}, {1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat3PackedInOneFloat,
                                      {{{.offset = 0, .scale = 1}}}, {1, 1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat3PackedInTwoFloats,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1, 1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsByteVector(
          AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1, 1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat4PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
          {1, 1, 1, 1}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(PackAttributeAndGetAsFloatArray(
                                AttrType::kFloat4PackedInTwoFloats,
                                {{{.offset = 0, .scale = 1}}}, {1, 1, 1, 1}),
                            "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(AttrType::kFloat4PackedInThreeFloats,
                                      {{}}, {1, 1, 1, 1}),
      "Invalid packing params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotUnpackWrongNumberOfUnpackingParams) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat2PackedInOneFloat,
                                    {{{.offset = 0, .scale = 1}}}, {1, 1}),
      "Invalid unpacking params");
  std::vector<std::byte> byte_vector_1(MeshFormat::PackedAttributeSize(
      AttrType::kFloat1PackedInOneUnsignedByte));
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat1PackedInOneUnsignedByte,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_1)),
      "Invalid unpacking params");
  std::vector<std::byte> byte_vector_xy12(MeshFormat::PackedAttributeSize(
      AttrType::kFloat2PackedInThreeUnsignedBytes_XY12));
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                      {{{.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xy12)),
      "Invalid unpacking params");
  std::vector<std::byte> byte_vector_x12_y20(MeshFormat::PackedAttributeSize(
      AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20));
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                      {{{.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_x12_y20)),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat3PackedInOneFloat,
                                    {{{.offset = 0, .scale = 1}}}, {1, 1, 1}),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat3PackedInTwoFloats,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {1, 1, 1}),
      "Invalid unpacking params");
  std::vector<std::byte> byte_vector_xyz10(MeshFormat::PackedAttributeSize(
      AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10));
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttribute(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                      {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
                      absl::MakeSpan(byte_vector_xyz10)),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat4PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}},
          {1, 1, 1, 1}),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(UnpackAttributeFromFloatArray(
                                AttrType::kFloat4PackedInTwoFloats,
                                {{{.offset = 0, .scale = 1}}}, {1, 1, 1, 1}),
                            "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(AttrType::kFloat4PackedInThreeFloats, {{}},
                                    {1, 1, 1, 1}),
      "Invalid unpacking params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotHaveNonFiniteOffset) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = kInf, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = kNaN, .scale = 1}}}, {0, 0}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = -kInf, .scale = 1}}}, {0}),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = kNaN, .scale = 1}, {.offset = 0, .scale = 1}}}, {0}),
      "Invalid unpacking params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotHaveNegativeScale) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = -1}}}, {0, 0}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = -1}, {.offset = 0, .scale = 1}}}, {0}),
      "Invalid unpacking params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, CannotHaveNonFiniteScale) {
  // There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
  // compiled in debug mode.
#ifndef NDEBUG
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = -kInf}}}, {0, 0}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = kNaN}, {.offset = 0, .scale = 1}}}, {0, 0}),
      "Invalid packing params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = kInf}, {.offset = 0, .scale = kInf}}}, {0}),
      "Invalid unpacking params");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          AttrType::kFloat2PackedInOneFloat,
          {{{.offset = 0, .scale = kNaN}, {.offset = 0, .scale = kNaN}}}, {0}),
      "Invalid unpacking params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, BadAttributeType) {
  EXPECT_DEATH_IF_SUPPORTED(
      PackAttributeAndGetAsFloatArray(
          static_cast<AttrType>(50),
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      UnpackAttributeFromFloatArray(
          static_cast<AttrType>(50),
          {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}, {0, 0}),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      ComputeCodingParams(static_cast<AttrType>(50),
                          {.minimum = {1, 0}, .maximum = {1, 1}})
          .IgnoreError(),
      "");
}

TEST(MeshPackingTest, ReadAndWrite16BitTriangleIndices) {
  std::vector<std::byte> bytes(sizeof(uint16_t) * 12);

  WriteTriangleIndicesToByteArray(0, sizeof(uint16_t), {0, 1, 2}, bytes);
  WriteTriangleIndicesToByteArray(1, sizeof(uint16_t), {3, 4, 5}, bytes);
  WriteTriangleIndicesToByteArray(2, sizeof(uint16_t), {6, 7, 8}, bytes);
  WriteTriangleIndicesToByteArray(3, sizeof(uint16_t), {9, 10, 11}, bytes);

  EXPECT_THAT(ReadTriangleIndicesFromByteArray(0, sizeof(uint16_t), bytes),
              ElementsAre(0, 1, 2));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(1, sizeof(uint16_t), bytes),
              ElementsAre(3, 4, 5));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(2, sizeof(uint16_t), bytes),
              ElementsAre(6, 7, 8));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(3, sizeof(uint16_t), bytes),
              ElementsAre(9, 10, 11));
}

TEST(MeshPackingTest, ReadAndWrite32BitTriangleIndices) {
  std::vector<std::byte> bytes(sizeof(uint32_t) * 12);

  WriteTriangleIndicesToByteArray(0, sizeof(uint32_t), {0, 1, 2}, bytes);
  WriteTriangleIndicesToByteArray(1, sizeof(uint32_t), {3, 4, 5}, bytes);
  WriteTriangleIndicesToByteArray(2, sizeof(uint32_t), {6, 7, 8}, bytes);
  WriteTriangleIndicesToByteArray(3, sizeof(uint32_t), {9, 10, 11}, bytes);

  EXPECT_THAT(ReadTriangleIndicesFromByteArray(0, sizeof(uint32_t), bytes),
              ElementsAre(0, 1, 2));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(1, sizeof(uint32_t), bytes),
              ElementsAre(3, 4, 5));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(2, sizeof(uint32_t), bytes),
              ElementsAre(6, 7, 8));
  EXPECT_THAT(ReadTriangleIndicesFromByteArray(3, sizeof(uint32_t), bytes),
              ElementsAre(9, 10, 11));
}

template <typename T>
std::vector<std::byte> AsByteVector(absl::Span<const T> t_data) {
  std::vector<std::byte> byte_data(sizeof(T) * t_data.size());
  std::memcpy(byte_data.data(), t_data.data(), byte_data.size());
  return byte_data;
}

TEST(MeshPackingTest, ReadUnpackedFloatAttributeFromByteArrayDefaultFormat) {
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({-5, 5,  //
                                                      0, 10});

  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(0, 0, bytes, format).Values(),
      ElementsAre(-5, 5));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(1, 0, bytes, format).Values(),
      ElementsAre(0, 10));
}

TEST(MeshPackingTest, ReadUnpackedFloatAttributeFromByteArrayCustomFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  std::vector<std::byte> bytes =
      AsByteVector<float>({25,                    // custom
                           50, 40,                // position
                           0, 1, 0, 1,            // color
                           700,                   // custom
                           100, 200,              // position
                           0.5, 0.5, 0.5, 0.5});  // color

  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(0, 0, bytes, *format).Values(),
      ElementsAre(25));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(0, 1, bytes, *format).Values(),
      ElementsAre(50, 40));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(0, 2, bytes, *format).Values(),
      ElementsAre(0, 1, 0, 1));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(1, 0, bytes, *format).Values(),
      ElementsAre(700));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(1, 1, bytes, *format).Values(),
      ElementsAre(100, 200));
  EXPECT_THAT(
      ReadUnpackedFloatAttributeFromByteArray(1, 2, bytes, *format).Values(),
      ElementsAre(0.5, 0.5, 0.5, 0.5));
}

TEST(MeshPackingTest, PartitionTrianglesOnePartition) {
  std::vector<std::byte> triangle_bytes = AsByteVector<uint32_t>({0, 1, 2,  //
                                                                  3, 4, 5,  //
                                                                  6, 7, 8,  //
                                                                  9, 10, 11});

  absl::InlinedVector<PartitionInfo, 1> partitions = PartitionTriangles(
      triangle_bytes, MeshFormat::IndexFormat::k32BitUnpacked16BitPacked, 20);

  EXPECT_THAT(partitions, ElementsAre(FieldsAre(
                              ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
                              ElementsAre(ElementsAre(0, 1, 2),  //
                                          ElementsAre(3, 4, 5),  //
                                          ElementsAre(6, 7, 8),  //
                                          ElementsAre(9, 10, 11)))));
}

TEST(MeshPackingTest, PartitionTrianglesTwoPartitions) {
  std::vector<std::byte> triangle_bytes = AsByteVector<uint32_t>({0, 1, 2,  //
                                                                  3, 4, 5,  //
                                                                  6, 7, 8,  //
                                                                  9, 10, 11});

  absl::InlinedVector<PartitionInfo, 1> partitions = PartitionTriangles(
      triangle_bytes, MeshFormat::IndexFormat::k32BitUnpacked16BitPacked, 6);

  EXPECT_THAT(partitions,
              ElementsAre(FieldsAre(ElementsAre(0, 1, 2, 3, 4, 5),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(3, 4, 5))),
                          FieldsAre(ElementsAre(6, 7, 8, 9, 10, 11),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(3, 4, 5)))));
}

TEST(MeshPackingTest, PartitionTrianglesTwoPartitionsRepeatedVertices) {
  std::vector<std::byte> triangle_bytes = AsByteVector<uint32_t>({0, 1, 2,  //
                                                                  1, 3, 2,  //
                                                                  2, 3, 4,  //
                                                                  3, 5, 4,  //
                                                                  4, 5, 6,  //
                                                                  5, 7, 6,  //
                                                                  5, 8, 7,  //
                                                                  5, 9, 8});

  absl::InlinedVector<PartitionInfo, 1> partitions = PartitionTriangles(
      triangle_bytes, MeshFormat::IndexFormat::k32BitUnpacked16BitPacked, 6);

  EXPECT_THAT(partitions,
              ElementsAre(FieldsAre(ElementsAre(0, 1, 2, 3, 4, 5),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(1, 3, 2),  //
                                                ElementsAre(2, 3, 4),  //
                                                ElementsAre(3, 5, 4))),
                          FieldsAre(ElementsAre(4, 5, 6, 7, 8, 9),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(1, 3, 2),  //
                                                ElementsAre(1, 4, 3),  //
                                                ElementsAre(1, 5, 4)))));
}

TEST(MeshPackingTest, PartitionTriangles16BitIndex) {
  std::vector<std::byte> triangle_bytes = AsByteVector<uint16_t>({0, 1, 2,  //
                                                                  1, 3, 2,  //
                                                                  2, 3, 4,  //
                                                                  3, 5, 4,  //
                                                                  4, 5, 6,  //
                                                                  5, 7, 6,  //
                                                                  5, 8, 7,  //
                                                                  5, 9, 8});

  absl::InlinedVector<PartitionInfo, 1> partitions = PartitionTriangles(
      triangle_bytes, MeshFormat::IndexFormat::k16BitUnpacked16BitPacked, 6);

  EXPECT_THAT(partitions,
              ElementsAre(FieldsAre(ElementsAre(0, 1, 2, 3, 4, 5),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(1, 3, 2),  //
                                                ElementsAre(2, 3, 4),  //
                                                ElementsAre(3, 5, 4))),
                          FieldsAre(ElementsAre(4, 5, 6, 7, 8, 9),
                                    ElementsAre(ElementsAre(0, 1, 2),  //
                                                ElementsAre(1, 3, 2),  //
                                                ElementsAre(1, 4, 3),  //
                                                ElementsAre(1, 5, 4)))));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat1Unpacked) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat1Unpacked, {.minimum = {1, 0}, .maximum = {1, 1}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat1PackedInOneUnsignedByte) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat1PackedInOneUnsignedByte,
                          {.minimum = {2}, .maximum = {10}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params, MeshAttributeCodingParamsEq(
                                  {{{.offset = 2, .scale = 8.f / kMax8Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat2Unpacked) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat2Unpacked, {.minimum = {2, 0}, .maximum = {2, 1}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 0, .scale = 1}, {.offset = 0, .scale = 1}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat2PackedInOneFloat) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {0, 1}, .maximum = {5, 10}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params, MeshAttributeCodingParamsEq(
                                  {{{.offset = 0, .scale = 5.f / kMax12Bit},
                                    {.offset = 1, .scale = 9.f / kMax12Bit}}}));
}

TEST(MeshPackingTest,
     ComputeCodingParamskFloat2PackedInThreeUnsignedBytes_XY12) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat2PackedInThreeUnsignedBytes_XY12,
                          {.minimum = {0, 1}, .maximum = {5, 10}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params, MeshAttributeCodingParamsEq(
                                  {{{.offset = 0, .scale = 5.f / kMax12Bit},
                                    {.offset = 1, .scale = 9.f / kMax12Bit}}}));
}

TEST(MeshPackingTest,
     ComputeCodingParamskFloat2PackedInFourUnsignedBytes_X12_Y20) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
                          {.minimum = {2, 5}, .maximum = {10, 21}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 2, .scale = 8.f / kMax12Bit},
                    {.offset = 5, .scale = 16.f / kMax20Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat3Unpacked) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat3Unpacked, {.minimum = {3, 0, 5}, .maximum = {3, 1, 7}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1},
                                            {.offset = 0, .scale = 1},
                                            {.offset = 0, .scale = 1}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat3PackedInOneFloat) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat3PackedInOneFloat,
                          {.minimum = {4, 5, 6}, .maximum = {5, 10, 15}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params, MeshAttributeCodingParamsEq(
                                  {{{.offset = 4, .scale = 1.f / kMax8Bit},
                                    {.offset = 5, .scale = 5.f / kMax8Bit},
                                    {.offset = 6, .scale = 9.f / kMax8Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat3PackedInTwoFloats) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat3PackedInTwoFloats,
                          {.minimum = {-4, -8, -4}, .maximum = {20, 30, 10}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = -4, .scale = 24.f / kMax16Bit},
                    {.offset = -8, .scale = 38.f / kMax16Bit},
                    {.offset = -4, .scale = 14.f / kMax16Bit}}}));
}

TEST(MeshPackingTest,
     ComputeCodingParamskFloat3PackedInFourUnsignedBytes_XYZ10) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat3PackedInFourUnsignedBytes_XYZ10,
                          {.minimum = {12, -21, -13}, .maximum = {28, 4, 37}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 12, .scale = 16.f / kMax10Bit},
                    {.offset = -21, .scale = 25.f / kMax10Bit},
                    {.offset = -13, .scale = 50.f / kMax10Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat4Unpacked) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat4Unpacked,
      {.minimum = {4, 0, 2, 5}, .maximum = {4, 20, 50, 10}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1},
                                            {.offset = 0, .scale = 1},
                                            {.offset = 0, .scale = 1},
                                            {.offset = 0, .scale = 1}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat4PackedInOneFloat) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat4PackedInOneFloat,
                          {.minimum = {-1, 1, -3, 3}, .maximum = {1, 2, 3, 5}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params, MeshAttributeCodingParamsEq(
                                  {{{.offset = -1, .scale = 2.f / kMax6Bit},
                                    {.offset = 1, .scale = 1.f / kMax6Bit},
                                    {.offset = -3, .scale = 6.f / kMax6Bit},
                                    {.offset = 3, .scale = 2.f / kMax6Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat4PackedInTwoFloats) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat4PackedInTwoFloats,
      {.minimum = {100, 200, 300, 400}, .maximum = {900, 400, 310, 500}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 100, .scale = 800.f / kMax12Bit},
                    {.offset = 200, .scale = 200.f / kMax12Bit},
                    {.offset = 300, .scale = 10.f / kMax12Bit},
                    {.offset = 400, .scale = 100.f / kMax12Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamskFloat4PackedInThreeFloats) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat4PackedInThreeFloats,
      {.minimum = {0.1, -0.5, 1.3, -2.9}, .maximum = {0.2, -0.1, 1.5, -2.7}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 0.1, .scale = 0.1f / kMax18Bit},
                    {.offset = -0.5, .scale = 0.4f / kMax18Bit},
                    {.offset = 1.3, .scale = 0.2f / kMax18Bit},
                    {.offset = -2.9, .scale = 0.2f / kMax18Bit}}}));
}

TEST(MeshPackingTest, ComputeCodingParamsHandlesMinAndMaxBeingTheSame) {
  absl::StatusOr<MeshAttributeCodingParams> coding_params =
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {10, 20}, .maximum = {10, 20}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  EXPECT_THAT(*coding_params,
              MeshAttributeCodingParamsEq(
                  {{{.offset = 10, .scale = 1}, {.offset = 20, .scale = 1}}}));
}

TEST(MeshPackingTest, ComputeCodingParamsRangeIsLargerThanFloatMax) {
  absl::Status status =
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {10, -3e38}, .maximum = {10, 3e38}})
          .status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("exceeds float precision"));
}

TEST(MeshPackingTest, ComputeCodingParamsArray) {
  absl::StatusOr<CodingParamsArray> coding_params_array =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-50, 5}, .maximum = {100, 10}},
           {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}});
  ASSERT_EQ(coding_params_array.status(), absl::OkStatus());
  EXPECT_THAT(
      *coding_params_array,
      Property("Values", &CodingParamsArray::Values,
               ElementsAre(
                   MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1}}}),
                   MeshAttributeCodingParamsEq(
                       {{{.offset = -50, .scale = 150. / kMax12Bit},
                         {.offset = 5, .scale = 5. / kMax12Bit}}}),
                   MeshAttributeCodingParamsEq(
                       {{{.offset = -1, .scale = 3. / kMax6Bit},
                         {.offset = 0, .scale = 1. / kMax6Bit},
                         {.offset = 0.5, .scale = 0.25 / kMax6Bit},
                         {.offset = 0.25, .scale = 0.55 / kMax6Bit}}}))));
}

TEST(MeshPackingTest, ComputeCodingParamsArrayWithCustomParams) {
  absl::StatusOr<CodingParamsArray> coding_params_array =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-50, 5}, .maximum = {100, 10}},
           {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
          {std::nullopt,
           MeshAttributeCodingParams{
               {{.offset = -200, .scale = .1}, {.offset = 0, .scale = .01}}},
           MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1}}}});
  ASSERT_EQ(coding_params_array.status(), absl::OkStatus());
  EXPECT_THAT(
      *coding_params_array,
      Property(
          "Values", &CodingParamsArray::Values,
          ElementsAre(
              MeshAttributeCodingParamsEq({{{.offset = 0, .scale = 1}}}),
              MeshAttributeCodingParamsEq({{{.offset = -200, .scale = .1},
                                            {.offset = 0, .scale = .01}}}),
              MeshAttributeCodingParamsEq({{{.offset = -1, .scale = .1},
                                            {.offset = -1, .scale = .1},
                                            {.offset = -1, .scale = .1},
                                            {.offset = -1, .scale = .1}}}))));
}

TEST(MeshPackingTest, ComputeCodingParamsArrayWrongNumberOfBounds) {
  absl::Status size_mismatch =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-50, 5}, .maximum = {100, 10}}})
          .status();
  EXPECT_EQ(size_mismatch.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(size_mismatch.message(), HasSubstr("Size mismatch"));
}

TEST(MeshPackingTest, ComputeCodingParamsArrayWrongNumberOfCustomParams) {
  absl::Status wrong_num_params =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-50, 5}, .maximum = {100, 10}},
           {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
          {std::nullopt,
           MeshAttributeCodingParams{
               {{.offset = -200, .scale = .1}, {.offset = 0, .scale = .01}}}})
          .status();
  EXPECT_EQ(wrong_num_params.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(wrong_num_params.message(),
              HasSubstr("Wrong number of coding params"));
}

TEST(MeshPackingTest,
     ComputeCodingParamsArrayCustomParamsForUnpackedAttribute) {
  absl::Status status =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-50, 5}, .maximum = {100, 10}},
           {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
          {MeshAttributeCodingParams{{{.offset = 1, .scale = 2}}},
           MeshAttributeCodingParams{
               {{.offset = -200, .scale = .1}, {.offset = 0, .scale = .01}}},
           MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1},
                                      {.offset = -1, .scale = .1}}}})
          .status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("but the attribute is unpacked"));
}

TEST(MeshPackingTest, ComputeCodingParamsArrayCustomParamsIsInvalid) {
  {
    absl::Status wrong_num_components =
        ComputeCodingParamsArray(
            *MeshFormat::Create(
                {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
                 {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                 {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
                MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            {{.minimum = {-3}, .maximum = {500}},
             {.minimum = {-50, 5}, .maximum = {100, 10}},
             {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
            {std::nullopt,
             MeshAttributeCodingParams{
                 {{.offset = -200, .scale = .1}, {.offset = 0, .scale = .01}}},
             // Wrong number of components.
             MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1}}}})
            .status();
    EXPECT_EQ(wrong_num_components.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(wrong_num_components.message(),
                HasSubstr("is not valid for format"));
  }
  {
    absl::Status non_finite_value =
        ComputeCodingParamsArray(
            *MeshFormat::Create(
                {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
                 {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                 {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
                MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            {{.minimum = {-3}, .maximum = {500}},
             {.minimum = {-50, 5}, .maximum = {100, 10}},
             {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
            {std::nullopt,
             // Non-finite value.
             MeshAttributeCodingParams{{{.offset = std::nanf(""), .scale = .1},
                                        {.offset = 0, .scale = .01}}},
             MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1}}}})
            .status();
    EXPECT_EQ(non_finite_value.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(non_finite_value.message(),
                HasSubstr("is not valid for format"));
  }
}

TEST(MeshPackingTest,
     ComputeCodingParamsArrayCustomParamsCannotRepresentValues) {
  {
    absl::Status range_starts_too_high =
        ComputeCodingParamsArray(
            *MeshFormat::Create(
                {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
                 {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                 {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
                MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            {{.minimum = {-3}, .maximum = {500}},
             {.minimum = {-50, 5}, .maximum = {100, 10}},
             {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
            {std::nullopt,
             // This can't represent the minimum value.
             MeshAttributeCodingParams{
                 {{.offset = -20, .scale = .1}, {.offset = 0, .scale = .01}}},
             MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1}}}})
            .status();
    EXPECT_EQ(range_starts_too_high.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(range_starts_too_high.message(),
                HasSubstr("cannot represent all values of its attribute"));
  }
  {
    absl::Status range_ends_too_low =
        ComputeCodingParamsArray(
            *MeshFormat::Create(
                {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
                 {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
                 {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
                MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
            {{.minimum = {-3}, .maximum = {500}},
             {.minimum = {-50, 5}, .maximum = {100, 10}},
             {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}},
            {std::nullopt,
             MeshAttributeCodingParams{
                 {{.offset = -200, .scale = .1}, {.offset = 0, .scale = .01}}},
             // This can't represent the maximum value.
             MeshAttributeCodingParams{{{.offset = -1, .scale = .1},
                                        {.offset = 0, .scale = .01},
                                        {.offset = -1, .scale = .1},
                                        {.offset = -1, .scale = .1}}}})
            .status();
    EXPECT_EQ(range_ends_too_low.code(), absl::StatusCode::kInvalidArgument);
    EXPECT_THAT(range_ends_too_low.message(),
                HasSubstr("cannot represent all values of its attribute"));
  }
}

TEST(MeshPackingTest, ComputeCodingParamsArrayPercolatesErrors) {
  absl::Status exceeds_precision =
      ComputeCodingParamsArray(
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked),
          {{.minimum = {-3}, .maximum = {500}},
           {.minimum = {-3e38, 5}, .maximum = {3e38, 10}},
           {.minimum = {-1, 0, 0.5, 0.25}, .maximum = {2, 1, 0.75, 0.8}}})
          .status();
  EXPECT_EQ(exceeds_precision.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(exceeds_precision.message(),
              HasSubstr("exceeds float precision"));
}

TEST(MeshPackingTest, CopyAndPackPartitionVerticesDefaultFormat) {
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({-4, 5,   //
                                                      0, 10,   //
                                                      4, 15,   //
                                                      8, 20,   //
                                                      12, 25,  //
                                                      16, 30});
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat2Unpacked, {.minimum = {-4, 5}, .maximum = {16, 30}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *coding_params;

  EXPECT_THAT(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3, 5}, format, {},
                                   unpacking_params_array, {}),
      ElementsAreArray(AsByteVector<float>({-4, 5, 0, 10, 8, 20, 16, 30})));
}

TEST(MeshPackingTest, CopyAndPackPartitionVerticesCustomFormat) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  std::vector<std::byte> bytes =
      AsByteVector<float>({30,                 // custom
                           0,   10,            // position
                           0,   0.5,  1, 1,    // color
                           -10,                // custom
                           50,  100,           // position
                           0.5, 1,    0, 1,    // color
                           60,                 // custom
                           -50, -50,           // position
                           1,   0,    0, 1,    // color
                           -20,                // custom
                           200, -200,          // position
                           0,   0,    0, 0});  // color
  CodingParamsArray unpacking_params_array(3);
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat1Unpacked,
                            {.minimum = {-20}, .maximum = {60}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[0] = *unpacking_params;
  }
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                            {.minimum = {-50, -200}, .maximum = {200, 100}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[1] = *unpacking_params;
  }
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat4PackedInOneFloat,
                            {.minimum = {0, 0, 0, 0}, .maximum = {1, 1, 1, 1}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[2] = *unpacking_params;
  }

  EXPECT_THAT(CopyAndPackPartitionVertices(bytes, {0, 1, 3}, *format, {},
                                           unpacking_params_array, {}),
              ElementsAreArray(AsByteVector<float>({30,        // custom
                                                    3357491,   // position
                                                    131071,    // color
                                                    -10,       // custom
                                                    6713343,   // position
                                                    8384575,   // color
                                                    -20,       // custom
                                                    16773120,  // position
                                                    0})));     // color
}

TEST(MeshPackingTest,
     CopyAndPackPartitionVerticesDefaultFormatWithCorrectedPositions) {
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({-4, 5,   //
                                                      0, 10,   //
                                                      4, 15,   //
                                                      8, 20,   //
                                                      12, 25,  //
                                                      16, 30});
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
      ComputeCodingParams(AttrType::kFloat2Unpacked,
                          {.minimum = {-4, 5}, .maximum = {16, 30}});
  ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *unpacking_params;
  absl::flat_hash_map<uint32_t, Point> corrected_positions = {{1, {100, 200}},
                                                              {3, {30, 50}}};

  EXPECT_THAT(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3, 5}, format, {},
                                   unpacking_params_array, corrected_positions),
      ElementsAreArray(AsByteVector<float>({-4, 5, 100, 200, 30, 50, 16, 30})));
}

TEST(MeshPackingTest,
     CopyAndPackPartitionVerticesCustomFormatWithCorrectedPositions) {
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
       {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
       {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  std::vector<std::byte> bytes =
      AsByteVector<float>({30,                 // custom
                           0,   10,            // position
                           0,   0.5,  1, 1,    // color
                           -10,                // custom
                           50,  100,           // position
                           0.5, 1,    0, 1,    // color
                           60,                 // custom
                           -50, -50,           // position
                           1,   0,    0, 1,    // color
                           -20,                // custom
                           200, -200,          // position
                           0,   0,    0, 0});  // color

  CodingParamsArray unpacking_params_array(3);
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat1Unpacked,
                            {.minimum = {-20}, .maximum = {60}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[0] = *unpacking_params;
  }
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                            {.minimum = {-50, -200}, .maximum = {200, 100}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[1] = *unpacking_params;
  }
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat4PackedInOneFloat,
                            {.minimum = {0, 0, 0, 0}, .maximum = {1, 1, 1, 1}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[2] = *unpacking_params;
  }

  absl::flat_hash_map<uint32_t, Point> corrected_positions = {{0, {100, 20}},
                                                              {3, {-10, 45}}};

  EXPECT_THAT(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3}, *format, {},
                                   unpacking_params_array, corrected_positions),
      ElementsAreArray(AsByteVector<float>({30,        // custom
                                            10066875,  // position
                                            131071,    // color
                                            -10,       // custom
                                            6713343,   // position
                                            8384575,   // color
                                            -20,       // custom
                                            2686224,   // position
                                            0})));     // color
}

TEST(MeshPackingDeathTest, WriteTriangleIndicesWrongNumberOfIndices) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(12);
  EXPECT_DEATH_IF_SUPPORTED(
      WriteTriangleIndicesToByteArray(0, 2, {0, 1}, bytes), "");
  EXPECT_DEATH_IF_SUPPORTED(
      WriteTriangleIndicesToByteArray(0, 4, {0, 1, 2, 3}, bytes), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     ReadOrWriteTriangleIndicesByteVectorSizeNotDivisibleByTriangleStride) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(19);
  EXPECT_DEATH_IF_SUPPORTED(
      WriteTriangleIndicesToByteArray(0, 2, {0, 1, 2}, bytes), "");
  EXPECT_DEATH_IF_SUPPORTED(ReadTriangleIndicesFromByteArray(0, 4, bytes), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, ReadOrWriteTriangleIndicesIndexOutOfBounds) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(24);
  EXPECT_DEATH_IF_SUPPORTED(
      WriteTriangleIndicesToByteArray(12, 2, {0, 1, 2}, bytes),
      "Triangle index out-of-bounds");
  EXPECT_DEATH_IF_SUPPORTED(ReadTriangleIndicesFromByteArray(6, 4, bytes),
                            "Triangle index out-of-bounds");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest, ReadOrWriteTriangleBadIndexStride) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(24);
  EXPECT_DEATH_IF_SUPPORTED(
      WriteTriangleIndicesToByteArray(0, 3, {0, 1, 2}, bytes), "");
  EXPECT_DEATH_IF_SUPPORTED(ReadTriangleIndicesFromByteArray(0, 3, bytes), "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     ReadUnpackedFloatAttributeFromByteArraySizeNotDivisibleByStride) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(37);

  // The default format has an unpacked vertex stride of 8 bytes (2 floats).
  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(0, 0, bytes, MeshFormat()), "");
  // This custom format has an unpacked vertex stride of 24 bytes (6 floats).
  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(
          0, 0, bytes,
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     ReadUnpackedFloatAttributeFromByteArrayVertexIndexOutOfBounds) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  // This would hold 6 vertices of the default format, or 2 of this custom
  // format.
  std::vector<std::byte> bytes(48);

  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(6, 0, bytes, MeshFormat()), "");
  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(
          2, 0, bytes,
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     ReadUnpackedFloatAttributeFromByteArrayAttributeIndexOutOfBounds) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(48);

  // The default format has only one attribute.
  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(0, 1, bytes, MeshFormat()), "");
  // This custom format has three attributes.
  EXPECT_DEATH_IF_SUPPORTED(
      ReadUnpackedFloatAttributeFromByteArray(
          0, 3, bytes,
          *MeshFormat::Create(
              {{AttrType::kFloat1Unpacked, AttrId::kCustom0},
               {AttrType::kFloat2PackedInOneFloat, AttrId::kPosition},
               {AttrType::kFloat4PackedInOneFloat, AttrId::kColorShiftHsl}},
              MeshFormat::IndexFormat::k16BitUnpacked16BitPacked)),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     PartitionTrianglesByteVectorSizeNotDivisibleByTriangleStride) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  std::vector<std::byte> bytes(19);
  EXPECT_DEATH_IF_SUPPORTED(
      PartitionTriangles(
          bytes, MeshFormat::IndexFormat::k32BitUnpacked16BitPacked, 100),
      "");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingTestDeathTest, ComputeCodingParamsMinOrMaxIsWrongSize) {
  EXPECT_DEATH_IF_SUPPORTED(
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {1}, .maximum = {1, 1}})
          .IgnoreError(),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {1, 1}, .maximum = {1, 1, 1}})
          .IgnoreError(),
      "");
}

TEST(MeshPackingTestDeathTest, ComputeCodingParamsMaxIsLessThanMin) {
  EXPECT_DEATH_IF_SUPPORTED(
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {2, 3}, .maximum = {1, 4}})
          .IgnoreError(),
      "");
  EXPECT_DEATH_IF_SUPPORTED(
      ComputeCodingParams(AttrType::kFloat2PackedInOneFloat,
                          {.minimum = {1, 1}, .maximum = {2, 0}})
          .IgnoreError(),
      "");
}

TEST(MeshPackingDeathTest,
     CopyAndPackPartitionVerticesUnpackedVerticesDataIsEmpty) {
  MeshFormat format;
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
      ComputeCodingParams(AttrType::kFloat2Unpacked,
                          {.minimum = {0, 0}, .maximum = {1, 1}});
  ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *unpacking_params;

  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices({}, {0, 1, 3}, format, {},
                                   unpacking_params_array, {}),
      "Vertex data is empty");
}

TEST(MeshPackingDeathTest, CopyAndPackPartitionVerticesPartitionIsEmpty) {
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({0, 1,  //
                                                      2, 3,  //
                                                      4, 5,  //
                                                      6, 7});
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
      ComputeCodingParams(AttrType::kFloat2Unpacked,
                          {.minimum = {0, 0}, .maximum = {1, 1}});
  ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *unpacking_params;

  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices(bytes, {}, format, {},
                                   unpacking_params_array, {}),
      "Partition is empty");
}

TEST(MeshPackingDeathTest,
     CopyAndPackPartitionVerticesByteVectorSizeNotDivisibleByVertexStride) {
  MeshFormat format;
  std::vector<std::byte> bytes(31);
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
      ComputeCodingParams(AttrType::kFloat2Unpacked,
                          {.minimum = {0, 0}, .maximum = {1, 1}});
  ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *unpacking_params;

  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3}, format, {},
                                   unpacking_params_array, {}),
      "not divisible");
}

TEST(MeshPackingDeathTest,
     CopyAndPackPartitionVerticesWrongNumberOfPackingParams) {
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({0, 1,  //
                                                      2, 3,  //
                                                      4, 5,  //
                                                      6, 7});
  CodingParamsArray unpacking_params_array(2);
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat2Unpacked,
                            {.minimum = {0, 0}, .maximum = {1, 1}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[0] = *unpacking_params;
  }
  {
    absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
        ComputeCodingParams(AttrType::kFloat2Unpacked,
                            {.minimum = {0, 0}, .maximum = {1, 1}});
    ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
    unpacking_params_array[1] = *unpacking_params;
  }
  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3}, format, {},
                                   unpacking_params_array, {}),
      "Wrong number of packing params");
}

TEST(MeshPackingDeathTest,
     CopyAndPackPartitionVerticesWrongNumberOfUnpackingParamsComponents) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  // We use a custom format because the default format uses kFloat2Unpacked, and
  // unpacked attributes ignore the content of the unpacking params.
  absl::StatusOr<MeshFormat> format = MeshFormat::Create(
      {{AttrType::kFloat2PackedInOneFloat, AttrId::kPosition}},
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked);
  ASSERT_EQ(format.status(), absl::OkStatus());
  std::vector<std::byte> bytes = AsByteVector<float>({0, 1,  //
                                                      2, 3,  //
                                                      4, 5,  //
                                                      6, 7});
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> unpacking_params =
      ComputeCodingParams(AttrType::kFloat3PackedInOneFloat,
                          {.minimum = {0, 0, 0}, .maximum = {1, 1, 1}});
  ASSERT_EQ(unpacking_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *unpacking_params;

  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices(bytes, {0, 1, 3}, *format, {},
                                   unpacking_params_array, {}),
      "Invalid packing params");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

TEST(MeshPackingDeathTest,
     CopyAndPackPartitionVerticesPartitionRefersToNonExistentVertex) {
// There is no EXPECT_DEBUG_DEATH_IF_SUPPORTED, so we only run these when
// compiled in debug mode.
#ifndef NDEBUG
  MeshFormat format;
  std::vector<std::byte> bytes = AsByteVector<float>({0, 1,  //
                                                      2, 3,  //
                                                      4, 5,  //
                                                      6, 7});
  CodingParamsArray unpacking_params_array(1);
  absl::StatusOr<MeshAttributeCodingParams> coding_params = ComputeCodingParams(
      AttrType::kFloat2Unpacked, {.minimum = {0, 0}, .maximum = {1, 1}});
  ASSERT_EQ(coding_params.status(), absl::OkStatus());
  unpacking_params_array[0] = *coding_params;

  EXPECT_DEATH_IF_SUPPORTED(
      CopyAndPackPartitionVertices(bytes, {0, 1, 4}, format, {},
                                   unpacking_params_array, {}),
      "non-existent vertex");
#else
  GTEST_SKIP() << "This tests behavior that is disabled in opt builds.";
#endif
}

}  // namespace
}  // namespace ink::mesh_internal
