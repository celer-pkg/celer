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

#ifndef INK_STROKES_BRUSH_BRUSH_FAMILY_H_
#define INK_STROKES_BRUSH_BRUSH_FAMILY_H_

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_paint.h"
#include "ink/brush/brush_tip.h"

namespace ink {

// A `BrushFamily` combines one or more `BrushCoat`s with an optional
// client-specified string ID, which exists for the convenience of higher level
// serialization and asset management APIs.
class BrushFamily {
 public:
  // LINT.IfChange(input_model_types)

  // Spring-based input modeler. Stored in the `InputModel` variant below to
  // allow future input models to be added without changing the shape of
  // existing strokes.
  struct SpringModel {};

  // Model that attempts to preserve input positions as closely as possible.
  // This is an experimental configuration which may be adjusted or removed
  // later.
  struct ExperimentalRawPositionModel {};

  // Specifies a model for turning a sequence of raw hardware inputs (e.g. from
  // a stylus, touchscreen, or mouse) into a sequence of smoothed, modeled
  // inputs. Raw hardware inputs tend to be noisy, and must be smoothed before
  // being passed into a brush's behaviors and extruded into a mesh in order to
  // get a good-looking stroke.
  using InputModel = std::variant<SpringModel, ExperimentalRawPositionModel>;

  // Returns the default `InputModel` that will be used by
  // `BrushFamily::Create()` when none is specified.
  static InputModel DefaultInputModel();

  // LINT.ThenChange(../strokes/internal/stroke_input_modeler.cc:input_model_types)

  // Returns the maximum number of `BrushCoat`s that a `BrushFamily` is allowed
  // to have. Note that this limit may increase in the future.
  static uint32_t MaxBrushCoats();

  // Creates a `BrushFamily` with the given `tip`, `paint`, and optional
  // string `client_brush_family_id`.
  //
  // Performs validation of the `tip` and `paint`.
  //
  // For a tip to be valid the following must hold:
  //   * Every numeric, `Angle` or `Duration32` property must be finite and
  //     within valid bounds that may be specified in `BrushTip`,
  //     `BrushBehavior`, and `EasingFunction`.
  //   * Every enum property must be equal to one of the named enumerators for
  //     that property's type.
  //
  // For a brush paint to be valid the following must hold:
  //  * For each texture_layer the following must hold:
  //    - size components must be finite and greater than 0.
  //    - offset components must be in interval [0, 1].
  //    - rotation component must be finite.
  //    - size_jitter components must be smaller than or equal to their size
  //      counterparts.
  //    - offset_jitter must be in interval [0, 1].
  //    - rotation jitter must be a finite.
  //    - opacity must be in interval [0, 1].
  //    - animation_frames must be greater than 0.
  //    - For each TextureKeyframe the following must hold:
  //      ~ progress has to be in interval [0, 1].
  //      ~ size components, if present, must be finite and greater than 0.
  //      ~ offset components, if present, must be in interval [0, 1].
  //      ~ rotation, if present, must finite.
  //      ~ opacity, if present, must be in interval [0, 1].
  //  * All texture layers must have the same animation_frames value.
  //  * For now, all texture layers must use the same `TextureMapping` value.
  //    TODO: b/375203215 - Relax this requirement once we are able to mix
  //    rendering tiling and winding textures in a single `BrushPaint`.
  //  * Every enum property must be equal to one of the named enumerators for
  //    that property's type.
  static absl::StatusOr<BrushFamily> Create(
      const BrushTip& tip, const BrushPaint& paint,
      absl::string_view client_brush_family_id = "",
      const InputModel& input_model = DefaultInputModel());
  static absl::StatusOr<BrushFamily> Create(
      absl::Span<const BrushCoat> coats,
      absl::string_view client_brush_family_id = "",
      const InputModel& input_model = DefaultInputModel());

  // Constructs a brush-family with default tip and paint and empty ID.
  BrushFamily() = default;

  BrushFamily(const BrushFamily&) = default;
  BrushFamily(BrushFamily&&) = default;
  BrushFamily& operator=(const BrushFamily&) = default;
  BrushFamily& operator=(BrushFamily&&) = default;

  absl::Span<const BrushCoat> GetCoats() const;
  const InputModel& GetInputModel() const;

  // Returns the ID for this brush family specified by the client that
  // originally created it, or an empty string if no ID was specified. This is
  // considered when comparing `BrushFaily` objects for equality, but it is
  // not assumed that two `BrushFamily` objects with the same IDs are
  // equivalent, and the ID is not otherwise used internally by Ink.
  const std::string& GetClientBrushFamilyId() const;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const BrushFamily& family) {
    sink.Append(family.ToFormattedString());
  }

 private:
  BrushFamily(absl::Span<const BrushCoat> coats,
              absl::string_view client_brush_family_id,
              const InputModel& input_model);

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  std::vector<BrushCoat> coats_ = {BrushCoat{.tip = BrushTip{}}};
  std::string client_brush_family_id_;
  InputModel input_model_;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline absl::Span<const BrushCoat> BrushFamily::GetCoats() const {
  return coats_;
}

inline const std::string& BrushFamily::GetClientBrushFamilyId() const {
  return client_brush_family_id_;
}

inline const BrushFamily::InputModel& BrushFamily::GetInputModel() const {
  return input_model_;
}

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_FAMILY_H_
