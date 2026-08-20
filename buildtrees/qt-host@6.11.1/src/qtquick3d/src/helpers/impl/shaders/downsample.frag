// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "texturesample.glsllib"

VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;

float getDepthValue( vec4 depth_texture_sample, vec2 cameraProperties )
{
    float zNear = cameraProperties.x;
    float zFar = cameraProperties.y;
    float zRange = zFar - zNear;
    float z_b = depth_texture_sample.x;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zRange));
    return 1.0 - ((z_e - cameraProperties.x) / (zRange));
}

float depthValueToLinearDistance( float depth_value, vec2 cameraProperties )
{
    float FarClipDistance = cameraProperties.y;
    float NearClipDistance = cameraProperties.x;
    float DepthRange = FarClipDistance - NearClipDistance;
    float linearDepth = NearClipDistance + (DepthRange * (1.0 - depth_value));
    return linearDepth;
}

#if QSHADER_VIEW_COUNT >= 2
float getDepthMultiplier(vec2 inTexCoord, sampler2DArray inDepthSampler, float inFocusDistance, float inFocusWidth, float inFocusPenumbra)
#else
float getDepthMultiplier(vec2 inTexCoord, sampler2D inDepthSampler, float inFocusDistance, float inFocusWidth, float inFocusPenumbra)
#endif
{
    float depthTap = getDepthValue( SAMPLE(inDepthSampler, inTexCoord), CAMERA_PROPERTIES );
    float linearDepth = depthValueToLinearDistance( depthTap, CAMERA_PROPERTIES );
    float depthDiff = max( 0.0, abs( linearDepth - inFocusDistance ) - (inFocusWidth/2.0) ) / inFocusPenumbra;
    return clamp( depthDiff, 0.0, 1.0 );
}

#if QSHADER_VIEW_COUNT >= 2
vec4 boxDepthBlur(sampler2DArray inDepthSampler, sampler2DArray inBlurSampler, float inFocusDistance, float inFocusWidth, float inFocusPenumbra )
#else
vec4 boxDepthBlur(sampler2D inDepthSampler, sampler2D inBlurSampler, float inFocusDistance, float inFocusWidth, float inFocusPenumbra )
#endif
{
    float mult0 = .25 * getDepthMultiplier( TexCoord0, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra );
    float mult1 = .25 * getDepthMultiplier( TexCoord1, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra );
    float mult2 = .25 * getDepthMultiplier( TexCoord2, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra );
    float mult3 = .25 * getDepthMultiplier( TexCoord3, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra );
    float multTotal = mult0 + mult1 + mult2 + mult3;
    float totalDivisor = multTotal != 0.0 ? 1.0 / multTotal : 0.0;
    vec4 OutCol = SAMPLE(inBlurSampler, TexCoord0) * mult0;
    OutCol += SAMPLE(inBlurSampler, TexCoord1) * mult1;
    OutCol += SAMPLE(inBlurSampler, TexCoord2) * mult2;
    OutCol += SAMPLE(inBlurSampler, TexCoord3) * mult3;
    return OutCol * totalDivisor;
}

void MAIN()
{
    FRAGCOLOR = boxDepthBlur(DEPTH_TEXTURE, INPUT, focusDistance, focusRange, focusRange);
}
