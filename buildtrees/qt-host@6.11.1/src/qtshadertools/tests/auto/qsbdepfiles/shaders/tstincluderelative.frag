#version 440
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl"
#include "inner/common2.glsl"

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = getColor();
  fragColor = getColor2();
}
