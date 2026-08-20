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

#ifndef INK_GEOMETRY_FUZZ_DOMAINS_H_
#define INK_GEOMETRY_FUZZ_DOMAINS_H_

#include "fuzztest/fuzztest.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/rect.h"
#include "ink/geometry/segment.h"
#include "ink/geometry/triangle.h"
#include "ink/geometry/vec.h"

namespace ink {

// The domain of all finite angles.
fuzztest::Domain<Angle> FiniteAngle();
// The domain of all angles in the interval [min, max].
fuzztest::Domain<Angle> AngleInRange(Angle min, Angle max);
// The domain of all angles in the interval [0, 2π).
fuzztest::Domain<Angle> NormalizedAngle();

// The domain of all MeshFormat::AttributeTypes.
fuzztest::Domain<MeshFormat::AttributeType> ArbitraryMeshAttributeType();
// The domain of MeshFormat::AttributeTypes that are valid for an
// AttributeId::kPosition attribute.
fuzztest::Domain<MeshFormat::AttributeType> PositionMeshAttributeType();

// The domain of all MeshFormats.
fuzztest::Domain<MeshFormat> ArbitraryMeshFormat();
// The domain of all MeshFormat::IndexFormats.
fuzztest::Domain<MeshFormat::IndexFormat> ArbitraryMeshIndexFormat();

// The domain of all points, including ones with infinite/NaN components.
fuzztest::Domain<Point> ArbitraryPoint();
// The domain of all points with non-NaN (but possibly infinite) components.
fuzztest::Domain<Point> NotNanPoint();
// The domain of all points with finite components.
fuzztest::Domain<Point> FinitePoint();
// The domain of all points contained by the given rect.
fuzztest::Domain<Point> PointInRect(Rect rect);

// The domain of all rects with non-NaN (but possibly infinite) bounds.
fuzztest::Domain<Rect> NotNanRect();
// The domain of all rects with finite bounds.
fuzztest::Domain<Rect> FiniteRect();

// The domain of all segments with finite endpoints.
fuzztest::Domain<Segment> FiniteSegment();
// The domain of all segments whose endpoints are contained by the given rect.
fuzztest::Domain<Segment> SegmentInRect(Rect rect);

// The domain of all triangles whose corners are contained by the given rect.
fuzztest::Domain<Triangle> TriangleInRect(Rect rect);

// The domain of all vectors, including ones with infinite/NaN components.
fuzztest::Domain<Vec> ArbitraryVec();
// The domain of all vectors with non-NaN (but possibly infinite) components.
fuzztest::Domain<Vec> NotNanVec();

// The domain of non-empty `MutableMesh`es with only a position attribute of
// `position_attribute_type`, index of `index_format`, and for which
// `MutableMesh::ValidateTriangles` succeeds.
fuzztest::Domain<MutableMesh> ValidPackableNonEmptyPositionOnlyMutableMesh(
    MeshFormat::AttributeType position_attribute_type,
    MeshFormat::IndexFormat index_format =
        MeshFormat::IndexFormat::k16BitUnpacked16BitPacked);

}  // namespace ink

#endif  // INK_GEOMETRY_FUZZ_DOMAINS_H_
