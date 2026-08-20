// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: stockDetail
    width: 360
    height: 65
    color: "#1d1d1d"
    property alias stockId: idText.text
    property alias price: priceText.text
    property alias stockName: nameText.text
    property alias priceChange: priceChangeText.text
    property bool rising

    state: rising? "Rising" : "Falling"

    Rectangle {
        id: priceFrame
        x: 261
        width: 99
        height: 65
        color: "transparent"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 5

        Text {
            id: priceText
            x: 16
            width: 72
            height: 19
            color: "#f2f2f2"
            text: qsTr("$287.0")
            font.pixelSize: 18
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            wrapMode: Text.Wrap
            font.weight: Font.DemiBold
            font.family: "Roboto"
            Layout.preferredWidth: 72
            Layout.preferredHeight: 19
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        }

        Rectangle {
            id: priceChangeFrame
            x: 25
            y: 31
            width: 58
            height: 24
            color: "#33fa8a8a"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.rightMargin: 6
                anchors.leftMargin: 2
                spacing: 2

                Image {
                    id: arrow
                    x: 2
                    y: 20
                    width: 16
                    height: 16
                    Layout.leftMargin: -2
                    Layout.topMargin: -4
                    clip: true
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Image {
                        id: arrowImage
                        anchors.fill: parent
                        source: "../images/arrowDown.svg"
                    }
                }

                Text {
                    id: priceChangeText
                    x: 20
                    y: 5
                    width: 33
                    height: 14
                    color: "#fa8a8a"
                    text: qsTr("50,6%")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    Layout.leftMargin: -6
                    Layout.topMargin: -5
                    font.weight: Font.Medium
                    font.family: "Roboto"
                    Layout.preferredWidth: 33
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                }
            }
        }
    }

    Rectangle {
        id: stockDetailFrame
        width: 201
        height: 55
        color: "transparent"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 5

        Text {
            id: uSD
            width: 24
            height: 14
            color: "#bfbfbf"
            text: qsTr("USD")
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: 12
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            anchors.leftMargin: 34
            anchors.topMargin: 41
            font.weight: Font.Normal
            font.family: "Roboto"
        }

        Text {
            id: idText
            width: 35
            height: 16
            color: "#f2f2f2"
            text: qsTr("AMD")
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: 16
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            anchors.leftMargin: 34
            anchors.topMargin: 3
            font.weight: Font.Bold
            font.family: "Titillium Web"
        }

        Text {
            id: nameText
            width: 169
            height: 16
            color: "#bfbfbf"
            text: qsTr("Advance Micro Devices Inc")
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: 14
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            anchors.leftMargin: 34
            anchors.topMargin: 21
            font.weight: Font.Normal
            font.family: "Roboto"
        }

        Image {
            id: logo
            width: 24
            height: 24
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: 16
            source: stockDetail.stockId? "../images/logos/" + stockDetail.stockId + ".svg" : "../images/logoBG.png"
        }
    }
    states: [
        State {
            name: "Falling"
        },

        State {
            name: "Rising"
            PropertyChanges {
                target: arrowImage
                source: "../images/arrowUp.svg"
            }
            PropertyChanges {
                target: priceChangeFrame
                color: "#33279646"
            }
            PropertyChanges {
                target: priceChangeText
                color: "#279646"
            }
        }
    ]
}
