// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Created with Qt Quick Effect Maker (version 0.43), Mon Feb 12 15:05:22 2024

import QtQuick

Item {
    id: rootItem

    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    property color color1: Qt.rgba(0, 0, 0, 1)
    property color color2: Qt.rgba(1, 1, 1, 1)
    property real barSize: 30
    property real speed: 50
    property real barAngle: 0.5
    // This property defines the item that is going to be used as the mask. The mask item alpha values are used to determine the source item's pixels visibility in the display.
    property var opacityMaskSource: imageItemopacityMaskSource
    property real value: 0.5561

    FrameAnimation {
        id: frameAnimation
        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iTime: rootItem.animatedTime
        readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
        readonly property alias color1: rootItem.color1
        readonly property alias color2: rootItem.color2
        readonly property alias barSize: rootItem.barSize
        readonly property alias speed: rootItem.speed
        readonly property alias barAngle: rootItem.barAngle
        readonly property alias opacityMaskSource: rootItem.opacityMaskSource
        readonly property alias value: rootItem.value

        Image {
            id: imageItemopacityMaskSource
            anchors.fill: parent
            source: "bar_mask.png"
            visible: false
        }

        vertexShader: 'barseffect.vert.qsb'
        fragmentShader: 'barseffect.frag.qsb'
        anchors.fill: parent
    }
}
