// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls.Basic
import ImageTestingItem 1.0

Window {
    visible: true
    width: 700
    height: 800
    color: "#202020"

    FrameAnimation {
        id: fa
        running: animateGradientButton.checked
        onTriggered: testingItem.update();
    }

    ImageTestingItem {
        id: testingItem
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.top: infoBar.bottom
        fillColor: "#404040"
    }

    Column {
        id: toolbar
        width: parent.width
        spacing: 4
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 16
            color: "#ffffff"
            text: "Images"
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                text: "Generate one"
                onClicked: testingItem.generate();
            }
            Button {
                text: "Remove newest"
                onClicked: testingItem.removeNewestImage();
            }
            Button {
                text: "Hide newest"
                onClicked: testingItem.hideNewestImage();
            }
            Button {
                text: "Show hidden"
                onClicked: testingItem.showHiddenImages();
            }
            Button {
                text: "Remove hidden"
                onClicked: testingItem.removeHiddenImages();
            }
            Button {
                text: "Remove all"
                onClicked: testingItem.clearImages();
            }
        }
        Item {
            width: 1
            height: 5
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 16
            color: "#ffffff"
            text: "Gradient"
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                id: showGradientButton
                text: "Show"
                checkable: true
                checked: testingItem.showGradient
                onClicked: testingItem.showGradient = checked;
            }
            Button {
                id: animateGradientButton
                text: "Animate"
                checkable: true
                checked: testingItem.animateGradient
                onClicked: testingItem.animateGradient = checked;
            }
            Button {
                text: "Cleanup resources"
                onClicked: testingItem.clearResources();
                ToolTip.delay: 500; ToolTip.visible: hovered
                ToolTip.text: "This is expected to remove all not currently visible gradient textures."
            }
        }
        Item {
            width: 1
            height: 5
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 16
            color: "#ffffff"
            text: "Offscreen Canvas"
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                text: "Generate one"
                onClicked: testingItem.generateCanvas();
                ToolTip.delay: 500; ToolTip.visible: hovered
                ToolTip.text: "NB! Offscreen canvases are not included in the textures/mem statistics."
            }
            Button {
                text: "Unregister newest"
                onClicked: testingItem.unregisterNewestCanvas();
                ToolTip.delay: 500; ToolTip.visible: hovered
                ToolTip.text: "Calls removeImage() only. The canvas is not destroyed."
            }
            Button {
                text: "Re-register all"
                onClicked: testingItem.reregisterCanvases();
                ToolTip.delay: 500; ToolTip.visible: hovered
                ToolTip.text: "Calls addImage() for all canvases that do not have a valid image."
            }
            Button {
                text: "Remove newest"
                onClicked: testingItem.removeNewestCanvas();
                ToolTip.delay: 500; ToolTip.visible: hovered
                ToolTip.text: "Calls destroyCanvas()."
            }
        }
    }
    Row {
        id: infoBar
        anchors.top: toolbar.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        Text {
            font.pixelSize: 20
            color: "#ffffff"
            text: "User+gradient textures: " + testingItem.imageAmount
        }
        Text {
            font.pixelSize: 20
            color: "#ffffff"
            text: "User+gradient mem (kb): " + testingItem.dataAmount
        }
    }
}
