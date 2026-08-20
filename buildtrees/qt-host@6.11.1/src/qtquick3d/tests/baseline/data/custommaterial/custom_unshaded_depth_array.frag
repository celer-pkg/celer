// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE(uv) texture(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX))
#else
#define SAMPLE(uv) texture(DEPTH_TEXTURE, uv)
#endif

void MAIN()
{
    // not really using the texture array version, unless multiview rendering is active,
    // nonetheless this test verifies that QSHADER_VIEW_COUNT is defined etc.
#if QSHADER_VIEW_COUNT >= 2
    vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(DEPTH_TEXTURE, 0).xy);
    vec4 depthSample = texture(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX));
#else
    vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(DEPTH_TEXTURE, 0));
    vec4 depthSample = texture(DEPTH_TEXTURE, uv);
#endif

    // test that the #define lines are handled correctly; this is a typical
    // approach to simplify the above without having to repeat the #if
    // QSHADER_VIEW_COUNT everywhere.
    depthSample = SAMPLE(uv);

    float zNear = CAMERA_PROPERTIES.x;
    float zFar = CAMERA_PROPERTIES.y;
    float zRange = zFar - zNear;

    float z_n = 2.0 * depthSample.r - 1.0;
    float d = 2.0 * zNear * zFar / (zFar + zNear - z_n * zRange);
    d /= zFar;

    FRAGCOLOR = vec4(d, d, d, 1.0);
}
