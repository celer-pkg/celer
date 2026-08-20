// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: tabMenu
    width: 357
    height: 44
    color: "transparent"

    property alias historyButton: historyButtonMouse
    property alias volumeButton: volumeButtonMouse
    property alias liveButton: liveButtonMouse

    state: "History"

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        anchors.bottomMargin: 8
        spacing: 8

        Rectangle {
            id: history
            x: 16
            y: 8
            width: 103
            height: 28
            color: "#3c3535"
            radius: 4
            clip: true
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            MouseArea
            {
                id: historyButtonMouse
                anchors.fill: parent
            }

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 12
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
                    id: historyText
                    x: 29
                    y: 6
                    width: 47
                    height: 16
                    color: "#f2f2f2"
                    text: qsTr("History")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.preferredWidth: 47
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Item {
                    id: spacer2
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            id: history_Volume
            x: 127
            y: 8
            width: 103
            height: 28
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"

            MouseArea
            {
                id: volumeButtonMouse
                anchors.fill: parent

            }

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 12
                anchors.leftMargin: 12
                anchors.bottomMargin: 6
                spacing: 8

                Item {
                    id: spacer3
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }

                Text {
                    id: history_VolumeText
                    x: 2
                    y: 6
                    width: 97
                    height: 16
                    color: "#f2f2f2"
                    text: qsTr("History Volume")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.leftMargin: -15
                    Layout.preferredWidth: 97
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Item {
                    id: spacer4
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            id: live_Data
            x: 238
            y: 8
            width: 103
            height: 28
            Layout.preferredHeight: 28
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"

            MouseArea
            {
                id: liveButtonMouse
                anchors.fill: parent
            }

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 12
                anchors.leftMargin: 12
                anchors.bottomMargin: 6
                spacing: 0

                Item {
                    id: spacer5
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }

                Text {
                    id: live_Data1
                    x: 23
                    y: 6
                    width: 59
                    height: 16
                    color: "#f2f2f2"
                    text: qsTr("Live Data")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 59
                    Layout.preferredHeight: 16
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }

                Item {
                    id: spacer6
                    width: 1
                    height: 1
                    Layout.fillWidth: true
                }
            }
        }
    }
    states: [
        State {
            name: "History"
        },
        State {
            name: "Volume"
            extend: "History"

            PropertyChanges {
                target: historyText
                width: 46
                font.weight: Font.Normal
                Layout.preferredWidth: 46
            }

            PropertyChanges {
                target: history_Volume
                color: "#3c3535"
                radius: 4
            }

            PropertyChanges {
                target: history
                color: "transparent"
            }

            PropertyChanges {
                target: history_VolumeText
                x: 3
                width: 98
                font.weight: Font.DemiBold
                Layout.preferredWidth: 98
            }


        },
        State {
            name: "LiveData"
            extend: "Default"

            PropertyChanges {
                target: historyText
                width: 46
                font.weight: Font.Normal
                Layout.preferredWidth: 46
            }

            PropertyChanges {
                target: live_Data
                color: "#3c3535"
                radius: 4
            }

            PropertyChanges {
                target: live_Data1
                x: 22
                width: 60
                font.weight: Font.DemiBold
                Layout.preferredWidth: 60
            }

            PropertyChanges {
                target: history
                color: "transparent"
            }
        }
    ]
}
