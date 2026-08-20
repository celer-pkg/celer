// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440
layout (location = 0) in vec3 aPos;
layout (location = 0) out vec3 localPos;

layout(std140, binding = 0) uniform buf {
    mat4 projection;
    mat4 view;
} ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(ubuf.view));
    vec4 clipPos = ubuf.projection * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}
