// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

QC_INCLUDE "common.glsl"

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 tcoord;
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec2 fragCoord;

layout(std140, binding = 0) uniform vertUBuf {
    vec4 viewRect;
    int ndcIsYDown;
#ifdef CUSTOM_MATRIX
    mat4 customMatrix;
#endif
};

layout(std140, binding = 4) uniform vertUBuf2 {
    mat3 vertMatrix;
};

void main()
{
    texCoord = tcoord;
    fragCoord = vertex;
    vec2 v = (vertMatrix * vec3(vertex, 1.0)).xy;

#ifdef CUSTOM_MATRIX
    gl_Position = customMatrix * vec4(v, 0.0, 1.0);
#else
    if (ndcIsYDown != 0)
        gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0, -1.0 + 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
    else
        gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0, 1.0 - 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
#endif
}
