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

#ifndef INK_STROKES_INPUT_STROKE_INPUT_H_
#define INK_STROKES_INPUT_STROKE_INPUT_H_

#include <cstdint>
#include <string>

#include "ink/geometry/angle.h"
#include "ink/geometry/point.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink {

// A single stroke input specifying input type, position, and time, as well as
// optional pressure, tilt, and/or orientation.
struct StrokeInput {
  // Input devices that can be used to generate stroke inputs.  These are
  // roughly analogous to the Android `MotionEvent.TOOL_TYPE_*` constants
  // (though not with the same integer values).
  //
  // This should match the enum in InputToolType.kt and BrushExtensions.kt.
  enum class ToolType : int8_t { kUnknown, kMouse, kTouch, kStylus };

  static constexpr PhysicalDistance kNoStrokeUnitLength =
      PhysicalDistance::Zero();
  static constexpr float kNoPressure = -1;
  static constexpr Angle kNoTilt = Angle::Radians(-1);
  static constexpr Angle kNoOrientation = Angle::Radians(-1);

  bool HasStrokeUnitLength() const {
    return stroke_unit_length != kNoStrokeUnitLength;
  }
  bool HasPressure() const { return pressure != kNoPressure; }
  bool HasTilt() const { return tilt != kNoTilt; }
  bool HasOrientation() const { return orientation != kNoOrientation; }

  // The input device used to generate this stroke input.
  ToolType tool_type = ToolType::kUnknown;
  // The input position, in stroke space.
  Point position;
  // Time elapsed since the start of a stroke.
  Duration32 elapsed_time;
  // The physical distance that the pointer must travel in order to produce an
  // input motion of one stroke unit. For stylus/touch, this is the real-world
  // distance that the stylus/fingertip must move in physical space; for mouse,
  // this is the visual distance that the mouse pointer must travel along the
  // surface of the display.
  //
  // A value of kNoStrokeUnitLength indicates that the relationship between
  // stroke space and physical space is unknown or ill-defined.
  PhysicalDistance stroke_unit_length = kNoStrokeUnitLength;
  // Pressure value in the normalized, unitless range of [0, 1] indicating the
  // force exerted during input.
  //
  // A value of kNoPressure indicates that pressure is not reported.
  float pressure = kNoPressure;
  // Tilt is the angle between a stylus and the line perpendicular to the plane
  // of the screen. The value should be normalized to fall between 0 and π/2 in
  // radians, where 0 is perpendicular to the screen and π/2 is flat against the
  // drawing surface, but can be set with either radians or degrees.
  //
  // kNotTilt indicates that tilt is not reported.
  Angle tilt = kNoTilt;
  // Orientation is the angle that indicates the direction in which the stylus
  // is pointing in relation to the positive x axis. The value should be
  // normalized to fall between 0 and 2π in radians, where 0 means the ray from
  // the stylus tip to the end is along positive x and values increase towards
  // the positive y-axis, but can be set with either radians or degrees.
  //
  // kNoOrientation indicates that orientation is not reported. Note, that this
  // is a separate condition from the orientation being indeterminant when
  // `tilt` is π/2.
  Angle orientation = kNoOrientation;
};

namespace stroke_input_internal {
std::string ToFormattedString(StrokeInput::ToolType tool_type);
std::string ToFormattedString(const StrokeInput& input);
}  // namespace stroke_input_internal

template <typename Sink>
void AbslStringify(Sink& sink, StrokeInput::ToolType tool_type) {
  sink.Append(stroke_input_internal::ToFormattedString(tool_type));
}

template <typename Sink>
void AbslStringify(Sink& sink, const StrokeInput& input) {
  sink.Append(stroke_input_internal::ToFormattedString(input));
}

}  // namespace ink

#endif  // INK_STROKES_INPUT_STROKE_INPUT_H_
