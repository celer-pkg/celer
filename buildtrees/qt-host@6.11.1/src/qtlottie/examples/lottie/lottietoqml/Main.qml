// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import Qt.labs.folderlistmodel
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Lottietoqml Example")
    color: grid.selectedItem == null ? "black" : "darkslategray"
    visibility: Window.Maximized

    Behavior on color {
        ColorAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
    }

    property int animationDuration: 300
    property int animationEasing: Easing.OutQuad

    FolderListModel {
        id: examplesModel
        folder: "qrc:/qt/qml/lottietoqmlexample/generated/"
        nameFilters: [ "*.qml" ]
    }


    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: centerItem.left
        anchors.left: parent.left
        anchors.margins: 50
        text: "Lottie vector animations converted to Qt Quick with lottietoqml"
        color: "white"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: 30
    }

    Item {
        id: centerItem
        anchors.centerIn: parent
        height: parent.height
        width: childrenRect.width

        Grid {
            id: grid
            transformOrigin: Item.Center

            property var selectedItem: null
            function zoom(icon)
            {
                if (selectedItem != null || icon == null) {
                    selectedItem.scale = 1
                    grid.x = 0
                    grid.y = 0
                    selectedItem = null
                } else {
                    var p = grid.mapFromItem(icon, Qt.point(icon.width / 2, icon.height / 2))
                    var myCenter = Qt.point(grid.width / 2, grid.height / 2)
                    grid.x = -p.x + myCenter.x
                    grid.y = -p.y + myCenter.y

                    var scale = Math.min(window.width / icon.width, window.height / icon.height)
                    icon.scale = scale
                    selectedItem = icon
                }
            }

            property int iconSize: Math.floor(window.height / rows) - padding - spacing
            padding: 10
            spacing: 5
            rows: 3
            x: 0
            y: 0

            Behavior on x {
                NumberAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
            }

            Behavior on y {
                NumberAnimation { duration: window.animationDuration; easing.type: window.animationEasing }
            }

            width: childrenRect.width
            height: parent.height
            Repeater {
                model: examplesModel

                GridIcon {
                    source: fileUrl

                    width: grid.iconSize
                    height: width
                }
            }
        }
    }
}
