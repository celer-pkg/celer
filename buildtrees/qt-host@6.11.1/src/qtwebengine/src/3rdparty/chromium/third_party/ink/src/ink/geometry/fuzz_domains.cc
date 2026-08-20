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

#include "ink/geometry/fuzz_domains.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "fuzztest/fuzztest.h"
#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink {

namespace {

fuzztest::Domain<float> NotNanFloat() {
  return fuzztest::Filter([](float f) { return !std::isnan(f); },
                          fuzztest::Arbitrary<float>());
}

// LINT.IfChange(attribute_id)
fuzztest::Domain<MeshFormat::AttributeId> StandardAttributeId() {
  return fuzztest::ElementOf({
      MeshFormat::AttributeId::kPosition,
      MeshFormat::AttributeId::kColorShiftHsl,
      MeshFormat::AttributeId::kTexture,
      MeshFormat::AttributeId::kSideDerivative,
      MeshFormat::AttributeId::kSideLabel,
      MeshFormat::AttributeId::kForwardDerivative,
      MeshFormat::AttributeId::kForwardLabel,
      MeshFormat::AttributeId::kSurfaceUv,
      MeshFormat::AttributeId::kAnimationOffset,
  });
}

fuzztest::Domain<MeshFormat::AttributeId> CustomAttributeId() {
  return fuzztest::ElementOf({
      MeshFormat::AttributeId::kCustom0,
      MeshFormat::AttributeId::kCustom1,
      MeshFormat::AttributeId::kCustom2,
      MeshFormat::AttributeId::kCustom3,
      MeshFormat::AttributeId::kCustom4,
      MeshFormat::AttributeId::kCustom5,
      MeshFormat::AttributeId::kCustom6,
      MeshFormat::AttributeId::kCustom7,
      MeshFormat::AttributeId::kCustom8,
      MeshFormat::AttributeId::kCustom9,
  });
}
// LINT.ThenChange(mesh_format.h:attribute_id)

fuzztest::Domain<MeshFormat::AttributeId> ArbitraryAttributeId() {
  return fuzztest::OneOf(StandardAttributeId(), CustomAttributeId());
}

fuzztest::Domain<MeshFormat::AttributeId> NonPositionAttributeId() {
  return fuzztest::Filter(
      [](MeshFormat::AttributeId id) {
        return id != MeshFormat::AttributeId::kPosition;
      },
      ArbitraryAttributeId());
}

}  // namespace

fuzztest::Domain<Angle> FiniteAngle() {
  return fuzztest::Map([](float f) { return Angle::Radians(f); },
                       fuzztest::Finite<float>());
}

fuzztest::Domain<Angle> AngleInRange(Angle min, Angle max) {
  return fuzztest::Map(
      [](float radians) { return Angle::Radians(radians); },
      fuzztest::InRange(min.ValueInRadians(), max.ValueInRadians()));
}

fuzztest::Domain<Angle> NormalizedAngle() {
  // AngleInRange and fuzztest::InRange are inclusive on both ends of the range,
  // but NormalizedAngle needs an exclusive upper bound, so we use a Filter.
  return fuzztest::Filter([](Angle angle) { return angle < kFullTurn; },
                          AngleInRange(Angle(), kFullTurn));
}

// LINT.IfChange(attribute_types)
fuzztest::Domain<MeshFormat::AttributeType> ArbitraryMeshAttributeType() {
  return fuzztest::ElementOf({
      MeshFormat::AttributeType::kFloat1Unpacked,
      MeshFormat::AttributeType::kFloat1PackedInOneUnsignedByte,
      MeshFormat::AttributeType::kFloat2Unpacked,
      MeshFormat::AttributeType::kFloat2PackedInOneFloat,
      MeshFormat::AttributeType::kFloat2PackedInThreeUnsignedBytes_XY12,
      MeshFormat::AttributeType::kFloat2PackedInFourUnsignedBytes_X12_Y20,
      MeshFormat::AttributeType::kFloat3Unpacked,
      MeshFormat::AttributeType::kFloat3PackedInOneFloat,
      MeshFormat::AttributeType::kFloat3PackedInTwoFloats,
      MeshFormat::AttributeType::kFloat3PackedInFourUnsignedBytes_XYZ10,
      MeshFormat::AttributeType::kFloat4Unpacked,
      MeshFormat::AttributeType::kFloat4PackedInOneFloat,
      MeshFormat::AttributeType::kFloat4PackedInTwoFloats,
      MeshFormat::AttributeType::kFloat4PackedInThreeFloats,
  });
}
fuzztest::Domain<MeshFormat::AttributeType> PositionMeshAttributeType() {
  return fuzztest::ElementOf({
      MeshFormat::AttributeType::kFloat2Unpacked,
      MeshFormat::AttributeType::kFloat2PackedInOneFloat,
  });
}
// LINT.ThenChange(mesh_format.h:attribute_types)

