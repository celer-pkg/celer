// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

VARYING vec4 TexCoordBLL;
VARYING vec4 TexCoordTLT;
VARYING vec4 TexCoordTRR;
VARYING vec4 TexCoordBRB;

void MAIN()
{
    vec2 delta = vec2(1.0 / INPUT_SIZE.x, 1.0 / INPUT_SIZE.y);
    TexCoordBLL = vec4(INPUT_UV.st, INPUT_UV.st) + vec4(-delta.xy, -delta.x, 0);
    TexCoordTLT = vec4(INPUT_UV.st, INPUT_UV.st) + vec4(-delta.x, delta.y, 0, delta.y);
    TexCoordTRR = vec4(INPUT_UV.st, INPUT_UV.st) + vec4(delta.xy, delta.x, 0);
    TexCoordBRB = vec4(INPUT_UV.st, INPUT_UV.st) + vec4(delta.x, -delta.y, 0, -delta.y);
}
