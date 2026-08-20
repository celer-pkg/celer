// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec3 pos;
VARYING vec2 coord;

void MAIN()
{
    vec3 rgb = vec3(pos.x * 0.02, pos.y * 0.02, pos.z * 0.02) * texture(tex, coord).rgb;
    FRAGCOLOR = vec4(rgb, alpha);
}
