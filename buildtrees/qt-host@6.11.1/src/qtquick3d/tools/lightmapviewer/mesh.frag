// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

VARYING vec2 texcoord;

void MAIN() {
    if (debugUV) {
        FRAGCOLOR = vec4(texcoord.x, texcoord.y, 0.0, 1.0);
    } else {
        vec4 texel = texture(baseMap, texcoord);
        FRAGCOLOR = vec4(texel.rgb, 1.0);
    }
}
