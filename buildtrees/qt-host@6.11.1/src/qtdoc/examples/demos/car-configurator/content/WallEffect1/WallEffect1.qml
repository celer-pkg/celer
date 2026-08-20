// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Created with Qt Quick Effect Maker (version 0.43), Fri Feb 24 08:38:48 2023

import QtQuick

Item {
    id: rootItem

    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    // Repeating couds texture. The default image is 512x512 pixels and created using the GIMP tool by the following steps:
    // 1. Filters -> Render -> Noise -> Difference Clouds
    // 2. Filters -> Map -> Tile Seamless
    //
    // The effect only uses the r channel of the texture.
    property var cloudsTexture: imageItemcloudsTexture
    // This property defines the perspective for the clouds which makes the horizon appear further away.
    property real cloudsPerspective: 0.8
    // Amount of texture scale horizontally.
    property real cloudsXScale: 0.8
    // Amount of texture scale vertically.
    property real cloudsYScale: 1
    // Color fading used in clouds horizon.
    property color cloudsHorizonColor: Qt.rgba(1, 0.413962, 0, 1)
    // The size of cloud horizon glow.
    property real cloudsHorizonGlowSize: 0.637534
    // The color for clouds.
    property color cloudsColor: Qt.rgba(1, 1, 1, 1)
    // The base color of sky behind the clouds.
    property color cloudsSkyColor: Qt.rgba(0.2, 0.4, 0.8, 1)
    // This property adjusts the thickness/amount of the clouds
    property real cloudsThickness: 0.5
    // This property adjusts the brightness of the clouds
    property real cloudsBrightness: 1.2
    // This property defines how sharp or smooth the edges of the clouds are.
    property real cloudsSharpness: 0.2
    // This property affects the angle of the movement of the clouds if iTime is animated. This is a helper property that results in correctly calculated cloudsXSpeed and cloudsYSpeed values. The angle is in degrees and 0 is vertically aligned movement from bottom to top.
    property real cloudsMovementAngle: 0
    property real cloudsSpeed: 0.05
    required property url downloadBase

    FrameAnimation {
        id: frameAnimation

        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iTime: rootItem.animatedTime
        readonly property alias cloudsTexture: rootItem.cloudsTexture
        readonly property alias cloudsPerspective: rootItem.cloudsPerspective
        readonly property alias cloudsXScale: rootItem.cloudsXScale
        readonly property alias cloudsYScale: rootItem.cloudsYScale
        readonly property alias cloudsHorizonColor: rootItem.cloudsHorizonColor
        readonly property alias cloudsHorizonGlowSize: rootItem.cloudsHorizonGlowSize
        readonly property alias cloudsColor: rootItem.cloudsColor
        readonly property alias cloudsSkyColor: rootItem.cloudsSkyColor
        readonly property alias cloudsThickness: rootItem.cloudsThickness
        readonly property alias cloudsBrightness: rootItem.cloudsBrightness
        readonly property alias cloudsSharpness: rootItem.cloudsSharpness
        readonly property alias cloudsMovementAngle: rootItem.cloudsMovementAngle
        readonly property alias cloudsSpeed: rootItem.cloudsSpeed
        property real cloudsXSpeed: Math.sin(cloudsMovementAngle / 180 * Math.PI) * cloudsSpeed
        property real cloudsYSpeed: Math.cos(cloudsMovementAngle / 180 * Math.PI) * cloudsSpeed

        Image {
            id: imageItemcloudsTexture

            anchors.fill: parent
            visible: false
            source: rootItem.downloadBase + "/content/WallEffect1/fog.png"
        }

        vertexShader: rootItem.downloadBase + "/content/WallEffect1/walleffect1.vert.qsb"
        fragmentShader: rootItem.downloadBase + "/content/WallEffect1/walleffect1.frag.qsb"
        anchors.fill: parent

        mesh: GridMesh {
            resolution: Qt.size(20, 20)
        }
    }
}
