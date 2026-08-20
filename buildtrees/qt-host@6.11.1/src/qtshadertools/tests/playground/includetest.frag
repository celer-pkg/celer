#version 440
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
#include "fragcolor.inc"
}
