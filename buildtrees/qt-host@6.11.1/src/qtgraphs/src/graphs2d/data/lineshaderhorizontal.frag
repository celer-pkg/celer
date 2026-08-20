// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#version 440

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec3 iResolution;
    vec4 color;
    float lineWidth;
    float smoothing;
};


#define HORIZONTAL 1

float createBars(float coord, float spacing, float strokeWidth) {
    float sbar = 1.0 / spacing;
    float barWidth = strokeWidth / spacing;
    float firstPos = ((barWidth + smoothing / spacing) * 1.0);
    float p = firstPos + coord * sbar;
    //float p = (coord - 0.5) * sbar;
    float bar = abs(2.0 * fract(p) - 1.0);
    float barSmoothness = smoothing / spacing;
    bar = smoothstep(1.0 - barWidth - barSmoothness, 1.0 - barWidth + barSmoothness, bar);
    return bar;
}


void main() {
    {
        float line = 0.0;
    #if (HORIZONTAL == 1)
        float spacing = iResolution.y * 0.5 + lineWidth + smoothing;
        line += createBars(fragCoord.y, spacing, lineWidth);
    #else
        float spacing = iResolution.x * 0.5 + lineWidth + smoothing;
        line += createBars(fragCoord.x, spacing, lineWidth);
    #endif
        vec3 lineColor = vec3(0.0);
        float alpha = line * color.a;
        lineColor = mix(lineColor, color.rgb, alpha);
        fragColor = vec4(lineColor, alpha);
    }
    fragColor = fragColor * qt_Opacity;
}
