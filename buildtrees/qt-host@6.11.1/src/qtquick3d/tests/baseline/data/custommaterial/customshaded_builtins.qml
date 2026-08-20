// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: v3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#444845"
            backgroundMode: SceneEnvironment.Color
        }

        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        PointLight {
            position: Qt.vector3d(0, 500, 0)
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            brightness: 5
            castsShadow: true
            shadowMapQuality: Light.ShadowMapQualityHigh
        }

        // the ground will be not green but rather green-cyan-ish
        Model {
            source: "#Rectangle"
            y: -200
            scale: Qt.vector3d(5, 5, 5)
            eulerRotation.x: -90
            materials: [
                CustomMaterial {
                    vertexShader: "customshaded_builtins.vert"
                    fragmentShader: "customshaded_builtins.frag"
                    property real uTime: 0.0
                    property real uAmplitude: 0.0
                    property real uSel: 2.0
                }
            ]
        }

        // approx. yellow at top left, purple at bottom left, red/purple at top right, cyan at bottom right
        Model {
            position: Qt.vector3d(-50, 0, -50)
            eulerRotation.x: 30.0
            eulerRotation.y: 100.0
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"
            materials: [
                CustomMaterial {
                    vertexShader: "customshaded_builtins.vert"
                    fragmentShader: "customshaded_builtins.frag"
                    property real uTime: 1.0
                    property real uAmplitude: 50.0
                    property real uSel: 1.0
                }
            ]
        }

        // the result is a blue cylinder
        Model {
            position: Qt.vector3d(50, 200, -50)
            eulerRotation.x: 30.0
            eulerRotation.y: 100.0
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"
            materials: [
                CustomMaterial {
                    vertexShader: "customshaded_builtins.vert"
                    fragmentShader: "customshaded_builtins.frag"
                    property real uTime: 1.0
                    property real uAmplitude: 50.0
                    property real uSel: 0.0
                }
            ]
        }

        // same here
        Model {
            position: Qt.vector3d(200, 0, -50)
            eulerRotation.x: 30.0
            eulerRotation.y: 100.0
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            source: "#Cylinder"
            materials: [
                CustomMaterial {
                    vertexShader: "customshaded_builtins.vert"
                    fragmentShader: "customshaded_builtins.frag"
                    property real uTime: 1.0
                    property real uAmplitude: 50.0
                    property real uSel: -1.0
                }
            ]
        }
    }
}
