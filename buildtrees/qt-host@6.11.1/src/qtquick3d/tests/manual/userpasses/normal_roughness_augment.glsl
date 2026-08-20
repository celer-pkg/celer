// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void foo(inout float a)
{
    a += 0.2;
}

void bar(inout float b)
{
    b += 0.2;
}

void MAIN_FRAGMENT_AUGMENT()
{
    // Hello from augmented shader
    float x = 0.0;
    foo(x);
    bar(x);

    //fragOutput = vec4(x, 0, 0, 1.0);
    NORMAL_ROUGHNESS = vec4(qt_world_normal, qt_roughnessAmount);
    BASECOLOR_OUTPUT = BASE_COLOR;
    DIFFUSELIGHT_OUTPUT = vec4(DIFFUSE_LIGHT, 1.0) * vec4(animatingColor, 1.0);
}
