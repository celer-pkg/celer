// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

float getHeight(vec2 pos) {
    return textureLod(heightMap, pos, 0).r;
}

void MAIN()
{
    const float offset = 0.004;

    VERTEX.y += getHeight(UV0);
    TANGENT = normalize(vec3(0.0, getHeight(UV0 + vec2(0.0, offset)) - getHeight(UV0 + vec2(0.0, -offset)), offset * 2.0));
    BINORMAL = normalize(vec3(offset * 2.0, getHeight(UV0 + vec2(offset, 0.0)) - getHeight(UV0 + vec2(-offset, 0.0)), 0.0));
    NORMAL = cross(TANGENT, BINORMAL);
}
