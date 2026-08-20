// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec3 attr_pos;
layout(location = 1) in vec3 attr_color;

layout(std140, binding = 0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 viewProjection[QSHADER_VIEW_COUNT];
#else
    mat4 viewProjection;
#endif
} ubuf;

layout(location = 0) out vec3 var_color;

out gl_PerVertex { vec4 gl_Position; float gl_PointSize;};

void main()
{
    var_color = attr_color;
#if QSHADER_VIEW_COUNT >= 2
    gl_Position = ubuf.viewProjection[gl_ViewIndex] * vec4(attr_pos, 1.0);
#else
    gl_Position = ubuf.viewProjection * vec4(attr_pos, 1.0);
#endif
    gl_PointSize = 4.0;
}
