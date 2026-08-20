// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import AerospaceHubExample

Rectangle {
    id: notification

    radius: GlobalSettings.quarterradius
    height: GlobalSettings.notificationheight

    color: "#D2E2FF"

    RowLayout {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: GlobalSettings.halfspacing

        Image {
            id: infoicon
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            source: "../images/info_circle_24px.png"
        }

        Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            text: "Notification"
            color: "#1F52F1"
            font.pixelSize: GlobalSettings.fontsize14px
            font.bold: true
        }

        Label {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            Layout.fillWidth: true
            text: "No warnings or alarms"
            color: "#2D2D2D"
            font.pixelSize: GlobalSettings.fontsize14px
        }

        Button {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            icon.source: "../images/close_large_16px.png"
            background: Rectangle {
                anchors.centerIn: parent
                width: GlobalSettings.fontsize14px * 1.5
                height: width
                color: parent.hovered ? "#BEBE55" : "transparent"
            }
            onClicked: notification.visible = false
        }
    }
}
