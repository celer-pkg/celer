// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

layout(location = 0) out vec4 fragOutput;

layout(std140, binding = 0) uniform buf {
    uvec4 clearColor;
    ivec2 viewSize;
    uint samples;
    uint viewCount;
} ubuf;

#if QSSG_CLEAR_BUFFER == 1
layout(std430, binding = 1) buffer qt_imgAux
{
    uint auxData[];
};
#else
layout(binding = 1, rgba32ui) uniform restrict writeonly uimage2D qt_imgAux;
#endif

void main()
{
    fragOutput = vec4(0.0);
    const ivec2 coord = ivec2(gl_FragCoord.x * ubuf.samples, gl_FragCoord.y * ubuf.viewCount);
    for (uint s = 0; s < ubuf.samples; s++) {
#if QSSG_CLEAR_BUFFER == 1
        auxData[ubuf.viewSize.x * ubuf.samples * coord.y + coord.x + s] = ubuf.clearColor.x;
        if (ubuf.viewCount > 1)
            auxData[ubuf.viewSize.x * ubuf.samples * (coord.y + 1) + coord.x + s] = ubuf.clearColor.x;
#else
        imageStore(qt_imgAux, ivec2(coord.x + s, coord.y), ubuf.clearColor);
        if (ubuf.viewCount > 1)
            imageStore(qt_imgAux, ivec2(coord.x + s, coord.y + 1), ubuf.clearColor);
#endif
    }
}
