// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import StocQt

Rectangle {
    id: search
    width: 340
    height: 51
    color: "transparent"
    property alias placeHolderText: textField.placeholderText

    Rectangle {
        id: frame_3
        color: "#1d1d1d"
        radius: 4
        border.color: "#3c3535"
        border.width: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        anchors.topMargin: 8

        RowLayout {
            anchors.fill: parent
            spacing: 12

            Image {
                id: searchIcon
                width: 24
                height: 24
                source: "../images/searchIcon.svg"
                clip: true
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.leftMargin: 5
            }

            TextField {
                id: textField
                color: "#787878"
                background: Rectangle {
                    color: "transparent"
                }
                font.pixelSize: 14
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                renderType: Text.QtRendering
                placeholderText: qsTr("Search")
                placeholderTextColor: "#787878"
                font.weight: Font.Normal
                font.family: "Roboto"
                Layout.preferredWidth: 128
                Layout.preferredHeight: 23
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.topMargin: 3
                onTextChanged: StockEngine.filterModel.setFilterFixedString(text)
            }

            Item {
                id: spacer
                width: 1
                height: 1
                Layout.fillWidth: true
            }
        }
    }
}
