// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_SCENE(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
#define SAMPLE_DEPTH(uv) textureLod(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX), 0).r
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#define SAMPLE_SSR_MASK(uv) textureLod(ssrMaskSampler, vec3(uv, VIEW_INDEX), 0)
#define PROJECTION PROJECTION_MATRIX[VIEW_INDEX]
#define INVERSE_PROJECTION INVERSE_PROJECTION_MATRIX[VIEW_INDEX]
#else
#define SAMPLE_SCENE(uv) texture(INPUT, uv)
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#define SAMPLE_DEPTH(uv) textureLod(DEPTH_TEXTURE, uv, 0).r
#define SAMPLE_SSR_MASK(uv) textureLod(ssrMaskSampler, uv, 0)
#define PROJECTION PROJECTION_MATRIX
#define INVERSE_PROJECTION INVERSE_PROJECTION_MATRIX
#endif

// UV <-> NDC conversion helpers (handles D3D Y-flip)
vec2 ndcFromUv(vec2 uv)
{
    float y_flip = 1.0;
    if (qt_rhi_properties.x < 0 && qt_rhi_properties.y > -0.5)
        y_flip = -1.0;

    return vec2(
        uv.x * 2.0 - 1.0,
        (uv.y * 2.0 - 1.0) * y_flip
    );
}

vec2 uvFromNdc(vec2 ndc)
{
    float y_flip = 1.0;
    if (qt_rhi_properties.x < 0 && qt_rhi_properties.y > -0.5)
        y_flip = -1.0;

    return vec2(
        0.5 * (ndc.x + 1.0),
        0.5 * (ndc.y * y_flip + 1.0)
    );
}

// Convert packed normal to view-space normal
vec3 getViewNormal(vec4 normalAndRoughness)
{
    return normalize(mat3(VIEW_MATRIX) * normalize(normalAndRoughness.rgb));
}

// Convert device depth → NDC depth
float ndcDepthFromDevice(float d)
{
    return (qt_rhi_properties.z > -0.5) ? d : (2.0 * d - 1.0);
}

// Depth to view-space position reconstruction
vec3 getViewPos(vec2 uv, float deviceDepth)
{
    vec2 ndcXY = ndcFromUv(uv);
    vec4 viewPos = INVERSE_PROJECTION * vec4(ndcXY, ndcDepthFromDevice(deviceDepth), 1.0);
    return viewPos.xyz / viewPos.w;
}

// Project view-space position to screen UV + NDC depth
vec3 computeScreenPos(vec3 viewPos)
{
    vec4 p = PROJECTION * vec4(viewPos, 1.0);
    p.xyz /= p.w;       // perspective divide
    p.xy = uvFromNdc(p.xy);
    return p.xyz;       // xy = uv, z = ndc depth
}


// rayMarch result package
struct SSRResult
{
    vec2  uv;
    float confidence;
    int   steps;
    float distance;
};

// Binary search final refinement for precise surface hit
vec3 binaryRefinement(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    vec2 uv;
    float depthDevice;
    float depthVS;

    for (int i = 0; i < binarySteps; ++i)
    {
        vec3 proj = computeScreenPos(hitCoord);
        uv = proj.xy;

        depthDevice = SAMPLE_DEPTH(uv);
        depthVS = getViewPos(uv, depthDevice).z;

        dDepth = hitCoord.z - depthVS;

        dir *= 0.5;
        hitCoord += (dDepth > 0.0) ? dir : -dir;
    }

    return vec3(uv, depthDevice);
}

SSRResult rayMarch(vec3 dir_in, vec3 hitCoord)
{
    float dDepth = 0.f;
    vec3 dir = dir_in * stepSize;

    for (int steps = 0; steps < maxSteps; ++steps)
    {
        // Advance ray in view-space
        hitCoord += dir;

        // Project to UV + depth
        vec3 proj = computeScreenPos(hitCoord);
        vec2 uv = proj.xy;

        // Early-out: clipped / offscreen
        if (uv.x < 0.0 || uv.x > 1.0 ||
            uv.y < 0.0 || uv.y > 1.0)
            continue;

        // Fetch depth once
        float depthDevice = SAMPLE_DEPTH(uv);

        // Sky/clear color -> keep marching
        if (depthDevice >= 1.0)
            continue;

        // Convert depth -> view-space Z
        float depthVS = getViewPos(uv, depthDevice).z;

        // Invalid depth
        if (depthVS > 1e5 || depthVS > 0.99)
            continue;

        // Compare ray's depth to scene depth
        dDepth = hitCoord.z - depthVS;

        // Thickness test: have we crossed geometry?
        if ((dir.z - dDepth) < baseThickness && dDepth <= 0.0)
        {
            // Final refinement step
            vec3 bs = binaryRefinement(dir, hitCoord, dDepth);
            return SSRResult(bs.xy, 1.0, steps, length(hitCoord - dir_in));
        }
    }

    return SSRResult(vec2(0.0), 0.0, maxSteps, 0.0);
}

void MAIN()
{
    // Normal + roughness
    vec4 normalAndRoughness = SAMPLE_NORMAL_AND_ROUGHNESS(INPUT_UV);
    vec3 N = getViewNormal(normalAndRoughness);

    // SSR mask: enables reflections only on selected surfaces
    if (SAMPLE_SSR_MASK(INPUT_UV).a <= 0.5)
    {
        FRAGCOLOR = vec4(0.0);
        return;
    }

    // Reconstruct view-space position
    float depthDevice = SAMPLE_DEPTH(INPUT_UV);
    vec3 viewPos = getViewPos(INPUT_UV, depthDevice);

    // View-space reflection vector
    vec3 viewDir   = normalize(-viewPos);
    vec3 reflected = normalize(-reflect(viewDir, N));

    // Scale ray to avoid marching too close to surface
    vec3 rayDirVS = reflected * max(minRayStep, -viewPos.z);

    // March ray
    SSRResult hit = rayMarch(rayDirVS, viewPos);

    // Fetch reflected color
    vec3 reflColor = SAMPLE_SCENE(hit.uv).rgb;
    FRAGCOLOR = vec4(reflColor, hit.confidence);
}
