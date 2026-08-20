// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec3 vNormal;
VARYING vec3 vViewVec;

void MAIN()
{
    VERTEX.x += sin(uTime * 4.0 + VERTEX.y) * uAmplitude;
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
    vViewVec = CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
