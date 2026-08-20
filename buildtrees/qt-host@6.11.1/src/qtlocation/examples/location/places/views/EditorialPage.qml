// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var editorial

    ScrollView {
        id: scrollView
        flickableItem.interactive: true
        anchors.fill: parent
        anchors.margins: 15

        ColumnLayout {
            width: scrollView.width - 30
            spacing: 10

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: "#46a2da"
            }

            Label {
                text: editorial.title
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
            }

            Label {
                text: editorial.text
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: "#46a2da"
            }

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: editorial.supplier.icon.url(Qt.size(width, height), Icon.List)
            }

            Label {
                text: editorial.supplier.name
                Layout.alignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
            }

            Button {
                id: button
                text: qsTr("Open url")
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                      Qt.openUrlExternally(editorial.supplier.url)
                }
            }
        }
    }
}
