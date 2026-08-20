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

#include "ink/rendering/skia/native/internal/path_drawable.h"

#include <cstdint>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "ink/color/color.h"
#include "ink/color/color_space.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/point.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRefCnt.h"

namespace ink::skia_native_internal {
namespace {

// Creates an `SkPath` using `outline_indices` to retrieve path positions from
// `mesh`.
SkPath MakePolygonPath(const MutableMesh& mesh,
                       absl::Span<const uint32_t> outline_indices) {
  ABSL_DCHECK(!outline_indices.empty());

  SkPath path;
  path.setFillType(SkPathFillType::kWinding);

  Point position = mesh.VertexPosition(outline_indices.front());
  outline_indices.remove_prefix(1);

  path.moveTo(position.x, position.y);
  for (uint32_t index : outline_indices) {
    position = mesh.VertexPosition(index);
    path.lineTo(position.x, position.y);
  }
  path.close();

  return path;
}

// Creates an `SkPath` using `group_outline_indices` to retrieve path positions
// from the meshes in `mesh_group`.
SkPath MakePolygonPath(
    absl::Span<const Mesh> mesh_group,
    absl::Span<const PartitionedMesh::VertexIndexPair> group_outline_indices) {
  ABSL_DCHECK(!group_outline_indices.empty());

  SkPath path;
  path.setFillType(SkPathFillType::kWinding);

  Point position =
      mesh_group[group_outline_indices.front().mesh_index].VertexPosition(
          group_outline_indices.front().vertex_index);
  group_outline_indices.remove_prefix(1);

  path.moveTo(position.x, position.y);
  for (PartitionedMesh::VertexIndexPair index_pair : group_outline_indices) {
    position = mesh_group[index_pair.mesh_index].VertexPosition(
        index_pair.vertex_index);
    path.lineTo(position.x, position.y);
  }
  path.close();

  return path;
}

void SetPaintDefaultsForPath(SkPaint& paint) {
  paint.setAntiAlias(true);
  paint.setStyle(SkPaint::kFill_Style);
}

}  // namespace

PathDrawable::PathDrawable(
    const MutableMesh& mesh,
    absl::Span<const absl::Span<const uint32_t>> index_outlines,
    const Color& color, float opacity_multiplier)
    : opacity_multiplier_(opacity_multiplier) {
  for (absl::Span<const uint32_t> indices : index_outlines) {
    if (indices.empty()) continue;

    paths_.push_back(MakePolygonPath(mesh, indices));
  }
  SetPaintDefaultsForPath(paint_);
  SetPaintColor(color);
}

PathDrawable::PathDrawable(const PartitionedMesh& shape,
                           uint32_t render_group_index, const Color& color,
                           float opacity_multiplier)
    : opacity_multiplier_(opacity_multiplier) {
  absl::Span<const Mesh> mesh_group =
      shape.RenderGroupMeshes(render_group_index);
  for (uint32_t i = 0; i < shape.OutlineCount(render_group_index); ++i) {
    absl::Span<const PartitionedMesh::VertexIndexPair> indices =
        shape.Outline(render_group_index, i);
    if (indices.empty()) continue;

    paths_.push_back(MakePolygonPath(mesh_group, indices));
  }
  SetPaintDefaultsForPath(paint_);
  SetPaintColor(color);
}

void PathDrawable::SetPaintColor(const Color& color) {
  Color::RgbaFloat c =
      color.InColorSpace(ColorSpace::kSrgb).AsFloat(Color::Format::kLinear);
  sk_sp<SkColorSpace> srgb_linear = SkColorSpace::MakeSRGBLinear();
  paint_.setColor(
      {.fR = c.r, .fG = c.g, .fB = c.b, .fA = c.a * opacity_multiplier_},
      srgb_linear.get());
}

void PathDrawable::SetImageFilter(sk_sp<SkImageFilter> image_filter) {
  paint_.setImageFilter(image_filter);
}

void PathDrawable::Draw(SkCanvas& canvas) const {
  for (const SkPath& path : paths_) {
    canvas.drawPath(path, paint_);
  }
}

}  // namespace ink::skia_native_internal
