// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    id: wb_screen

    width: 800
    height: 480
    color: "white"

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            oitMethod: SceneEnvironment.OITWeightedBlended
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 500)
            clipFar: 1000
        }

        DirectionalLight {
        }

        Node {
            id: sceneRoot
            Model {
                source: "#Rectangle"
                position: Qt.vector3d(0, -110, -20)
                scale: Qt.vector3d(3, 3, 1)
                eulerRotation.z: 0
                opacity: 0.5
                materials: [
                    DefaultMaterial {
                        diffuseColor: "red"
                    }
                ]
            }

            Model {
                source: "#Rectangle"
                position: Qt.vector3d(-90, 90, -10)
                scale: Qt.vector3d(3, 3, 1)
                eulerRotation.z: 120
                opacity: 0.5
                materials: [
                    DefaultMaterial {
                        diffuseColor: "green"
                    }
                ]
            }

            Model {
                source: "#Rectangle"
                position: Qt.vector3d(90, 90, 0)
                scale: Qt.vector3d(3, 3, 1)
                eulerRotation.z: 240
                opacity: 0.5
                materials: [
                    DefaultMaterial {
                        diffuseColor: "blue"
                    }
                ]
            }
        }
    }
}
