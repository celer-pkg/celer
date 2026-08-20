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

#include "ink/geometry/type_matchers.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/envelope.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/quad.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"
#include "ink/types/small_array.h"

namespace ink {
namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::ExplainMatchResult;
using ::testing::Field;
using ::testing::FloatEq;
using ::testing::FloatNear;
using ::testing::Matcher;
using ::testing::Matches;
using ::testing::NanSensitiveFloatEq;
using ::testing::Optional;
using ::testing::Pointwise;
using ::testing::PrintToString;
using ::testing::Property;

MATCHER_P(AffineTransformEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " AffineTransform (expected: ", PrintToString(expected),
                       ")")) {
  return ExplainMatchResult(
      AllOf(Property("a", &AffineTransform::A, FloatEq(expected.A())),
            Property("b", &AffineTransform::B, FloatEq(expected.B())),
            Property("c", &AffineTransform::C, FloatEq(expected.C())),
            Property("d", &AffineTransform::D, FloatEq(expected.D())),
            Property("e", &AffineTransform::E, FloatEq(expected.E())),
            Property("f", &AffineTransform::F, FloatEq(expected.F()))),
      arg, result_listener);
}

MATCHER_P2(AffineTransformNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " AffineTransform (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(AllOf(Property("a", &AffineTransform::A,
                                           FloatNear(expected.A(), tolerance)),
                                  Property("b", &AffineTransform::B,
                                           FloatNear(expected.B(), tolerance)),
                                  Property("c", &AffineTransform::C,
                                           FloatNear(expected.C(), tolerance)),
                                  Property("d", &AffineTransform::D,
                                           FloatNear(expected.D(), tolerance)),
                                  Property("e", &AffineTransform::E,
                                           FloatNear(expected.E(), tolerance)),
                                  Property("f", &AffineTransform::F,
                                           FloatNear(expected.F(), tolerance))),
                            arg, result_listener);
}

MATCHER_P(AngleEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Angle (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(Property("ValueInRadians", &Angle::ValueInRadians,
                                     FloatEq(expected.ValueInRadians())),
                            arg, result_listener);
}

MATCHER_P2(AngleNearMatcher, expected, tolerance_radians,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Angle (expected: ", expected,
                        ", tolerance: ", tolerance_radians, " radians)")) {
  return ExplainMatchResult(
      Property("ValueInRadians", &Angle::ValueInRadians,
               FloatNear(expected.ValueInRadians(), tolerance_radians)),
      arg, result_listener);
}

MATCHER_P2(NormalizedAngleNearMatcher, expected, tolerance_radians,
           absl::StrCat(negation ? "isn't" : "is",
                        " approximately equivalent to ", expected,
                        " mod 2π (tolerance: ", tolerance_radians,
                        " radians)")) {
  return ExplainMatchResult(
      Property("ValueInRadians", &Angle::ValueInRadians,
               FloatNear(0.f, tolerance_radians)),
      (arg.Normalized() - expected.Normalized()).NormalizedAboutZero(),
      result_listener);
}

MATCHER(IsNanAngleMatcher, absl::StrCat(negation ? "isn't" : "is", " NaN")) {
  return std::isnan(arg.ValueInRadians());
}

MATCHER_P(SegmentEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Segment (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("start ", &Segment::start, PointEq(expected.start)),
            Field("end ", &Segment::end, PointEq(expected.end))),
      arg, result_listener);
}

MATCHER_P2(SegmentNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Segment (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Field("start ", &Segment::start,
                  PointNear(expected.start, tolerance)),
            Field("end ", &Segment::end, PointNear(expected.end, tolerance))),
      arg, result_listener);
}

MATCHER_P(PointEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Point (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(AllOf(Field("x", &Point::x, FloatEq(expected.x)),
                                  Field("y", &Point::y, FloatEq(expected.y))),
                            arg, result_listener);
}

MATCHER_P2(PointNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Point (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Field("x", &Point::x, FloatNear(expected.x, tolerance)),
            Field("y", &Point::y, FloatNear(expected.y, tolerance))),
      arg, result_listener);
}

MATCHER_P(NanSensitivePointEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Point (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("x", &Point::x, NanSensitiveFloatEq(expected.x)),
            Field("y", &Point::y, NanSensitiveFloatEq(expected.y))),
      arg, result_listener);
}

