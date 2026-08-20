// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "texturesample.glsllib"

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;

#define WT7_0 20.0
#define WT7_1 15.0
#define WT7_2 6.0
#define WT7_3 1.0
#define WT7_NORMALIZE (WT7_0+2.0*(WT7_1+WT7_2+WT7_3))

#if QSHADER_VIEW_COUNT >= 2
vec4 gaussianBlur(sampler2DArray inSampler)
#else
vec4 gaussianBlur(sampler2D inSampler)
#endif
{
    vec4 OutCol = vec4(0.0);
    OutCol += SAMPLE(inSampler, TexCoord0) * ( WT7_1/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord1) * ( WT7_2/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord2) * ( WT7_3/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord3) * ( WT7_0/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord4) * ( WT7_1/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord5) * ( WT7_2/WT7_NORMALIZE );
    OutCol += SAMPLE(inSampler, TexCoord6) * ( WT7_3/WT7_NORMALIZE );
    return OutCol;
}

void MAIN()
{
    FRAGCOLOR = gaussianBlur(lensFlareTexture);
}
