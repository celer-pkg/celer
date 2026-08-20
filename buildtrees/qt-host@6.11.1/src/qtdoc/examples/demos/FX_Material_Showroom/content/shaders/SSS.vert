// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

precision lowp int;
precision lowp float;

VARYING vec3 vColor;
VARYING vec3 vPos;
VARYING vec3 vNormal;
VARYING vec3 vViewVec;
VARYING vec2 vTexcoord;

void MAIN()
{
    vColor = COLOR.rgb;
    vPos = VERTEX.xyz;
    vNormal = normalize(NORMAL);
    vTexcoord = UV0;
    vViewVec = normalize(CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz);
}
