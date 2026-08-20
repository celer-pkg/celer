// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;
VARYING vec4 vcolor;

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

    float randx = random(vcolor.zw, 0);
    float randy = random(vcolor.zw, 3.3545);

    pos.y *= mix(0.5, 1.0, randx);
    pos.xz *= min(texcoord.y * 5. + baseSize, 1.0);

    vec2 displace = vec2(randx * 2.0 - 1.0, randy * 2.0 - 1.0);

    vec2 windDirStrength = windstrength * 0.6 * windDir;
    vec2 windDirStrengthNoise = windDirStrength * tur;
    vec2 wind = displace * 0.3 + windDirStrength;

    mat3 rotX = rotateX(texcoord.y * windDirStrengthNoise.x +
                        texcoord.y * wind.x +
                        windDirStrength.x * 0.3);
    mat3 rotZ = rotateZ(texcoord.y * windDirStrengthNoise.y +
                        texcoord.y * wind.y +
                        windDirStrength.y * 0.3);

#if QSHADER_VIEW_COUNT >= 2
    vec3 direction = normalize(CAMERA_POSITION[VIEW_INDEX] - vec3(pos.x, 0.0, pos.z));
#else
    vec3 direction = normalize(CAMERA_POSITION - vec3(pos.x, 0.0, pos.z));
#endif
    mat3 rotY = rotateY(3.1415 * vcolor.x);

    mat3 rotationMatrix = rotZ * rotX * rotY;

    pos = rotationMatrix * pos;

    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(pos, 1.0);
}
