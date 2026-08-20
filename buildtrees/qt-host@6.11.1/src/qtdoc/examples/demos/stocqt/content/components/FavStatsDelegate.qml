// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes

Rectangle {
    id: root
    width: 311
    height: 32
    color: "transparent"
    property alias selectButton: mouseArea
    required property string stockId
    required property string open
    required property string close

    RowLayout {
        id: rowLayout
        anchors.fill: parent

        Text {
            id: idText
            width: 28
            height: 14
            color: "#f2f2f2"
            text: root.stockId
            font.pixelSize: 12
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
            wrapMode: Text.NoWrap
            Layout.leftMargin: 7
            anchors.leftMargin: -327
            font.weight: Font.DemiBold
            font.family: "Roboto"
        }

        Item {
            id: numbers
            width: 119
            height: 14
            Layout.alignment: Qt.AlignRight
            Text {
                id: openNumber
                anchors.right: dash.left
                color: "#5ECCA3"
                text: root.open + "$"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                font.weight: Font.DemiBold
                font.family: "Roboto"
            }
            Text {
                id: dash
                anchors.right: closeNumber.left
                color: "#BFBFBF"
                text: qsTr("-")
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                font.weight: Font.DemiBold
                font.family: "Roboto"
            }

            Text {
                id: closeNumber
                anchors.right: parent.right
                color: "#FA8A8A"
                text: root.close + "$"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                font.weight: Font.DemiBold
                font.family: "Roboto"
            }
        }
        Star {
            id: star1
            width: 24
            height: 24
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            isFavorite: true
            stock: root.stockId
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.rightMargin: 33
    }

    Shape {
        id: divider
        width: parent.width
        height: 2
        anchors.bottom: parent.bottom
        ShapePath {
            strokeWidth: 1
            strokeColor: "#3c3535"
            startX: 0
            startY: 0
            PathLine {
                x : divider.width
                y: 0
            }
        }
    }
}
