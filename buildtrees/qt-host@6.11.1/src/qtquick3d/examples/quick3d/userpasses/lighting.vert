// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;

void MAIN()
{
    texcoord = UV0;
    vec2 ndc = UV0 * 2.0 - 1.0;
    ndc.y *= NDC_Y_UP;
    POSITION = vec4(ndc.x, ndc.y, 0.0, 1.0);
}
