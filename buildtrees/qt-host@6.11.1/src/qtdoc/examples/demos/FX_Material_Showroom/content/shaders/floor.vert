// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

precision lowp int;
precision lowp float;

VARYING vec3 vColor;
VARYING vec3 vPos;
VARYING vec3 vNormal;
VARYING vec3 vBinormal;
VARYING vec3 vTangent;
VARYING vec3 vViewVec;
VARYING vec2 vTexcoord;
VARYING mat3 TBNMat;

vec3 calculateTangent(vec3 normal)
{
    // Calculate the tangent vector
    vec3 Q1 = vec3(1.0, 0.0, 0.0);
    vec3 Q2 = vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(Q1 * Q2.y - Q1.y * Q2);

    // Calculate the handedness of the tangent basis
    float handedness = (dot(cross(normal, tangent), Q2) < 0.0) ? -1.0 : 1.0;

    return handedness * tangent;
}

vec3 calculateBinormal(vec3 normal, vec3 tangent)
{
    return normalize(cross(normal, tangent));
}

void MAIN()
{
    vColor = COLOR.rgb;
    vPos = VERTEX.xyz;
    vNormal = normalize(NORMAL_MATRIX * NORMAL);
    vTangent = calculateTangent(vNormal);
    vBinormal = calculateBinormal(vNormal, vTangent);

    TBNMat = mat3(vTangent, vBinormal, vNormal);

    vTexcoord = UV0;
    vViewVec = normalize(CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz);
}
