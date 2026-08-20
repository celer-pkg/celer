// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    Rectangle {
        id: displacement
        color: Qt.rgba(0.5, 0.5, 1.0, 1.0)
        anchors.fill: parent
        visible: false
        Image {
            anchors.centerIn: parent
            source: "images/glass_normal.png"
            sourceSize: Qt.size(parent.width/2, parent.height/2)
            smooth: true
        }
    }

    Displace {
        anchors.fill: parent
        source: bug
        displacementSource: displacement
        displacement: 0.1
    }
}
