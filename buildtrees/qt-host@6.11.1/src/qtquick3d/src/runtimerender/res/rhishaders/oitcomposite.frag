// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440
#extension GL_GOOGLE_include_directive : enable

#include "../effectlib/oitcommon.glsllib"

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;
#if QSHADER_VIEW_COUNT >= 2
layout(location = 1) flat in uint v_viewIndex;
#endif

ivec2 qt_indexToImageCoord(in uint index, const uint width)
{
    ivec2 uv;
    uv.y = int(index / width);
    uv.x = int(index - uv.y * width);
    return uv;
}

vec4 doBlend(vec4 dstColor, vec4 srcColor)
{
    vec4 ret = dstColor;
    const float oneMinusSrcAlpha = 1.0 - srcColor.a;
    ret.rgb = srcColor.a * srcColor.rgb + oneMinusSrcAlpha * dstColor.rgb;
    ret.a   = srcColor.a                + oneMinusSrcAlpha * dstColor.a;
    return ret;
}

vec4 doBlendPacked(vec4 baseColor, uint fragment)
{
    vec4 unpackedColor = unpackUnorm4x8(fragment);
    return doBlend(baseColor, unpackedColor);
}

vec4 doInverseBlendPacked(vec4 baseColor, uint fragment)
{
    vec4 unpackedColor = unpackUnorm4x8(fragment);
    return doBlend(unpackedColor, baseColor);
}

#if QSSG_OIT_METHOD == QSSG_OIT_WEIGHTED_BLENDED

#if QSHADER_VIEW_COUNT >= 2
#   ifdef QSSG_MULTISAMPLE
#       define samplerType sampler2DMSArray
#   else
#       define samplerType sampler2DArray
#   endif
#else
#   ifdef QSSG_MULTISAMPLE
#      define samplerType sampler2DMS
#   else
#      define samplerType sampler2D
#   endif
#endif

layout(binding = 1) uniform samplerType accumTexture;
layout(binding = 2) uniform samplerType revealageTexture;

void main()
{
#if QSHADER_HLSL || QSHADER_MSL
    vec2 uv = vec2(uv_coord.x, 1.0 - uv_coord.y);

#else
    vec2 uv = uv_coord;
#endif
#if QSHADER_VIEW_COUNT >= 2
#ifdef QSSG_MULTISAMPLE
    ivec2 iuv = ivec2(uv * textureSize(accumTexture).xy);
    vec4 accum = texelFetch(accumTexture, ivec3(iuv, v_viewIndex), gl_SampleID);
    float a = 1.0 - texelFetch(revealageTexture, ivec3(iuv, v_viewIndex), gl_SampleID).r;
#else
    vec4 accum = texture(accumTexture, vec3(uv, v_viewIndex));
    float a = 1.0 - texture(revealageTexture, vec3(uv, v_viewIndex)).r;
#endif
#else
#ifdef QSSG_MULTISAMPLE
    ivec2 iuv = ivec2(uv * textureSize(accumTexture));
    vec4 accum = texelFetch(accumTexture, iuv, gl_SampleID);
    float a = 1.0 - texelFetch(revealageTexture, iuv, gl_SampleID).r;
#else
    vec4 accum = texture(accumTexture, uv);
    float a = 1.0 - texture(revealageTexture, uv).r;
#endif
#endif
    vec4 color = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e6), a);
    fragOutput = vec4(color);
}

#endif // QSSG_OIT_WEIGHTED_BLENDED

#if QSSG_OIT_METHOD == QSSG_OIT_LINKED_LIST

#define QSSG_OIT_LAYERS 8
#define SORT_SIZE QSSG_OIT_LAYERS
#define SortType uvec2
#define SortOp compareNodes

layout(std140, binding = 0) uniform buf {
    uint qt_ABufImageWidth;
    uint qt_listNodeCount;
    uint qt_samples;
    uint pad;
    ivec2 qt_viewSize;
} ubuf;

bool compareNodes(const uvec2 a, const uvec2 b)
{
    return uintBitsToFloat(a.g) < uintBitsToFloat(b.g);
}

#include "../effectlib/sorting.glsllib"

#if QSSG_USE_BUFFER == 1
layout(std430, binding = 1) buffer restrict readonly qt_imgAbuffer
{
    uvec4 abufData[];
};
layout(std430, binding = 2) buffer restrict readonly qt_imgAux
{
    uint auxData[];
};
#else
layout(binding = 1, rgba32ui) uniform restrict readonly uimage2D qt_imgAbuffer;
layout(binding = 2, r32ui) uniform restrict readonly uimage2D qt_imgAux;
#endif

void main()
{
    uvec2 array[QSSG_OIT_LAYERS];

    vec4 color = vec4(0);
    uint fragments = 0;  // The number of fragments for this sample.

#if QSHADER_VIEW_COUNT >= 2
    ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y + v_viewIndex * ubuf.qt_viewSize.y);
#else
    ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
#endif
#ifdef QSSG_MULTISAMPLE
    coord.x += gl_SampleID * ubuf.qt_viewSize.x;
#endif

#if QSSG_USE_BUFFER == 1
    uint startOffset = auxData[coord.x + coord.y * ubuf.qt_viewSize.x * ubuf.qt_samples];
#else
    uint startOffset = imageLoad(qt_imgAux, coord).r;
#endif

    // Traverse the linked list:
    while (startOffset != uint(0) && fragments < QSSG_OIT_LAYERS && startOffset <= ubuf.qt_listNodeCount)
    {
#if QSSG_USE_BUFFER == 1
        const uvec4 stored = abufData[startOffset - 1];
#else
        const uvec4 stored = imageLoad(qt_imgAbuffer, qt_indexToImageCoord(startOffset - 1, ubuf.qt_ABufImageWidth));
#endif
        array[fragments]   = stored.rg;
        fragments++;

        startOffset = stored.b;
    }

    qt_bubbleSort(array, int(fragments));

    // Process the remaining fragments
    vec4 colorSum = vec4(0);
    float minKicked = 999999;
    int count = 0;
    while(startOffset != uint(0) && count < 8 && startOffset <= ubuf.qt_listNodeCount) {
#if QSSG_USE_BUFFER == 1
        const uvec4 stored = abufData[startOffset - 1];
#else
        uvec4 stored = imageLoad(qt_imgAbuffer, qt_indexToImageCoord(startOffset - 1, ubuf.qt_ABufImageWidth));
#endif

        uvec2 kicked = qt_insertionSort(array, stored.rg);
        float kd = uintBitsToFloat(kicked.g);
        if (kd < minKicked) {
            colorSum = doBlendPacked(colorSum, kicked.x);
            minKicked = kd;
        } else {
            colorSum = doInverseBlendPacked(colorSum, kicked.x);
        }

        startOffset = stored.b;
        count++;
    }
    // Blend all of the fragments together:
    for (uint i = 0; i < fragments; i++)
        colorSum = doBlendPacked(colorSum, array[i].x);

    fragOutput = colorSum;
}

#endif // QSSG_OIT_LINKED_LIST
