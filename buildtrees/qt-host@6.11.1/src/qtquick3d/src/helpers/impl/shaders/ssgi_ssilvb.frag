// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_INPUT(uv) textureLod(INPUT, vec3(uv, VIEW_INDEX), 0)
#define SAMPLE_DEPTH(uv) textureLod(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX), 0).r
#define SAMPLE_NORMAL(uv) normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb)
#define SAMPLE_LAST_FRAME_INDIRECT_AND_AO(uv) textureLod(blurredIndirectAndAoSampler, vec3(uv, VIEW_INDEX), 0)
#define PROJECTION PROJECTION_MATRIX[VIEW_INDEX]
#else
#define SAMPLE_INPUT(uv) textureLod(INPUT, uv, 0)
#define SAMPLE_DEPTH(uv) textureLod(DEPTH_TEXTURE, uv, 0).r
#define SAMPLE_NORMAL(uv) normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb)
#define SAMPLE_LAST_FRAME_INDIRECT_AND_AO(uv) textureLod(blurredIndirectAndAoSampler, uv, 0)
#define PROJECTION PROJECTION_MATRIX
#endif

//#define ENABLE_DEBUG_MODE

vec3 getNdcPos(vec2 uv)
{
    // not view space! rather, normalized device coordinates (-1..1)
    // hence no depth linearization
    float depth = SAMPLE_DEPTH(uv);

    float aspect = INPUT_SIZE.x / INPUT_SIZE.y;

    float fovRad = 2.0 * atan(1.0 / PROJECTION[1][1]);
    if (NDC_Y_UP < 0.0) // invert with Vulkan, due to the -1 from clipSpaceCorrMatrix
        fovRad *= -1.0;
    float halfFovTan = tan(0.5 * fovRad);

    vec2 ndc = uv * 2.0 - 1.0;
    vec3 pos = vec3(ndc.x * aspect * halfFovTan * depth, ndc.y * halfFovTan * depth, -depth);

    pos.y *= qt_normalAdjustViewportFactor; // isYUpInFramebuffer ? 1.0 : -1.0

    return pos;
}

vec3 getViewNormal(vec2 uv)
{
    return normalize(mat3(VIEW_MATRIX) * SAMPLE_NORMAL(uv));
}

uint countBits(uint value) // there is no bitCount() in GLSL < 400
{
    // https://graphics.stanford.edu/%7Eseander/bithacks.html
    value = value - ((value >> 1u) & 0x55555555u);
    value = (value & 0x33333333u) + ((value >> 2u) & 0x33333333u);
    return ((value + (value >> 4u) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}

const uint sectorCount = 32;
const float pi = 3.14159265359;

// Screen Space Indirect Lighting with Visibility Bitmask
// https://arxiv.org/abs/2301.11376
// https://cdrinmatane.github.io/posts/cgspotlight-slides/

vec4 SSILVB()
{
    // https://cybereality.com/screen-space-indirect-lighting-with-visibility-bitmask-improvement-to-gtao-ssao-real-time-ambient-occlusion-algorithm-glsl-shader-implementation/
    uint indirect = 0;
    float visibility = 0.0;
    vec3 lighting = vec3(0.0);

    const vec2 aspect = INPUT_SIZE.yx / INPUT_SIZE.x;
    const vec3 position = getNdcPos(INPUT_UV);
    const vec3 camera = normalize(-position);
    const vec3 normal = getViewNormal(INPUT_UV);

    const float sliceRotation = 2.0 * pi / (sliceCount - 1.0);
    const float sampleScale = (-sampleRadius * PROJECTION[0][0]) / position.z;
    const float sampleOffset = 0.01;

    // https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
    const float jitter = mod(52.9829189 * mod(0.06711056 * FRAGCOORD.x + 0.00583715 * FRAGCOORD.y, 1.0), 1.0) - 0.5;

    for (float slice = 0.0; slice < sliceCount + 0.5; slice += 1.0) {
        const float phi = sliceRotation * (slice + jitter) + pi;
        const vec2 omega = vec2(cos(phi), sin(phi));
        const vec3 direction = vec3(omega.x, omega.y, 0.0);
        const vec3 orthoDirection = direction - dot(direction, camera) * camera;
        const vec3 axis = cross(direction, camera);
        const vec3 projNormal = normal - axis * dot(normal, axis);
        const float projLength = length(projNormal);

        const float signN = sign(dot(orthoDirection, projNormal));
        const float cosN = clamp(dot(projNormal, camera) / projLength, 0.0, 1.0);
        const float n = signN * acos(cosN);

        uint occlusion = 0;

        for (float currentSample = 0.0; currentSample < sampleCount + 0.5; currentSample += 1.0) {
            const float sampleStep = (currentSample + jitter) / sampleCount + sampleOffset;
            // GL: - others: +
            const vec2 sampleUV = INPUT_UV - qt_normalAdjustViewportFactor * (sampleStep * sampleScale * omega * aspect);
            const vec3 sampleDistance = getNdcPos(sampleUV) - position;
            const vec3 sampleHorizon = sampleDistance / length(sampleDistance);
            vec2 minMaxHorizon = acos(vec2(dot(sampleHorizon, camera), dot(normalize(sampleDistance - camera * hitThickness), camera)));
            minMaxHorizon = clamp((minMaxHorizon + n + 0.5 * pi) / pi, 0.0, 1.0);

            // https://cdrinmatane.github.io/posts/ssaovb-code/
            uint startBit = uint(minMaxHorizon.x * float(sectorCount));
            uint horizonAngle = uint(ceil((minMaxHorizon.y - minMaxHorizon.x) * float(sectorCount)));
            uint angleBit = horizonAngle > 0 ? uint(0xFFFFFFFFu >> (sectorCount - horizonAngle)) : 0;
            indirect = angleBit << startBit;

#ifdef ENABLE_DEBUG_MODE
            if (indirectLightEnabled || debugMode != 0) {
#else
            if (indirectLightEnabled) {
#endif
                vec3 sampleLight = SAMPLE_INPUT(sampleUV).rgb;
                if (simulatedBounceEnabled) {
                    // Fake an additional bounce by sampling the previous frame's indirect
                    // lighting result and pretending it is part of the current input.
                    vec3 indirectFromLastFrame = SAMPLE_LAST_FRAME_INDIRECT_AND_AO(sampleUV).rgb;
                    sampleLight = sampleLight * (1.0 + indirectFromLastFrame * indirectLightBoost * simulatedBounceFactor);
                }
                lighting += (1.0 - float(countBits(indirect & ~occlusion)) / float(sectorCount))
                    * sampleLight
                    * clamp(dot(normal, sampleHorizon), 0.0, 1.0)
                    * clamp(dot(getViewNormal(sampleUV), -sampleHorizon), 0.0, 1.0);
            }

            occlusion |= indirect;
        }
        visibility += 1.0 - float(countBits(occlusion)) / float(sectorCount);
    }

    visibility /= sliceCount;
    lighting /= sliceCount;

    return vec4(lighting, visibility);
}

void MAIN()
{
    vec4 indirectAndAo = SSILVB();

#ifndef ENABLE_DEBUG_MODE
    FRAGCOLOR = indirectAndAo;
#else
    if (debugMode == 6)
        FRAGCOLOR = vec4(getNdcPos(INPUT_UV) * 0.5 + 0.5, 1.0);
    else if (debugMode == 7)
        FRAGCOLOR = vec4(getViewNormal(INPUT_UV) * 0.5 + 0.5, 1.0);
    else
        FRAGCOLOR = indirectAndAo;
#endif
}
