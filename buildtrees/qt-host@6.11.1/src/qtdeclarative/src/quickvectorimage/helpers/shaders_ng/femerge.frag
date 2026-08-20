// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    // qt_Matrix and qt_Opacity must always be both present
    // if the built-in vertex shader is used.
    mat4 qt_Matrix;
    float qt_Opacity;

    int sourceCount;
};

layout(binding = 1) uniform sampler2D source1;
layout(binding = 2) uniform sampler2D source2;
layout(binding = 3) uniform sampler2D source3;
layout(binding = 4) uniform sampler2D source4;
layout(binding = 5) uniform sampler2D source5;
layout(binding = 6) uniform sampler2D source6;
layout(binding = 7) uniform sampler2D source7;
layout(binding = 8) uniform sampler2D source8;

vec4 compositeOver(vec4 Cb, vec4 Cs)
{
    vec3 co = Cs.a * Cs.rgb + Cb.a * Cb.rgb * (1.0 - Cs.a);
    float ao = Cs.a + Cb.a * (1.0 - Cs.a);

    return vec4(co, ao);
}

void main()
{
    vec4 destColor = texture(source1, qt_TexCoord0.st);
    if (sourceCount > 1)
        destColor = compositeOver(destColor, texture(source2, qt_TexCoord0.st));
    if (sourceCount > 2)
        destColor = compositeOver(destColor, texture(source3, qt_TexCoord0.st));
    if (sourceCount > 3)
        destColor = compositeOver(destColor, texture(source4, qt_TexCoord0.st));
    if (sourceCount > 4)
        destColor = compositeOver(destColor, texture(source5, qt_TexCoord0.st));
    if (sourceCount > 5)
        destColor = compositeOver(destColor, texture(source6, qt_TexCoord0.st));
    if (sourceCount > 6)
        destColor = compositeOver(destColor, texture(source7, qt_TexCoord0.st));
    if (sourceCount > 7)
        destColor = compositeOver(destColor, texture(source8, qt_TexCoord0.st));

    fragColor = destColor;
}
