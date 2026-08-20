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
    bool isAlpha;
    bool isInverted;
};

layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D maskSource;

void main()
{
    vec4 maskSample = texture(maskSource, qt_TexCoord0.st);
    float maskVal = isAlpha ? maskSample.a
                            : 0.2126 * maskSample.r + 0.7152 * maskSample.g + 0.0722 * maskSample.b;
    maskVal = isInverted ? (1 - maskVal) : maskVal;
    fragColor = texture(source, qt_TexCoord0.st) * maskVal * qt_Opacity;
}
