// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtTest
import QtQuick3D

Item {
    width: 800
    height: 600
    visible: true

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(-200, 400, 1000)
            eulerRotation: Qt.vector3d(-20, -20, 0)
            clipFar: 5000
            clipNear: 1
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 100
        }

        Model {
            position: Qt.vector3d(0, -100, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            source: "#Rectangle"
            scale: Qt.vector3d(10, 10, 1)
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            castsShadows: false
            receivesShadows: true
        }

        Model {
            position: Qt.vector3d(-50, 300, 0)
            source: "#Cube"
            materials: PrincipledMaterial {
                baseColor: "yellow"
            }
        }
    }
}

