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
    int origo;
    vec4 gridVisibility;
    float gridWidth;
    float gridHeight;
    vec2 gridMovement;
    vec4 subGridColor;
    vec4 gridColor;
    float subGridLineWidth;
    float gridLineWidth;
    float smoothing;
    float verticalSubGridScale;
    float horizontalSubGridScale;
    vec4 plotAreaBackgroundColor;
};


float createBars(float coord, float spacing, float strokeWidth) {
    float sbar = 1.0 / spacing;// * 0.5;
    float barWidth = strokeWidth / spacing;// * 0.5;
    float firstPos = -((gridLineWidth / spacing) * 0.5);
    float p = firstPos + coord * sbar;
    //float p = (coord - 0.5) * sbar;
    float bar = abs(2.0 * fract(p) - 1.0);
    float barSmoothness = smoothing / spacing;
    bar = smoothstep(1.0 - barWidth - barSmoothness, 1.0 - barWidth + barSmoothness, bar);
    return bar;
}


void main() {
    {
        // sublines
        float subLines = 0.0;
        if (gridVisibility.z > 0)
            subLines += createBars(fragCoord.x, gridWidth * horizontalSubGridScale, subGridLineWidth);
        if (gridVisibility.w > 0)
            subLines += createBars(fragCoord.y, gridHeight * verticalSubGridScale, subGridLineWidth) * (1.0 - subLines);
        // Major lines
        float lines = 0.0;
        if (gridVisibility.x > 0)
            lines += createBars(fragCoord.x, gridWidth, gridLineWidth);
        if (gridVisibility.y > 0)
            lines += createBars(fragCoord.y, gridHeight, gridLineWidth) * (1.0 - lines);
        // Combine lines, sublines below
        vec3 barColor = plotAreaBackgroundColor.rgb;
        float gridColorAlpha = lines * gridColor.a;
        float subGridColorAlpha = subLines * subGridColor.a;
        barColor = mix(barColor, subGridColor.rgb, subGridColorAlpha);
        barColor = mix(barColor, gridColor.rgb, gridColorAlpha);
        fragColor = vec4(barColor, min(1.0, subGridColorAlpha + gridColorAlpha + plotAreaBackgroundColor.a));
    }
    fragColor = fragColor * qt_Opacity;
}
