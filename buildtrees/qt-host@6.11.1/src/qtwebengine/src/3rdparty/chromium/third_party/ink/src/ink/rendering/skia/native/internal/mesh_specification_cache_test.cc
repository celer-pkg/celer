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

#include "ink/rendering/skia/native/internal/mesh_specification_cache.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_family.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"
#include "ink/color/color.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_test_helpers.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/strokes/in_progress_stroke.h"
#include "ink/strokes/input/stroke_input_batch.h"
#include "ink/strokes/stroke.h"
#include "ink/types/duration.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::NotNull;
using ::testing::Pointer;

Brush GetTestBrush() {
  absl::StatusOr<BrushFamily> brush_family =
      BrushFamily::Create(BrushTip{}, BrushPaint{});
  ABSL_CHECK_OK(brush_family);
  absl::StatusOr<Brush> brush =
      Brush::Create(*brush_family, Color::Black(), 1, 0.1);
  ABSL_CHECK_OK(brush);
  return *brush;
}

TEST(MeshSpecificationCacheTest, GetForInProgressStroke) {
  MeshSpecificationCache cache;
  InProgressStroke first_stroke;
  first_stroke.Start(GetTestBrush());

  {
    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        cache.GetFor(first_stroke);
    ASSERT_EQ(specification.status(), absl::OkStatus());
    EXPECT_THAT(*specification, Pointer(NotNull()));
  }

  {
    // Getting the specification for the same stroke a second time should return
    // the cached value.
    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        cache.GetFor(first_stroke);
    ASSERT_EQ(specification.status(), absl::OkStatus());
    EXPECT_THAT(*specification, Eq(*specification));
  }

  InProgressStroke second_stroke;
  second_stroke.Start(GetTestBrush());

  {
    // Getting the specification for another in-progress stroke should return
    // the cached value.
    // TODO: b/284117747 - If we start to use SkSL shader uniform types in the
    // mesh, this will also depend on the objects having the same `BrushPaint`.
    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        cache.GetFor(second_stroke);
    ASSERT_EQ(specification.status(), absl::OkStatus());
    EXPECT_THAT(*specification, Eq(*specification));
  }
}

TEST(MeshSpecificationCacheTest, GetForStartedInProgressStrokeWithEmptyMesh) {
  MeshSpecificationCache cache;
  InProgressStroke stroke;
  stroke.Start(GetTestBrush());
  absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
      cache.GetFor(stroke);
  ASSERT_EQ(specification.status(), absl::OkStatus());
  EXPECT_THAT(*specification, Pointer(NotNull()));
}

TEST(MeshSpecificationCacheTest, GetForUnstartedInProgressStroke) {
  MeshSpecificationCache cache;
  InProgressStroke stroke;
  absl::Status not_started = cache.GetFor(stroke).status();
  EXPECT_EQ(not_started.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(not_started.message(), HasSubstr("stroke.Start()"));

  // The cache should be able to return a valid specification after previously
  // returning an error.
  stroke.Start(GetTestBrush());
  absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
      cache.GetFor(stroke);
  ASSERT_EQ(specification.status(), absl::OkStatus());
  EXPECT_THAT(*specification, Pointer(NotNull()));
}

StrokeInputBatch GetSingleValueTestBatch() {
  absl::StatusOr<StrokeInputBatch> batch = StrokeInputBatch::Create(
      {{.position = {0, 0}, .elapsed_time = Duration32::Zero()}});
  ABSL_CHECK_OK(batch);
  return *batch;
}

TEST(MeshSpecificationCacheTest, GetForStrokeWithGeneratedShape) {
  MeshSpecificationCache cache;
  Stroke first_stroke(GetTestBrush(), GetSingleValueTestBatch());

  absl::StatusOr<sk_sp<SkMeshSpecification>> original_spec =
      cache.GetForStroke(first_stroke.GetShape(), 0);
  ASSERT_EQ(original_spec.status(), absl::OkStatus());
  EXPECT_THAT(*original_spec, Pointer(NotNull()));

  {
    // Getting the specification for the same stroke a second time should return
    // the cached value.
    absl::StatusOr<sk_sp<SkMeshSpecification>> new_spec =
        cache.GetForStroke(first_stroke.GetShape(), 0);
    ASSERT_EQ(new_spec.status(), absl::OkStatus());
    EXPECT_THAT(*new_spec, Eq(*original_spec));
  }

  {
    Stroke second_stroke(GetTestBrush(), GetSingleValueTestBatch());
    // Getting the specification for another stroke with the same brush and a
    // generated shape should return the cached value.
    absl::StatusOr<sk_sp<SkMeshSpecification>> new_spec =
        cache.GetForStroke(second_stroke.GetShape(), 0);
    ASSERT_EQ(new_spec.status(), absl::OkStatus());
    EXPECT_THAT(*new_spec, Eq(*original_spec));
  }
}

TEST(MeshSpecificationCacheTest, GetForInvalidCoatIndex) {
  MeshSpecificationCache cache;
  Stroke stroke(GetTestBrush(), GetSingleValueTestBatch());

  absl::Status status = cache.GetForStroke(stroke.GetShape(), 99).status();
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("coat_index"));

  // The cache should be able to return a valid specification after previously
  // returning an error.
  absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
      cache.GetForStroke(stroke.GetShape(), 0);
  ASSERT_EQ(specification.status(), absl::OkStatus());
  EXPECT_THAT(*specification, Pointer(NotNull()));
}

TEST(MeshSpecificationCacheTest, GetForStrokeWithEmptyShape) {
  MeshSpecificationCache cache;
  absl::StatusOr<PartitionedMesh> shape = PartitionedMesh::FromMeshGroups(
      {PartitionedMesh::MeshGroup{.meshes = {}, .outlines = {}}});
  ASSERT_EQ(shape.status(), absl::OkStatus());
  absl::Status no_meshes = cache.GetForStroke(*shape, 0).status();
  EXPECT_EQ(no_meshes.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(no_meshes.message(), HasSubstr("has no meshes"));
}

TEST(MeshSpecificationCacheTest, GetForProvidedUnsupportedShape) {
  MeshSpecificationCache cache;

  PartitionedMesh provided_shape =
      MakeStraightLinePartitionedMesh(/* n_triangles = */ 4, MeshFormat());

  // The provided shape has the default position-only `MeshFormat` , which means
  // it is missing a number of required attributes.
  absl::Status missing_required_attr =
      cache.GetForStroke(provided_shape, 0).status();
  EXPECT_EQ(missing_required_attr.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(missing_required_attr.message(), HasSubstr("are required"));
}

}  // namespace
}  // namespace ink::skia_native_internal
