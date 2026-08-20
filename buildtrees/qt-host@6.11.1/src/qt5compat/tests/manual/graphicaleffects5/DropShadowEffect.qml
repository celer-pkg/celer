// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    DropShadow {
        anchors.fill: parent
        source: butterfly
        horizontalOffset: 3
        verticalOffset: 3
        color: "#000000"
    }
}
