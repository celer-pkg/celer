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

#ifndef INK_STROKES_INTERNAL_STROKE_VERTEX_H_
#define INK_STROKES_INTERNAL_STROKE_VERTEX_H_

#include <array>
#include <cstdint>
#include <optional>

#include "absl/types/span.h"
#include "ink/geometry/mesh_format.h"
#include "ink/geometry/mesh_packing_types.h"
#include "ink/geometry/mutable_mesh.h"
#include "ink/geometry/point.h"
#include "ink/geometry/vec.h"
#include "ink/types/small_array.h"

namespace ink::strokes_internal {

// Vertex type used for building stroke meshes.
//
// The layout of this struct is identical to the per-vertex byte layout inside a
// `MutableMesh` constructed with `StrokeVertex::FullMeshFormat()`. This
// reflects the intended data layout inside "unpacked" GPU buffers.
struct StrokeVertex {
  // Classifiers of vertex placement in the mesh used for anti-aliasing.
  enum class SideCategory { kExteriorLeft, kInterior, kExteriorRight };
  enum class ForwardCategory { kExteriorFront, kInterior, kExteriorBack };

  // An encoded label for the vertex to be consumed in shader code.
  //
  // Each label stores either a `SideCategory` or `ForwardCategory` in its
  // sign-bit and a "margin" in its magnitude.
  //
  // In order to perform anti-aliasing in the shader without causing the stroke
  // to visibly shrink, vertex positions must be outset by approximately 0.5
  // pixels during rendering. The target outset will be calculated using the
  // values of `side_derivative` and `forward_derivative` along with information
  // for the stroke-to-canvas transformation. However, if we always allow all
  // vertices to be moved by the calculated target outset, we will potentially
  // introduce artifacts in partially transparent strokes if two adjacent
  // vertices along a concave part of the stroke move past one another causing
  // extra self-overlap.
  //
  // To counteract this, we store the margin, defined as the maximum reposition
  // distance in units of the derivative magnitude to allow the shader to cap
  // how far vertices can be moved. This is done because the shader code prefers
  // to work in multiples of the derivative magnitude. This is also convenient
  // for the mesh format, because it will allow us to store the category and the
  // margin together in only 8 bits when we enable packing. This will allow each
  // label to live in one attribute alongside the 24 bits that will be used for
  // a packed derivative value.
  struct Label {
    // Returns a label with the same category, but encoded with a replacement
    // value for `margin`.
    //
    // The encoding of the margin will be lossy and will be clipped to a finite
    // range of non-negative values.
    Label WithMargin(float margin) const;

    SideCategory DecodeSideCategory() const;
    ForwardCategory DecodeForwardCategory() const;
    float DecodeMargin() const;

    // Returns the sign value (-1, 0, or 1) by which the vertex derivative
    // associated with this label should be multiplied such that it points
    // toward the stroke exterior.
    //
    // For example, this will return -1 for a side label of a vertex on the
    // left-exterior, because the side derivative will point left-to-right. See
    // also the sign function: https://en.wikipedia.org/wiki/Sign_function.
    float DerivativeOutsetSign() const;

    float encoded_value;
  };

  // The maximum value of the margin that will be encoded without clipping.
  //
  // For the side derivative and label as an example, the maximum value is the
  // most that a vertex can be moved as a multiple of approximate stroke width.
  // In other words, the position outset distance will be capped once the stroke
  // is as small as 1/kMaximumMargin-th of a pixel.
  // LINT.IfChange(margin_encoding)
  static constexpr float kMaximumMargin = 4;
  // LINT.ThenChange(
  //     ../../rendering/skia/common_internal/sksl_vertex_shader_helper_functions.h:margin_encoding)

  // Constant value used by interior vertices.
  static constexpr Label kInteriorLabel = {0};
  // Labels for vertices on the exterior of the stroke mesh. These labels
  // correspond to the largest encoded margins.
  static constexpr Label kExteriorLeftLabel = {-127};
  static constexpr Label kExteriorRightLabel = {127};
  static constexpr Label kExteriorFrontLabel = {-127};
  static constexpr Label kExteriorBackLabel = {127};

