// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#version 440

layout(location = 0) in vec4 qt_Vertex;
layout(location = 1) in vec2 qt_MultiTexCoord0;
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec2 fragCoord;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec3 iResolution;
    vec4 color;
    float lineWidth;
    float smoothing;
};

out gl_PerVertex { vec4 gl_Position; };

#define HORIZONTAL 0


void main() {
    texCoord = qt_MultiTexCoord0;
    fragCoord = qt_Vertex.xy;
    vec2 vertCoord = qt_Vertex.xy;
    gl_Position = qt_Matrix * vec4(vertCoord, 0.0, 1.0);
}
