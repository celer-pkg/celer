// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// simplified from https://www.shadertoy.com/view/WdscWN

#version 450

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 iResolution;
};

const float mag = 12.0;

// this function is from https://www.shadertoy.com/view/4djSRW
int hash12(vec2 p, float num_colors)
{
    // set the number of colors to be randomly generated
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return int(floor(fract((p3.x + p3.y) * p3.z) * num_colors));
}

int magnify(vec2 fragCoord, float mag, float num_colors) {
    for (float i = 1.0; i < mag; i++) {
        fragCoord += vec2(sin(fragCoord.y / (100.0 * i)) * 10.0,
                          sin(fragCoord.x / (100.0 * mag * i)) * 10.0) * mag * mag;
    }

    return hash12(floor(fragCoord / pow(3.0, mag)), num_colors);
}

ivec4 get_neighbors(vec2 fragCoord, float mag, float colors) {
    return ivec4(magnify(fragCoord+vec2(0, 1), mag, colors),
        magnify(fragCoord+vec2(1, 0), mag, colors),
        magnify(fragCoord-vec2(0, 1), mag, colors),
        magnify(fragCoord-vec2(1, 0), mag, colors));
}

bool is_next_to(int color, ivec4 neighbors) {
    return (neighbors.x == color)
        || (neighbors.y == color)
        || (neighbors.z == color)
        || (neighbors.w == color);
}

int next_biome(inout int color1, ivec4 neighbors) {
    return (color1 == 0 && is_next_to(2, neighbors)) ? 3
         : (color1 == 3 && is_next_to(2, neighbors)) ? 4
         : (color1 == 2) ? 0
         : (color1 == 1 && is_next_to(0, neighbors)) ? 0
         : (color1 == 4 && is_next_to(0, neighbors)) ? 0
         : color1;
}

int biome(in vec2 fragCoord, float mag, int colors[5]) {
    float num_colors = float(colors.length());
    int color1 = magnify(fragCoord, mag, num_colors);
    ivec4 neighbors;
    while(mag > 1.0) {
        neighbors = get_neighbors(fragCoord, mag, num_colors);
        color1 = next_biome(color1, neighbors);
        mag -= 1.0;
    }
    return colors[int(color1)];
}

void main() {
    vec2 fc = qt_TexCoord0 * iResolution.y * mag;
    vec2 uv = qt_TexCoord0;

    switch (biome(fc, mag, int[](0, 1, 2, 3, 4))) {
        case 0:
            fragColor = vec4(0.2, 0.2, 0.24, 1.0);
            break;
        case 1:
            fragColor = vec4(0.24, 0.16, 0.28, 1.0);
            break;
        case 2:
            fragColor = vec4(0.16, 0.18, 0.22, 1.0);
            break;
        case 3:
            fragColor = vec4(0.10, 0.10, 0.10, 1.0);
            break;
        default:
            fragColor = vec4(0.0, 0.252, 0.27, 1.0);
            break;
    }
    fragColor *= (0.35 + 0.65 * sin(uv.x * 2.35)) * (0.35 + 0.65 * sin(uv.y * 4.2)) * qt_Opacity;
}
