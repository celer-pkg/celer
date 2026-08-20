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

#ifndef INK_RENDERING_SKIA_COMMON_INTERNAL_MESH_SPECIFICATION_DATA_
#define INK_RENDERING_SKIA_COMMON_INTERNAL_MESH_SPECIFICATION_DATA_

#include <cstdint>
#include <optional>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "ink/geometry/mesh_format.h"
#include "ink/types/small_array.h"

namespace ink::skia_common_internal {

// Platform-independent data that roughly mirrors, and can be used to create, an
// Android `graphics.MeshSpecification` or a C++ `SkMeshSpecification`.
//
// This type also includes information about uniforms, which can be used after
// construction of the platform-specific specification. On Android, this
// information is needed because the `graphics.MeshSpecification` API does not
// include a way to query for available uniforms.
//
// These enums and constants should match CanvasMeshRenderer.kt.
//
// TODO: b/284117747 - Use this type in place of `ink::SkiaMeshFormat`.
struct MeshSpecificationData {
  //
  // Skia limits the number of attributes to 8 and the number of varyings to 6
  // (see https://api.skia.org/classSkMeshSpecification.html).
  static constexpr int kMaxAttributes = 8;
  static constexpr int kMaxVaryings = 6;
  // Skia doesn't seem to place any clear limit on the number of uniforms, so
  // this value is just the size we choose to use for our array. Currently it is
  // set to the actual number of uniforms we happen to use right now.
  static constexpr int kMaxUniforms = 10;

  // Subsets of shader variable types for attributes, varyings, and uniforms
  // that are used by Ink and available across platforms.
  //
  // Note that enumerators with equivalent names are given the same underlying
  // value to simplify passing the values across the JNI boundary.
  enum class AttributeType {
    kFloat2 = 1,
    kFloat3 = 2,
    kUByte4 = 4,
  };
  enum class VaryingType {
    kFloat2 = 1,
    kFloat4 = 3,
  };
  enum class UniformType {
    kFloat = 0,
    kFloat4 = 3,
    kInt = 5,
  };

  enum class UniformId {
    // The 2D linear component of an `AffineTransform` that describes the
    // complete transformation from "object" coordinates to canvas coordinates.
    // This requires that the object-to-canvas matrix used during drawing is an
    // affine transform.
    kObjectToCanvasLinearComponent = 0,
    // The unpremultiplied, gamma-encoded RGBA value `Brush::GetColor()`.
    kBrushColor = 1,
    // Transform parameters used to convert packed attribute values to back to
    // their original values. See `Mesh::VertexAttributeUnpackingParams()`.
    kPositionUnpackingTransform = 2,
    kSideDerivativeUnpackingTransform = 3,
    kForwardDerivativeUnpackingTransform = 4,
    // The `BrushPaint::TextureMapping` value.
    // TODO: b/375203215 - Get rid of this uniform once we are able to mix
    // tiling and winding textures in a single `BrushPaint`.
    kTextureMapping = 5,
    // The current progress, a float in [0, 1], of the texture animation.
    //
    // We must pass both animation progress and number of frames to the shader,
    // rather than computing a frame index from these on the CPU and passing
    // only that. Why? Each particle in a stroke can have a different progress
    // offset, and these offsets are not quantized to animation frame
    // boundaries. Therefore the conversion to frame indices depends on both the
    // stroke-wide progress and the per-particle offset, the latter of which is
    // only available in the vertex shader.
    kTextureAnimationProgress = 6,
    // The number of frames in the texture animation.
    kNumTextureAnimationFrames = 7,
    kNumTextureAnimationRows = 8,
    kNumTextureAnimationColumns = 9,
  };

  struct Attribute {
    AttributeType type;
    int32_t offset;
    absl::string_view name;
  };

  struct Varying {
    VaryingType type;
    absl::string_view name;
  };

  struct Uniform {
    UniformType type;
    UniformId id;
    // If the uniform represents an attribute unpacking transform, this value
    // gives the index of the associated vertex attribute in the `Mesh`.
    std::optional<int> unpacking_attribute_index;
  };

  // Returns the uniform name for a given `id`.
  //
  // It will return an empty `string_view` if `id` is not one of the named
  // enumerators of `UniformId`.
  static absl::string_view GetUniformName(UniformId id);

  // TODO: b/284117747 - The following factory functions for strokes will need a
  // `const BrushFamily&` parameter to determine texture mapping and other
  // rendering options that may use two identical `MeshFormat`s differently.

  // Returns the mesh specification data for an `InProgressStroke`.
  //
  // This function should be the preferred way to get the specification data for
  // native C++ Skia rendering, where we will be sure that we are drawing an
  // `InProgressStroke`.
  static MeshSpecificationData CreateForInProgressStroke();

  // Returns data for rendering a `MutableMesh` created by an
  // `InProgressStroke`.
  //
  // This function is useful for JNI code that may not receive a complete
  // `InProgressStroke` object as part of its API, and may not be certain that a
  // `MeshFormat` or `MutableMesh` came from an `InProgressStroke`.
  //
  // Returns an invalid argument error if `mesh_format` is not the format used
  // by `InProgressStroke`.
  static absl::StatusOr<MeshSpecificationData> CreateForInProgressStroke(
      const MeshFormat& mesh_format);

  // Returns data for rendering a `PartitionedMesh` created for a `Stroke`.
  //
  // Unlike the two overloads for `InProgressStroke`, this function accepting a
  // `MeshFormat` is the only way to get specification data for a `Stroke`.
  // This is because a `Stroke` may be constructed with a deserialized
  // `PartitionedMesh`, in which case there is no guarantee for the rendering
  // compatibility of the `MeshFormat` it contains.
  //
  // Returns an invalid argument error if `mesh_format` is not supported.
  // Support requirements:
  //   * The format must contain all properties of an `InProgressStroke` vertex
  //     with the exception of HSL color shift, which is optional.
  //   * The following `MeshFormat::Attribute`s must be at adjacent indices in
  //     the format:
  //       a. position immediately followed by opacity-shift
  //       b. side-derivative immediately followed by side-label
  //       c. forward-derivative immediately followed by forward-label
  //   * The packed `MeshFormat::AttributeType` must be supported, including the
  //     combination of types for each tuple of named `MeshFormat::Attribute`s
  //     above.
  //
  // The packed representation of the `MeshFormat` used by `InProgressStroke`
  // will always be supported.
  static absl::StatusOr<MeshSpecificationData> CreateForStroke(
      const MeshFormat& mesh_format);

  SmallArray<Attribute, kMaxAttributes> attributes;
  int32_t vertex_stride;
  SmallArray<Varying, kMaxVaryings> varyings;
  SmallArray<Uniform, kMaxUniforms> uniforms;
  std::string vertex_shader_source;
  std::string fragment_shader_source;
};

}  // namespace ink::skia_common_internal

#endif  // INK_RENDERING_SKIA_COMMON_INTERNAL_MESH_SPECIFICATION_DATA_
