// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vViewNormal;
VARYING vec3 vNormal;

float fresnel = 0.0;
vec3 fViewNormal = vec3(0.0);

void MAIN()
{
    BASE_COLOR = vec4(0.0);
    fresnel = 1.0 - max(0.0,dot(VAR_WORLD_NORMAL, VIEW_VECTOR));
    fViewNormal = normalize(vViewNormal);
    ROUGHNESS = 0.25;
    METALNESS = 0.0;
    SPECULAR_AMOUNT = 1.0;
}

void POST_PROCESS()
{
    vec2 centerCoord = fViewNormal.xy;
    float centerDist = smoothstep(1.0, 0.0, length(centerCoord));

    vec2 screenPos = gl_FragCoord.xy/textureSize(SCREEN_TEXTURE, 0);
    vec2 screenCoord = vec2(screenPos);

    vec2 refractionCoord = screenCoord;
    refractionCoord.x = fViewNormal.x > 0.0 ? mix(refractionCoord.x, 1.0, fViewNormal.x) : mix(refractionCoord.x, 0.0, -fViewNormal.x);
    refractionCoord.y = fViewNormal.y > 0.0 ? mix(refractionCoord.y, 0.0, fViewNormal.y) : mix(refractionCoord.y, 1.0, -fViewNormal.y);
    vec3 sceneSample_refr = texture(SCREEN_TEXTURE, refractionCoord).rgb;

    vec2 reflectionCoord = screenCoord;
    reflectionCoord.x = fViewNormal.x > 0.0 ? mix(reflectionCoord.x, 0.0, fViewNormal.x) : mix(reflectionCoord.x, 1.0, -fViewNormal.x);
    reflectionCoord.y = fViewNormal.y > 0.0 ? mix(reflectionCoord.y, 1.0, fViewNormal.y) : mix(reflectionCoord.y, 0.0, -fViewNormal.y);

    float aberrAmount = (1.0 - abs(fViewNormal.x));

    vec2 chromAberr_A = reflectionCoord;
    chromAberr_A.x += (screenCoord.x*2.0-1.0) * aberrAmount;

    vec2 chromAberr_B = reflectionCoord;
    chromAberr_B.y += (screenCoord.y*2.0-1.0) * aberrAmount;

    vec2 chromAberr_C = reflectionCoord;
    chromAberr_C.xy += ((gl_FragCoord.z/gl_FragCoord.y) * 2.0 - 1.0) * aberrAmount;

    float sceneSample_refl_x = texture(SCREEN_TEXTURE, chromAberr_A).r;
    float sceneSample_refl_y = texture(SCREEN_TEXTURE, chromAberr_B).g;
    float sceneSample_refl_z = texture(SCREEN_TEXTURE, chromAberr_C).b;

    vec3 sceneSample_refl = vec3(sceneSample_refl_x,sceneSample_refl_y,sceneSample_refl_z);

    vec3 fColor = (sceneSample_refr + sceneSample_refl / 2.0);
    float a = vColor.a * max(0.25, fresnel);

    vec3 lighting = vec3(DIFFUSE.rgb + SPECULAR.rgb + EMISSIVE.rgb);
    COLOR_SUM = vec4(fColor+lighting, a);
}
