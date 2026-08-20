// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING flat vec2 v;

void MAIN()
{
    BASE_COLOR = vec4(v.x, v.y, 1.0, 1.0);
}
