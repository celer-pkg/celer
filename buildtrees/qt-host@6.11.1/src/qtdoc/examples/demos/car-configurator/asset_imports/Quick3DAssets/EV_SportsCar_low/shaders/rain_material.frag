// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec3 normal;
VARYING vec3 binormal;
VARYING vec3 modelPos;
VARYING vec2 texcoord;

const float PI = 3.1415926535;

vec2 distanceLinePoint( vec2 a, vec2 b, vec2 p )
{
    vec2 v = b - a;
    vec2 vp = p - a;
    float r = clamp( dot( v, vp ) / dot( v, v ), 0.0, 1.0 );

    return mix(a - p, b - p, r);
}

vec3 normalMappedNormal(mat3 tsMatrix)
{
    vec3 sNormal = texture(uNormal, texcoord).xyz * 2.0 - vec3(1.0);
    return tsMatrix * normalize(sNormal);
}

vec2 rainCoord(vec3 normal, float N)
{
    vec2 tc = vec2(0.0, 0.0);
    if (normal.y < (1.0 - 0.0001)) {
        vec3 pos = modelPos;
        vec3 dir = -normalize(vec3(normal.x, 0.0, normal.z));
        pos += dir;
        tc.x = pos.x;
        tc.y = pos.z;
    } else {
        vec3 pos = modelPos;
        tc.x = pos.x;
        tc.y = pos.z;
    }
    return tc * rainScale * N;
}

vec3 rainNormal(vec3 normal, float N)
{
    vec2 tc;
    vec2 tc2 = rainCoord(normal, N);
    vec2 rn = normalize(vec2(normal.z, normal.x));
    vec2 nn = mod(tc2, 1.0);

    tc.x = rn.x * nn.x - rn.y * nn.y;
    tc.y = rn.y * nn.x + rn.x * nn.y;

    vec2 UV = tc;
    vec2 uvDrip = N * (UV * dripWaveSize + vec2( cos( UV.y * dripWaveFreq.x ), sin( UV.x * dripWaveFreq.y ) ) * dripWavePower + vec2(0.0, -uTime * dripSpeed));
    vec2 n = vec2( 0.5, 0.5 );
    float len = dripLength * 0.5;

    vec2 v = distanceLinePoint( vec2( 0.5, 0.5 - len ), vec2( 0.5, 0.5 + len ), mod( uvDrip, 1.0 ) );
    n = mix( n, n - v * dripSharpness, float( length( v ) < mod( uvDrip.y, 1.0 ) * dripSize ) );

    vec3 nv = normalize( vec3( n, -1.0 ) );
    return nv;
}

void MAIN()
{
    vec3 nn = normalize(normal);
    vec3 bn = normalize(binormal);
    vec3 tan = cross(bn, nn);
    mat3 tsMatrix = mat3(tan, bn, nn);
    vec3 sNormal = normalize(normalFactor * normalMappedNormal(tsMatrix) + normal);
    vec3 tsNormal = vec3(0.0);

    for (int i = 0; i < 3; i++)
        tsNormal += rainNormal(sNormal, float(i + 2));

    vec3 wsNormal = tsMatrix * normalize(tsNormal);

    vec3 dn = wsNormal - normal;
    vec2 dt = vec2(dot(bn, dn), dot(tan, dn)) * 0.001;

    vec4 baseColor = uBaseColor;
    if (sNormal.y > 0.0) {
        if (uBaseEnabled)
            baseColor *= texture(uBase, texcoord + dt);
        NORMAL = normalize(wsNormal * rainStrength + normal);
        //ROUGHNESS = uRoughness * (0.8 - 0.5 * dot(NORMAL, normal));

    } else {
        if (uBaseEnabled)
            baseColor *= texture(uBase, texcoord);
        NORMAL = normal;

    }
    baseColor.a *= uOpacity;
    ROUGHNESS = uRoughness;
    BASE_COLOR = baseColor;
    FRESNEL_POWER = uFresnelPower;
    FRESNEL_SCALE = uFresnelScale;
    FRESNEL_BIAS  = uFresnelBias;
    METALNESS = uMetalness;
    IOR = uIOR;
    SPECULAR_AMOUNT = uSpecularAmount;
    CLEARCOAT_AMOUNT = uClearcoatAmount;
    CLEARCOAT_ROUGHNESS = uClearcoatRoughnessAmount;
    CLEARCOAT_FRESNEL_POWER = uClearcoatFresnelPower;
    CLEARCOAT_FRESNEL_SCALE = uClearcoatFresnelScale;
    CLEARCOAT_FRESNEL_BIAS = uClearcoatFresnelBias;
}
