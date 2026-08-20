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

#ifndef INK_GEOMETRY_TYPE_MATCHERS_H_
#define INK_GEOMETRY_TYPE_MATCHERS_H_

#include "gtest/gtest.h"
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

namespace ink {

// This matcher compares AffineTransforms, delegating to ::testing::FloatEq().
::testing::Matcher<AffineTransform> AffineTransformEq(
    const AffineTransform &expected);
::testing::Matcher<AffineTransform> AffineTransformNear(
    const AffineTransform &expected, float tolerance);

// These matchers compare Angles, delegating to ::testing::FloatEq() and
// ::testing::FloatNear(), respectively.
::testing::Matcher<Angle> AngleEq(const Angle &expected);
::testing::Matcher<Angle> AngleNear(const Angle &expected,
                                    float tolerance_radians);
// This matcher compares Angles mod 2π, treating equivalent angles as a match.
// It uses ::testing::FloatNear() to allow for some error, while accounting for
// the modulo seam (e.g. an Angle just above zero counts as being "near" an
// Angle just below 2π).
::testing::Matcher<Angle> NormalizedAngleNear(const Angle &expected,
                                              float tolerance_radians);
// Matches an Angle whose value (whether in radians or degrees) is NaN.
::testing::Matcher<Angle> IsNanAngle();

// This matcher compares Segments, delegating to ::testing::FloatEq().
::testing::Matcher<Segment> SegmentEq(const Segment &expected);
::testing::Matcher<Segment> SegmentNear(const Segment &expected,
                                        float tolerance);

// These matchers compare Points component-wise, delegating to
// ::testing::FloatEq(), ::testing::FloatNear(), and
// ::testing::NanSensitiveFloatEq(), respectively.
::testing::Matcher<Point> PointEq(const Point &expected);
::testing::Matcher<Point> PointNear(const Point &expected, float tolerance);
::testing::Matcher<Point> PointNear(const Point &expected, float x_tolerance,
                                    float y_tolerance);
::testing::Matcher<Point> NanSensitivePointEq(const Point &expected);
// Matches any Point whose x- and y-components are both finite.
::testing::Matcher<Point> IsFinitePoint();

// This matcher compares Quads component-wise, delegating to
// ::testing::FloatEq().
::testing::Matcher<Quad> QuadEq(const Quad &expected);
::testing::Matcher<Quad> QuadNear(const Quad &expected, float tolerance);

// These matchers compare Rects component-wise, delegating to
// ::testing::FloatEq() and ::testing::FloatNear(), respectively.
::testing::Matcher<Rect> RectEq(float x_min, float y_min, float x_max,
                                float y_max);
::testing::Matcher<Rect> RectEq(const Rect &expected);
::testing::Matcher<Rect> RectNear(float x_min, float y_min, float x_max,
                                  float y_max, float tolerance);
::testing::Matcher<Rect> RectNear(const Rect &expected, float tolerance);

// Return matchers that compare Triangles component-wise, delegating to
// PointEq() and PointNear(), respectively.
::testing::Matcher<Triangle> TriangleEq(const Triangle &expected);
::testing::Matcher<Triangle> TriangleNear(const Triangle &expected,
                                          float tolerance);

// These matchers compare Vecs component-wise, delegating to
// ::testing::FloatEq() and ::testing::FloatNear(), respectively.
::testing::Matcher<Vec> VecEq(const Vec &expected);
::testing::Matcher<Vec> VecNear(const Vec &expected, float tolerance);

// This matcher compares two `MeshFormat`s.
::testing::Matcher<MeshFormat> MeshFormatEq(const MeshFormat &expected);

// Returns a matcher that compares `MeshAttributeUnpackingTransform`s, deferring
// to `::testing::FloatEq`.
::testing::Matcher<MeshAttributeCodingParams> MeshAttributeCodingParamsEq(
    const MeshAttributeCodingParams &expected);

// Returns a matcher that compares `MeshAttributeBounds` component-wise,
// deferring to `::testing::FloatEq` and `::testing::FloatNear`, respectively.
::testing::Matcher<MeshAttributeBounds> MeshAttributeBoundsEq(
    const MeshAttributeBounds &expected);
::testing::Matcher<MeshAttributeBounds> MeshAttributeBoundsNear(
    const MeshAttributeBounds &expected, float tolerance);

// These matchers compare `Envelope`s by their bounds, delegating to `RectEq`
// and `RectNear`, respectively.
::testing::Matcher<Envelope> EnvelopeEq(const Envelope &expected);
::testing::Matcher<Envelope> EnvelopeEq(const Rect &expected);
::testing::Matcher<Envelope> EnvelopeNear(const Rect &expected,
                                          float tolerance);

// Returns a matcher that compares two `Meshes`, which are considered equal iff:
// - They have the same format, compared via `MeshFormatEq`
// - They have the same vertex data, compared bitwise
// - They have the same index data, compared bitwise
// - They have the same vertex attribute unpacking params, compared via
//   `MeshAttributeCodingParamsEq`
// Note that all other `Mesh` properties are derived from these four.
::testing::Matcher<Mesh> MeshEq(const Mesh &mesh);

::testing::Matcher<PartitionedMesh::VertexIndexPair> VertexIndexPairEq(
    PartitionedMesh::VertexIndexPair expected);

// Returns a matcher that compares two `PartitionedMesh`s, which are considered
// equal iff:
// - They have the same meshes, compared via `MeshEq`.
// - They have the same outlines, compared via `OutlineEq`.
::testing::Matcher<PartitionedMesh> PartitionedMeshDeepEq(
    const PartitionedMesh &expected);

// Returns a matcher that compares two `PartitionedMesh`s, which are considered
// equal iff:
// - They have the same meshes instances.
// - They have the same outlines, compared via `OutlineEq`.
::testing::Matcher<PartitionedMesh> PartitionedMeshShallowEq(
    const PartitionedMesh &expected);

}  // namespace ink

#endif  // INK_GEOMETRY_TYPE_MATCHERS_H_
