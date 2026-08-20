#version 440
#extension GL_GOOGLE_include_directive : enable

#  include    "common.glsl"

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
