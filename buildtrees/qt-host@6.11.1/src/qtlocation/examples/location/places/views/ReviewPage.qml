// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var review

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
                text: review.title
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                text: Qt.formatDateTime(review.dateTime)
                width: parent.width
                Layout.alignment: Qt.AlignHCenter
            }

            RatingView {
                size: 16
                rating: review.rating
            }

            Label {
                text: review.text
                width: parent.width
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter
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
                source: review.supplier.icon.url(Qt.size(width, height), Icon.List)
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
                      Qt.openUrlExternally(review.supplier.url)
                }
            }
        }
    }
}
