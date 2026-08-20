// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// Created with Qt Quick Effect Maker (version 0.43), Mon Mar 27 11:23:23 2023

import QtQuick

Item {
    id: rootItem

    // Enable this to animate iTime property
    property bool timeRunning: false
    // When timeRunning is false, this can be used to control iTime manually
    property real animatedTime: frameAnimation.elapsedTime

    // Defines the vignette color. The default value is black (0, 0, 0, 1).
    property color vignetteColor: Qt.rgba(0, 0, 0, 1)
    // Inner radius from the center where vignette effect starts from. The value should be smaller than vignetteOuterRadius. The default value is 0.2.
    property real vignetteInnerRadius: 0.2
    // Outer radius from the center where vignette effect ends to. The value should be bigger than vignetteInnerRadius. The default value is 0.8.
    property real vignetteOuterRadius: 0.8

    FrameAnimation {
        id: frameAnimation
        running: rootItem.timeRunning
    }

    ShaderEffect {
        readonly property alias iTime: rootItem.animatedTime
        readonly property alias vignetteColor: rootItem.vignetteColor
        readonly property alias vignetteInnerRadius: rootItem.vignetteInnerRadius
        readonly property alias vignetteOuterRadius: rootItem.vignetteOuterRadius

        vertexShader: 'abouteffect1.vert.qsb'
        fragmentShader: 'abouteffect1.frag.qsb'
        anchors.fill: parent
    }
}
