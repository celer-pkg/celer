// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool requestDenied: false

    signal requestPermission

    color: Theme.backgroundColor

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillHeight: true
        }
        Text {
            id: textItem
            text: root.requestDenied
                  ? qsTr("The application cannot run because the Location permission\n"
                       + "was not granted.\n"
                       + "Please grant the permission and restart the application.")
                  : qsTr("The application requires the Location permission to get\n"
                       + "the position and satellite information.\n"
                       + "Please press the button to request the permission.")
            horizontalAlignment: Text.AlignHCenter
            color: Theme.textSecondaryColor
            font.pixelSize: Theme.mediumFontSize
            font.weight: Theme.fontDefaultWeight
            Layout.alignment: Qt.AlignHCenter
        }
        Item {
            Layout.fillHeight: true
        }
        Button {
            visible: !root.requestDenied
            implicitWidth: root.width * 0.8
            text: qsTr("Request Permission")
            Layout.alignment: Qt.AlignHCenter
            onClicked: root.requestPermission()
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
