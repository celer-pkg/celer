// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
import QtQuick
import QtQuick3D

Item {
    width: 640
    height: 480
    visible: true

    View3D {
        id: viewport
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "lightblue"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 2, 5)
            eulerRotation: Qt.vector3d(-10, 0, 0)
            clipFar: 100
            clipNear: 0.01
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: 45
            castsShadow: true
            brightness: 1
            shadowFactor: 50
            shadowBias: 0.2
            pcfFactor: 0.02
            csmNumSplits: 1
        }

        Model {
            eulerRotation: Qt.vector3d(-90, 0, 0)
            source: "#Rectangle"
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            castsShadows: false
            receivesShadows: true
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(1,1,1).times(0.01)
            position: Qt.vector3d(0, 0.5, 0)
            materials: PrincipledMaterial {
                baseColor: "white"
            }
        }

        Model {
            source: "#Sphere"
            position: Qt.vector3d(0, 1.1, 0)
            scale: Qt.vector3d(1,1,1).times(0.008)
            materials: PrincipledMaterial {
                baseColor: "white"
            }
        }

        Model {
            source: "#Sphere"
            position: Qt.vector3d(0, 1.6, 0)
            scale: Qt.vector3d(1,1,1).times(0.006)
            materials: PrincipledMaterial {
                baseColor: "white"
            }
        }
    }
}

