// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 450 core

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout (std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    vec3 iResolution;
    float iTime;
};


void main() {
    vec2 uv = qt_TexCoord0;
    uv.x *= iResolution.x / iResolution.y;

    uv -= vec2(0.5);
    float a = length(uv);
    a *= 20.;
    a = abs(sin(a - iTime));

    a *= 1. - length(uv * 2.);

    fragColor = vec4(a,a,a,a);
}
