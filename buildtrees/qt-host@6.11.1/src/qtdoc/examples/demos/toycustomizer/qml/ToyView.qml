// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: toyView
    color: "transparent"

    property alias toyIndex: currentToy.index
    property alias toy: currentToy.toy
    property alias accessoryModel: currentToy.accessoryModel

    signal hideRequested
    signal showRequested
    signal confirmRequested

    implicitWidth: currentToy.implicitWidth
    implicitHeight: currentToy.implicitHeight

    CurrentToyModel {
        id: currentToy
        isMaximized: false
        anchors.fill: parent
    }

    ToyButton {
        id: backButton
        type: ToyButton.Type.Secondary
        textStyle: ApplicationConfig.TextStyle.Button_L
        text: qsTr("Back")
        icon.source: "icons/back.svg"
        anchors {
            left: parent.left
            top: parent.top
        }
        onClicked: toyView.hideRequested()
    }

    ToyButton {
        id: maximizeButton
        type: ToyButton.Type.Secondary
        flat: true
        text: qsTr("Maximize")
        color: pressed ? "#1A53DB" : hovered ? "#1C44B1" : "#2269EE"
        icon {
            source: "icons/maximize_circle_fill.svg"
            width: ApplicationConfig.responsiveSize(96)
            height: ApplicationConfig.responsiveSize(96)
        }
        anchors {
            right: parent.right
            top: parent.top
        }
        onClicked: toyView.showRequested()
    }

    Item {
        id: reviewOrder
        anchors {
            right: parent.right
            bottom: parent.bottom
            bottomMargin: ApplicationConfig.responsiveSize(24)
        }
        implicitWidth: orderButton.implicitWidth
        implicitHeight: orderButton.implicitHeight

        ToyButton {
            id: orderButton
            height: ApplicationConfig.responsiveSize(144)
            textStyle: ApplicationConfig.TextStyle.Button_L
            text: qsTr("Review Order")
            onClicked: toyView.confirmRequested()
        }

        Rectangle {
            id: totalAccessory
            width: Math.round(ApplicationConfig.responsiveSize(115))
            height: Math.round(ApplicationConfig.responsiveSize(115))
            radius: width / 2
            color: "#FFFFFF"
            visible: toyView.accessoryModel.totalSelectedAccessory > 0 ? true : false

            anchors {
                right: orderButton.right
                top: orderButton.top
                rightMargin: Math.round(ApplicationConfig.responsiveSize(-48))
                topMargin: Math.round(ApplicationConfig.responsiveSize(-48))
            }

            ToyLabel {
                anchors.centerIn: parent
                text: toyView.accessoryModel.totalSelectedAccessory
                font {
                    family: "DynaPuff"
                    pixelSize: 13
                }
            }
        }
    }
}
