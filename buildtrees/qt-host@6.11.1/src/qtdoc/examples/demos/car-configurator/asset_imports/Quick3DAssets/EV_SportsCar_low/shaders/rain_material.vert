// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec3 normal;
VARYING vec3 binormal;
VARYING vec3 modelPos;
VARYING vec2 texcoord;

void MAIN()
{
    vec3 norm = normalize(NORMAL_MATRIX * NORMAL);
    vec3 pos = vec3((MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz);
    vec3 view = normalize(vec3(CAMERA_POSITION - pos));
    vec3 up = vec3(0, 1, 0);
    vec2 tc;
    float ndotu = dot(norm, up);

    binormal = vec3(0.0);

    if (ndotu < (1.0 - 0.0001)) {
        binormal = cross(norm, up);
    } else {
        vec3 dir = vec3(1.0,0, 0);
        binormal = vec3(0, 0, 1.0);
    }
    modelPos = pos;
    texcoord = UV0;
    normal = norm;
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
