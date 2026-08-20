// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vNormal;

float fresnel = 1.0;

void MAIN()
{
    BASE_COLOR = vec4(0.0);

    fresnel = smoothstep(0.0, 1.0, abs(dot(VIEW_VECTOR,VAR_WORLD_NORMAL)));
}

void POST_PROCESS()
{
    vec2 centerCoord = vTexCoord * 2.0 - 1.0;
    float centerDist = smoothstep(1.0, 0.0, length(centerCoord));

    vec2 screenPos = gl_FragCoord.xy / textureSize(SCREEN_TEXTURE, 0);
    vec2 screenCoord = screenPos;

    float distortion = 1. * centerDist;

    float depthSample = texture(DEPTH_TEXTURE, screenCoord).x;

    vec3 fNormal = normalize(vNormal);
    vec3 normalCoord = fNormal * 0.5 + 0.5;

    vec3 df = vec3(0.0);
    df.x = texture(dfMask, normalCoord.yz).x;
    df.y = texture(dfMask, normalCoord.xz).y;
    df.z = texture(dfMask, normalCoord.yx).z;

    float dfDot = dot(abs(fNormal), df);

    vec3 fColor = mix(baseColor.rgb, baseColor.rgb * 2.250, vColor.a);
    float a = baseColor.a * dfDot * vColor.a;
    a *= smoothstep(0.0, 1.0, dfDot * fresnel);

    float depthFade = smoothstep(0.0, 0.0025, abs(depthSample - gl_FragCoord.z));
    a *= depthFade;
    COLOR_SUM = vec4(fColor * a, a);
}
