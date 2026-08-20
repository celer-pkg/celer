// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import examples.Hangman

Rectangle {
    id: storeItem

    property Product product: undefined

    state: "NORMAL"

    visible: product.status == Product.Registered
    radius: 10
    color: "white"

    height: titleText.contentHeight + descriptionText.contentHeight + topLevel.globalMargin * 2
    // ![0]
    Text {
        id: titleText
        text: product.title
        font.bold: true
        anchors.right: priceText.left
        anchors.rightMargin: topLevel.globalMargin
        anchors.top: parent.top
        anchors.topMargin: topLevel.globalMargin
        anchors.left: parent.left
        anchors.leftMargin: topLevel.globalMargin
    }

    Text {
        id: descriptionText
        text: product.description
        anchors.right: priceText.left
        anchors.rightMargin: topLevel.globalMargin
        anchors.left: parent.left
        anchors.leftMargin: topLevel.globalMargin
        anchors.top: titleText.bottom
        anchors.topMargin: topLevel.globalMargin / 2
        wrapMode: Text.WordWrap
    }

    Text {
        id: priceText
        text: product.price
        anchors.right: parent.right
        anchors.rightMargin: topLevel.globalMargin
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            pendingRect.visible = true;
            spinBox.visible = true;
            statusText.text = "Purchasing...";
            storeItem.state = "PURCHASING";
            product.purchase();
        }
        onPressed: {
            storeItem.state = "PRESSED";
        }
        onReleased: {
            storeItem.state = "NORMAL";
        }
    }
    // ![0]

    Rectangle {
        id: pendingRect
        anchors.fill: parent
        opacity: 0.0
        color: "white"
        radius: parent.radius
        Text {
            id: statusText
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: spinBox.left
            anchors.margins: topLevel.globalMargin
            verticalAlignment: Text.AlignVCenter
        }
        BusyIndicator {
            id: spinBox
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: topLevel.globalMargin
            width: height
        }

        Connections {
            target: product
            function onPurchaseSucceeded() {
                statusText.text = "Purchase Succeeded";
                spinBox.visible = false;

            }
            function onPurchaseFailed() {
                statusText.text = "Purchase Failed";
                spinBox.visible = false;
                storeItem.state = "NORMAL";
            }
        }
    }

    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: storeItem
                color: "white"
                border.color: "transparent"
            }
            PropertyChanges {
                target: pendingRect
                opacity: 0.0
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: storeItem
                color: "transparent"
                border.color: "white"
            }
        },
        State {
            name: "PURCHASING"
            PropertyChanges {
                target: pendingRect
                opacity: 1.0
            }
        }
    ]

    transitions: [
        Transition {
            from: "PURCHASING"
            to: "NORMAL"
            NumberAnimation { target: pendingRect; property: "opacity"; duration: 2000; easing.type: Easing.InExpo }
        }
    ]
}
