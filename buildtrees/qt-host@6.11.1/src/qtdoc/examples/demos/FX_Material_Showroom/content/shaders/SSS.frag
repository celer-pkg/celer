// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

precision lowp int;
precision lowp float;

vec3 diff = vec3(0.0);
vec4 albedo = vec4(0.0);
float specVal = 0.0;
vec3 spec = vec3(0.0);
vec3 sss = vec3(0.0);
vec3 fNormal = vec3(0.0);
vec3 fView = vec3(0.0);
float fresnel = 0.0;
float fresnelBase = 0.0;
float rough = 0.0;
vec3 flakeNormal = vec3(0.0);

float curvature = 0.0;

VARYING vec3 vColor;
VARYING vec3 vPos;

vec3 testCol = vec3(0.0);

void MAIN()
{
    fNormal = normalize(VAR_WORLD_NORMAL);
    fView = normalize(VIEW_VECTOR);
    fresnelBase = 1.0 - max(0.0, dot(fNormal, fView));
    FRESNEL_POWER = 5.0;
    fresnel = pow(1.0 - fresnelBase, FRESNEL_POWER);
    specVal = mix(mix(fresnel * specular, specular, specular), 1.0, metalness + fresnelBase * clearcoat);
    SPECULAR_AMOUNT = specVal;
    albedo = baseColor;
    BASE_COLOR = albedo;

    vec3 vNormalWsDdx = dFdx(VAR_WORLD_NORMAL.xyz);
    vec3 vNormalWsDdy = dFdy(VAR_WORLD_NORMAL.xyz);
    float flGeometricRoughnessFactor = pow(clamp(max(dot(vNormalWsDdx, vNormalWsDdx), dot(vNormalWsDdy, vNormalWsDdy)), 0.0, 1.0), 0.333);
    METALNESS = mix(metalness, 0.0, fresnelBase * clearcoat);
    ROUGHNESS = mix(mix(roughness, 0.0, fresnel), 1.0, flGeometricRoughnessFactor);
    rough = ROUGHNESS;

    curvature = length(fwidth(fNormal));
}

void AMBIENT_LIGHT()
{
    diff += TOTAL_AMBIENT_COLOR;
}

const float PI = 3.141592;
const float e = 2.71828;
float r2 = roughness * roughness;

float sssDepth = 1.0-density;
float sssHalf = sssDepth*0.5;

float PixarBRDF(vec3 normalWS, vec3 lightDirectionWS, vec3 viewDirectionWS)
{
    vec3 H = normalize(lightDirectionWS + viewDirectionWS);
    float NH = dot(normalWS, H) * (1.0 - sssHalf) + sssHalf;
    float VH = dot(viewDirectionWS, H) * (1.0 - sssHalf) + sssHalf;
    float LN = max(0.0, dot(normalWS, lightDirectionWS) * (1.0 - sssHalf) + sssHalf);

    float NH2 = NH * NH;
    float step1 = NH2 - 1.0;
    float secondStepExp = (step1 / (r2 * NH2));
    float step2 = (exp(secondStepExp));
    float step3 = (4.0 * PI * r2 * NH2 * NH * VH);
    float BRDF = max(0.0, step2 / step3) * LN;

    return BRDF;
}

float DisneyDiffuse(vec3 l, vec3 n, vec3 v)
{
    float a = 1.0 - 0.5 * (r2 / (r2 + 0.57));
    float b = 0.45 * (r2 / (r2 + 0.09));

    float nl = max(0.0, dot(n, l) * (1.0 - sssHalf) + sssHalf);
    float nv = max(0.0, dot(n, v) * (1.0 - sssHalf) + sssHalf);

    float ga = dot(v - n * nv, n - n * nl);
    ga = max(0.0, ga);

    return nl * mix(1.0, (a + b * ga * sqrt((1.0 - nv * nv) * (1.0 - nl * nl)) / max(nl, nv)), roughness);
}

float DICE_SSS(vec3 l, vec3 n, vec3 v)
{
    vec3 vLTLight = (l + n);
    float fLTDot = max(0.0, dot(-vLTLight, v) * (1.0 - sssHalf) + sssHalf);

    return fLTDot;
}

void DIRECTIONAL_LIGHT()
{
    float lightDot = smoothstep(1.0 - density, 1.0, dot(-fNormal, TO_LIGHT_DIR));
    lightDot = smoothstep(.650, 0.350, dot(-fNormal, TO_LIGHT_DIR));
    float shadow = mix(SHADOW_CONTRIB, 1.0, lightDot);

    vec3 diffuse = LIGHT_COLOR * DisneyDiffuse(TO_LIGHT_DIR, fNormal, fView);
    vec3 specular = LIGHT_COLOR * PixarBRDF(fNormal, TO_LIGHT_DIR, fView);
    vec3 subsurface = LIGHT_COLOR * DICE_SSS(TO_LIGHT_DIR, fNormal, fView);

    diff += diffuse * shadow;
    spec += specular * shadow;
    sss += subsurface;
}

void POST_PROCESS()
{
    vec3 specularAcc = SPECULAR.rgb + spec;
    vec3 diffuseAcc = albedo.rgb * (DIFFUSE.rgb + diff);
    vec3 totalLight = mix(diffuseAcc, mix(specularAcc, mix(albedo.rgb * specularAcc, specularAcc + diffuseAcc * fresnel, fresnel), metalness), specVal);
    totalLight = (totalLight + subsurfaceColor.rgb * sss * (1.0 - density) * 0.5) * vColor.rgb;
    vec3 clearcoatAcc = specularAcc;
    COLOR_SUM = vec4((totalLight + specularAcc * fresnelBase * clearcoat) + EMISSIVE, opacity);
}
