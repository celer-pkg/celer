// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

Item {
    id: rootItem

    property string currentImage: imagesModel ? imagesModel.currentImageFile : ""
    property string description
    property int currentIndex: 0
    property var imagesModel: null
    property bool open: false
    property bool showAddButton: false

    signal addPressed

    height: parent.height - 20
    width: height

    onCurrentIndexChanged: {
        imagesModel.setImageIndex(currentIndex);
    }

    Item {
        anchors.fill: parent
        Rectangle {
            anchors.fill: parent
            color: mainView.backgroundColor2
            border.color: mainView.foregroundColor1
            opacity: 0.8
            radius: 4
        }
        Image {
            id: currentImageItem
            anchors.fill: parent
            anchors.margins: 6
            source: rootItem.currentImage
            fillMode: Image.PreserveAspectFit
            mipmap: true
            sourceSize: Qt.size(128, 128)
        }
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            rootItem.open = !rootItem.open;
        }
    }

    Column {
        id: imageSelector
        anchors.top: parent.bottom
        visible: rootItem.open
        Repeater {
            model: imagesModel
            Item {
                width: rootItem.width
                height: rootItem.height
                Rectangle {
                    anchors.fill: parent
                    color: mainView.backgroundColor2
                    border.color: mainView.foregroundColor1
                    border.width: 1
                    opacity: 0.8
                }
                Image {
                    anchors.fill: parent
                    anchors.margins: 6
                    source: model.file
                    fillMode: Image.PreserveAspectFit
                    mipmap: true
                    sourceSize: Qt.size(128, 128)
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        rootItem.currentIndex = index;
                        rootItem.open = false;
                    }
                }
            }
        }
        // Optionally show add button as the last item
        Rectangle {
            width: rootItem.width
            height: rootItem.height
            visible: rootItem.showAddButton
            color: mainView.backgroundColor1
            border.color: mainView.foregroundColor2
            border.width: 1
            radius: 4
            Image {
                anchors.fill: parent
                anchors.margins: 6
                source: "images/icon_add.png"
                fillMode: Image.PreserveAspectFit
                mipmap: true
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    rootItem.addPressed();
                    rootItem.open = false;
                }
            }
        }
    }
    ToolTip {
        parent: rootItem
        visible: mouseArea.containsMouse
        delay: 1000
        text: description
    }
}
