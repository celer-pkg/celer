// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) in vec3 attr_pos;
layout(location = 1) in vec2 attr_uv;

layout(location = 0) out vec2 uv_coord;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    uv_coord = attr_uv;
    gl_Position = vec4(attr_pos, 1.0);
}
