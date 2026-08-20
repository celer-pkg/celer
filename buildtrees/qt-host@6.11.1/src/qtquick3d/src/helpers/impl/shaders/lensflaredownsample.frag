// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "texturesample.glsllib"

void MAIN()
{
    FRAGCOLOR = vec4(max(vec3(0.0), SAMPLE(INPUT, INPUT_UV).rgb - lensFlareBloomBias) * lensFlareBloomScale, 1.0);
}
