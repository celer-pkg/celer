// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;
VARYING vec2 TexCoord7;

const vec3 poisson0 = vec3( 0.000000, 0.000000, 0.000000 );
const vec3 poisson1 = vec3( 0.527837, -0.085868, 0.534776 );
const vec3 poisson2 = vec3( -0.040088, 0.537087, 0.538581 );
const vec3 poisson3 = vec3( -0.670445, -0.017995, 0.670686 );
const vec3 poisson4 = vec3( -0.419418, -0.616039, 0.745262 );
const vec3 poisson5 = vec3( 0.440453, -0.639399, 0.776421 );
const vec3 poisson6 = vec3( -0.757088, 0.349334, 0.833796 );
const vec3 poisson7 = vec3( 0.574619, 0.685879, 0.894772 );

vec2 toRotatedPoissonTexCoord(vec3 poisson, vec2 inputTex, vec2 inc, mat2 rotation)
{
    vec2 rotatedPoisson = rotation * vec2(poisson.xy);
    return vec2(inputTex.x + rotatedPoisson.x * inc.x, inputTex.y + rotatedPoisson.y * inc.y);
}

void setupPoissonBlurCoordsRotation(float inBlurAmount, vec2 inTexInfo, float inRotationRadians)
{
    float rotCos = cos(inRotationRadians);
    float rotSin = sin(inRotationRadians);
    mat2 rotMatrix = mat2(rotCos, rotSin, -rotSin, rotCos);
    vec2 incVec = vec2(inBlurAmount / inTexInfo.x, inBlurAmount / inTexInfo.y);

    TexCoord0 = toRotatedPoissonTexCoord(poisson0, INPUT_UV, incVec, rotMatrix);
    TexCoord1 = toRotatedPoissonTexCoord(poisson1, INPUT_UV, incVec, rotMatrix);
    TexCoord2 = toRotatedPoissonTexCoord(poisson2, INPUT_UV, incVec, rotMatrix);
    TexCoord3 = toRotatedPoissonTexCoord(poisson3, INPUT_UV, incVec, rotMatrix);
    TexCoord4 = toRotatedPoissonTexCoord(poisson4, INPUT_UV, incVec, rotMatrix);
    TexCoord5 = toRotatedPoissonTexCoord(poisson5, INPUT_UV, incVec, rotMatrix);
    TexCoord6 = toRotatedPoissonTexCoord(poisson6, INPUT_UV, incVec, rotMatrix);
    TexCoord7 = toRotatedPoissonTexCoord(poisson7, INPUT_UV, incVec, rotMatrix);
}

void MAIN()
{
    setupPoissonBlurCoordsRotation(poissonDistance, INPUT_SIZE, poissonRotation);
}
