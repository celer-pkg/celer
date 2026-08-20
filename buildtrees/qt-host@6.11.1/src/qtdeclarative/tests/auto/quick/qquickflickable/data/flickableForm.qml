// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick

Flickable {
    id: flickable
    width: 220; height: 100
    contentWidth: 410; contentHeight: 170

    function goHome() {
        flickable.flickTo(Qt.point(0, 0))
    }

    Instantiator {
        model: 4
        Rectangle {
            x: (index + 1) * 50
            y: 10 + index * 40
            width: (index + 1) * 50
            height: 30
            color: "lightgrey"; border.color: "black"
            parent: flickable.contentItem
        }
    }

    Instantiator {
        model: 4
        Text {
            parent: flickable.contentItem
            x: 10 + index * 50
            y: 10 + index * 40
            width: 36
            height: 30
            verticalAlignment: Text.AlignVCenter
            text: "Field " + index
            font.pixelSize: 10
        }
    }
}
