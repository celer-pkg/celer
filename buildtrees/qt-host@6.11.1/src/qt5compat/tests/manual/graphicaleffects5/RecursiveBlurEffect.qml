// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    RecursiveBlur {
        anchors.fill: parent
        source: bug
        radius: 7.5
        loops: 20
    }
}