fuzztest::Domain<MeshFormat> ArbitraryMeshFormat() {
  return fuzztest::FlatMap(
      [](uint8_t num_attributes) {
        uint8_t num_other_attributes = num_attributes - 1;
        return fuzztest::Map(
            [num_attributes](
                MeshFormat::IndexFormat index_format, uint8_t position_index,
                MeshFormat::AttributeType position_type,
                absl::Span<const MeshFormat::AttributeType> other_types,
                absl::Span<const MeshFormat::AttributeId> other_ids) {
              ABSL_CHECK_EQ(other_types.size(), other_ids.size());
              std::vector<
                  std::pair<MeshFormat::AttributeType, MeshFormat::AttributeId>>
                  attributes;
              attributes.reserve(num_attributes);
              for (int i = 0; i < position_index; ++i) {
                attributes.emplace_back(other_types[i], other_ids[i]);
              }
              attributes.emplace_back(position_type,
                                      MeshFormat::AttributeId::kPosition);
              for (int i = position_index;
                   i < static_cast<int>(other_types.size()); ++i) {
                attributes.emplace_back(other_types[i], other_ids[i]);
              }
              auto format = MeshFormat::Create(attributes, index_format);
              ABSL_CHECK_OK(format);
              return *std::move(format);
            },
            ArbitraryMeshIndexFormat(),
            fuzztest::InRange<uint8_t>(0, num_other_attributes),
            PositionMeshAttributeType(),
            fuzztest::VectorOf(ArbitraryMeshAttributeType())
                .WithSize(num_other_attributes),
            fuzztest::UniqueElementsVectorOf(NonPositionAttributeId())
                .WithSize(num_other_attributes));
      },
      // The number of attributes (including a kPosition attribute at minimum):
      fuzztest::InRange<uint8_t>(1, MeshFormat::MaxAttributes()));
}

// LINT.IfChange(index_formats)
fuzztest::Domain<MeshFormat::IndexFormat> ArbitraryMeshIndexFormat() {
  return fuzztest::ElementOf({
      MeshFormat::IndexFormat::k16BitUnpacked16BitPacked,
      MeshFormat::IndexFormat::k32BitUnpacked16BitPacked,
  });
}
// LINT.ThenChange()

fuzztest::Domain<Point> ArbitraryPoint() {
  return fuzztest::StructOf<Point>(fuzztest::Arbitrary<float>(),
                                   fuzztest::Arbitrary<float>());
}

fuzztest::Domain<Point> NotNanPoint() {
  return fuzztest::StructOf<Point>(NotNanFloat(), NotNanFloat());
}

fuzztest::Domain<Point> FinitePoint() {
  return fuzztest::StructOf<Point>(fuzztest::Finite<float>(),
                                   fuzztest::Finite<float>());
}

fuzztest::Domain<Point> PointInRect(Rect rect) {
  return fuzztest::StructOf<Point>(fuzztest::InRange(rect.XMin(), rect.XMax()),
                                   fuzztest::InRange(rect.YMin(), rect.YMax()));
}

fuzztest::Domain<Rect> NotNanRect() {
  return fuzztest::Map(
      [](Point p1, Point p2) { return Rect::FromTwoPoints(p1, p2); },
      NotNanPoint(), NotNanPoint());
}

fuzztest::Domain<Rect> FiniteRect() {
  return fuzztest::Map(
      [](Point p1, Point p2) { return Rect::FromTwoPoints(p1, p2); },
      FinitePoint(), FinitePoint());
}

fuzztest::Domain<Segment> FiniteSegment() {
  return fuzztest::StructOf<Segment>(FinitePoint(), FinitePoint());
}

fuzztest::Domain<Segment> SegmentInRect(Rect rect) {
  return fuzztest::StructOf<Segment>(PointInRect(rect), PointInRect(rect));
}

