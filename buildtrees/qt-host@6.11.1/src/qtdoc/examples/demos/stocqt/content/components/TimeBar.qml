// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: timeBar
    width: 358
    height: 42
    color: "#0e0e0e"

    property alias date: dateText.text
    property string selectedTimeframe: "1M"
    state: "month"

    RowLayout {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        anchors.bottomMargin: 8
        spacing: 8

        Rectangle {
            id: week
            x: 55
            y: 8
            width: 32
            height: 26
            Layout.preferredWidth: 32
            Layout.preferredHeight: 26
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 8
                anchors.leftMargin: 8
                anchors.bottomMargin: 6
                spacing: 8

                Text {
                    id: weekText
                    x: 8
                    y: 6
                    width: 17
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("1W")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.preferredWidth: 17
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
            }

            MouseArea {
                id: weekButton
                anchors.fill: parent
                onClicked: {
                    timeBar.state = "week"
                    timeBar.selectedTimeframe = "1w"
                }
            }
        }

        Rectangle {
            id: month
            x: 95
            y: 8
            width: 34
            height: 26
            Layout.preferredWidth: 34
            Layout.preferredHeight: 26
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"
            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 8
                anchors.leftMargin: 8
                anchors.bottomMargin: 6
                spacing: 8
                Text {
                    id: monthText
                    x: 8
                    y: 6
                    width: 19
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("1M")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 19
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
            }

            MouseArea {
                id: monthButton
                anchors.fill: parent
                onClicked:{
                    timeBar.state = "month"
                    timeBar.selectedTimeframe = "1M"
                }
            }
        }

        Rectangle {
            id: three_Months
            x: 137
            y: 8
            width: 34
            height: 26
            Layout.preferredWidth: 34
            Layout.preferredHeight: 26
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"

            RowLayout {
                id: three_Months_RowLayout
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 8
                anchors.leftMargin: 8
                anchors.bottomMargin: 6

                Text {
                    id: threeMonthText
                    x: 8
                    y: 6
                    width: 19
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("3M")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 19
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
            }

            MouseArea {
                id: threeMonthButton
                anchors.fill: parent
                onClicked:{
                    timeBar.state = "threeMonth"
                    timeBar.selectedTimeframe = "3M"
                }
            }
        }

        Rectangle {
            id: six_Months
            x: 179
            y: 8
            width: 34
            height: 26
            Layout.preferredWidth: 34
            Layout.preferredHeight: 26
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            color: "transparent"
            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.rightMargin: 8
                anchors.leftMargin: 8
                anchors.bottomMargin: 6
                spacing: 8

                Text {
                    id: sixMonthText
                    x: 8
                    y: 6
                    width: 19
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("6M")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.Normal
                    font.family: "Roboto"
                    Layout.preferredWidth: 19
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                }
            }

            MouseArea {
                id: sixMonthButton
                anchors.fill: parent
                onClicked: {
                    timeBar.state = "sixMonth"
                    timeBar.selectedTimeframe = "6M"
                }
            }
        }

        Rectangle {
            id: tabDate
            color: "#1d1d1d"
            radius: 4
            width: 80
            height: 24
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.leftMargin: 46

            Text {
                id: dateText
                anchors.fill: parent
                text: ""
                color: "#bfbfbf"
                font.pixelSize: 12
                font.weight: Font.Normal
                font.family: "Roboto"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
    states: [
        State {
            name: "week"
            PropertyChanges {
                target: weekText
                color: "#7fdf9a"
            }
        },
        State {
            name: "month"
            PropertyChanges {
                target: monthText
                color: "#7fdf9a"
            }
        },
        State {
            name: "threeMonth"
            PropertyChanges {
                target: threeMonthText
                color: "#7fdf9a"
            }
        },
        State {
            name: "sixMonth"
            PropertyChanges {
                target: sixMonthText
                color: "#7fdf9a"
            }
        }
    ]
}
