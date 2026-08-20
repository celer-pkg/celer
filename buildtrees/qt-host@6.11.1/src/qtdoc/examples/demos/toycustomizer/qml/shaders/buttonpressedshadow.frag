// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 440

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec3 iResolution;
    vec2 rectSize;
    float radius;
    float blur;
    vec4 shadowColor;
    float thickness;
};

layout(binding = 1) uniform sampler2D iSource;

float roundedBox(vec2 centerPos, vec2 size, float radii) {
    return length(max(abs(centerPos) - size + radii, 0.0)) - radii;
}


void main() {
    fragColor = texture(iSource, texCoord);
    {
        const vec4 tex = texture(iSource, texCoord);

        if (tex.a <= 0.0) {
            fragColor = tex;
        } else {
            const float blur_ = blur * rectSize.y;
            const vec2 center = fragCoord - iResolution.xy * 0.5;
            const vec2 offset = vec2(0, rectSize.y * thickness);
            const vec2 scale = vec2(0.5, 1);

            // Shadow
            const float shadowDist = roundedBox(center - offset, rectSize * scale, radius);
            const float shadowAlpha = 1.0 - smoothstep(0.0, blur_, shadowDist);
            const vec4 bluryShadow = shadowColor * (1.0 - shadowAlpha * shadowAlpha);

            fragColor = bluryShadow;
        }
    }

    fragColor = fragColor * qt_Opacity;
}
