// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    implicitHeight: 60
    implicitWidth: 90

    property alias iconSource: icon.source
    property alias labelText: label.text

    Column {
        id: content

        anchors.centerIn: root

        Image {
            id: icon

            anchors.horizontalCenter: content.horizontalCenter
        }

        Label {
            id: label

            color: "white"
            anchors.horizontalCenter: content.horizontalCenter
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }
}
