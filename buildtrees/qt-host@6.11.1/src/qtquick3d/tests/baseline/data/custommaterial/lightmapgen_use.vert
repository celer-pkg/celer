// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec2 uv1;

void MAIN()
{
    uv1 = UV1;

    // The source View3D applied a flip when running with OpenGL, just
    // to get the on-screen lightmap identical with all graphics
    // APIs. Now we apply the opposite. Normally this would not be
    // necessary.
    if (FRAMEBUFFER_Y_UP == 1.0)
        uv1.y = 1.0 - uv1.y;

    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
