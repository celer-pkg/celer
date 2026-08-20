// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#version 440

layout(location = 0) in vec4 vertexCoord;
layout(location = 1) in vec3 vertexBarycentric;
layout(location = 2) in vec3 normalExt; // x and y: normal vector; z: strokeWidth multiplier (default 1)
layout(location = 0) out vec3 barycentric;

layout(std140, binding = 0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 qt_Matrix[QSHADER_VIEW_COUNT];
#else
    mat4 qt_Matrix;
#endif

    float matrixScale;
    float opacity;
    float devicePixelRatio;
    float strokeWidth;

    vec4 strokeColor;

    float debug;
    float reserved5;
    float reserved6;
    float reserved7;
} ubuf;

#define SQRT2 1.41421356237

void main()
{
    // the subset of calculations from shapestroke.vert necessary to get gl_Position

    vec4 normalV = vec4(normalExt.xy, 0.0, 0.0);
    vec4 P = vertexCoord // center of stroke
            + normalV * abs(ubuf.strokeWidth) // expand to stroke width
            + normalV * SQRT2/ubuf.matrixScale; // AA

    if (ubuf.strokeWidth < 0.) {
        float adjStrokeWidth = abs(ubuf.strokeWidth) * ubuf.devicePixelRatio / ubuf.matrixScale;
        float strokeDiff = max(ubuf.devicePixelRatio / ubuf.matrixScale, adjStrokeWidth) *
                (ubuf.matrixScale / ubuf.devicePixelRatio - 1) / 2.;
        P -= normalV * strokeDiff; // cosmetic adjustment
    }

    barycentric = vertexBarycentric;
#if QSHADER_VIEW_COUNT >= 2
    gl_Position = ubuf.qt_Matrix[gl_ViewIndex] * P;
#else
    gl_Position = ubuf.qt_Matrix * P;
#endif
}
