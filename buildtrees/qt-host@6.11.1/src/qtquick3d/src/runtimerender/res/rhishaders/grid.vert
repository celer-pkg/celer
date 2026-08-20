// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec3 attr_pos;

layout(location=0) out vec3 v_nearPoint;
layout(location=1) out vec3 v_farPoint;
layout(location=2) out flat uint v_viewIndex;

layout(std140, binding=0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 viewProj[QSHADER_VIEW_COUNT];
    mat4 invViewProj[QSHADER_VIEW_COUNT];
#else
    mat4 viewProj;
    mat4 invViewProj;
#endif
    float near;
    float far;
    float scale;
    float yFactor;
    uint gridFlags;
} u_buf;

vec3 unprojectPoint(float x, float y, float z, mat4 invViewProj) {

    vec4 unprojectedPoint = invViewProj * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz/unprojectedPoint.w;
}

void main()
{
    vec3 p = attr_pos;

#if QSHADER_VIEW_COUNT >= 2
    v_nearPoint = unprojectPoint(p.x, u_buf.yFactor * p.y, 0.0, u_buf.invViewProj[gl_ViewIndex]).xyz;
    v_farPoint = unprojectPoint(p.x, u_buf.yFactor * p.y, 1.0, u_buf.invViewProj[gl_ViewIndex]).xyz;
    v_viewIndex = gl_ViewIndex;
#else
    v_nearPoint = unprojectPoint(p.x, u_buf.yFactor * p.y, 0.0, u_buf.invViewProj).xyz;
    v_farPoint = unprojectPoint(p.x, u_buf.yFactor * p.y, 1.0, u_buf.invViewProj).xyz;
    v_viewIndex = 0;
#endif

    gl_Position = vec4(p, 1.0);
}
