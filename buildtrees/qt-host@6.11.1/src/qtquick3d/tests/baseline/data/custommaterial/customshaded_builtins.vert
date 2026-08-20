// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

void MAIN()
{
    VERTEX.x += sin(uTime * 4.0 + VERTEX.y) * uAmplitude;
}
