// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#else
#define SAMPLE_NORMAL_AND_ROUGHNESS(uv) texture(NORMAL_ROUGHNESS_TEXTURE, uv)
#endif

void MAIN()
{
    float r = clamp(SAMPLE_NORMAL_AND_ROUGHNESS(INPUT_UV).a, 0.0, 1.0);
    bool roughOK = r < roughnessCut;
    bool eligible = roughOK;
    FRAGCOLOR = vec4(0.0, 0.0, 0.0, eligible ? 1.0 : 0.0);
}
