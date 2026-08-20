// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;

const vec3 poisson0 = vec3( 0.000000, 0.000000, 0.000000 );
const vec3 poisson1 = vec3( 0.527837, -0.085868, 0.534776 );
const vec3 poisson2 = vec3( -0.040088, 0.537087, 0.538581 );
const vec3 poisson3 = vec3( -0.670445, -0.017995, 0.670686 );
const vec3 poisson4 = vec3( -0.419418, -0.616039, 0.745262 );

void setupPoissonBlurCoords(float inBlurAmount, vec2 inTexInfo )
{
    float incX = inBlurAmount / inTexInfo.x;
    float incY = inBlurAmount / inTexInfo.y;
    TexCoord0 = vec2( INPUT_UV.x + poisson0.x * incX, INPUT_UV.y + poisson0.y * incY );
    TexCoord1 = vec2( INPUT_UV.x + poisson1.x * incX, INPUT_UV.y + poisson1.y * incY );
    TexCoord2 = vec2( INPUT_UV.x + poisson2.x * incX, INPUT_UV.y + poisson2.y * incY );
    TexCoord3 = vec2( INPUT_UV.x + poisson3.x * incX, INPUT_UV.y + poisson3.y * incY );
    TexCoord4 = vec2( INPUT_UV.x + poisson4.x * incX, INPUT_UV.y + poisson4.y * incY );
}

void MAIN()
{
    setupPoissonBlurCoords(blurAmount, OUTPUT_SIZE);
}
