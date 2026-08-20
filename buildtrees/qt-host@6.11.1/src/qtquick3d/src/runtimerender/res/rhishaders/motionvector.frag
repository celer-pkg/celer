// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#version 440

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vPrevPosition;

layout (location = 0) out vec4 fragColor;

layout (std140, binding = 0) uniform buf {
    mat4 mvp;
    mat4 prevMVP;
    mat4 instanceLocal;
    mat4 instanceGlobal;
    mat4 prevInstanceLocal;
    mat4 prevInstanceGlobal;
    vec4 currentAndLastJitter;
    float velocityAmount;
    int morphTargetCount;
    vec2 padding;
} ubuf;


void main()
{
    vec2 a = (vPosition.xy / vPosition.w);
    vec2 b = (vPrevPosition.xy / vPrevPosition.w);

    a -= ubuf.currentAndLastJitter.zw;
    b -= ubuf.currentAndLastJitter.xy;

    a = a * 0.5 - 0.5;
    b = b * 0.5 - 0.5;

    vec2 v;
    v.x = b.x - a.x;

    #if defined(QSHADER_HLSL) || defined(QSHADER_MSL)
        v.y = b.y - a.y;
    #else
        v.y = a.y - b.y;
    #endif

    fragColor = vec4(v * ubuf.velocityAmount, v);
}
