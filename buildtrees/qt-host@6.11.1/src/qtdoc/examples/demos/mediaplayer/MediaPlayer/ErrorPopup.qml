// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Fusion
import MediaControls

Popup {
    id: errorPopup
    anchors.centerIn: Overlay.overlay
    padding: 30

    property alias errorMsg: label.text

    onOpened: closeTimer.restart()

    background: Rectangle {
        color: "transparent"
    }

    Column {
        spacing: 15

        Image {
            source: ControlImages.iconSource("Error", false)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: label
            color: "#FFE353"
            font.pixelSize: 24
        }
    }

    Timer {
        id: closeTimer
        interval: 5000
        onTriggered: errorPopup.close()
    }
}
