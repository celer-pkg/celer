// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_INPUT(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#define SAMPLE_SSR(uv) texture(ssrSampler, vec3(uv, VIEW_INDEX))
#else
#define SAMPLE_INPUT(uv) texture(INPUT, uv)
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#define SAMPLE_SSR(uv) texture(ssrSampler, uv)
#endif

void MAIN()
{
    vec2 uv2 = INPUT_UV;

    // Flip for OpenGL
    if (qt_rhi_properties.z < -0.5)
        uv2.y = 1.0 - uv2.y;

    vec3 scene = SAMPLE_INPUT(INPUT_UV).rgb;
    vec4 ssr = SAMPLE_SSR(INPUT_UV);
    float roughness = clamp(SAMPLE_NORMAL_AND_ROUGHNESS(INPUT_UV).a, 0.0, roughnessCut) / roughnessCut;
    float confidence = clamp(ssr.a, 0.0, 1.0);
    vec3 outRGB = mix(scene, ssr.rgb, (1.0 - roughness) * confidence);
    FRAGCOLOR = vec4(outRGB, 1.0);
}
