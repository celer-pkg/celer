// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec3 pos;
VARYING vec2 coord;

void MAIN()
{
    pos = VERTEX;
    pos.x += sin(time * 4.0 + pos.y) * amplitude;
    coord = UV0;
    POSITION = INSTANCE_MODELVIEWPROJECTION_MATRIX * vec4(pos, 1.0);
}
