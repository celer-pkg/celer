// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vViewNormal;
VARYING vec3 vNormal;

void MAIN()
{
    vTexCoord = UV0;
    vColor = COLOR;
    vViewNormal = normalize((VIEW_MATRIX * vec4(NORMAL, 0.0)).xyz);
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
}
