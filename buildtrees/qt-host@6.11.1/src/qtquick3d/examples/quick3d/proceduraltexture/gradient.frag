// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 440


layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;

layout(std140, binding = 0) uniform buf {
    vec4 startColor;
    vec4 endColor;
};

void main()
{
    // Calculate the gradient based on the UV coordinates
    float gradientFactor = uv_coord.x; // Use the y-coordinate for vertical gradient
    vec4 color = mix(startColor, endColor, gradientFactor);

    // Set the output color
    fragOutput = color;
}
