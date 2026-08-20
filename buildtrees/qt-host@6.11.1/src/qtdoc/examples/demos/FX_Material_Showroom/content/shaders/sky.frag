// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

precision lowp int;
precision lowp float;

vec3 fView = vec3(0.0);

void MAIN()
{
    fView = normalize(VIEW_VECTOR);
}

void POST_PROCESS()
{
    vec2 skyCoord = fView.xy;
    skyCoord.y = -skyCoord.y * 0.5 + 0.5;
    skyCoord.x = degrees(atan(fView.x, fView.z)) / 360.0 + 0.5;
    vec3 sky = texture(skyboxTexture, skyCoord.xy).rgb;
    float horizonmask = (1.0 - abs(fView.y)) * fogAmount;
    float mask = mix(horizonmask, 1.0, max(0.0, (fogAmount - 0.5)) * 2.0);
    sky = mix(sky, fogColor.rgb, mask);

    COLOR_SUM = vec4(sky, 1.0);
}
