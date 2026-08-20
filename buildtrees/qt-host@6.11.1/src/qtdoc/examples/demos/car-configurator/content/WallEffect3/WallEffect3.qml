// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Created with Qt Quick Effect Maker (version 0.43), Tue Mar 7 11:28:56 2023

import QtQuick

Item {
    id: rootItem

    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    // The levels of details for the electic clouds. Bigger value means more detailed rending which also requires more processing power. The default value is 6 and practical range is between 1 and 10.
    property int electricCloudLevels: 5
    // The color used for the clouds. Alpha channel defines the amount of opacity this effect has.
    property color electricCloudColor: Qt.rgba(0.499001, 0.532998, 0.919005, 1)
    // Defines color of the light. The default value is white (1.0, 1.0, 1.0, 1.0).
    property color lightColor: Qt.rgba(1, 1, 1, 1)
    required property url downloadBase

    FrameAnimation {
        id: frameAnimation
        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iTime: rootItem.animatedTime
        readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
        readonly property alias electricCloudLevels: rootItem.electricCloudLevels
        readonly property alias electricCloudColor: rootItem.electricCloudColor
        readonly property alias lightColor: rootItem.lightColor

        vertexShader: rootItem.downloadBase + '/content/WallEffect3/walleffect3.vert.qsb'
        fragmentShader: rootItem.downloadBase + '/content/WallEffect3/walleffect3.frag.qsb'
        anchors.fill: parent
    }
}