MATCHER(IsFinitePointMatcher,
        absl::StrCat(negation ? "isn't" : "is", " finite")) {
  return std::isfinite(arg.x) && std::isfinite(arg.y);
}

MATCHER_P(QuadEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Quad (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Property("center", &Quad::Center, PointEq(expected.Center())),
            Property("width", &Quad::Width, FloatEq(expected.Width())),
            Property("height", &Quad::Height, FloatEq(expected.Height())),
            Property("rotation", &Quad::Rotation, AngleEq(expected.Rotation())),
            Property("shear factor", &Quad::ShearFactor,
                     FloatEq(expected.ShearFactor()))),
      arg, result_listener);
}

MATCHER_P2(QuadNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Quad (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Property("center", &Quad::Center,
                     PointNear(expected.Center(), tolerance)),
            Property("width", &Quad::Width,
                     FloatNear(expected.Width(), tolerance)),
            Property("height", &Quad::Height,
                     FloatNear(expected.Height(), tolerance)),
            Property("rotation", &Quad::Rotation,
                     AngleNear(expected.Rotation(), tolerance)),
            Property("shear factor", &Quad::ShearFactor,
                     FloatNear(expected.ShearFactor(), tolerance))),
      arg, result_listener);
}

MATCHER_P4(RectEqMatcher, x_min, y_min, x_max, y_max,
           absl::StrCat(negation ? "doesn't equal" : "equals",
                        " Rect (expected: (", x_min, ", ", y_min, ", ", x_max,
                        ", ", y_max, ")")) {
  return ExplainMatchResult(
      AllOf(Property("x_min", &Rect::XMin, FloatEq(x_min)),
            Property("y_min", &Rect::YMin, FloatEq(y_min)),
            Property("x_max", &Rect::XMax, FloatEq(x_max)),
            Property("y_max", &Rect::YMax, FloatEq(y_max))),
      arg, result_listener);
}

MATCHER_P(RectEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Rect (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Property("x_min", &Rect::XMin, FloatEq(expected.XMin())),
            Property("y_min", &Rect::YMin, FloatEq(expected.YMin())),
            Property("x_max", &Rect::XMax, FloatEq(expected.XMax())),
            Property("y_max", &Rect::YMax, FloatEq(expected.YMax()))),
      arg, result_listener);
}

MATCHER_P5(RectNearMatcher, x_min, y_min, x_max, y_max, tolerance,
           absl::StrCat(negation ? "doesn't equal" : "equals",
                        " Rect (expected: (", x_min, ", ", y_min, ", ", x_max,
                        ", ", y_max, "), tolerance: ", tolerance, ")")) {
  return ExplainMatchResult(
      AllOf(Property("x_min", &Rect::XMin, FloatNear(x_min, tolerance)),
            Property("y_min", &Rect::YMin, FloatNear(y_min, tolerance)),
            Property("x_max", &Rect::XMax, FloatNear(x_max, tolerance)),
            Property("y_max", &Rect::YMax, FloatNear(y_max, tolerance))),
      arg, result_listener);
}

MATCHER_P2(RectNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Rect (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(
          Property("x_min", &Rect::XMin, FloatNear(expected.XMin(), tolerance)),
          Property("y_min", &Rect::YMin, FloatNear(expected.YMin(), tolerance)),
          Property("x_max", &Rect::XMax, FloatNear(expected.XMax(), tolerance)),
          Property("y_max", &Rect::YMax,
                   FloatNear(expected.YMax(), tolerance))),
      arg, result_listener);
}

MATCHER_P(TriangleEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Triangle(expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(
      AllOf(Field("p0", &Triangle::p0, PointEq(expected.p0)),
            Field("p1", &Triangle::p1, PointEq(expected.p1)),
            Field("p2", &Triangle::p2, PointEq(expected.p2))),
      arg, result_listener);
}

MATCHER_P2(TriangleNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Triangle (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Field("p0", &Triangle::p0, PointNear(expected.p0, tolerance)),
            Field("p1", &Triangle::p1, PointNear(expected.p1, tolerance)),
            Field("p2", &Triangle::p2, PointNear(expected.p2, tolerance))),
      arg, result_listener);
}

MATCHER_P(VecEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " Vec (expected: ", PrintToString(expected), ")")) {
  return ExplainMatchResult(AllOf(Field("x", &Vec::x, FloatEq(expected.x)),
                                  Field("y", &Vec::y, FloatEq(expected.y))),
                            arg, result_listener);
}

