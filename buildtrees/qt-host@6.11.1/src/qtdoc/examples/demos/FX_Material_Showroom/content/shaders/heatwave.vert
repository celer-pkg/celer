// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vNormal;
VARYING vec3 vViewVec;
VARYING float vFresnel;

void MAIN()
{
    vTexCoord = UV0;
    vColor = COLOR;
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
    vViewVec = normalize(CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz);
    vFresnel = -dot(vViewVec, vNormal);
}
