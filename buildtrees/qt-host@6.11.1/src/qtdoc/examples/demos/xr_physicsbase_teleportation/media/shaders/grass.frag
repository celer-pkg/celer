// Copyright (C) 2024 The Qt Company Ltd.*
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;
VARYING vec4 vcolor;
VARYING vec3 vViewVec;

void MAIN()
{
    vec2 tc = texcoord;

    vec3 base = qt_sRGBToLinear(texture(grass_base, tc).xyz);
    float noise = texture(perlin, vcolor.zw).x;

    float invert = sign(dot(NORMAL, vViewVec));
    NORMAL *= invert;
    TANGENT *= invert;
    BINORMAL *= invert;

    ROUGHNESS = 1.;
    METALNESS = 0;
    OCCLUSION_AMOUNT = (smoothstep(0, 0.4, tc.y) + 0.1) * (1.0 - noise + 0.2);
    BASE_COLOR = vec4(base * baseColor.xyz, 1.0);
}
