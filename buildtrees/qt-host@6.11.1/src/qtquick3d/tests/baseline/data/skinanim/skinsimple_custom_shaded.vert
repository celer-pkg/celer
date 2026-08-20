// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    JOINTS.x = 0;
    JOINTS.y = 1;
    if (VERTEX.y < 0.1)
        WEIGHTS = vec4(1.0, 0.0, 0.0, 0.0);
    else if (VERTEX.y < 0.6)
        WEIGHTS = vec4(0.75, 0.25, 0.0, 0.0);
    else if (VERTEX.y < 1.1)
        WEIGHTS = vec4(0.5, 0.5, 0.0, 0.0);
    else if (VERTEX.y < 1.6)
        WEIGHTS = vec4(0.25, 0.75, 0.0, 0.0);
    else
        WEIGHTS = vec4(0.0, 1.0, 0.0, 0.0);
}
