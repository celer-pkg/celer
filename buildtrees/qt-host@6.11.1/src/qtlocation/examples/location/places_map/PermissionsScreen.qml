// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: root

    property bool requestDenied: false

    signal requestPermission

    Text {
        id: textItem
        anchors.centerIn: parent
        text: root.requestDenied
              ? qsTr("The application cannot run because the Location permission\n"
                   + "was not granted.\n"
                   + "Please grant the permission and restart the application.")
              : qsTr("The application requires the Location permission to get\n"
                   + "the position and satellite information.\n"
                   + "Please press the button to request the permission.")
    }
    // custom button without importing Quick.Controls
    Rectangle {
        anchors {
            top: textItem.bottom
            topMargin: 10
            horizontalCenter: parent.horizontalCenter
        }
        visible: !root.requestDenied
        width: parent.width * 0.8
        height: buttonText.implicitHeight * 2
        border.width: 1

        Text {
            id: buttonText
            anchors.fill: parent
            text: qsTr("Request Permission")
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
            MouseArea {
                anchors.fill: parent
                onClicked: root.requestPermission()
            }
        }
    }
}
