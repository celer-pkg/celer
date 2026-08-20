// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 vTexCoord;
VARYING vec4 vColor;
VARYING vec3 vPos;

float fresnel = 0.0;
vec3 fView = vec3(1.0);
vec3 fNormal = vec3(0.0);
float a = 1.0;

void MAIN()
{
    BASE_COLOR = vec4(0.0);
    fView = normalize(VIEW_VECTOR);
    fNormal = normalize(VAR_WORLD_NORMAL);
    fresnel = smoothstep(0.0, 1.0, abs(dot(fView, fNormal)));
    ROUGHNESS = 1.;
    METALNESS = 0.0;
    SPECULAR_AMOUNT = 0.0;

    float depthSample = texture(DEPTH_TEXTURE, gl_FragCoord.xy / textureSize(SCREEN_TEXTURE, 0)).x;
    float depthFade = smoothstep(0.0, 0.0025, abs(depthSample - gl_FragCoord.z));
    depthFade *= smoothstep(0.5, 1.0, gl_FragCoord.z);

    a = depthFade;
}

#define MAX_MARCHING_STEPS 100
#define MIN_DISTANCE 0.01
#define MAX_DENSITY 10.0

vec3 smokeSource = vec3(1.0);

float hash(float n)
{
    return fract(sin(n) * 1399763.5453123);
}

float noise3D(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 157.0 + 113.0 * p.z;

    return mix(mix(mix(hash(n + 0.0),   hash(n + 1.0),f.x),
                   mix(hash(n + 157.0), hash(n + 158.0),f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0),f.x),
                   mix(hash(n + 270.0), hash(n + 271.0),f.x), f.y), f.z);
}

float smokeDensity(vec3 position)
{
    float dist = length(position);
    float noise = noise3D(position * .10) * 0.5 + 0.5;

    return clamp(densityFalloff / pow(dist, 2.0), 0.0, MAX_DENSITY) * noise;
}

vec4 smokeColor(vec3 position, vec3 ray)
{
    float totalDensity = 0.0;
    float fstep = MIN_DISTANCE;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        vec3 samplePos = position + ray * fstep;
        float density = smokeDensity(samplePos);
        totalDensity += density;
        fstep += density * stepSize;
    }

    return vec4(vec3(totalDensity), 1.0);
}

void POST_PROCESS()
{
    vec3 ray = fView;
    vec4 smoke = smokeColor(vPos, ray);

    vec2 screenPos = gl_FragCoord.xy / textureSize(SCREEN_TEXTURE, 0);
    vec4 sceneSample = texture(SCREEN_TEXTURE, screenPos);

    a = (smoke.r / MAX_DENSITY) * baseColor.a;

    COLOR_SUM = vec4(baseColor.rgb, a);
}
