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

#include "ink/strokes/input/stroke_input.h"

#include <string>

#include "absl/strings/str_cat.h"

namespace ink {
namespace stroke_input_internal {

std::string ToFormattedString(StrokeInput::ToolType tool_type) {
  switch (tool_type) {
    case StrokeInput::ToolType::kMouse:
      return "Mouse";
    case StrokeInput::ToolType::kTouch:
      return "Touch";
    case StrokeInput::ToolType::kStylus:
      return "Stylus";
    case StrokeInput::ToolType::kUnknown:
      break;
      // Don't include a default case, so that we get a compile error if we've
      // forgotten a case.
  }
  return "Unknown";
}

std::string ToFormattedString(const StrokeInput& input) {
  std::string formatted =
      absl::StrCat("StrokeInput[", input.tool_type, ", ", input.position, ", ",
                   input.elapsed_time);
  if (input.HasStrokeUnitLength()) {
    absl::StrAppend(&formatted,
                    ", stroke_unit_length=", input.stroke_unit_length);
  }
  if (input.HasPressure()) {
    absl::StrAppend(&formatted, ", pressure=", input.pressure);
  }
  if (input.HasTilt()) {
    absl::StrAppend(&formatted, ", tilt=", input.tilt);
  }
  if (input.HasOrientation()) {
    absl::StrAppend(&formatted, ", orientation=", input.orientation);
  }
  formatted.push_back(']');
  return formatted;
}

}  // namespace stroke_input_internal
}  // namespace ink
