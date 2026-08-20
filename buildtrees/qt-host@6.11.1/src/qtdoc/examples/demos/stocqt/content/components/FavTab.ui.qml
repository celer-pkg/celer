// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: favTab
    width: 357
    height: 44
    color: "transparent"
    state: "PriceActive"

    property alias priceButton: priceButtonMouse
    property alias volumeButton: volumeButtonMouse

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        anchors.bottomMargin: 8
        spacing: 8

        Rectangle {
            id: priceRect
            x: 16
            y: 8
            width: 159
            height: 28
            clip: true
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 13
                anchors.leftMargin: 12
                anchors.bottomMargin: 6
                spacing: 8

                Item {
                    id: spacer
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }

                Text {
                    id: price
                    x: 63
                    y: 6
                    width: 34
                    height: 16
                    color: "#f2f2f2"
                    text: qsTr("Price")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Item {
                    id: spacer1
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: priceButtonMouse
                anchors.fill: parent
            }
        }

        Rectangle {
            id: volumeRect
            x: 183
            y: 8
            width: 159
            height: 28
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 13
                anchors.leftMargin: 12
                anchors.bottomMargin: 6
                spacing: 8

                Item {
                    id: spacer2
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }

                Text {
                    id: volume
                    x: 54
                    y: 6
                    width: 52
                    height: 16
                    color: "#f2f2f2"
                    text: qsTr(" Volume")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 52
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Item {
                    id: spacer3
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }
            }

            MouseArea {
                id: volumeButtonMouse
                anchors.fill: parent
            }
        }
    }
    states: [
        State {
            name: "PriceActive"
            PropertyChanges {
                target: priceRect
                color: "#3c3535"
            }
        },
        State {
            name: "VolumeActive"
            extend: "Default"

            PropertyChanges {
                target: volumeRect
                color: "#3c3535"
            }
        }
    ]
}
