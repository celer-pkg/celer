// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Rectangle {
    id: debugBox
    property color colorBG: "#ca000000"
    property color colorText: "#ffffff"
    required property View3D sView
    property int secondaryinfoSize: 24
    property int fpsSize: 32
    property int fps: sView.renderStats.fps
    property int frameTime: sView.renderStats.frameTime
    property int syncTime: sView.renderStats.syncTime
    property int renderTime: sView.renderStats.renderTime
    property int maxTime: sView.renderStats.maxFrameTime

    height: framerate.height + sync.height + render.height + max.height + 32
    Rectangle {
        id: rectangle2
        color: "#00000000"
        anchors.fill: parent
        anchors.rightMargin: 16
        anchors.leftMargin: 16
        anchors.bottomMargin: 16
        anchors.topMargin: 16

        Text {
            id: framerate
            x: 0
            y: 0
            color: debugBox.colorText
            text: debugBox.fps + " FPS (" + debugBox.frameTime + " ms)"
            anchors.left: parent.left
            font.pixelSize: debugBox.fpsSize
            height: font.pixelSize * 1.5
            anchors.leftMargin: 0
        }

        Text {
            id: sync
            x: 0
            y: framerate.height
            color: debugBox.colorText
            text: "Sync: " + debugBox.syncTime + " ms"
            anchors.left: parent.left
            font.pixelSize: debugBox.secondaryinfoSize
            height: font.pixelSize * 1.5
            anchors.leftMargin: 0
        }

        Text {
            id: render
            x: 0
            y: sync.y + sync.height
            color: debugBox.colorText
            text: "Render: " + debugBox.renderTime + " ms"
            anchors.left: parent.left
            font.pixelSize: debugBox.secondaryinfoSize
            height: font.pixelSize * 1.5
            anchors.leftMargin: 0
        }

        Text {
            id: max
            x: 0
            y: render.y + render.height
            color: debugBox.colorText
            text: "Max: " + debugBox.maxTime + " ms"
            anchors.left: parent.left
            font.pixelSize: debugBox.secondaryinfoSize
            height: font.pixelSize * 1.5
            anchors.leftMargin: 0
        }
    }

    Item {
        id: __materialLibrary__
    }
}
