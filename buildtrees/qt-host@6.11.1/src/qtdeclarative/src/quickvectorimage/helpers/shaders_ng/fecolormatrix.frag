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

    // red
    float m_0_0;
    float m_0_1;
    float m_0_2;
    float m_0_3;
    float m_0_4;

    // green
    float m_1_0;
    float m_1_1;
    float m_1_2;
    float m_1_3;
    float m_1_4;

    // blue
    float m_2_0;
    float m_2_1;
    float m_2_2;
    float m_2_3;
    float m_2_4;

    // alpha
    float m_3_0;
    float m_3_1;
    float m_3_2;
    float m_3_3;
    float m_3_4;
};

layout(binding = 1) uniform sampler2D source;

void main()
{
    vec4 color = texture(source, qt_TexCoord0.st);

    // Un-premultiply, avoiding divide by zero
    float epsilon = 0.0000001;
    color.rgb /= max(epsilon, color.a);

    float red = color.r * m_0_0
            + color.g * m_0_1
            + color.b * m_0_2
            + color.a * m_0_3
            + m_0_4;

    float green = color.r * m_1_0
            + color.g * m_1_1
            + color.b * m_1_2
            + color.a * m_1_3
            + m_1_4;

    float blue = color.r * m_2_0
            + color.g * m_2_1
            + color.b * m_2_2
            + color.a * m_2_3
            + m_2_4;

    float alpha = color.r * m_3_0
            + color.g * m_3_1
            + color.b * m_3_2
            + color.a * m_3_3
            + m_3_4;

    fragColor = vec4(red, green, blue, 1.0) * alpha;
}
