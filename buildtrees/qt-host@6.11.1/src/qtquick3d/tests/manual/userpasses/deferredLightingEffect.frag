// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#define QSSG_ENABLE_DIFFUSE_MODEL_BURLEY 1
#define QSSG_ENABLE_SPECULAR 1
#define QSSG_ENABLE_METALLIC_ROUGHNESS_WORKFLOW 1
#define QSSG_ENABLE_SHADOWMAPPING 1
#define QSSG_SHADOW_SOFTNESS 8

#include "lightsData.glsllib"
#include "funcprocessPunctualLighting.glsllib"

#if QSSG_ENABLE_LIGHT_PROBE
#include "sampleProbe.glsllib"
#endif

// Helper to decode normal from 0..1 back to -1..1
vec3 decodeNormal(vec3 enc)
{
    return normalize(enc * 2.0 - 1.0);
}

// Simple Fresnel F0 helper
vec3 computeF0(vec3 baseColor, float metalness)
{
    // 4 % default specular for dielectrics
    vec3 dielectricF0 = vec3(0.04);
    return mix(dielectricF0, baseColor, metalness);
}

void MAIN()
{
//     vec4 c = texture(tex, TEXTURE_UV);
//     c.r *= redLevel;
// #if QSHADER_VIEW_COUNT >= 2
//     FRAGCOLOR = c * texture(INPUT, vec3(INPUT_UV, VIEW_INDEX));
// #else
//     FRAGCOLOR = c * texture(INPUT, INPUT_UV);
// #endif
    vec2 uv = TEXTURE_UV;
    if (FRAMEBUFFER_Y_UP < 0.0)
        uv.y = 1.0 - uv.y;

    // Sample G-buffer
    vec4 gb0 = texture(gbuffer0, uv);
    vec4 gb1 = texture(gbuffer1, uv);
    vec4 gb2 = texture(gbuffer2, uv);

    vec3 baseColor  = gb0.rgb;
    float metalness = gb0.a;

    vec3 N = decodeNormal(gb1.rgb);
    float roughness = gb1.a;

    vec3 worldPos = gb2.xyz;

    // View vector (from fragment to camera)
    vec3 V = normalize(CAMERA_POSITION - worldPos);

    vec3 diffuseAccum = vec3(0.0);
    vec3 specAccum    = vec3(0.0);


    // ambient light
    diffuseAccum += qt_light_ambient_total * baseColor;

    vec3 F0 = computeF0(baseColor, metalness);

    qt_processPunctualLighting(diffuseAccum,
                               specAccum,
                               baseColor,
                               worldPos,
                               N,
                               V,
#if QSSG_ENABLE_SPECULAR
                               vec3(1.0), // specularAmount
                               vec3(1.0), // specularTint
#endif //QSSG_ENABLE_SPECULAR
                               roughness,
                               metalness,
                               F0,
                               vec3(1.0)  // F90
                               );

#if QSSG_ENABLE_LIGHT_PROBE
    vec4 probeDiffuse = vec4(baseColor, 1.0) * qt_sampleDiffuse(N);
    vec4 probeSpecular = qt_sampleGlossyPrincipled(N, V, F0, roughness);
    diffuseAccum += probeDiffuse.rgb;
    specAccum += probeSpecular.rgb;
#endif

    vec3 color = diffuseAccum + specAccum;
    FRAGCOLOR = vec4(color, 1.0);
}
