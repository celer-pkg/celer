// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

void MAIN()
{
    VERTEX.y += sin(uTime + VERTEX.x*10.0) * uAmplitude;
}
