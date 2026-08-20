// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts

Rectangle {
    width: 328
    height: 30
    color: "#101010"
    property alias date: dateText.text

    RowLayout {
        anchors.fill: parent
        spacing: 16

        Text {
            id: title
            y: 7
            width: 65
            height: 16
            color: "#f2f2f2"
            text: qsTr("Top Stocks")
            font.pixelSize: 14
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            Layout.leftMargin: 10
            font.weight: Font.DemiBold
            font.family: "Titillium Web"
            Layout.preferredWidth: 65
            Layout.preferredHeight: 16
        }

        Item {
            id: spacer
            width: 65
            height: 16
        }

        Rectangle {
            id: element
            x: 244
            y: 5
            width: 120
            height: 20
            color: "transparent"
            radius: 4
            Layout.rightMargin: 18
            Layout.preferredWidth: 84
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            Text {
                id: dateText
                color: "#bfbfbf"
                text: qsTr("11.Sep.2023")
                anchors.fill: parent
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                Layout.leftMargin: 15
                font.weight: Font.DemiBold
                font.family: "Titillium Web"
                Layout.preferredWidth: 67
                Layout.preferredHeight: 16
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
        }
    }
}
