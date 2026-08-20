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

        property real time: 10
        property real amplitude: 20

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            x: -200
            materials: [
                CustomMaterial {
                    fragmentShader: "worldnormal.frag"
                }
            ]
        }

        Model {
            source: "../shared/models/monkey_object.mesh"
            scale: Qt.vector3d(80, 80, 80)
            eulerRotation.y: 90
            x: 100
            materials: [
                CustomMaterial {
                    fragmentShader: "worldnormal.frag"
                    cullMode: CustomMaterial.NoCulling
                }
            ]
        }
    }
}
