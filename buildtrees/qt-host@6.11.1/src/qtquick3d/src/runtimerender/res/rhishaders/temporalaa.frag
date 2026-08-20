// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#version 440

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;

layout(std140, binding = 0) uniform buf {
    vec2 invSize;
    float strength;
    float padding;
} ubuf;

layout(binding = 1) uniform sampler2D currentTexture;
layout(binding = 2) uniform sampler2D prevTexture;
layout(binding = 3) uniform sampler2D depthTexture;
layout(binding = 4) uniform sampler2D velocityTexture;

vec2 getDepthClosestUVOffset(vec2 uv)
{
    float closestDepth = 100.0;
    vec2 depthClosestUVOffset = vec2(0.0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 uvOffset = vec2(x, y) * ubuf.invSize;
            float neighborDepth = texture(depthTexture, uv + uvOffset).r;
            if(neighborDepth < closestDepth)
            {
                depthClosestUVOffset = uvOffset;
                closestDepth = neighborDepth;
            }
        }
    }
    return depthClosestUVOffset;
}

void main()
{
    vec2 uv = uv_coord;
#if QSHADER_HLSL || QSHADER_MSL
    uv.y = 1.0 - uv.y;
#endif
    vec4 currentColor = texture(currentTexture, uv);
    vec2 depthClosestUVOffset = getDepthClosestUVOffset(uv);
    // raw veclocity without per object amount
    vec2 velocity = texture(velocityTexture, uv + depthClosestUVOffset).zw;
    vec4 prevColor = texture(prevTexture, uv + velocity);

    vec4 minColor = vec4(9999.0);
    vec4 maxColor = vec4(-9999.0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 uvoffset = vec2(x, y) * ubuf.invSize;
            vec4 color = texture(currentTexture, uv + uvoffset);
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
        }
    }

    vec4 clampPrevColor = clamp(prevColor, minColor, maxColor);

    fragOutput = mix(currentColor, clampPrevColor, ubuf.strength);
}
