// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Item {
    id: window
    visible: true
    width: 1200
    height: 720

    SceneEnvironment {
        id: sceneEnvironment
        clearColor: "lightblue"
        backgroundMode: SceneEnvironment.Color
        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
    }

    View3D {
        height: parent.height
        width: parent.width/2
        environment: sceneEnvironment

        PerspectiveCamera {
            position: Qt.vector3d(0, 400, 700)
            rotation: Quaternion.fromEulerAngles(-30, 0, 0)
            clipFar: 20000
        }

        DirectionalLight {
            castsShadow: true
            eulerRotation: Qt.vector3d(-45, 45, 0)
            shadowMapFar: 2000
            shadowFactor: 100
            softShadowQuality: Light.Hard
            use32BitShadowmap: false
            shadowMapQuality: Light.ShadowMapQualityUltra
            shadowBias: 20
        }

        Model {
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(5, 0.1, 5)
            source: "#Cylinder"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(-1, -1, -1).times(100000)
            source: "#Cube"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(1, 1, 1).times(100000)
            source: "#Cube"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(0, 100, 0)
            source: "#Cylinder"
            materials: DefaultMaterial {diffuseColor: "lightgray"}
        }
    }


    View3D {
        height: parent.height
        width: parent.width/2
        x: parent.width/2
        environment: sceneEnvironment

        PerspectiveCamera {
            position: Qt.vector3d(0, 400, 700)
            rotation: Quaternion.fromEulerAngles(-30, 0, 0)
            clipFar: 20000
        }

        DirectionalLight {
            castsShadow: true
            eulerRotation: Qt.vector3d(-45, 45, 0)
            shadowMapFar: 2000
            shadowFactor: 100
            softShadowQuality: Light.Hard
            use32BitShadowmap: true
            shadowMapQuality: Light.ShadowMapQualityUltra
            shadowBias: 20
        }

        Model {
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(5, 0.1, 5)
            source: "#Cylinder"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(-1, -1, -1).times(100000)
            source: "#Cube"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(1, 1, 1).times(100000)
            source: "#Cube"
            materials: DefaultMaterial {diffuseColor: "gray"}
        }

        Model {
            position: Qt.vector3d(0, 100, 0)
            source: "#Cylinder"
            materials: DefaultMaterial {diffuseColor: "lightgray"}
        }
    }
}
