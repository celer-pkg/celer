// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: button
    signal clicked
    property string text: "hello"
    property bool enabled: true
    opacity: enabled ? 1.0 : 0.5

    Rectangle {
        x: 5
        y: 5
        width: parent.width - 10
        height: parent.height - 10
        radius: 5
        color: "lightsteelblue"

        Text {
            anchors.fill: parent
            color: "white"
            text: button.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: if (button.enabled) button.clicked()
        }
    }
}
