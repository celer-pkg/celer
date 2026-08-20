// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#define PI 3.14159265359
#define PI2_INV 0.15915494309

layout(std140, binding = 1) uniform fragUBuf {
    mat3 scissorMat;
    vec2 scissorExt;
    vec2 scissorScale;
    float alphaMult;
    float strokeThr;
    float fontAlphaMin;
    float fontAlphaMax;
    vec4 colorEffects;
    int texType;
    int type;
    // Take these into use when needed
    float globalAlpha;
    int unused1;
    // Custom data starts
    mat3 paintMat;
    vec4 innerCol;
    vec4 outerCol;
    vec2 extent;
    float radius;
    float feather;
    // Take these into use when needed
    vec4 unused3;
};
