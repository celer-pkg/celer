// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_INPUT(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
#define SAMPLE_INDIRECT_AND_AO(uv) texture(blurredIndirectAndAoSampler, vec3(uv, VIEW_INDEX))
#else
#define SAMPLE_INPUT(uv) texture(INPUT, uv)
#define SAMPLE_INDIRECT_AND_AO(uv) texture(blurredIndirectAndAoSampler, uv)
#endif

//#define ENABLE_DEBUG_MODE

void MAIN()
{
    vec4 sceneColor = SAMPLE_INPUT(INPUT_UV);
    vec4 indirectAndAo = SAMPLE_INDIRECT_AND_AO(INPUT_UV);

    float ao = indirectAndAo.a;
    vec3 indirect = indirectAndAo.rgb;

#ifndef ENABLE_DEBUG_MODE
    FRAGCOLOR = vec4(sceneColor.rgb * ao * (1.0 + indirect * indirectLightBoost), sceneColor.a);
#else
    if (debugMode == 0)
        FRAGCOLOR = vec4(sceneColor.rgb * ao * (1.0 + indirect * indirectLightBoost), sceneColor.a);
    else if (debugMode == 1)
        FRAGCOLOR = sceneColor;
    else if (debugMode == 2)
        FRAGCOLOR = vec4(sceneColor.rgb * (1.0 + indirect * indirectLightBoost), sceneColor.a);
    else if (debugMode == 3)
        FRAGCOLOR = vec4(sceneColor.rgb * ao, sceneColor.a);
    else if (debugMode == 4)
        FRAGCOLOR = vec4(indirect, 1.0);
    else if (debugMode == 5)
        FRAGCOLOR = vec4(vec3(ao), 1.0);
    else if (debugMode == 6)
        FRAGCOLOR = vec4(indirectAndAo.rgb, 1.0); // just the positions (screen/ndc space, not view)
    else if (debugMode == 7)
        FRAGCOLOR = vec4(indirectAndAo.rgb, 1.0); // just the (view space) normals
#endif
}
