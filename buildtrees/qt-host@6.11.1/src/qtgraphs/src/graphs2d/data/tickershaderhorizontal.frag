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
    bool subTicksVisible;
    float spacing;
    float subTickLength;
    float displacement;
    vec4 subTickColor;
    vec4 tickColor;
    float subTickLineWidth;
    float tickLineWidth;
    float smoothing;
    float subTickScale;
    bool flipped;
};


#define HORIZONTAL 1

float createBars(float coord, float spacing, float strokeWidth) {
    float sbar = 1.0 / spacing;// * 0.5;
    float barWidth = strokeWidth / spacing;// * 0.5;
    float firstPos = -((tickLineWidth / spacing) * 0.5);
    float p = firstPos + coord * sbar;
    //float p = (coord - 0.5) * sbar;
    float bar = abs(2.0 * fract(p) - 1.0);
    float barSmoothness = smoothing / spacing;
    bar = smoothstep(1.0 - barWidth - barSmoothness, 1.0 - barWidth + barSmoothness, bar);
    return bar;
}


void main() {
    {
        float subLines = 0.0;
        float lines = 0.0;
    #if (HORIZONTAL == 1)
        // sublines
        if (subTicksVisible) {
            if (flipped && fragCoord.y < iResolution.y * subTickLength)
                subLines += createBars(fragCoord.x, spacing * subTickScale, subTickLineWidth);
            if (!flipped && fragCoord.y > iResolution.y * (1.0 - subTickLength))
                subLines += createBars(fragCoord.x, spacing * subTickScale, subTickLineWidth);
        }
        // major lines
        lines += createBars(fragCoord.x, spacing, tickLineWidth);
    #else
        // sublines
        if (subTicksVisible) {
            if (flipped && fragCoord.x < iResolution.x * subTickLength)
                subLines += createBars(fragCoord.y, spacing * subTickScale, subTickLineWidth);
            if (!flipped && fragCoord.x > iResolution.x * (1.0 - subTickLength))
                subLines += createBars(fragCoord.y, spacing * subTickScale, subTickLineWidth);
        }
        // major lines
        lines += createBars(fragCoord.y, spacing, tickLineWidth);
    #endif
        // Combine lines, subLines below
        vec3 lineColor = vec3(0.0);
        float tickColorAlpha = lines * tickColor.a;
        float subTickColorAlpha = subLines * subTickColor.a;
        lineColor = mix(lineColor, subTickColor.rgb, subTickColorAlpha);
        lineColor = mix(lineColor, tickColor.rgb, tickColorAlpha);
        fragColor = vec4(lineColor, min(1.0, subTickColorAlpha + tickColorAlpha));
    }
    fragColor = fragColor * qt_Opacity;
}
