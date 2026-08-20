// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vNormal;

float fresnel = 1.0;

void MAIN()
{
    BASE_COLOR = baseColor;
    SPECULAR_AMOUNT = 0.0;
    ROUGHNESS = 0.50;
    METALNESS = 0.0;

    fresnel = smoothstep(0.0, 1.0, abs(dot(VIEW_VECTOR,VAR_WORLD_NORMAL)));
}

void POST_PROCESS()
{
    vec2 centerCoord = vTexCoord*2.0 - 1.0;
    float centerDist = smoothstep(1.0, 0.0, length(centerCoord));

    vec2 screenPos = gl_FragCoord.xy / textureSize(SCREEN_TEXTURE, 0);
    vec2 screenCoord = screenPos;

    float distortion = 1. * centerDist;

    float depthSample = texture(DEPTH_TEXTURE, screenCoord).x;
    vec4 sceneSample = texture(SCREEN_TEXTURE, screenCoord);

    vec3 fNormal = normalize(vNormal);
    vec3 normalCoord = fNormal * 0.5 + 0.5;

    vec3 df = vec3(0.0);
    df.x = texture(dfMask, normalCoord.yz).x;
    df.y = texture(dfMask, normalCoord.xz).y;
    df.z = texture(dfMask, normalCoord.yx).z;

    float dfDot = dot(abs(fNormal), df);

    float a = vColor.a*dfDot;
    a *= mix(smoothstep(0.50, 1.0, dfDot * fresnel), 1.0, pow(fresnel, 3.0));

    float depthFade = smoothstep(0.0, 0.0025, abs(depthSample - gl_FragCoord.z));
    depthFade *= smoothstep(0.50, 1.0, gl_FragCoord.z);
    a *= depthFade;
    COLOR_SUM = vec4((DIFFUSE.rgb + SPECULAR.rgb) * mix(0.50, 1.0, a), a);
}
