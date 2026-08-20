// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 450 core

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout (std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    float iTime;
    vec4 iColor;
    vec3 iResolution;
};


void main() {
    vec2 uv = qt_TexCoord0;

    uv *= 6. * vec2(iResolution.x / iResolution.y, 1.);
    uv = fract(uv);
    uv -= 0.5;

    float k = length(uv);
    k = step(k, 0.4);

    fragColor = iColor * k;
}
