// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "logic.js" as Logic

Item {
    id: container
    width: 64
    height: 64
    property alias source: img.source
    property int index
    property int row: 0
    property int col: 0
    property int towerType
    property bool canBuild: true
    property Item gameCanvas: parent.parent.parent
    signal clicked()

    Image {
        id: img
        opacity: (container.canBuild && (container.gameCanvas as GameCanvas).coins >= Logic.towerData[container.towerType-1].cost) ? 1.0 : 0.4
    }
    Text {
        anchors.right: parent.right
        font.pointSize: 14
        font.bold: true
        color: "#ffffff"
        text: Logic.towerData[container.towerType - 1].cost
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            Logic.buildTower(container.towerType, container.col, container.row)
            container.clicked()
        }
    }
    Image {
        visible: container.col == container.index && container.row != 0
        source: "gfx/dialog-pointer.png"
        anchors.top: parent.bottom
        anchors.topMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
    }
    Image {
        visible: container.col == container.index && container.row == 0
        source: "gfx/dialog-pointer.png"
        rotation: 180
        anchors.bottom: parent.top
        anchors.bottomMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