MATCHER_P2(VecNearMatcher, expected, tolerance,
           absl::StrCat(negation ? "doesn't approximately equal"
                                 : "approximately equals",
                        " Vec (expected: ", PrintToString(expected),
                        ", tolerance: ", PrintToString(tolerance), ")")) {
  return ExplainMatchResult(
      AllOf(Field("x", &Vec::x, FloatNear(expected.x, tolerance)),
            Field("y", &Vec::y, FloatNear(expected.y, tolerance))),
      arg, result_listener);
}

MATCHER(MeshFormatAttributeTypeAndIdEqForPointwise, "") {
  return ExplainMatchResult(
      AllOf(Field("type", &MeshFormat::Attribute::type, std::get<1>(arg).type),
            Field("id", &MeshFormat::Attribute::id, std::get<1>(arg).id)),
      std::get<0>(arg), result_listener);
}

MATCHER_P(MeshFormatEqMatcher, expected,
          absl::StrCat(negation ? "does not equal" : "equals",
                       " MeshFormat (expected: ", PrintToString(expected),
                       ")")) {
  // We only need to check that position attribute index, index format, and
  // types and IDs of the attributes. Everything else is derived from those,
  // which is validated in mesh_format_test.
  return ExplainMatchResult(
      AllOf(Property("PositionAttributeIndex",
                     &MeshFormat::PositionAttributeIndex,
                     expected.PositionAttributeIndex()),
            Property("IndexFormat", &MeshFormat::GetIndexFormat,
                     expected.GetIndexFormat()),
            Property("Attributes", &MeshFormat::Attributes,
                     Pointwise(MeshFormatAttributeTypeAndIdEqForPointwise(),
                               expected.Attributes()))),
      arg, result_listener);
}

MATCHER_P(MeshAttributeCodingParamsEqMatcher, expected,
          absl::StrCat(negation ? "doesn't equal" : "equals",
                       " MeshAttributeCodingParams(expected: ",
                       PrintToString(expected), ")")) {
  if (arg.components.Size() != expected.components.Size()) {
    *result_listener << "MeshAttributeCodingParams has "
                     << arg.components.Size() << " components, expected "
                     << expected.components.Size();
    return false;
  }
  for (int i = 0; i < arg.components.Size(); ++i) {
    const auto& arg_comp = arg.components[i];
    const auto& expected_comp = expected.components[i];
    if (!Matches(FloatEq(expected_comp.offset))(arg_comp.offset) ||
        !Matches(FloatEq(expected_comp.scale))(arg_comp.scale)) {
      *result_listener << "ComponentCodingParams at index " << i
                       << " has offset = " << arg_comp.offset
                       << " and scale = " << arg_comp.scale
                       << ", expected offset = " << expected_comp.offset
                       << " and expected scale = " << expected_comp.scale;
      return false;
    }
  }
  return true;
}

// Because `Mesh` is non-copyable, we can't use the `MATCHER` family of macros
// to define this, and need to implement it as a class.
class MeshEqMatcher {
 public:
  using is_gtest_matcher = void;

  explicit MeshEqMatcher(const Mesh& expected) : expected_(expected) {}

  bool MatchAndExplain(const Mesh& mesh,
                       testing::MatchResultListener* listener) const {
    if (!ExplainMatchResult(
            Property("Format", &Mesh::Format, MeshFormatEq(expected_.Format())),
            mesh, listener)) {
      *listener << "MeshFormat differs";
      return false;
    }

    int n_attrs = expected_.Format().Attributes().size();
    for (int i = 0; i < n_attrs; ++i) {
      if (!ExplainMatchResult(MeshAttributeCodingParamsEq(
                                  expected_.VertexAttributeUnpackingParams(i)),
                              mesh.VertexAttributeUnpackingParams(i),
                              listener)) {
        *listener << absl::Substitute("Unpacking transform at index $0 differs",
                                      i);
        return false;
      }
    }

    return ExplainMatchResult(
        AllOf(Property("RawVertexData", &Mesh::RawVertexData,
                       expected_.RawVertexData()),
              Property("RawIndexData", &Mesh::RawIndexData,
                       expected_.RawIndexData())),
        mesh, listener);
  }

  void DescribeTo(std::ostream* os) const {
    *os << " equals Mesh (" << PrintToString(expected_) << ")";
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << " does not equal Mesh (" << PrintToString(expected_) << ")";
  }

