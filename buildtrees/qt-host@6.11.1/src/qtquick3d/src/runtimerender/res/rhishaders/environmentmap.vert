// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout (location = 0) in vec3 attr_pos;
layout (location = 0) out vec3 localPos;

layout(std140, binding = 0) uniform buf {
    mat4 projection;
    mat4 view;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    localPos = attr_pos;
    gl_Position = ubuf.projection * ubuf.view * vec4(localPos, 1.0);
}
