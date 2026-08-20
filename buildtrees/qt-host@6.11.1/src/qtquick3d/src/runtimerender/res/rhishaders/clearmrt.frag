// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) out vec4 fragOutput0;
layout(location = 1) out vec4 fragOutput1;

layout(std140, binding = 0) uniform buf {
    vec4 clearColor0;
    vec4 clearColor1;
} ubuf;

void main()
{
    fragOutput0 = ubuf.clearColor0;
    fragOutput1 = ubuf.clearColor1;
}
