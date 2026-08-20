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

#ifndef INK_STROKES_BRUSH_BRUSH_H_
#define INK_STROKES_BRUSH_BRUSH_H_

#include <cstdint>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "ink/brush/brush_coat.h"
#include "ink/brush/brush_family.h"
#include "ink/color/color.h"

namespace ink {

// A `Brush` defines how stroke inputs are interpreted to create the visual
// representation of a stroke.
//
// The type completely describes how inputs are used to create stroke meshes,
// and how those meshes should be drawn by stroke renderers. In an analogous way
// to "font" and "font family", a `Brush` can be considered an instance of a
// `BrushFamily` with a particular color, size, and an extra parameter
// controlling visual fidelity, called epsilon.
class Brush {
 public:
  // Creates a `Brush` with the given `family`, `color`, `size` and `epsilon`.
  //
  // The value of `size` determines the overall thickness of strokes created
  // with a given brush.
  //
  // The value of `epsilon` determines the visual fidelity of strokes created
  // with a given brush. It is the smallest distance for which two points should
  // be considered visually distinct. Lower values of `epsilon` result in higher
  // fidelity strokes at the cost of somewhat higher memory usage.
  //
  // The values of `size` and `epsilon` should be in the same units as those of
  // `StrokeInput::x` and `::y`.
  //
  // Returns an error if either `size` or `epsilon` is not a finite, positive
  // value or if `size` is smaller than `epsilon`.
  static absl::StatusOr<Brush> Create(const BrushFamily& family,
                                      const Color& color, float size,
                                      float epsilon);

  // Constructs a brush with a default `BrushFamily` and placeholder color, size
  // and epsilon values.
  Brush() = default;

  Brush(const Brush&) = default;
  Brush(Brush&&) = default;
  Brush& operator=(const Brush&) = default;
  Brush& operator=(Brush&&) = default;

  void SetFamily(const BrushFamily& family);
  const BrushFamily& GetFamily() const;

  uint32_t CoatCount() const { return GetCoats().size(); }
  absl::Span<const BrushCoat> GetCoats() const;

  void SetColor(const Color& color);
  const Color& GetColor() const;

  // Sets the `size` of the brush if the value is finite and positive.
  // Otherwise, returns an error and leaves the brush unmodified.
  absl::Status SetSize(float size);
  float GetSize() const;

  // Sets the `epsilon` value for the brush if the value is finite and positive.
  // Otherwise, returns an error and leaves the brush unmodified.
  absl::Status SetEpsilon(float epsilon);
  float GetEpsilon() const;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Brush& brush) {
    sink.Append(brush.ToFormattedString());
  }

 private:
  Brush(const BrushFamily& family, const Color& color, float size,
        float epsilon);

  // Implementation helper for AbslStringify.
  std::string ToFormattedString() const;

  BrushFamily family_;
  Color color_ = Color::Red();
  float size_ = 1;
  float epsilon_ = 0.1f;
};

// ---------------------------------------------------------------------------
//                     Implementation details below

inline void Brush::SetFamily(const BrushFamily& family) { family_ = family; }

inline const BrushFamily& Brush::GetFamily() const { return family_; }

inline absl::Span<const BrushCoat> Brush::GetCoats() const {
  return family_.GetCoats();
}

inline void Brush::SetColor(const Color& color) { color_ = color; }

inline const Color& Brush::GetColor() const { return color_; }

inline float Brush::GetSize() const { return size_; }

inline float Brush::GetEpsilon() const { return epsilon_; }

}  // namespace ink

#endif  // INK_STROKES_BRUSH_BRUSH_H_