  // Collection of all non-position attributes.
  struct NonPositionAttributes {
    // Value to be used by a renderer to shift the per-vertex opacity.
    //
    // Values will usually be within the range [-1, 1], but extrapolation is
    // allowed to create values outside of this range.
    float opacity_shift = 0;
    // Values to be used by a renderer to shift the per-vertex color by
    // individually adjusting hue, saturation, and luminosity.
    //
    // Values will usually be within the range [-1, 1], but extrapolation is
    // allowed to create values outside of this range.
    std::array<float, 3> hsl_shift = {0, 0, 0};
    // Approximate derivative of position with respect to the barycentric
    // coordinate that points across the width of the stroke in triangles that
    // include this vertex.
    Vec side_derivative = {0, 0};
    // Vertex label to be used together with the `side_derivative`.
    Label side_label = kInteriorLabel;
    // Approximate derivate of position with respect to the barycentric
    // coordinate that points in the directon of stroke travel in triangles that
    // include this vertex.
    Vec forward_derivative = {0, 0};
    // Vertex label to be used together with the `forward_derivative`.
    Label forward_label = kInteriorLabel;
    // Texture UV coordinates for winding textures.
    Point surface_uv;
    // Offset for texture animation progress, in the range [0, 1).
    float animation_offset = 0;
  };

  // Indices into `MeshFormat::Attributes()` for each stroke vertex attribute.
  //
  // A value of -1 indicates that the particular attribute is missing.
  struct FormatAttributeIndices {
    int8_t position = -1;
    int8_t opacity_shift = -1;
    int8_t hsl_shift = -1;
    int8_t side_derivative = -1;
    int8_t side_label = -1;
    int8_t forward_derivative = -1;
    int8_t forward_label = -1;
    int8_t surface_uv = -1;
    int8_t animation_offset = -1;
  };

  // Finds and returns the indices into `format.Attributes()` for each of the
  // attributes specific to stroke vertices.
  static FormatAttributeIndices FindAttributeIndices(const MeshFormat& format);

  // Attribute index constants for the "full" `MeshFormat` returned by
  // `FullMeshFormat()` below.
  static constexpr FormatAttributeIndices kFullFormatAttributeIndices = {
      .position = 0,
      .opacity_shift = 1,
      .hsl_shift = 2,
      .side_derivative = 3,
      .side_label = 4,
      .forward_derivative = 5,
      .forward_label = 6,
      .surface_uv = 7,
      .animation_offset = 8,
  };

  // The maximum number of `MeshFormat::Attribute`s that might be used by a
  // stroke. This equals the member variable count of `FormatAttributeIndices`.
  //
  // "Attribute" refers to the term used for ink `Mesh` and `MutableMesh` in
  // this context, which need not map 1:1 to the GPU attributes used by
  // rendering APIs.
  static constexpr int kMaxAttributeCount = 9;

  using CustomPackingArray =
      SmallArray<std::optional<MeshAttributeCodingParams>, kMaxAttributeCount>;

  // Returns the array of custom mesh packing parameters for the given
  // `mesh_format` for a stroke.
  //
  // Only color-shift and label attributes that have a recognized packed format
  // will be given non-nullopt values of custom packing parameters. Attributes
  // in `mesh_format` with an id found in `skipped_attribute_ids` will be
  // skipped. See also the comment on `PartitionedMesh::FromMutableMesh()`.
  //
  // CHECK-fails if the format has more than `kMaxAttributeCount` attributes.
  static CustomPackingArray MakeCustomPackingArray(
      const MeshFormat& mesh_format,
      absl::Span<const MeshFormat::AttributeId> skipped_attribute_ids = {});

  // Returns the mesh format using all of the attributes of the `StrokeVertex`.
  static MeshFormat FullMeshFormat();

  // The following helper functions interact with a `MutableMesh` to get,
  // append, and set the value of a `StrokeVertex` or a specific one of its
  // attributes.
  //
  // `MeshFormat::IsUnpackedEquivalent(mesh.Format(), FullMeshFormat())` should
  // return true.
  static StrokeVertex GetFromMesh(const MutableMesh& mesh, uint32_t index);
  static Vec GetSideDerivativeFromMesh(const MutableMesh& mesh, uint32_t index);
  static Vec GetForwardDerivativeFromMesh(const MutableMesh& mesh,
                                          uint32_t index);
  static Label GetSideLabelFromMesh(const MutableMesh& mesh, uint32_t index);
  static Label GetForwardLabelFromMesh(const MutableMesh& mesh, uint32_t index);
  static Point GetSurfaceUvFromMesh(const MutableMesh& mesh, uint32_t index);
  static float GetAnimationOffsetFromMesh(const MutableMesh& mesh,
                                          uint32_t index);
  static void AppendToMesh(MutableMesh& mesh, const StrokeVertex& vertex);
  static void SetInMesh(MutableMesh& mesh, uint32_t index,
                        const StrokeVertex& vertex);
  static void SetSideDerivativeInMesh(MutableMesh& mesh, uint32_t index,
                                      Vec derivative);
  static void SetForwardDerivativeInMesh(MutableMesh& mesh, uint32_t index,
                                         Vec derivative);
  static void SetSideLabelInMesh(MutableMesh& mesh, uint32_t index,
                                 Label label);
  static void SetForwardLabelInMesh(MutableMesh& mesh, uint32_t index,
                                    Label label);