fuzztest::Domain<Triangle> TriangleInRect(Rect rect) {
  return fuzztest::StructOf<Triangle>(PointInRect(rect), PointInRect(rect),
                                      PointInRect(rect));
}

fuzztest::Domain<Vec> ArbitraryVec() {
  return fuzztest::StructOf<Vec>(fuzztest::Arbitrary<float>(),
                                 fuzztest::Arbitrary<float>());
}

fuzztest::Domain<Vec> NotNanVec() {
  return fuzztest::StructOf<Vec>(NotNanFloat(), NotNanFloat());
}

fuzztest::Domain<MutableMesh> ValidPackableNonEmptyPositionOnlyMutableMesh(
    MeshFormat::AttributeType position_attribute_type,
    MeshFormat::IndexFormat index_format) {
  ABSL_CHECK_EQ(MeshFormat::ComponentCount(position_attribute_type), 2);

  // We need at least three vertices. We restrict the values to [-1e18, 1e18] to
  // ensure that neither the dimensions of the mesh bounds nor the area of
  // individual triangles overflow
  auto positions_domain = fuzztest::VectorOf(PointInRect(Rect::FromTwoPoints(
                                                 {-1e18, -1e18}, {1e18, 1e18})))
                              .WithMinSize(3);

  auto positions_and_tris_domain = fuzztest::FlatMap(
      [](std::vector<Point> positions) {
        // Triangle indices must lie in [0, n_verts - 1], and contain no
        // repeated values.
        //
        // We generate the indices by sequentially choosing from the remaining
        // valid values, instead of filtering triples that contain duplicates,
        // because the filtering approach fails too often and the framework
        // complains.
        auto tri_indices_domain = fuzztest::Map(
            [](uint32_t a, uint32_t b, uint32_t c) {
              uint32_t i0 = a;

              // The next index can't be `i0`, so it must lie in either
              // [0, a - 1] or [a + 1, n_verts - 1].
              uint32_t i1 = b < a ? b : b + 1;

              // The final index can't be either `i0` or `i1`, so it must lie in
              // one of [0, min - 1], [min + 1, max - 1], or
              // [max + 1, n_verts - 1].
              auto [min, max] = std::minmax(i0, i1);
              uint32_t i2 = c < min ? c : c < max - 1 ? c + 1 : c + 2;
              return std::array<uint32_t, 3>{i0, i1, i2};
            },
            fuzztest::InRange<uint32_t>(0, positions.size() - 1),
            fuzztest::InRange<uint32_t>(0, positions.size() - 2),
            fuzztest::InRange<uint32_t>(0, positions.size() - 3));

        // Triangles must have positive area, but we can reverse them instead of
        // filtering them out.
        //
        // Note that we need to capture `positions` by value here (rather than
        // by reference) to avoid a stack-use-after-return error, since the
        // calls to this will occur after we leave this scope.  We can't capture
        // it by move either, because we need to use `positions` again below.
        auto tri_winding_correction_map = [positions](
                                              std::array<uint32_t, 3> tri) {
          Triangle t{positions[tri[0]], positions[tri[1]], positions[tri[2]]};
          if (t.SignedArea() >= 0) {
            return tri;
          } else {
            return std::array<uint32_t, 3>{tri[0], tri[2], tri[1]};
          }
        };

        return fuzztest::PairOf(
            fuzztest::Just(std::move(positions)),
            // We need at least one triangle.
            fuzztest::VectorOf(
                fuzztest::Map(std::move(tri_winding_correction_map),
                              std::move(tri_indices_domain)))
                .WithMinSize(1));
      },
      positions_domain);

  return fuzztest::Map(
      [position_attribute_type, index_format](
          std::pair<std::vector<Point>, std::vector<std::array<uint32_t, 3>>>
              pos_and_tri) {
        auto format = MeshFormat::Create(
            {{position_attribute_type, MeshFormat::AttributeId::kPosition}},
            index_format);
        ABSL_CHECK_OK(format);
        MutableMesh m(*format);
        for (auto p : pos_and_tri.first) m.AppendVertex(p);
        for (auto t : pos_and_tri.second) m.AppendTriangleIndices(t);
        return m;
      },
      std::move(positions_and_tris_domain));
}

}  // namespace ink
