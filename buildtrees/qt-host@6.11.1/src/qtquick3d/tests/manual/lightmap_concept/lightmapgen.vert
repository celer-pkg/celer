// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    vec2 uv = useUV1 ? UV1 : UV0;
    float u = uv.x * 2.0 - 1.0;
    float v = uv.y * 2.0 - 1.0;

    // For D3D, GL, Metal. Not for Vulkan.
    if (FRAMEBUFFER_Y_UP != NDC_Y_UP)
        v *= -1.0;

    POSITION = vec4(u, v, 0.0, 1.0);
}