 private:
  Mesh expected_;
};

MATCHER(VertexIndexPairEqForPointwise, "") {
  return ExplainMatchResult(VertexIndexPairEq(std::get<1>(arg)),
                            std::get<0>(arg), result_listener);
}

MATCHER_P2(PartitionedMeshEqMatcher, expected, deep,
           absl::StrCat(negation ? "doesn't equal" : "equals",
                        deep ? " deeply" : " shallowly",
                        " PartitionedMesh(expected: ", PrintToString(expected),
                        ")")) {
  if (arg.RenderGroupCount() != expected.RenderGroupCount()) {
    *result_listener << "PartitionedMesh has " << arg.RenderGroupCount()
                     << " render groups, expected "
                     << expected.RenderGroupCount();
    return false;
  }
  for (uint32_t group_index = 0; group_index < expected.RenderGroupCount();
       ++group_index) {
    if (arg.RenderGroupMeshes(group_index).size() !=
        expected.RenderGroupMeshes(group_index).size()) {
      *result_listener << "PartitionedMesh has "
                       << arg.RenderGroupMeshes(group_index).size()
                       << " meshes in render group " << group_index
                       << ", expected "
                       << expected.RenderGroupMeshes(group_index).size();
      return false;
    }
    if (arg.OutlineCount(group_index) != expected.OutlineCount(group_index)) {
      *result_listener << "PartitionedMesh has "
                       << arg.OutlineCount(group_index)
                       << " outlines in render group " << group_index
                       << ", expected " << expected.OutlineCount(group_index);
      return false;
    }
    for (size_t i = 0; i < arg.OutlineCount(group_index); ++i) {
      const auto& arg_outline = arg.Outline(group_index, i);
      const auto& expected_outline = expected.Outline(group_index, i);
      if (!ExplainMatchResult(
              Pointwise(VertexIndexPairEqForPointwise(), expected_outline),
              arg_outline, result_listener)) {
        *result_listener << "Outline at index " << i << " in render group "
                         << group_index << " differs";
        return false;
      }
    }
  }
  for (size_t i = 0; i < arg.Meshes().size(); ++i) {
    const Mesh& arg_mesh = arg.Meshes()[i];
    const Mesh& expected_mesh = expected.Meshes()[i];
    if (deep) {
      if (!ExplainMatchResult(MeshEq(expected_mesh), arg_mesh,
                              result_listener)) {
        *result_listener << "Mesh at index " << i << " differs deeply";
        return false;
      }
    } else {
      if (&arg_mesh != &expected_mesh) {
        *result_listener << "Mesh at index " << i
                         << " is not the same instance";
        return false;
      }
    }
  }
  return true;
}

}  // namespace

Matcher<AffineTransform> AffineTransformEq(const AffineTransform& expected) {
  return AffineTransformEqMatcher(expected);
}

Matcher<AffineTransform> AffineTransformNear(const AffineTransform& expected,
                                             float tolerance) {
  return AffineTransformNearMatcher(expected, tolerance);
}

Matcher<Angle> AngleEq(const Angle& expected) {
  return AngleEqMatcher(expected);
}

Matcher<Angle> AngleNear(const Angle& expected, float tolerance_radians) {
  return AngleNearMatcher(expected, tolerance_radians);
}

Matcher<Angle> NormalizedAngleNear(const Angle& expected,
                                   float tolerance_radians) {
  return NormalizedAngleNearMatcher(expected, tolerance_radians);
}

Matcher<Angle> IsNanAngle() { return IsNanAngleMatcher(); }

Matcher<Segment> SegmentEq(const Segment& expected) {
  return SegmentEqMatcher(expected);
}

Matcher<Segment> SegmentNear(const Segment& expected, float tolerance) {
  return SegmentNearMatcher(expected, tolerance);
}

Matcher<Point> PointEq(const Point& expected) {
  return PointEqMatcher(expected);
}

Matcher<Point> PointNear(const Point& expected, float tolerance) {
  return PointNearMatcher(expected, tolerance);
}

Matcher<Point> PointNear(const Point& expected, float x_tolerance,
                         float y_tolerance) {
  return AllOf(Field("x", &Point::x, FloatNear(expected.x, x_tolerance)),
               Field("y", &Point::y, FloatNear(expected.y, y_tolerance)));
}

Matcher<Point> NanSensitivePointEq(const Point& expected) {
  return NanSensitivePointEqMatcher(expected);
}

