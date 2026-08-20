// Copyright 2024-2025 Google LLC
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

#include "ink/strokes/internal/jni/stroke_input_jni_helper.h"

#include <jni.h>

#include "ink/geometry/angle.h"
#include "ink/jni/internal/jni_defines.h"
#include "ink/jni/internal/jni_jvm_interface.h"
#include "ink/strokes/input/stroke_input.h"
#include "ink/types/duration.h"
#include "ink/types/physical_distance.h"

namespace ink::jni {

namespace {

jobject ToolTypeToJObjectOrThrow(JNIEnv* env, StrokeInput::ToolType tool_type) {
  return env->CallStaticObjectMethod(ClassInputToolType(env),
                                     MethodInputToolTypeFromInt(env),
                                     ToolTypeToJInt(tool_type));
}

}  // namespace

// Convert an int to an StrokeInput::ToolType enum.
//
// This should match the enum in InputToolType.kt.
StrokeInput::ToolType JIntToToolType(jint val) {
  return static_cast<StrokeInput::ToolType>(val);
}

jint ToolTypeToJInt(StrokeInput::ToolType type) {
  switch (type) {
    case StrokeInput::ToolType::kMouse:
      return 1;
    case StrokeInput::ToolType::kTouch:
      return 2;
    case StrokeInput::ToolType::kStylus:
      return 3;
    default:
      return 0;
  }
}

void UpdateJObjectInputOrThrow(JNIEnv* env, const StrokeInput& input_in,
                               jobject j_input_out) {
  jobject j_inputtooltype = ToolTypeToJObjectOrThrow(env, input_in.tool_type);
  if (env->ExceptionCheck()) return;

  jlong elapsed_time_millis = input_in.elapsed_time.ToMillis();
  env->CallVoidMethod(
      j_input_out, MethodStrokeInputUpdate(env), input_in.position.x,
      input_in.position.y, elapsed_time_millis, j_inputtooltype,
      input_in.stroke_unit_length.ToCentimeters(), input_in.pressure,
      input_in.tilt.ValueInRadians(), input_in.orientation.ValueInRadians());
}

}  // namespace ink::jni
