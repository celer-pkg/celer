// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    vec4 tmp = vec4(INSTANCE_MODELVIEWPROJECTION_MATRIX[0][3],
                     INSTANCE_MODELVIEWPROJECTION_MATRIX[1][3],
                     INSTANCE_MODELVIEWPROJECTION_MATRIX[2][3],
                     INSTANCE_MODELVIEWPROJECTION_MATRIX[3][3]);

    float weight = dot(tmp, offset);
    vec3 pos = sin(weight) * MORPH_POSITION(0) + cos(weight) * MORPH_POSITION(1) +
                (1 - sin(weight) - cos(weight)) * VERTEX;

    POSITION = INSTANCE_MODELVIEWPROJECTION_MATRIX * vec4(pos, 1.0);
}
