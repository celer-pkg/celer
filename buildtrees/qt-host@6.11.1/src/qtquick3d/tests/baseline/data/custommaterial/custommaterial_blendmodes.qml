// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#848895"
        }
        PerspectiveCamera {
            id: camera
            z: 500
        }
        DirectionalLight {
            brightness: 2
         }

        // Background model
        Model {
            source: "#Cube"
            scale: Qt.vector3d(2, 2, 2)
            rotation: Quaternion.fromEulerAngles(-45, -45, 22.5)
            materials: [
                CustomMaterial {
                    // Blend mode Screen
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.One
                    sourceAlphaBlend: CustomMaterial.One
                    destinationAlphaBlend: CustomMaterial.One
                    property color uColor: "#a8171a"
                    fragmentShader: "customblend2.frag"
                }
            ]
        }

        // Foreground model
        Model {
            source: "#Cone"
            position.y: -100
            scale: Qt.vector3d(3, 3, 3)
            materials: [
                CustomMaterial {
                    // Blend mode Multiply
                    sourceBlend: CustomMaterial.DstColor
                    destinationBlend: CustomMaterial.Zero
                    sourceAlphaBlend: CustomMaterial.One
                    destinationAlphaBlend: CustomMaterial.One
                    property color uColor: "#17a81a"
                    fragmentShader: "customblend2.frag"
                }
            ]
        }

    }
}
