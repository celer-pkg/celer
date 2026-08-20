// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec3 attr_pos;

layout(location = 0) out vec3 tex_coords;

// the layout must match between the skybox and skyboxcube shaders
layout(std140, binding = 0) uniform buf {
    vec4 skyboxProperties;
    mat3 orientation;
#if QSHADER_VIEW_COUNT >= 2
    mat4 viewProjection[QSHADER_VIEW_COUNT];
    mat4 inverseProjection[QSHADER_VIEW_COUNT];
    mat3 viewMatrix[QSHADER_VIEW_COUNT];
#else
    mat4 viewProjection;
    mat4 inverseProjection;
    mat3 viewMatrix;
#endif
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    //mirror texture lookup since we're looking from the inside of the cube
    tex_coords = attr_pos * vec3(-1, 1, 1);

#if QSHADER_VIEW_COUNT >= 2
    vec4 pos = ubuf.viewProjection[gl_ViewIndex] * vec4(attr_pos, 1.0);
#else
    vec4 pos = ubuf.viewProjection * vec4(attr_pos, 1.0);
#endif
    pos.y *= ubuf.skyboxProperties.x;
    gl_Position = pos.xyww;
}
