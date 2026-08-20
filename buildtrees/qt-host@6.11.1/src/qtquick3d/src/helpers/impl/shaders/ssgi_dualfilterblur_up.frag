// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_INPUT(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
#else
#define SAMPLE_INPUT(uv) texture(INPUT, uv)
#endif

//#define ENABLE_DEBUG_MODE

void MAIN()
{
    vec4 c0 = SAMPLE_INPUT(INPUT_UV);

#ifdef ENABLE_DEBUG_MODE
    if (debugMode == 6 || debugMode == 7) {
        FRAGCOLOR = c0;
        return;
    }
#endif

    const vec2 uv = INPUT_UV;
    const vec2 halfpixel = 0.5 / (INPUT_SIZE / 2.0);
    const float offset = 3.0;

    vec3 sum = SAMPLE_INPUT(uv + vec2(-halfpixel.x * 2.0, 0.0) * offset).rgb;
    sum += SAMPLE_INPUT(uv + vec2(-halfpixel.x, halfpixel.y) * offset).rgb * 2.0;
    sum += SAMPLE_INPUT(uv + vec2(0.0, halfpixel.y * 2.0) * offset).rgb;
    sum += SAMPLE_INPUT(uv + vec2(halfpixel.x, halfpixel.y) * offset).rgb * 2.0;
    sum += SAMPLE_INPUT(uv + vec2(halfpixel.x * 2.0, 0.0) * offset).rgb;
    sum += SAMPLE_INPUT(uv + vec2(halfpixel.x, -halfpixel.y) * offset).rgb * 2.0;
    sum += SAMPLE_INPUT(uv + vec2(0.0, -halfpixel.y * 2.0) * offset).rgb;
    sum += SAMPLE_INPUT(uv + vec2(-halfpixel.x, -halfpixel.y) * offset).rgb * 2.0;

    FRAGCOLOR = vec4(sum / 12.0, c0.a);
}
