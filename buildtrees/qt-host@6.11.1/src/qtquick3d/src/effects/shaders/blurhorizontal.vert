// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;

void setupHorizontalGaussianBlur( float inDestWidth, float inBlurAmount, vec2 inTexCoord )
{
    float increment = inBlurAmount/inDestWidth;
    TexCoord0 = vec2(inTexCoord.x + increment            , inTexCoord.y );
    TexCoord1 = vec2(inTexCoord.x + increment * 2.0      , inTexCoord.y);
    TexCoord2 = vec2(inTexCoord.x + increment * 3.0      , inTexCoord.y);
    TexCoord3 = vec2(inTexCoord.x                        , inTexCoord.y);
    TexCoord4 = vec2(inTexCoord.x - increment            , inTexCoord.y);
    TexCoord5 = vec2(inTexCoord.x - increment * 2.0      , inTexCoord.y);
    TexCoord6 = vec2(inTexCoord.x - increment * 3.0      , inTexCoord.y);
}

void MAIN()
{
    setupHorizontalGaussianBlur(INPUT_SIZE.x, amount, INPUT_UV);
}
