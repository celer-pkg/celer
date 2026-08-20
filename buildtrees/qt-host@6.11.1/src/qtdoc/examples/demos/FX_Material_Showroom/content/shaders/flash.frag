// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;

float fresnel = 1.0;

void MAIN()
{
    BASE_COLOR = vec4(0.0);

    float fresnelBase = abs(dot(VAR_WORLD_NORMAL, VIEW_VECTOR));
    fresnel = smoothstep(0.0, 1.0, fresnelBase);
    fresnel *= vColor.a;
}

void POST_PROCESS()
{
    vec2 centerCoord = vTexCoord * 2.0 - 1.0;
    float centerDist = smoothstep(1.0, 0.0, length(centerCoord));

    vec2 screenPos = gl_FragCoord.xy / textureSize(SCREEN_TEXTURE, 0);
    vec2 screenCoord = vec2(screenPos);

    float distortion = centerDist * 0.5;

    float depthSample = texture(DEPTH_TEXTURE, screenCoord).x;

    vec4 sceneSample = texture(SCREEN_TEXTURE, screenCoord);

    vec3 fColor = sceneSample.rgb * baseColor.rgb;
    float a = baseColor.a;

    float depthFade = smoothstep(0.0, 0.05, abs(depthSample - gl_FragCoord.z));
    depthFade *= smoothstep(0.50, 1.0, gl_FragCoord.z);
    a *= depthFade;
    a *= fresnel;

    COLOR_SUM = vec4((fColor + a) * a * a, a);
}
