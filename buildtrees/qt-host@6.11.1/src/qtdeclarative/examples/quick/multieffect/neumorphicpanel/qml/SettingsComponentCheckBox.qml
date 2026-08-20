// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Effects
import QtQuick.Controls.Basic

Item {
    id: control

    property alias text: checkBox.text
    property alias checked: checkBox.checked

    property real animatedPressed: checkBox.down ? 1.0 : 0.0
    property real animatedChecked: checkBox.checked ? 1.0 : 0.0
    Behavior on animatedPressed {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    x: 10 * dp
    width: checkBox.width
    height: checkBox.height
    opacity: enabled ? 1.0 : 0.5

    CheckBox {
        id: checkBox
        spacing: 10
        indicator: Rectangle {
            implicitWidth: 26
            implicitHeight: 26
            x: checkBox.leftPadding
            y: parent.height / 2 - height / 2
            opacity: enabled ? 1 : 0.3
            radius: height * 0.5
            color: Qt.lighter(mainWindow.mainColor, 0.3)
            border.width: 1
            border.color: Qt.lighter(mainWindow.mainColor, 0.1)
            RectangularShadow {
                // Inner Glow
                anchors.fill: parent
                anchors.margins: blur
                radius: height * 0.4
                blur: height * 0.2
                color: Qt.lighter(mainWindow.mainColor, 0.4 + animatedPressed * 0.2 + animatedChecked * 0.6)
            }
            RectangularShadow {
                // Glow
                anchors.fill: parent
                z: -1
                radius: height * 0.4
                blur: 20
                opacity: 0.1 + animatedPressed * 0.4
                color: Qt.lighter(mainWindow.mainColor, 1.2)
            }
        }

        contentItem: Text {
            text: checkBox.text
            opacity: enabled ? 1.0 : 0.3
            verticalAlignment: Text.AlignVCenter
            leftPadding: checkBox.indicator.width + checkBox.spacing
            color: "#e0e0e0"
            font.pixelSize: 16 * dp
        }
    }
}
