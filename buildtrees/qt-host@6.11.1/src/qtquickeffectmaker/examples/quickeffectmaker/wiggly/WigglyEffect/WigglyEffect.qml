// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Created with Qt Quick Effect Maker (version 0.43), Fri Feb 17 09:45:06 2023

import QtQuick

Item {
    id: rootItem

    // This is the main source for the effect
    property Item source: null
    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    property real wigglyAmountX: 20
    property real wigglyAmountY: 50
    property real wigglyShadows: 0.5
    // The levels of details for the electic clouds. Bigger value means more detailed rending which also requires more processing power. The default value is 6 and practical range is between 1 and 10.
    property int electricCloudLevels: 6
    // The color used for the clouds. Alpha channel defines the amount of opacity this effect has.
    property color electricCloudColor: Qt.rgba(1, 1, 1, 1)

    FrameAnimation {
        id: frameAnimation
        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iSource: rootItem.source
        readonly property alias iTime: rootItem.animatedTime
        readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
        readonly property alias wigglyAmountX: rootItem.wigglyAmountX
        readonly property alias wigglyAmountY: rootItem.wigglyAmountY
        readonly property alias wigglyShadows: rootItem.wigglyShadows
        readonly property alias electricCloudLevels: rootItem.electricCloudLevels
        readonly property alias electricCloudColor: rootItem.electricCloudColor

        vertexShader: 'wigglyeffect.vert.qsb'
        fragmentShader: 'wigglyeffect.frag.qsb'
        anchors.fill: parent
        mesh: GridMesh {
            resolution: Qt.size(63, 1)
        }
    }
}
