// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

vec3 sRGBToLinear(vec3 srgb) {
    bvec3 cutoff = lessThanEqual(srgb, vec3(0.04045));
    vec3 lower = srgb / 12.92;
    vec3 higher = pow((srgb + 0.055) / 1.055, vec3(2.4));
    return mix(higher, lower, vec3(cutoff));
}

vec4 sRGBToLinear(vec4 srgbIn)
{
    return vec4(sRGBToLinear(srgbIn.rgb), srgbIn.a);
}

void MAIN()
{
    vec2 uv = INPUT_UV;
    vec2 texelSize = 1.0 / vec2(textureSize(INPUT, 0));

    float currentColor = texture(INPUT, uv).x;
    float prevColor = texture(lastColorTexture, uv).x;

    float accumulation = mix(currentColor, min(prevColor * 1.11, 1.), 0.9);
    float dist = distance(uv, brushPositionInUV);
    accumulation *= smoothstep(0.0, brushSize, dist);
    FRAGCOLOR = vec4(accumulation);
}
