// Copyright (C) 2024 The Qt Company Ltd.*
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec2 texcoord;

// Linear interpolation, parameters as smoothstep: returns 0 at upper and 1 at lower

float interpolate(float lower, float upper, float x)
{
    return (min(upper, max(lower, x)) - lower) / (upper - lower);
}

void MAIN()
{
    float dist = length(texcoord - vec2(0.5, 0.5));

    float dx = dFdx(dist);
    float dy = dFdy(dist);
    float df = length(vec2(dx, dy));

    const float radius = 0.5;
    float alpha =  (1.0 - interpolate(radius - df, radius, dist)) * opacity;
    float dark = interpolate(radius - 2*df, radius - df, dist);
    float light = (1.0 - dark * contrast) * alpha;

    FRAGCOLOR = vec4(light, light, light, alpha);
}
