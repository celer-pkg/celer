// Copyright (C) 2024 The Qt Company Ltd.*
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;

void MAIN()
{
    vec2 tc = texcoord;
    tc.y = 1.0 - tc.y;
    vec3 base = texture(fireTexture, tc).xyz * color.xyz;
    FRAGCOLOR = vec4(base.xyz, 1.0);
}
