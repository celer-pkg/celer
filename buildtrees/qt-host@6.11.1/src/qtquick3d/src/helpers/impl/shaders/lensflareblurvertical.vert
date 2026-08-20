// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;

void setupVerticalGaussianBlur( float inDestHeight, float inBlurAmount, vec2 inTexCoord )
{
    float increment = inBlurAmount/inDestHeight;
    TexCoord0 = vec2(inTexCoord.x, inTexCoord.y + increment );
    TexCoord1 = vec2(inTexCoord.x, inTexCoord.y + increment * 2.0 );
    TexCoord2 = vec2(inTexCoord.x, inTexCoord.y + increment * 3.0);
    TexCoord3 = vec2(inTexCoord.x, inTexCoord.y);
    TexCoord4 = vec2(inTexCoord.x, inTexCoord.y - increment);
    TexCoord5 = vec2(inTexCoord.x, inTexCoord.y - increment * 2.0);
    TexCoord6 = vec2(inTexCoord.x, inTexCoord.y - increment * 3.0);
}

void MAIN()
{
    setupVerticalGaussianBlur(INPUT_SIZE.y, lensFlareBlurAmount, INPUT_UV);
}
