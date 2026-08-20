// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: keystats
    width: 343
    height: 164
    color: "#1d1d1d"
    radius: 4

    property alias close: closePrice.text
    property alias open: openPrice.text
    property alias averageVolume: averageVolPrice.text
    property alias volume: volumePrice.text

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 6
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        anchors.bottomMargin: 6
        spacing: 0

        Rectangle {
            id: container
            x: 16
            y: 6
            width: 311
            height: 26
            Layout.preferredHeight: 26
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 201
                anchors.topMargin: 6
                anchors.bottomMargin: 6

                Text {
                    id: prev_Close
                    x: 0
                    y: 6
                    width: 28
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("Prev.Close")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Titillium Web"
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }

                Text {
                    id: closePrice
                    x: 256
                    y: 6
                    width: 56
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("€4,600.00")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.leftMargin: 30
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
            }
        }

        Rectangle {
            id: container1
            x: 16
            y: 48
            width: 311
            height: 26
            color: "transparent"
            Layout.preferredHeight: 26
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop

            RowLayout {
                anchors.fill: parent
                spacing: 201
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                Text {
                    id: openName
                    x: 0
                    y: 6
                    width: 28
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("Open")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Titillium Web"
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }

                Text {
                    id: openPrice
                    x: 256
                    y: 6
                    width: 56
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("€4,630.00")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.leftMargin: 30
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
            }
        }

        Rectangle {
            id: container2
            x: 16
            y: 90
            width: 311
            height: 26
            Layout.preferredHeight: 26
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                spacing: 201
                Text {
                    id: volumeName
                    x: 0
                    y: 6
                    width: 40
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("Volume")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Titillium Web"
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }

                Text {
                    id: volumePrice
                    x: 256
                    y: 6
                    width: 56
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("€4,980.00")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    Layout.leftMargin: 18
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
            }
        }

        Rectangle {
            id: container3
            x: 16
            y: 132
            width: 311
            height: 26
            Layout.preferredHeight: 26
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                spacing: 201
                Text {
                    id: average_Vol
                    x: 0
                    y: 6
                    width: 65
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("Average Vol.")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    font.weight: Font.DemiBold
                    font.family: "Titillium Web"
                    Layout.preferredWidth: 65
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                }

                Text {
                    id: averageVolPrice
                    x: 256
                    y: 6
                    width: 56
                    height: 14
                    color: "#f2f2f2"
                    text: qsTr("€4,560.00")
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.NoWrap
                    Layout.leftMargin: -7
                    font.weight: Font.DemiBold
                    font.family: "Roboto"
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 14
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
            }
        }
    }
}
