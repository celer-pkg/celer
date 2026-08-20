// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    VERTEX.x += sin(INSTANCE_DATA.x * 4.0 + VERTEX.y) * INSTANCE_DATA.y;
    POSITION = INSTANCE_MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
