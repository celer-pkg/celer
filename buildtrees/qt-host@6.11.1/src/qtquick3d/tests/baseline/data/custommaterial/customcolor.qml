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
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        Model {
            source: "#Cube"
            x: -150
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 30
            eulerRotation.y: 20
            materials: [
                CustomMaterial {
                    fragmentShader: "customcolor.frag"
                }
            ]
        }

        Model {
            source: "#Cube"
            x: 150
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation.x: 30
            eulerRotation.y: 20
            materials: [
                CustomMaterial {
                    vertexShader: "customcolor2.vert"
                    fragmentShader: "customcolor2.frag"
                }
            ]
        }
    }
}