  Point position;
  NonPositionAttributes non_position_attributes;
};

// Computes the linear interpolation between `a` and `b` when `t` is in the
// range [0, 1], and the linear extrapolation otherwise.
//
// Note that this naming follows the behavior of the C++20 `std::lerp`, which
// diverges from legacy code that would call this `Lerpnc`, with "nc"
// designating "non-clamping".
//
// Behavior for different kinds of attributes:
//   * Returned derivative attributes will always be zero as they must be
//     separately recalculated for any new or repositioned vertex.
//   * For each label attribute, the returned value will be:
//       * the value on `a` if `t <= 0`,
//       * the value on `b` if `t >= 1`,
//       * the value on `a` or `b` if the labels equal,
//       * `kInteriorLabel` otherwise.
//   * The returned animation offset attribute will just be the value on `a`. In
//     practice, we should only ever be lerping between vertices that already
//     have the same animation offset, because the animation offset should not
//     vary within a single particle or extrusion.
StrokeVertex::NonPositionAttributes Lerp(
    const StrokeVertex::NonPositionAttributes& a,
    const StrokeVertex::NonPositionAttributes& b, float t);

// Computes the attributes at the barycentric coordinates, `t`, interpolating
// between `a`, `b`, and `c`. See
// https://en.wikipedia.org/wiki/Barycentric_coordinate_system.
//
// Behavior for different kinds of attributes:
//   * Returned derivative attributes will always be zero as they must be
//     separately recalculated for any new or repositioned vertex.
//   * For each label attribute, the returned value will be:
//       * The linearly interpolated value between two sets of attributes if the
//         complimentary third value of `t` equals zero. This corresponds to `t`
//         lying on one of the lines coinciding with an edge of the triangle.
//       * `kInteriorLabel` otherwise.
//   * The returned animation offset attribute will just be the value on `a`. In
//     practice, we should only ever be lerping between vertices that already
//     have the same animation offset, because the animation offset should not
//     vary within a single particle or extrusion.
StrokeVertex::NonPositionAttributes BarycentricLerp(
    const StrokeVertex::NonPositionAttributes& a,
    const StrokeVertex::NonPositionAttributes& b,
    const StrokeVertex::NonPositionAttributes& c,
    const std::array<float, 3>& t);

inline bool operator==(StrokeVertex::Label a, StrokeVertex::Label b) {
  return a.encoded_value == b.encoded_value;
}

inline bool operator!=(StrokeVertex::Label a, StrokeVertex::Label b) {
  return !(a == b);
}

inline bool operator==(const StrokeVertex::NonPositionAttributes& a,
                       const StrokeVertex::NonPositionAttributes& b) {
  return a.opacity_shift == b.opacity_shift && a.hsl_shift == b.hsl_shift &&
         a.side_derivative == b.side_derivative &&
         a.side_label == b.side_label &&
         a.forward_derivative == b.forward_derivative &&
         a.forward_label == b.forward_label && a.surface_uv == b.surface_uv &&
         a.animation_offset == b.animation_offset;
}

inline StrokeVertex::SideCategory StrokeVertex::Label::DecodeSideCategory()
    const {
  return encoded_value < 0    ? StrokeVertex::SideCategory::kExteriorLeft
         : encoded_value == 0 ? StrokeVertex::SideCategory::kInterior
                              : StrokeVertex::SideCategory::kExteriorRight;
}

inline StrokeVertex::ForwardCategory
StrokeVertex::Label::DecodeForwardCategory() const {
  return encoded_value < 0    ? StrokeVertex::ForwardCategory::kExteriorFront
         : encoded_value == 0 ? StrokeVertex::ForwardCategory::kInterior
                              : StrokeVertex::ForwardCategory::kExteriorBack;
}

inline float StrokeVertex::Label::DerivativeOutsetSign() const {
  return (encoded_value > 0) - (encoded_value < 0);
}

}  // namespace ink::strokes_internal

#endif  // INK_STROKES_INTERNAL_STROKE_VERTEX_H_
