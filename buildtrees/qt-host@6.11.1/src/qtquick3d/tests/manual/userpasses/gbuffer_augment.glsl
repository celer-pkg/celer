// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN_FRAGMENT_AUGMENT()
{
    vec3 baseColor   = BASE_COLOR.rgb;
    float metalness  = METALNESS;
    float roughness  = ROUGHNESS;
    vec3 worldNormal = normalize(WORLD_NORMAL);

    // GBuffer 0: albedo + metalness
    GBUFFER0 = vec4(baseColor, metalness);

    // GBuffer 1: normal (encoded to 0..1) + roughness
    GBUFFER1 = vec4(worldNormal * 0.5 + 0.5, roughness);

    // GBuffer 2: world position
    GBUFFER2 = vec4(qt_varWorldPos, 1.0);
}
