// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#define TAU 6.28318530718
#define SQRT2 1.41421356237

layout(binding = 2) uniform sampler2D tex;
layout(binding = 3) uniform sampler2D fontTex;
layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec2 fragCoord;
layout(location = 0) out vec4 fragColor;

// Custom brushes use binding 1
layout(std140, binding = 1) uniform commonUBuf {
    mat3 scissorMat;
    vec2 scissorExt;
    vec2 scissorScale;
    float alphaMult;
    float strokeThr;
    float fontAlphaMin;
    float fontAlphaMax;
    vec4 colorEffects;
    int texType;
    int type;
    float globalAlpha;
    int unused1;
    // Custom input
    float iTime;
    int alphaIsRed;
    float unused2;
    float unused3;
    vec4 data1;
    vec4 data2;
    vec4 data3;
    vec4 data4;
    vec4 unused4[2];
};

float clipMask() {
    vec2 sc = abs(scissorMat * vec3(fragCoord, 1.0)).xy - scissorExt;
    sc = vec2(0.5) - sc * scissorScale;
    return clamp(sc.x, 0.0, 1.0) * clamp(sc.y, 0.0, 1.0);
}

float antialiasingAlpha() {
    return min(1.0, (1.0 - abs(texCoord.x * 2.0 - 1.0)) * alphaMult) * min(1.0, texCoord.y);
}

// Returns text texture alpha as-is in the SDF texture.
// Use smoothstep() to antialiase it as needed.
float sdfFontAlphaRaw() {
    vec2 texSize = vec2(textureSize(fontTex, 0));
    if (alphaIsRed == 0)
        return texture(fontTex, texCoord / texSize).a;
    else
        return texture(fontTex, texCoord / texSize).r;
}

// Returns text texture alpha with default antialiasing.
float sdfFontAlpha() {
    vec2 texSize = vec2(textureSize(fontTex, 0));
    float distance;
    if (alphaIsRed == 0)
        distance = texture(fontTex, texCoord / texSize).a;
    else
        distance = texture(fontTex, texCoord / texSize).r;
    // Smooth interpolation to get anti-aliased edges
    return smoothstep(fontAlphaMin, fontAlphaMax, distance);
}

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
