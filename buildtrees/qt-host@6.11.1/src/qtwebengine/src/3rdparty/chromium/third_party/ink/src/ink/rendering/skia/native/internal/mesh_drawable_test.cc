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

#include "ink/rendering/skia/native/internal/mesh_drawable.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "ink/rendering/skia/common_internal/mesh_specification_data.h"
#include "ink/rendering/skia/native/internal/create_mesh_specification.h"
#include "ink/rendering/skia/native/internal/mesh_uniform_data.h"
#include "ink/strokes/internal/stroke_vertex.h"
#include "include/core/SkMesh.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {
namespace {

using ::ink::skia_common_internal::MeshSpecificationData;
using ::ink::strokes_internal::StrokeVertex;
using ::testing::HasSubstr;

sk_sp<SkMeshSpecification> SpecificationForInProgressStroke() {
  auto spec = CreateMeshSpecification(
      MeshSpecificationData::CreateForInProgressStroke());
  ABSL_CHECK_OK(spec);
  return *std::move(spec);
}

sk_sp<SkMeshSpecification> SpecificationForFullFormatStroke() {
  auto data =
      MeshSpecificationData::CreateForStroke(StrokeVertex::FullMeshFormat());
  ABSL_CHECK_OK(data);
  auto spec = CreateMeshSpecification(*data);
  ABSL_CHECK_OK(data);
  return *std::move(spec);
}

// Returns a placeholder partition with CPU-backed vertex and index buffers.
MeshDrawable::Partition MakeNonEmptyTestPartition(int vertex_stride) {
  int32_t vertex_count = 6;
  int32_t index_count = 9;
  std::vector<std::byte> placeholder_vertex_data(vertex_count * vertex_stride);
  std::vector<std::byte> placeholder_index_data(index_count * sizeof(uint16_t));
  return {
      .vertex_buffer = SkMeshes::MakeVertexBuffer(
          placeholder_vertex_data.data(), placeholder_vertex_data.size()),
      .index_buffer = SkMeshes::MakeIndexBuffer(placeholder_index_data.data(),
                                                placeholder_index_data.size()),
      .vertex_count = vertex_count,
      .index_count = index_count,
      .bounds = SkRect::MakeLTRB(0, 0, 1, 1),
  };
}

TEST(MeshDrawableTest, DefaultConstructed) {
  MeshDrawable drawable;
  EXPECT_FALSE(drawable.HasObjectToCanvas());
  EXPECT_FALSE(drawable.HasBrushColor());
}

TEST(MeshDrawableTest, CreateWithoutPartitionsIsNotAnError) {
  auto drawable =
      MeshDrawable::Create(SpecificationForInProgressStroke(),
                           /* blender= */ nullptr, /* shader= */ nullptr,
                           /* partitions = */ {});
  ASSERT_EQ(absl::OkStatus(), drawable.status());
  EXPECT_TRUE(drawable->HasObjectToCanvas());
  EXPECT_TRUE(drawable->HasBrushColor());
}

TEST(MeshDrawableTest, CreateNonEmptyWithoutUnpackingTransform) {
  sk_sp<SkMeshSpecification> spec = SpecificationForInProgressStroke();

  auto drawable =
      MeshDrawable::Create(spec, /* blender= */ nullptr, /* shader= */ nullptr,
                           {MakeNonEmptyTestPartition(spec->stride()),
                            MakeNonEmptyTestPartition(spec->stride())});
  ASSERT_EQ(absl::OkStatus(), drawable.status());
  EXPECT_TRUE(drawable->HasObjectToCanvas());
  EXPECT_TRUE(drawable->HasBrushColor());
}

TEST(MeshDrawableTest, CreateSucceedsWithProvidedStartingUniforms) {
  sk_sp<SkMeshSpecification> spec = SpecificationForFullFormatStroke();

  MeshUniformData starting_uniforms(*spec);
  auto drawable =
      MeshDrawable::Create(spec, /* blender= */ nullptr, /* shader= */ nullptr,
                           {MakeNonEmptyTestPartition(spec->stride()),
                            MakeNonEmptyTestPartition(spec->stride())},
                           starting_uniforms);
  ASSERT_EQ(absl::OkStatus(), drawable.status());
  EXPECT_TRUE(drawable->HasObjectToCanvas());
  EXPECT_TRUE(drawable->HasBrushColor());
}

TEST(MeshDrawableTest, ReturnsSkiaError) {
  sk_sp<SkMeshSpecification> spec = SpecificationForInProgressStroke();

  int incompatible_stride = 4;
  ASSERT_NE(spec->stride(), incompatible_stride);

  absl::Status skia_error =
      MeshDrawable::Create(spec, /* blender= */ nullptr, /* shader= */ nullptr,
                           {MakeNonEmptyTestPartition(incompatible_stride)})
          .status();
  EXPECT_EQ(skia_error.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(skia_error.message(),
              HasSubstr("`SkMesh::MakeIndex()` returned error:"));
}

TEST(MeshDrawableDeathTest, CreateWithNullSpecification) {
  EXPECT_DEATH_IF_SUPPORTED(auto drawable = MeshDrawable::Create(
                                /* specification= */ nullptr,
                                /* blender= */ nullptr, /* shader= */ nullptr,
                                /* partitions= */ {}),
                            "specification");
}

TEST(MeshDrawableDeathTest, CreateWithNullVertexBuffer) {
  sk_sp<SkMeshSpecification> spec = SpecificationForInProgressStroke();
  MeshDrawable::Partition partition = MakeNonEmptyTestPartition(spec->stride());
  partition.vertex_buffer.reset();

  EXPECT_DEATH_IF_SUPPORTED(auto drawable = MeshDrawable::Create(
                                spec, /* blender= */ nullptr,
                                /* shader= */ nullptr, {std::move(partition)}),
                            "vertex_buffer");
}

TEST(MeshDrawableDeathTest, CreateWithNullIndexBuffer) {
  sk_sp<SkMeshSpecification> spec = SpecificationForInProgressStroke();
  MeshDrawable::Partition partition = MakeNonEmptyTestPartition(spec->stride());
  partition.index_buffer.reset();

  EXPECT_DEATH_IF_SUPPORTED(auto drawable = MeshDrawable::Create(
                                spec, /* blender= */ nullptr,
                                /* shader= */ nullptr, {std::move(partition)}),
                            "index_buffer");
}

}  // namespace
}  // namespace ink::skia_native_internal
