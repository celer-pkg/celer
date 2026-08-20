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

#include "ink/rendering/skia/native/skia_renderer.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "absl/base/nullability.h"
#include "absl/container/inlined_vector.h"
#include "absl/functional/overload.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/brush/brush.h"
#include "ink/brush/brush_paint.h"
#include "ink/color/color.h"
#include "ink/geometry/affine_transform.h"
#include "ink/geometry/mesh.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/partitioned_mesh.h"
#include "ink/geometry/rect.h"
#include "ink/rendering/skia/native/internal/mesh_drawable.h"
#include "ink/rendering/skia/native/internal/mesh_uniform_data.h"
#include "ink/rendering/skia/native/internal/path_drawable.h"
#include "ink/rendering/skia/native/texture_bitmap_store.h"
#include "ink/strokes/in_progress_stroke.h"
#include "ink/strokes/stroke.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkMeshGanesh.h"

namespace ink {
namespace {

using ::ink::skia_native_internal::MeshDrawable;
using ::ink::skia_native_internal::MeshUniformData;
using ::ink::skia_native_internal::PathDrawable;

void FillTemporaryIndices(const MutableMesh& mesh,
                          std::vector<uint16_t>& temporary_indices) {
  temporary_indices.clear();
  temporary_indices.reserve(3 * mesh.TriangleCount());
  for (uint32_t i = 0; i < mesh.TriangleCount(); ++i) {
    for (uint32_t index : mesh.TriangleIndices(i)) {
      temporary_indices.push_back(index);
    }
  }
}

SkRect ToSkiaRect(const Rect& rect) {
  return SkRect::MakeLTRB(rect.XMin(), rect.YMin(), rect.XMax(), rect.YMax());
}

// Returns true if the renderer should use `SkPath` instead of `SkMesh` for
// rendering.
//
// TODO: b/346530293 - Also use the `BrushPaint` for the decision once the paint
// has a setting for opacity behavior on self-overlap. This would make it
// possible that a single `Drawable` holds a mix of meshes and paths.
bool UsePathRendering(GrDirectContext* context, const BrushPaint&) {
  return context == nullptr;
}

// Returns the color opacity multiplier when `SkPath` should be used for
// rendering instead of `SkMesh`.
float OpacityMultiplierForPath(const Brush& brush, uint32_t coat_index) {
  return brush.GetCoats()[coat_index].tip.opacity_multiplier;
}

// Returns the `TextureMapping` used by the given `BrushPaint`. Right now, we
// don't support rendering a `BrushPaint` that mixes different `TextureMapping`
// modes, so this just returns the `TextureMapping` of the first texture layer,
// if any. If the `BrushPaint` has no `TextureLayers`, then the return value
// doesn't really matter either way, so it just returns `kTiling` (since that
// mode is marginally easier for the shader to calculate).
//
// TODO: b/375203215 - Get rid of this function once we are able to mix tiling
// and winding textures in a single `BrushPaint`.
BrushPaint::TextureMapping GetBrushPaintTextureMapping(
    const BrushPaint& paint) {
  return !paint.texture_layers.empty() ? paint.texture_layers[0].mapping
                                       : BrushPaint::TextureMapping::kTiling;
}

}  // namespace

SkiaRenderer::SkiaRenderer(
    absl_nullable std::shared_ptr<TextureBitmapStore> texture_provider)
    : texture_provider_(std::move(texture_provider)),
      shader_cache_(texture_provider_.get()) {}

absl::StatusOr<SkiaRenderer::Drawable> SkiaRenderer::CreateDrawable(
    GrDirectContext* context, const InProgressStroke& stroke,
    const AffineTransform& object_to_canvas) {
  const Brush* brush = stroke.GetBrush();
  if (brush == nullptr) {
    return Drawable(object_to_canvas, {});
  }

  uint32_t num_coats = brush->CoatCount();
  absl::InlinedVector<Drawable::Implementation, 1> drawables;
  drawables.reserve(num_coats);
  for (uint32_t coat_index = 0; coat_index < num_coats; ++coat_index) {
    if (stroke.GetMeshBounds(coat_index).IsEmpty()) continue;

    if (UsePathRendering(context, brush->GetCoats()[coat_index].paint)) {
      drawables.push_back(PathDrawable(
          stroke.GetMesh(coat_index), stroke.GetCoatOutlines(coat_index),
          brush->GetColor(), OpacityMultiplierForPath(*brush, coat_index)));
      continue;
    }

    const BrushPaint& brush_paint = brush->GetCoats()[coat_index].paint;
    absl::StatusOr<sk_sp<SkShader>> shader = shader_cache_.GetShaderForPaint(
        brush_paint, brush->GetSize(), stroke.GetInputs());
    if (!shader.ok()) return shader.status();

    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        specification_cache_.GetFor(stroke);
    if (!specification.ok()) return specification.status();

    const MutableMesh& mesh = stroke.GetMesh(coat_index);
    if (mesh.VertexCount() >= std::numeric_limits<uint16_t>::max()) {
      return absl::UnimplementedError(
          "Strokes requiring at least 2^16 indices are not supported yet.");
    }

    absl::Span<const std::byte> vertex_data = mesh.RawVertexData();
    FillTemporaryIndices(mesh, temporary_indices_);

    absl::StatusOr<MeshDrawable> mesh_drawable = MeshDrawable::Create(
        *std::move(specification),
        shader_cache_.GetBlenderForPaint(brush_paint), *std::move(shader),
        {{
            .vertex_buffer = SkMeshes::MakeVertexBuffer(
                context, vertex_data.data(), vertex_data.size()),
            .index_buffer = SkMeshes::MakeIndexBuffer(
                context, temporary_indices_.data(),
                temporary_indices_.size() * sizeof(uint16_t)),
            .vertex_count = static_cast<int32_t>(mesh.VertexCount()),
            .index_count = static_cast<int32_t>(3 * mesh.TriangleCount()),
            .bounds = ToSkiaRect(*stroke.GetMeshBounds(coat_index).AsRect()),
        }});
    if (!mesh_drawable.ok()) return mesh_drawable.status();

    mesh_drawable->SetBrushColor(brush->GetColor());
    mesh_drawable->SetTextureMapping(GetBrushPaintTextureMapping(brush_paint));
    drawables.push_back(*std::move(mesh_drawable));
  }

  return Drawable(object_to_canvas, std::move(drawables));
}

absl::StatusOr<SkiaRenderer::Drawable> SkiaRenderer::CreateDrawable(
    GrDirectContext* context, const Stroke& stroke,
    const AffineTransform& object_to_canvas) {
  const PartitionedMesh& stroke_shape = stroke.GetShape();
  if (stroke_shape.RenderGroupCount() == 0) {
    return Drawable(object_to_canvas, {});
  }

  const Brush& brush = stroke.GetBrush();
  uint32_t num_coats = brush.CoatCount();
  // This is guaranteed by the `Stroke` class.
  ABSL_DCHECK_EQ(stroke_shape.RenderGroupCount(), num_coats);

  absl::InlinedVector<Drawable::Implementation, 1> drawables;
  drawables.reserve(num_coats);
  for (uint32_t coat_index = 0; coat_index < num_coats; ++coat_index) {
    absl::Span<const Mesh> meshes = stroke_shape.RenderGroupMeshes(coat_index);
    if (meshes.empty()) continue;

    if (UsePathRendering(context, brush.GetCoats()[coat_index].paint)) {
      drawables.push_back(
          PathDrawable(stroke_shape, coat_index, brush.GetColor(),
                       OpacityMultiplierForPath(brush, coat_index)));
      continue;
    }

    const BrushPaint& brush_paint = brush.GetCoats()[coat_index].paint;
    absl::StatusOr<sk_sp<SkShader>> shader = shader_cache_.GetShaderForPaint(
        brush_paint, brush.GetSize(), stroke.GetInputs());
    if (!shader.ok()) return shader.status();

    // TODO: b/284117747 - Pass `brush.GetCoats()[coat_index].paint` to the
    // `specification_cache_`.
    absl::StatusOr<sk_sp<SkMeshSpecification>> specification =
        specification_cache_.GetForStroke(stroke_shape, coat_index);
    if (!specification.ok()) return specification.status();

    absl::InlinedVector<MeshDrawable::Partition, 1> partitions;
    partitions.reserve(meshes.size());
    for (const Mesh& mesh : meshes) {
      absl::Span<const std::byte> vertex_data = mesh.RawVertexData();
      absl::Span<const std::byte> index_data = mesh.RawIndexData();
      partitions.push_back({
          .vertex_buffer = SkMeshes::MakeVertexBuffer(
              context, vertex_data.data(), vertex_data.size()),
          .index_buffer = SkMeshes::MakeIndexBuffer(context, index_data.data(),
                                                    index_data.size()),
          .vertex_count = static_cast<int32_t>(mesh.VertexCount()),
          .index_count = static_cast<int32_t>(3 * mesh.TriangleCount()),
          .bounds = ToSkiaRect(*mesh.Bounds().AsRect()),
      });
    }

    const Mesh& first_mesh = meshes.front();
    auto get_attribute_unpacking_transform =
        [&first_mesh](int attribute_index) -> const MeshAttributeCodingParams& {
      return first_mesh.VertexAttributeUnpackingParams(attribute_index);
    };

    MeshUniformData uniform_data(**specification,
                                 first_mesh.Format().Attributes(),
                                 get_attribute_unpacking_transform);
    absl::StatusOr<MeshDrawable> mesh_drawable = MeshDrawable::Create(
        *std::move(specification),
        shader_cache_.GetBlenderForPaint(brush_paint), *std::move(shader),
        std::move(partitions), std::move(uniform_data));
    if (!mesh_drawable.ok()) return mesh_drawable.status();

    mesh_drawable->SetBrushColor(brush.GetColor());
    mesh_drawable->SetTextureMapping(GetBrushPaintTextureMapping(brush_paint));
    drawables.push_back(*std::move(mesh_drawable));
  }

  return Drawable(object_to_canvas, std::move(drawables));
}

absl::Status SkiaRenderer::Draw(GrDirectContext* context,
                                const InProgressStroke& stroke,
                                const AffineTransform& object_to_canvas,
                                SkCanvas& canvas) {
  // TODO: b/286547863 - Implement `RenderCache` to save and update the created
  // drawable inside the stroke.

  auto drawable = CreateDrawable(context, stroke, object_to_canvas);
  if (!drawable.ok()) return drawable.status();
  drawable->Draw(canvas);
  return absl::OkStatus();
}

absl::Status SkiaRenderer::Draw(GrDirectContext* context, const Stroke& stroke,
                                const AffineTransform& object_to_canvas,
                                SkCanvas& canvas) {
  // TODO: b/286547863 - Implement `RenderCache` to save and update the created
  // drawable inside the stroke.

  auto drawable = CreateDrawable(context, stroke, object_to_canvas);
  if (!drawable.ok()) return drawable.status();
  drawable->Draw(canvas);
  return absl::OkStatus();
}

namespace {

SkM44 ToSkiaM44(const AffineTransform& t) {
  // The constructor parameters are documented to be in row-major order.
  return SkM44(t.A(), t.B(), 0, t.C(),  //
               t.D(), t.E(), 0, t.F(),  //
               0, 0, 1, 0,              //
               0, 0, 0, 1);             //
}

}  // namespace

void SkiaRenderer::Drawable::Draw(SkCanvas& canvas) const {
  canvas.setMatrix(ToSkiaM44(object_to_canvas_));
  for (const Implementation& impl : drawable_implementations_) {
    std::visit([&canvas](const auto& drawable) { drawable.Draw(canvas); },
               impl);
  }
}

SkiaRenderer::Drawable::Drawable(
    const AffineTransform& object_to_canvas,
    absl::InlinedVector<Implementation, 1> drawable_impls)
    : drawable_implementations_(std::move(drawable_impls)) {
  SetObjectToCanvas(object_to_canvas);
}

void SkiaRenderer::Drawable::SetObjectToCanvas(
    const AffineTransform& object_to_canvas) {
  object_to_canvas_ = object_to_canvas;
  for (Implementation& drawable_impl : drawable_implementations_) {
    auto* drawable = std::get_if<MeshDrawable>(&drawable_impl);
    if (drawable != nullptr && drawable->HasObjectToCanvas()) {
      drawable->SetObjectToCanvas(object_to_canvas);
    }
  }
}

bool SkiaRenderer::Drawable::HasBrushColor() const {
  for (const Implementation& drawable_impl : drawable_implementations_) {
    if (std::visit(absl::Overload(
                       [](const MeshDrawable& drawable) {
                         return drawable.HasBrushColor();
                       },
                       [](const PathDrawable& drawable) { return true; }),
                   drawable_impl)) {
      return true;
    }
  }
  return false;
}

void SkiaRenderer::Drawable::SetBrushColor(const Color& color) {
  bool has_color = false;
  for (Implementation& drawable_impl : drawable_implementations_) {
    std::visit(absl::Overload(
                   [&color, &has_color](MeshDrawable& drawable) {
                     if (drawable.HasBrushColor()) {
                       drawable.SetBrushColor(color);
                       has_color = true;
                     }
                   },
                   [&color, &has_color](PathDrawable& drawable) {
                     drawable.SetPaintColor(color);
                     has_color = true;
                   }),
               drawable_impl);
  }
  ABSL_CHECK(has_color);
}

void SkiaRenderer::Drawable::SetImageFilter(sk_sp<SkImageFilter> image_filter) {
  for (Implementation& drawable_impl : drawable_implementations_) {
    std::visit(absl::Overload(
                   [&image_filter](MeshDrawable& drawable) {
                     drawable.SetImageFilter(image_filter);
                   },
                   [&image_filter](PathDrawable& drawable) {
                     drawable.SetImageFilter(image_filter);
                   }),
               drawable_impl);
  }
}

}  // namespace ink
