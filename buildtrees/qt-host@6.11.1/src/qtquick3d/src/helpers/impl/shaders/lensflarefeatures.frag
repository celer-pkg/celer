// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "texturesample.glsllib"

#if QSHADER_VIEW_COUNT >= 2
vec3 textureDistorted(in sampler2DArray tex, in vec2 texcoord, in vec2 direction, vec3 distortion)
#else
vec3 textureDistorted(in sampler2D tex, in vec2 texcoord, in vec2 direction, vec3 distortion)
#endif
{
    return vec3(SAMPLE(tex, texcoord + direction * distortion.r).r,
                SAMPLE(tex, texcoord + direction * distortion.g).g,
                SAMPLE(tex, texcoord + direction * distortion.b).b);
}

void MAIN()
{
    vec2 texCoord = -INPUT_UV + vec2(1.0);
    vec2 texelSize = vec2(1.0 / INPUT_SIZE.x, 1.0 / INPUT_SIZE.y);

    vec3 distortion = vec3(-texelSize.x * lensFlareDistortion, 0.0, texelSize.x * lensFlareDistortion);

    // ghost vector to image center:
    vec2 ghostVec = (vec2(0.5) - texCoord) * lensFlareGhostDispersal;
    vec2 direction = normalize(ghostVec);

    // sample ghosts:
    vec3 result = vec3(0.0);
    for (int i = 0; i < lensFlareGhostCount; ++i) {
        vec2 offset = fract(texCoord + ghostVec * float(i));
        float weight = length(vec2(0.5) - offset) / length(vec2(0.5));
              weight = pow(1.0 - weight, 10.0);
        result += textureDistorted(lensFlareDownsampleBuffer, offset, direction, distortion) * weight;
    }

    result *= texture(lensColorTexture, vec2(length(vec2(0.5) - texCoord) / length(vec2(0.5)), 0)).rgb;

    // sample halo:
    vec2 aspect = vec2(1.0, mix(1.0, INPUT_SIZE.x / INPUT_SIZE.y, lensFlareStretchToAspect));
    vec2 haloVec = normalize(ghostVec / aspect) * aspect * lensFlareHaloWidth;
    float weight = length(vec2(0.5) - fract(texCoord + haloVec)) / length(vec2(0.5));
    weight = pow(1.0 - weight, 5.0);
    result += textureDistorted(lensFlareDownsampleBuffer, texCoord + haloVec, direction, distortion) * weight;

    FRAGCOLOR = vec4(result, 1.0);
}
