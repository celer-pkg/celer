// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Created with Qt Quick Effect Maker (version 0.43), Fri Feb 24 09:13:14 2023

import QtQuick

Item {
    id: rootItem

    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    // Color used as a basis for the plasma effect.
    property color plasmaColors: Qt.rgba(1, 1, 0, 0.615503)
    // The scale of the plasma waves. Practical range is between 0.0 and 1.0. The default value is 0.2.
    property real plasmaScale: 0.269301
    // This property defines the change factor for how the value of each pixel color channel is altered.
    //
    // Setting the gamma values under Qt.vector3d(1.0, 1.0, 1.0) makes the image darker, the values above Qt.vector3d(1.0, 1.0, 1.0) lighten it.
    //
    // The value ranges from Qt.vector3d(0.0, 0.0, 0.0) (darkest) to inf (lightest). By default, the property is set to Qt.vector3d(1.0, 1.0, 1.0) (no change).
    property vector3d levelAdjustGamma: Qt.vector3d(1.8688, 1, 1)
    // This property defines the maximum input level for each color channel. It sets the white-point, all pixels having higher value than this property are rendered as white (per color channel). Decreasing the value lightens the light areas.
    //
    // The value ranges from "#ffffffff" to "#00000000". By default, the property is set to "#ffffffff" (no change).
    property color levelAdjustMaximumInput: Qt.rgba(0.667582, 0.600183, 0.597116, 1)
    // This property defines the maximum output level for each color channel. Decreasing the value darkens the light areas, reducing the contrast.
    //
    // The value ranges from "#ffffffff" to "#00000000". By default, the property is set to "#ffffffff" (no change).
    property color levelAdjustMaximumOutput: Qt.rgba(1, 1, 1, 1)
    // This property defines the minimum input level for each color channel. It sets the black-point, all pixels having lower value than this property are rendered as black (per color channel). Increasing the value darkens the dark areas.
    //
    // The value ranges from "#00000000" to "#ffffffff". By default, the property is set to "#00000000" (no change).
    property color levelAdjustMinimumInput: Qt.rgba(0, 0, 0, 0)
    // This property defines the minimum output level for each color channel. Increasing the value lightens the dark areas, reducing the contrast.
    //
    // The value ranges from "#00000000" to "#ffffffff". By default, the property is set to "#00000000" (no change).
    property color levelAdjustMinimumOutput: Qt.rgba(0, 0, 0, 0)
    required property url downloadBase

    FrameAnimation {
        id: frameAnimation
        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iTime: rootItem.animatedTime
        readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)
        readonly property alias plasmaColors: rootItem.plasmaColors
        readonly property alias plasmaScale: rootItem.plasmaScale
        readonly property alias levelAdjustGamma: rootItem.levelAdjustGamma
        readonly property alias levelAdjustMaximumInput: rootItem.levelAdjustMaximumInput
        readonly property alias levelAdjustMaximumOutput: rootItem.levelAdjustMaximumOutput
        readonly property alias levelAdjustMinimumInput: rootItem.levelAdjustMinimumInput
        readonly property alias levelAdjustMinimumOutput: rootItem.levelAdjustMinimumOutput

        vertexShader: rootItem.downloadBase + '/content/WallEffect2/walleffect2.vert.qsb'
        fragmentShader: rootItem.downloadBase + '/content/WallEffect2/walleffect2.frag.qsb'
        anchors.fill: parent
    }
}
