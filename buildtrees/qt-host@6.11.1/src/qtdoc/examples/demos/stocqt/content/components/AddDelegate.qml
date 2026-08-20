// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls 2.15
import StocQt

Rectangle {
    id: addDelegate
    width: 172
    height: 27
    color: "#3c3535"
    radius: 2
    property alias addButton: mouseArea

    required property string stockId
    required property bool favorite
    required property real price
    required property real change
    required property real changePercentage
    required property string name
    required property Popup popup

    Component.onCompleted: {
        update()
        StockEngine.onFavoritesChanged.connect(update)
    }

    function update() {

        if (mouseArea && dimmer) {
            if (addDelegate.favorite){
                mouseArea.enabled = false
                dimmer.visible = true
            } else {
                mouseArea.enabled = true
                dimmer.visible = false
            }
        }
    }

    Rectangle {
        id: priceIndicator
        height: 24
        color: "transparent"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 100

        ColumnLayout {
            anchors.left: parent.left
            anchors.leftMargin: 20
            height: parent.height
            width: 50
            spacing: 0
            Text {
                id: priceText
                height: 15
                color: "#f2f2f2"
                text: qsTr("%L1").arg(addDelegate.price.toFixed(1))
                font.pixelSize: 10
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                wrapMode: Text.NoWrap
                font.weight: Font.Bold
                font.family: "Titillium Web"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            Text {
                id: price_status
                height: 9
                color: addDelegate.change >= 0? "#5ecca3" : "#fa8a8a"
                text: qsTr("%L1 (%L2%)").arg(addDelegate.change.toFixed()).arg(addDelegate.changePercentage.toFixed(1))
                font.pixelSize: 8
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                font.weight: Font.Normal
                font.family: "Roboto"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
        }
    }

    Text {
        id: idText
        height: 27
        color: "#f2f2f2"
        text: addDelegate.stockId
        anchors.left: parent.left
        anchors.top: parent.top
        font.pixelSize: 12
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors.leftMargin: 4
        font.weight: Font.DemiBold
        font.family: "Titillium Web"
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    }

    Text {
        id: nameText
        width: 169
        height: 20
        color: "#f2f2f2"
        text: addDelegate.name
        anchors.left: idText.right
        anchors.top: parent.top
        font.pixelSize: 5
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        wrapMode: Text.Wrap
        anchors.leftMargin: 3
        font.weight: Font.DemiBold
        font.family: "Titillium Web"
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            addDelegate.popup.close()
            StockEngine.addFavorite(addDelegate.stockId)
        }
    }

    Rectangle {
        id: dimmer
        anchors.fill: parent
        color: Qt.rgba(0,0,0,0.8)
        visible: false
    }
}
