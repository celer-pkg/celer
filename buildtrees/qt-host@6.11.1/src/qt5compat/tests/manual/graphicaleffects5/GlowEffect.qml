// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    color: "black"
    Glow {
        anchors.fill: parent
        radius: 8
        color: "white"
        source: butterfly
    }
}
