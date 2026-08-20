// Copyright (C) 2023 The Qt Company Ltd.
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
            clearColor: "#d6dbdf"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 300)
            clipFar: 1000
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
            eulerRotation: Qt.vector3d(-45, 0, 0)
            source: "#Rectangle"
            scale: Qt.vector3d(2, 2, 2)
            materials: DefaultMaterial {
                diffuseColor: "green"
            }
            castsShadows: false
            receivesShadows: true
        }

        Model {
            source: "#Cube"
            scale: Qt.vector3d(1, 1, 1)
            materials: DefaultMaterial {
                diffuseColor: "blue"
            }
            castsShadows: false
            receivesShadows: true
        }
    }
}
