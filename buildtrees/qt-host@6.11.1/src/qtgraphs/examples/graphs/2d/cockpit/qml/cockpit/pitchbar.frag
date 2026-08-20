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
};


void main() {
    vec2 uv = qt_TexCoord0;

    uv.y *= iResolution.y / iResolution.x;

    float b = length(uv - 0.5);
    b = step(b, 0.5);
    float k = step(uv.y, 1.0);
    float a = step(1 - qt_TexCoord0.y, 0.9) * (1 - qt_TexCoord0.y);
    a += b;
    k *= a * b;
    fragColor = vec4(k,k,k,a);
}
