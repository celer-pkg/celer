// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Effects

View3D {
    id: view3d
    property var effect: ({})
    property bool animated: false

    height: 200
    width: 200
    renderMode: View3D.Offscreen

    environment: SceneEnvironment {
        clearColor: "skyblue"
        backgroundMode: view3d.animated ? SceneEnvironment.Transparent : SceneEnvironment.Color
        effects: effect
    }

    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 300)
        eulerRotation.x: -20
        clipNear: 100
        clipFar: 800
    }

    DirectionalLight {
        eulerRotation.x: -20
        eulerRotation.y: 20
        ambientColor: Qt.rgba(0.8, 0.8, 0.8, 1.0);
    }

    Texture {
        id: checkers
        source: "../shared/maps/checkers2.png"
        scaleU: 20
        scaleV: 20
        tilingModeHorizontal: Texture.Repeat
        tilingModeVertical: Texture.Repeat
    }

    Model {
        source: "#Rectangle"
        scale.x: 10
        scale.y: 10
        eulerRotation.x: -90
        materials: [ DefaultMaterial { diffuseMap: checkers } ]
    }

    Model {
        source: "#Cone"
        position: Qt.vector3d(100, 0, -200)
        scale.y: 3
        materials: [ DefaultMaterial { diffuseColor: "green" } ]
    }

    Model {
        id: sphere
        source: "#Sphere"
        property int displacement: 0
        NumberAnimation on displacement {
            // Short-lasting animation that stops is acceptable to lancelot
            // It will grab the steady state when animation is finished
            duration: 50
            loops: 1
            running: view3d.animated
            from: 0
            to: 1
        }

        position: Qt.vector3d(displacement < 1 ? -150 : -100, 200, -200)
        materials: [ DefaultMaterial { diffuseColor: "#808000" } ]
    }

    Model {
        source: "#Cube"
        position.y: 50
        eulerRotation.y: 20
        materials: [ DefaultMaterial { diffuseColor: "gray" } ]
    }
}
