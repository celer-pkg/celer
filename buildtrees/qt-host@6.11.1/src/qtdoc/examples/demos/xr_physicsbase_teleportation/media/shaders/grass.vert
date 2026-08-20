// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;
VARYING vec4 vcolor;
VARYING vec3 vViewVec;

float random(vec2 uv, float seed) {
    return fract(sin(dot(uv.xy, vec2(12.9898 + seed, 78.233 + seed))) * 43758.5453 + seed);
}

mat3 rotateX(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
        1.0, 0.0, 0.0,
        0.0,  c, -s,
        0.0,  s,  c
    );
}

mat3 rotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
         c, 0.0,  s,
        0.0, 1.0, 0.0,
        -s, 0.0,  c
    );
}

mat3 rotateZ(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat3(
         c, -s, 0.0,
         s,  c, 0.0,
        0.0, 0.0, 1.0
    );
}

void MAIN()
{
    texcoord = UV0;
    vec3 pos = VERTEX;
    vcolor = COLOR;

    float tur = texture(turbulence, vcolor.zw + fract(time * windDir)).x;
    float noise = texture(perlin, vcolor.zw).x;

    pos.xz -= vcolor.xy;
    float randx = random(vcolor.zw, 0);
    float randy = random(vcolor.zw, 3.3545);

    float mid = mix(0.9, 0.3, randx);
    float ramp = 1.0 - max(smoothstep(mid, 1.02, texcoord.y),
                           smoothstep(mid, -0.3, texcoord.y));

    pos.y *= noise;
    pos.xz *= ramp;
    pos.xz *= mix(0.3, 1.0, randx);

    vec2 displace = vec2(randx * 2.0 - 1.0, randy * 2.0 - 1.0);

    vec2 windDirStrength = windstrength * 0.4 * windDir;
    vec2 windDirStrengthNoise = windDirStrength * tur;
    vec2 wind = displace * 0.3 + windDirStrength;

    mat3 rotX = rotateX(texcoord.y * windDirStrengthNoise.x +
                        windDirStrengthNoise.x +
                        texcoord.y * wind.x +
                        windDirStrength.x);
    mat3 rotZ = rotateZ(texcoord.y * windDirStrengthNoise.y +
                        windDirStrengthNoise.y +
                        texcoord.y * wind.y +
                        windDirStrength.y);

    vec3 PrePos = pos;
    PrePos.xz += displace * 1.0;
    PrePos.xz += vcolor.xy;
    PrePos = (MODEL_MATRIX * vec4(PrePos, 1.0)).xyz;

#if QSHADER_VIEW_COUNT >= 2
    vec3 direction = normalize(CAMERA_POSITION[VIEW_INDEX] - vec3(PrePos.x, 0.0, PrePos.z));
#else
    vec3 direction = normalize(CAMERA_POSITION - vec3(PrePos.x, 0.0, PrePos.z));
#endif
    mat3 rotY = rotateY(-atan(direction.x, direction.z));

    mat3 rotationMatrix = rotZ * rotX * rotY;

    vec3 centerPos = rotationMatrix * vec3(0.0, pos.y + 1.0, 0.0);

    pos = rotationMatrix * pos;

    vec3 tangent = normalize(centerPos);
    vec3 binormal = cross(tangent, vec3(0., 1.0, 0.));
    vec3 normal = cross(binormal, tangent);

    pos.xz += displace * 1.0;
    pos.xz += vcolor.xy;

    TANGENT = tangent;
    BINORMAL = binormal;
    NORMAL = normal;

    VERTEX = pos;
#if QSHADER_VIEW_COUNT >= 2
    vViewVec = CAMERA_POSITION[VIEW_INDEX] - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
#else
    vViewVec = CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
#endif
    COLOR = vec4(1);
}
