// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec3 attr_pos;

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

// skyboxProperties
// x: adjustY
// y: exposure
// z: blurAmount
// w: maxMipLevel

layout(location = 0) out vec3 eye_direction;

void main()
{
    gl_Position = vec4(attr_pos, 1.0);
#if QSHADER_VIEW_COUNT >= 2
    vec3 unprojected = (ubuf.inverseProjection[gl_ViewIndex] * gl_Position).xyz;
    eye_direction = normalize(ubuf.viewMatrix[gl_ViewIndex] * unprojected);
#else
    vec3 unprojected = (ubuf.inverseProjection * gl_Position).xyz;
    eye_direction = normalize(ubuf.viewMatrix * unprojected);
#endif
    eye_direction = normalize(ubuf.orientation * eye_direction);
    gl_Position.y *= ubuf.skyboxProperties.x;
}
