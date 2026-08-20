// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#version 440

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 color;
    vec3 iResolution;
    vec2 rectSize;
    float radius;
    float blur;
};

float roundedBox(vec2 centerPos, vec2 size, float radii) {
    return length(max(abs(centerPos) - size + radii, 0.0)) - radii;
}

void main()
{
    float box = roundedBox(fragCoord - iResolution.xy * 0.5, rectSize, radius);
    float a = 1.0 - smoothstep(0.0, blur, box);
    fragColor = color * qt_Opacity * a * a;
}
