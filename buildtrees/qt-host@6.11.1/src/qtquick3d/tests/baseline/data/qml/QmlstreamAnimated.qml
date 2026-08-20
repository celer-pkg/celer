// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick.Window
import "qml"

Rectangle {
    width: 600
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 350)
            clipFar: 5000
        }

        DirectionalLight {
        }

        // Model with animated texture
        Model {
            x: -100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            DefaultMaterial {
                id: myMaterial
                diffuseMap: Texture {
                    id: myTexture
                    sourceItem: AnimatedItem {
                        id: myItem
                    }
                }
            }
            materials: [myMaterial]
        }

        // Model using the same material
        Model {
            x: 100
            y: 100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: myMaterial
        }

        // Model using the same texture
        Model {
            x: -100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: myTexture
            }
        }

        // Model using the same item
        Model {
            x: 100
            y: -100
            eulerRotation: Qt.vector3d(20, 40, 0)
            source: "#Cube"
            materials: DefaultMaterial {
                diffuseMap: Texture {
                    // Note: Flipping the texture this time
                    sourceItem: myItem
                    flipV: true
                }
            }
        }
    }
}