Matcher<Point> IsFinitePoint() { return IsFinitePointMatcher(); }

Matcher<Quad> QuadEq(const Quad& expected) { return QuadEqMatcher(expected); }

Matcher<Quad> QuadNear(const Quad& expected, float tolerance) {
  return QuadNearMatcher(expected, tolerance);
}

Matcher<Rect> RectEq(float x_min, float y_min, float x_max, float y_max) {
  return RectEqMatcher(x_min, y_min, x_max, y_max);
}

Matcher<Rect> RectEq(const Rect& expected) { return RectEqMatcher(expected); }

Matcher<Rect> RectNear(float x_min, float y_min, float x_max, float y_max,
                       float tolerance) {
  return RectNearMatcher(x_min, y_min, x_max, y_max, tolerance);
}

Matcher<Rect> RectNear(const Rect& expected, float tolerance) {
  return RectNearMatcher(expected, tolerance);
}

Matcher<Triangle> TriangleEq(const Triangle& expected) {
  return TriangleEqMatcher(expected);
}
Matcher<Triangle> TriangleNear(const Triangle& expected, float tolerance) {
  return TriangleNearMatcher(expected, tolerance);
}

Matcher<Vec> VecEq(const Vec& expected) { return VecEqMatcher(expected); }

Matcher<Vec> VecNear(const Vec& expected, float tolerance) {
  return VecNearMatcher(expected, tolerance);
}

Matcher<MeshFormat> MeshFormatEq(const MeshFormat& expected) {
  return MeshFormatEqMatcher(expected);
}

Matcher<MeshAttributeCodingParams> MeshAttributeCodingParamsEq(
    const MeshAttributeCodingParams& expected) {
  return MeshAttributeCodingParamsEqMatcher(expected);
}

Matcher<MeshAttributeBounds> MeshAttributeBoundsEq(
    const MeshAttributeBounds& expected) {
  return AllOf(
      Field("minimum", &MeshAttributeBounds::minimum,
            Property("Values", &SmallArray<float, 4>::Values,
                     Pointwise(FloatEq(), expected.minimum.Values()))),
      Field("maximum", &MeshAttributeBounds::maximum,
            Property("Values", &SmallArray<float, 4>::Values,
                     Pointwise(FloatEq(), expected.maximum.Values()))));
}

Matcher<MeshAttributeBounds> MeshAttributeBoundsNear(
    const MeshAttributeBounds& expected, float tolerance) {
  return AllOf(
      Field(
          "minimum", &MeshAttributeBounds::minimum,
          Property("Values", &SmallArray<float, 4>::Values,
                   Pointwise(FloatNear(tolerance), expected.minimum.Values()))),
      Field("maximum", &MeshAttributeBounds::maximum,
            Property(
                "Values", &SmallArray<float, 4>::Values,
                Pointwise(FloatNear(tolerance), expected.maximum.Values()))));
}

Matcher<Envelope> EnvelopeEq(const Envelope& expected) {
  if (expected.IsEmpty())
    return Property("AsRect", &Envelope::AsRect, Eq(std::nullopt));
  return EnvelopeEq(*expected.AsRect());
}

Matcher<Envelope> EnvelopeEq(const Rect& expected) {
  return Property("AsRect", &Envelope::AsRect, Optional(RectEq(expected)));
}

Matcher<Envelope> EnvelopeNear(const Rect& expected, float tolerance) {
  return Property("AsRect", &Envelope::AsRect,
                  Optional(RectNear(expected, tolerance)));
}

Matcher<Mesh> MeshEq(const Mesh& mesh) { return MeshEqMatcher(mesh); }

Matcher<PartitionedMesh::VertexIndexPair> VertexIndexPairEq(
    PartitionedMesh::VertexIndexPair expected) {
  return AllOf(
      Field("mesh_index", &PartitionedMesh::VertexIndexPair::mesh_index,
            expected.mesh_index),
      Field("vertex_index", &PartitionedMesh::VertexIndexPair::vertex_index,
            expected.vertex_index));
}

Matcher<PartitionedMesh> PartitionedMeshDeepEq(
    const PartitionedMesh& expected) {
  return PartitionedMeshEqMatcher(expected, true);
}

Matcher<PartitionedMesh> PartitionedMeshShallowEq(
    const PartitionedMesh& expected) {
  return PartitionedMeshEqMatcher(expected, false);
}

}  // namespace ink
