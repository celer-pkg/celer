// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#version 440

QC_INCLUDE "common.glsl"

layout(binding = 2) uniform sampler2D tex;
layout(binding = 3) uniform sampler2D fontTex;
layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

float sdfFontAlpha()
{
        vec2 texSize = vec2(textureSize(fontTex, 0));
#ifdef TEXT_A
        float distance = texture(fontTex, texCoord / texSize).a;
#else
        float distance = texture(fontTex, texCoord / texSize).r;
#endif
        // Smooth interpolation to get anti-aliased edges
        return smoothstep(fontAlphaMin, fontAlphaMax, distance);
}

float quadraticSmoothstep(float e0, float e1, float x) {
    x = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    float d = 1.0 + 2.0 * x * (x - 1.0);
    return (x * x) / d;
}

float roundedBox(vec2 pos, vec2 size, float r) {
    vec2 q = abs(pos) - size + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0))) - r;
}

float roundedBox4R(vec2 pos, vec2 size, vec4 r) {
    // Get corner index [0..3] from pos
    int i = int(2.0 + (0.5 * sign(pos.x)) + sign(pos.y));
    vec2 q = abs(pos) - size + r[i];
    return length(max(q, vec2(0.0))) - r[i];
}

float createBars(float coord, float spacing, float strokeWidth) {
    float sbar = 1.0 / spacing;
    float p = coord * sbar;
    float barSmoothness = feather * sbar;
    float barWidth = 1.0 - strokeWidth * sbar;
    float bar = abs(2.0 * fract(p) - 1.0);
    return smoothstep(barWidth - barSmoothness, barWidth + barSmoothness, bar);
}

#ifdef SCISSORING
float clipMask() {
    vec2 sc = abs(scissorMat * vec3(fragCoord, 1.0)).xy - scissorExt;
    sc = vec2(0.5) - sc * scissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}
#endif
#ifdef EDGE_AA
float antialiasingAlpha() {
    float a = min(1.0, (1.0 - abs(texCoord.x * 2.0 - 1.0)) * alphaMult) * min(1.0, texCoord.y);
    return smoothstep(0.0, 1.0, a);
}
#endif

void applyColorEffects(inout vec4 color) {
    if (colorEffects[0] > 0.0) {
        // Contrast + Brightness
        color.rgb = (color.rgb - 0.5 * color.a) * colorEffects[2] +
                0.5 * color.a + colorEffects[1] * color.a;
        // Saturation
        const float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color.rgb = mix(vec3(gray), color.rgb, colorEffects[3]);
    }
}

void main()
{
    vec4 color;
#ifdef EDGE_AA
    const float aaAlpha = antialiasingAlpha();
#ifdef STENCIL_STROKES
    if (aaAlpha < strokeThr) discard;
#endif
#else
    const float aaAlpha = 1.0;
#endif
#ifdef SCISSORING
    const float clip = clipMask();
#else
    const float clip = 1.0;
#endif
    if (type == 0) { // Color
        color = innerCol;
        color *= globalAlpha * aaAlpha * clip;
    } else if (type == 11) { // Stencil fill
        color = vec4(1.0);
    } else {
        vec2 pt = (paintMat * vec3(fragCoord, 1.0)).xy;
        if (type == 1 || type == 2) { // Linear Gradient
            float d = clamp(pt.y / feather, 0.0, 1.0);
            if (type == 2)
                color = texture(tex, vec2(d, 0.5));
            else
                color = mix(innerCol, outerCol, d);
        } else if (type == 3 || type == 4) { // Radial Gradient
            float d = clamp((length(pt) - radius + feather * 0.5) / feather, 0.0, 1.0);
            if (type == 4)
                color = texture(tex, vec2(d, 0.5));
            else
                color = mix(innerCol, outerCol, d);
        } else if (type == 5 || type == 6) { // Conical gradient
            float d = 0.5 - atan(pt.x, pt.y) * PI2_INV;
            if (type == 6)
                color = texture(tex, vec2(d, 0.5));
            else
                color = mix(innerCol, outerCol, d);
        } else if (type == 7 || type == 8) { // Box Gradient
            float d = clamp((roundedBox(pt, extent, radius) + feather) / feather, 0.0, 1.0);
            if (type == 8)
                color = texture(tex, vec2(d, 0.5));
            else
                color = mix(innerCol, outerCol, d);
        } else if (type == 9) { // Box Shadow
            // Note: outerCol contains radius per-corner.
            float d = quadraticSmoothstep(0.0, feather, roundedBox4R(pt, extent, outerCol));
            color = (1.0 - d) * innerCol;
        } else if (type == 10) { // Image
            color = texture(tex, pt / extent);
            if (texType == 1) color = vec4(color.rgb * color.a, color.a);
            if (texType == 2) color = vec4(color.r);
            // Apply color tint and alpha.
            color *= innerCol;
        } else { // Grid pattern
            float hLines = extent.x > 0 ? createBars(pt.x, extent.x, radius) : 0;
            float vLines = extent.y > 0 ? createBars(pt.y, extent.y, radius) : 0;
            float lines = max(hLines, vLines);
            color = mix(outerCol, innerCol, lines);
        }
        color *= globalAlpha * aaAlpha * clip;
    }

    // See if rendering text
    if (fontAlphaMin > -1.0)
        color *= sdfFontAlpha();

    applyColorEffects(color);

    fragColor = color;
}
