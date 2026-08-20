#version 440
#extension GL_GOOGLE_include_directive : enable

#include "common.glsl" // The followup comment
#include "inner/common2.glsl" // The followup comment with "
// #include "common.glsl" commented line

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = getColor();
}
