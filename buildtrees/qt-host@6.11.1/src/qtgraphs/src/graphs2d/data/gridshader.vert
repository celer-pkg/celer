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
    int origo;
    vec4 gridVisibility;
    float gridWidth;
    float gridHeight;
    vec2 gridMovement;
    vec4 subGridColor;
    vec4 gridColor;
    float subGridLineWidth;
    float gridLineWidth;
    float smoothing;
    float verticalSubGridScale;
    float horizontalSubGridScale;
    vec4 plotAreaBackgroundColor;
};

out gl_PerVertex { vec4 gl_Position; };


void main() {
    texCoord = qt_MultiTexCoord0;
    if (origo == 0)
        fragCoord = vec2(qt_Vertex.x, iResolution.y - qt_Vertex.y);
    else if (origo == 1)
        fragCoord = qt_Vertex.xy;
    else if (origo == 2)
        fragCoord = vec2(iResolution.x - qt_Vertex.x, qt_Vertex.y);
    else
        fragCoord = vec2(iResolution.x - qt_Vertex.x, iResolution.y - qt_Vertex.y);
    fragCoord += gridMovement;
    vec2 vertCoord = qt_Vertex.xy;
    gl_Position = qt_Matrix * vec4(vertCoord, 0.0, 1.0);
}
