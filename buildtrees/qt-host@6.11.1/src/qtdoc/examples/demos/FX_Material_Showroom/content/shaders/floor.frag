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
float fresnel = 0.0;
float fresnelBase = 0.0;
float rough = 0.0;
vec3 flakeNormal = vec3(0.0);

float curvature = 0.0;

VARYING vec3 vColor;
VARYING vec3 vPos;
VARYING vec2 vTexcoord;

VARYING vec3 vNormal;
VARYING vec3 vBinormal;
VARYING vec3 vTangent;
VARYING mat3 TBNMat;

float ao = 0.0;
vec3 testCol = vec3(0.0);

void MAIN()
{
    vec3 aoSrc = texture(aoTexture,(((UV0 - 0.5) * (1.0 / aoMaskScale.xy)) + 0.5)+aoMaskPos.xy).rgb;
    ao = dot(aoMask.rgb, aoSrc);

    mat3 fTBN = mat3(normalize(vTangent), normalize(vBinormal), normalize(vNormal));

    vec4 matParams = texture(paramsTexture, vPos.xz * 0.2);

    fNormal = texture(normalsTexture, vPos.xz * 0.1).rgb * 2.0 - 1.0;
    fNormal.xy *= 0.25;
    fNormal = normalize(fNormal);
    fNormal = normalize(fTBN * fNormal);
    fresnelBase = 1.0 - max(0.0, dot(fNormal, VIEW_VECTOR));

    NORMAL = fNormal;

    fresnel = 1.0-max(0.0, dot(fNormal, VIEW_VECTOR));
    FRESNEL_POWER = 5.0;
    fresnel = pow(fresnel, FRESNEL_POWER);

    specVal = mix(mix(mix(fresnel * specular, specular, specular), 1.0, metalness), 1.0, fresnelBase * clearcoat);
    SPECULAR_AMOUNT = specVal;
    albedo = baseColor;
    BASE_COLOR = albedo;

    vec3 vNormalWsDdx = dFdx(fNormal.xyz);
    vec3 vNormalWsDdy = dFdy(fNormal.xyz);
    float flGeometricRoughnessFactor = pow(clamp(max(dot(vNormalWsDdx, vNormalWsDdx), dot(vNormalWsDdy, vNormalWsDdy)), 0.0, 1.0), 0.333);
    METALNESS = mix(metalness, 0.0, fresnelBase * clearcoat);
    ROUGHNESS = mix(mix(mix(matParams.r*roughness, 0.0, fresnel), 0.0, fresnelBase*clearcoat), 1.0, flGeometricRoughnessFactor);
    rough = ROUGHNESS;

    curvature = length(fwidth(fNormal));
}

float shadowAcc = 1.0;

void AMBIENT_LIGHT()
{
    diff += TOTAL_AMBIENT_COLOR * ao;
}

const float PI = 3.141592;
const float e = 2.71828;
float r2 = roughness * roughness;

float sssDepth = 1.0 - density;
float sssHalf = sssDepth * 0.5;

void SPECULAR_LIGHT()
{
}

float PixarBRDF(vec3 normalWS, vec3 lightDirectionWS, vec3 viewDirectionWS)
{
    vec3 H = normalize(lightDirectionWS + viewDirectionWS);
    float NH = (dot(normalWS, H));
    NH = NH * (1.0 - sssHalf) + sssHalf;
    float NH2 = NH * NH;
    float NH3 = NH2 * NH;
    float VH = (dot(viewDirectionWS, H));
    VH = VH * (1.0 - sssHalf) + sssHalf;
    float LN = dot(normalWS, lightDirectionWS);
    LN = LN * (1.0 - sssHalf) + sssHalf;
    LN=max(0.0, LN);

    float step1 = NH2 - 1.0;
    float secondStepExp = (step1 / (r2 * NH2));
    float step2 = (pow(e, secondStepExp));
    float step3 = (4.0 * PI * r2 * NH3 * VH);
    float BRDF = max(0.0,step2 / step3) * LN;

    return BRDF;
}

float DisneyDiffuse(vec3 l, vec3 n, vec3 v)
{
    float a = 1.0 - 0.5 * (r2 / (r2 + 0.57));
    float b = 0.45 * (r2 / (r2 + 0.09));

    float nl = dot(n, l);
    nl = nl * (1.0 - sssHalf) + sssHalf;
    float nv = (dot(n, v));
    nv = nv * (1.0 - sssHalf) + sssHalf;

    float ga = dot(v - n * nv, n - n * nl);

    return max(0.0, nl) * mix(1.0, (a + b * max(0.0, ga) * sqrt((1.0 - nv * nv) * (1.0 - nl * nl)) / max(nl, nv)), roughness);
}

float DICE_SSS(vec3 l, vec3 n, vec3 v)
{
    vec3 vLTLight = (l + n * sssDistortion);
    float fLTDot = pow(max(dot(v, -vLTLight), 0.0), sssPower) * sssScale;
    float flt = 1.0 / length(l) * fLTDot * sssDepth;

    return flt;
}


void DIRECTIONAL_LIGHT()
{
    float lightDot = smoothstep(1.0, density, dot(-fNormal, TO_LIGHT_DIR));
    float shadow = mix(SHADOW_CONTRIB, 1.0, lightDot * sssDepth);
    shadowAcc *= shadow;
    diff += LIGHT_COLOR * DisneyDiffuse(TO_LIGHT_DIR, fNormal, VIEW_VECTOR) * shadow;
    spec += LIGHT_COLOR * PixarBRDF(fNormal, TO_LIGHT_DIR, VIEW_VECTOR) * shadow;
    sss += LIGHT_COLOR * DICE_SSS(TO_LIGHT_DIR, fNormal, VIEW_VECTOR);
}

void POST_PROCESS()
{
    vec3 specularAcc = SPECULAR.rgb + spec;
    vec3 diffuseAcc = albedo.rgb * (DIFFUSE.rgb + diff);
    vec3 totalLight = (mix(diffuseAcc, mix(specularAcc, mix(albedo.rgb * specularAcc, specularAcc + diffuseAcc * fresnel, fresnel), metalness), specVal) + subsurfaceColor.rgb * sss * (1.0 - density) * 0.5) * vColor.rgb;
    vec3 clearcoatAcc = specularAcc;

    float brightAO = ao * 0.5;
    float darkAO = ao;
    float aoRefined = mix(darkAO, brightAO, shadowAcc);

    float a = smoothstep(0.50, .750, length(UV0 * 2.0 - 1.0));
    vec3 col = (totalLight + specularAcc * fresnelBase * clearcoat * shadowAcc) + EMISSIVE;
    col = mix(col.rgb, vec3(0.0), aoRefined);

    COLOR_SUM = vec4(col, 1.0);
}
