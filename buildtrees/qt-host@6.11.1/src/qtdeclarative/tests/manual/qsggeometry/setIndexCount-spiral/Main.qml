// Copyright (C) 2024 Stan Morris
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import IndexedSpiral

Window {
    width: spiral.width + 40
    height: spiral.height + 40
    visible: true
    title: qsTr("QSGGeometry::setIndexCount() tester")
    color: "#444"

    // Rectangle { anchors.fill: spiral; color: "#4400FFFF" }
    IndexedSpiralItem {
        id: spiral
        anchors.centerIn: parent
        indexCount: slider.value
    }
    Slider {
        id: slider
        anchors {
            left: parent.left; top: parent.top; bottom: parent.bottom
        }
        orientation: Qt.Vertical
        from: 2 * spiral.maxIndices - 1
        to: 0
        value: spiral.maxIndices
    }
}
