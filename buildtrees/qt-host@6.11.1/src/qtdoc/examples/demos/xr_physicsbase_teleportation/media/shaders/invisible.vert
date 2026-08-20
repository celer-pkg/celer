// Copyright (C) 2024 The Qt Company Ltd.*
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

void MAIN()
{
    vec3 pos = VERTEX;
    pos.y += 1000000;
    POSITION = vec4(pos, 1.0);
}
