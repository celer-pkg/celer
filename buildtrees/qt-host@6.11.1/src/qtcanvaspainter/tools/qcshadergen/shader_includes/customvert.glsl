// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 tcoord;
layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec2 fragCoord;

layout(std140, binding = 0) uniform vertUBuf {
    vec4 viewRect;
    int ndcIsYDown;
    vec4 customMatrix;
};

layout(std140, binding = 4) uniform vertUBuf2 {
    mat3 vertMatrix;
};

// Custom brushes use binding 1
layout(std140, binding = 1) uniform commonUBuf {
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
    float globalAlpha;
    int unused1;
    // Custom input
    float iTime;
    int alphaIsRed;
    float unused2;
    float unused3;
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    vec4 unused4[2];
};
