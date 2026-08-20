// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 texcoord;

#if !(QSSG_ENABLE_DEPTH_PASS || QSSG_ENABLE_ORTHO_SHADOW_PASS || QSSG_ENABLE_PERSPECTIVE_SHADOW_PASS || QSSG_ENABLE_OPAQUE_DEPTH_PRE_PASS)
#define LIGHTING_PASS
#endif

#if defined (LIGHTING_PASS)
#include "lightsData.glsllib"
#include "funcprocessPunctualLighting.glsllib"
#include "tonemapping.glsllib"

#if QSSG_ENABLE_LIGHT_PROBE
#include "sampleProbe.glsllib"
#endif
#endif // LIGHTING_PASS

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
#if defined (LIGHTING_PASS)
    vec2 uv = texcoord;
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
    float renderMask = gb2.w;

    if (renderMask < 1.0)
        discard;

    // View vector (from fragment to camera)
    vec3 V = normalize(CAMERA_POSITION - worldPos);

    vec3 diffuseAccum = vec3(0.0);
    vec3 specAccum    = vec3(0.0);

    vec3 F0 = computeF0(baseColor, metalness);

    // void qt_processPunctualLighting(inout vec3 global_diffuse_light,
    //                                 inout vec3 global_specular_light,
    //                                 in vec3 qt_diffuseColor,
    //                                 in vec3 qt_world_position,
    //                                 in vec3 qt_world_normal,
    //                                 in vec3 qt_view_vector,
    // #if QSSG_ENABLE_SPECULAR
    //                                 in vec3 specularAmount,
    //                                 in vec3 specularTint,
    // #endif // << QSSG_ENABLE_SPECULAR
    //                                 in float qt_roughnessAmount,
    //                                 in float qt_metalnessAmount,
    // #if QSSG_CUSTOM_MATERIAL_DIRECTIONAL_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_POINT_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_SPOT_LIGHT_PROCESSOR || QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR
    //                                 in vec4 qt_customBaseColor,
    // #endif // QSSG_CUSTOM_MATERIAL_*
    // #if QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR
    //                                 in float qt_customSpecularAmount,
    // #endif // QSSG_CUSTOM_MATERIAL_SPECULAR_PROCESSOR
    // #if QSSG_CUSTOM_MATERIAL_SHARED_VARIABLES
    //                                 inout QT_SHARED_VARS qt_customShared,
    // #endif // QSSG_CUSTOM_MATERIAL_SHARED_VARIABLES
    // #if QSSG_ENABLE_CLEARCOAT
    //                                 inout vec3 qt_global_clearcoat,
    //                                 in vec3 qt_clearcoatNormal,
    //                                 in float qt_clearcoatRoughness,
    //                                 in vec3 qt_clearcoatF0,
    //                                 in vec3 qt_clearcoatF90,
    // #endif // QSSG_ENABLE_CLEARCOAT
    // #if QSSG_ENABLE_TRANSMISSION
    //                                 inout vec3 qt_global_transmission,
    //                                 in float qt_thicknessFactor,
    //                                 in float qt_iOR,
    //                                 in float qt_transmissionFactor,
    //                                 in vec3 qt_attenuationColor,
    //                                 in float qt_attenuationDistance,
    // #endif
    //                                 in vec3 f0,
    //                                 in vec3 f90
    //                                 )
    qt_processPunctualLighting(diffuseAccum,
                               specAccum,
                               baseColor,
                               worldPos,
                               N,
                               V,
                               vec3(1.0), // specularAmount
                               vec3(1.0), // specularTint
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

    // Clamp a bit so you do not get wild values before you add tone mapping
    color = max(color, vec3(0.0));

    FRAGCOLOR = vec4(qt_tonemap(color), 1.0);
#endif // LIGHTING_PASS
}
