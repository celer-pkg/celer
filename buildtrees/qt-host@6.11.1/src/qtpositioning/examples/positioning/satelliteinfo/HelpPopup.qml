// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls.Basic as QC
import QtQuick.Layouts

QC.Popup {
    id: root
    padding: 0
    Rectangle {
        anchors.fill: parent
        color: Theme.backgroundColor
        ColumnLayout {
            anchors.fill: parent

            Item {
                implicitHeight: root.height * 0.1
                Layout.fillWidth: true
            }
            Image {
                source: Theme.darkMode ? "icons/qtlogo_white.png" : "icons/qtlogo_green.png"
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: qsTr("Satellite Info")
                color: Theme.textMainColor
                font.pixelSize: Theme.largeFontSize
                font.weight: Theme.fontDefaultWeight
                Layout.alignment: Qt.AlignHCenter
            }
            Item {
                implicitHeight: root.height * 0.05
                Layout.fillWidth: true
            }
            Text {
                text: qsTr("Explore satellite information using the SkyView and\nRSSI View. ")
                      + qsTr("Track the satellites contributing to your\nGPS fix in real-time.")
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textSecondaryColor
                font.pixelSize: Theme.smallFontSize
                font.weight: Theme.fontDefaultWeight
                Layout.alignment: Qt.AlignHCenter
            }
            Item {
                Layout.fillHeight: true
            }
            Button {
                text: qsTr("Back to the Application")
                implicitWidth: root.width * 0.8
                Layout.alignment: Qt.AlignHCenter
                onClicked: root.close()
            }
            Item {
                implicitHeight: root.height * 0.2
                Layout.fillWidth: true
            }
        }
    }
}
