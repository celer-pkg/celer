// Copyright (C) 2024 The Qt Company Ltd.*
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;

void MAIN()
{
    float val = clamp(pow(1.0 - texcoord.y, 20.) * mix(30., 5., sin(time)), 0., 1.0);
    if (val < 0.01)
        discard;
    FRAGCOLOR = vec4(mix(vec3(0.), indicatorColor, val), val);
}
