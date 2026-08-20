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

    vec4 k;
};

layout(binding = 1) uniform sampler2D source;
layout(binding = 2) uniform sampler2D source2;

void main()
{
    vec4 s1 = texture(source, qt_TexCoord0.st);
    vec4 s2 = texture(source2, qt_TexCoord0.st);

#if defined(COMPOSITE_ARITHMETIC)
    fragColor = k.x * s1 * s2 + k.y * s1 + k.z * s2 + k.w;
#else
    // General Porter-Duff
    // https://www.w3.org/TR/compositing-1/#porterduffcompositingoperators

    float as = s1.a;
    float ab = s2.a;

    vec3 Cs = s1.rgb;
    vec3 Cb = s2.rgb;

#  if defined(COMPOSITE_OVER)
    float Fa = 1.0;
    float Fb = 1.0 - as;
#  elif defined(COMPOSITE_IN)
    float Fa = ab;
    float Fb = 0.0;
#  elif defined(COMPOSITE_OUT)
    float Fa = 1.0 - ab;
    float Fb = 0.0;
#  elif defined(COMPOSITE_ATOP)
    float Fa = ab;
    float Fb = 1.0 - as;
#  elif defined(COMPOSITE_XOR)
    float Fa = 1.0 - ab;
    float Fb = 1.0 - as;
#  else // COMPOSITE_LIGHTER
    float Fa = 1.0;
    float Fb = 1.0;
#  endif

    vec3 co = as * Fa * Cs + ab * Fb * Cb;
    float ao = as * Fa + ab * Fb;

    fragColor = clamp(vec4(co, ao), 0.0, 1.0);

#endif // COMPOSITE_ARITHMETIC

}
