// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    // qt_Matrix and qt_Opacity must always be both present
    // if the built-in vertex shader is used.
    mat4 qt_Matrix;
    float qt_Opacity;
};

layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D source2;

// https://www.w3.org/TR/compositing-1/#blending
vec3 B(vec3 Cb, vec3 Cs)
{
#if defined(BLEND_NORMAL)
    return Cs;
#elif defined(BLEND_MULTIPLY)
    return Cb * Cs;
#elif defined(BLEND_SCREEN)
    return Cb + Cs - (Cb * Cs);
#elif defined(BLEND_DARKEN)
    return min(Cb, Cs);
#elif defined(BLEND_LIGHTEN)
    return max(Cb, Cs);
#else
    return vec3(0.0);
#endif
}

void main()
{
    vec4 s1 = texture(source, qt_TexCoord0.st);
    vec4 s2 = texture(source2, qt_TexCoord0.st);

    float as = s1.a;
    float ab = s2.a;

    vec3 Cs = s1.rgb;
    vec3 Cb = s2.rgb;

    // "Un-premultiply" source
    float epsilon = 0.0000001;
    Cs /= max(epsilon, as);
    Cb /= max(epsilon, ab);

    vec3 Cr = (1.0 - ab) * Cs + ab * clamp(B(Cb, Cs), 0.0, 1.0);

    // co is premultiplied
    vec3 co = as * Cr + (1.0 - as) * ab * Cb;
    float ao = as + (1.0 - as) * ab;

    fragColor = vec4(co, ao);
}
