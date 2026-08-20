// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    vec4 pos = (BONE_TRANSFORMS[JOINTS.x] * WEIGHTS.y + BONE_TRANSFORMS[JOINTS.y] * WEIGHTS.x)
                    * vec4(VERTEX, 1.0);
    POSITION = MODELVIEWPROJECTION_MATRIX * pos;
}
