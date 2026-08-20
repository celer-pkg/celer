// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

VARYING vec3 ray_direction_model; // The direction from camera eye to vertex in model space

//! [main]
void MAIN()
{
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
    ray_direction_model = VERTEX - (inverse(MODEL_MATRIX) * vec4(CAMERA_POSITION, 1.0)).xyz;
}
//! [main]
