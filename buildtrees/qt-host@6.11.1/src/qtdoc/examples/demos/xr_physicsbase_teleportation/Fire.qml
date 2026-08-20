// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
import QtQuick
import QtQuick3D

Node {
    id: fire
    scale: Qt.vector3d(1, 1, 1)
    property alias windDir: material.windDir
    property alias windstrength: material.windstrength
    property alias fireColor: material.color
    property alias lightColor: light.color
    property alias baseSize: material.baseSize
    property real brightness: 10
    property real life: 0
    readonly property bool isOn: (life > 0)
    function start() {
        life = 1
    }

    function stop() {
        life = 0
    }

    Behavior on life {
        NumberAnimation {
            duration: 500
            easing.type: Easing.OutBack
        }
    }

    CustomMaterial {
        id: material
        shadingMode: CustomMaterial.Unshaded
        sourceBlend: CustomMaterial.One
        destinationBlend: CustomMaterial.OneMinusSrcColor
        sourceAlphaBlend: CustomMaterial.One
        destinationAlphaBlend: CustomMaterial.OneMinusSrcColor
        cullMode: Material.NoCulling
        vertexShader: "media/shaders/fire.vert"
        fragmentShader: "media/shaders/fire.frag"
        property real time: 0
        property color color: "white"
        property real baseSize: 0

        FrameAnimation {
            id: frameAnimation
            running: true
            onTriggered: {
                material.time += 0.1 * frameTime
            }
        }

        property real windstrength: 1.0
        property vector2d windDir: Qt.vector2d(0.5, 0.5);

        property TextureInput fireTexture : TextureInput {
            texture: Texture {
                sourceItem: FireResources.fireVideo
            }
        }

        property TextureInput turbulence : TextureInput {
            texture: CommonResources.turbulenceTexture
        }
    }

    PointLight {
        id: light
        brightness:fire.brightness * fire.life + 0.01
        color: "orange"
    }
    Model {
        visible: fire.isOn
        materials: [
            material
        ]

        opacity: Math.min(1., fire.life)
        scale: Qt.vector3d(fire.life, fire.life, fire.life)
        geometry: FireResources.fireMesh
    }
}
