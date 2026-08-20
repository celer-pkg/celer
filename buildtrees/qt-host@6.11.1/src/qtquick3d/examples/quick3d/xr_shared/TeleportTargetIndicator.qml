// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D

Node {
    id: targetIndicator
    function show(){
        hideAnimation.stop()
        showAnimation.start()
    }
    function hide(){
        showAnimation.stop()
        hideAnimation.start()
    }
    function moveTo(pos : vector3d) {
        targetIndicator.position = pos
    }

    visible: opacity > 0.01
    ParallelAnimation {
        id: showAnimation
        NumberAnimation  {
            target: targetIndicator
            properties: "opacity"
            to: 0.99
            duration: 50
        }
        NumberAnimation {
            target: targetIndicator
            properties: "scaleValue"
            to: 0.5
            easing.type: Easing.OutBack
        }
    }
    ParallelAnimation {
        id: hideAnimation
        NumberAnimation  {
            target: targetIndicator
            properties: "opacity"
            to: 0.0
            duration: 200
        }
        NumberAnimation {
            target: targetIndicator
            properties: "scaleValue"
            to: 0.0
            easing.type: Easing.OutBack
        }
    }
    property real scaleValue: 0.0
    scale: Qt.vector3d(scaleValue, 3.0, scaleValue)
    opacity: 0.0
    Model {
        y: 50
        source: "#Cylinder"
        materials: CustomMaterial {
            id: material
            property real time: 0
            NumberAnimation {
                target: material
                property: "time"
                running: targetIndicator.visible
                loops: -1
                from: 0
                to: Math.PI
                duration: 500
            }
            property vector3d indicatorColor: Qt.vector3d(0, 1, 0)
            depthDrawMode: Material.AlwaysDepthDraw
            shadingMode: CustomMaterial.Unshaded
            cullMode: Material.BackFaceCulling
            vertexShader: "shaders/target_indicator.vert"
            fragmentShader: "shaders/target_indicator.frag"
        }
    }
}
