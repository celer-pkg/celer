// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "black"

    View3D {
        anchors.fill: parent
        camera: camera

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 200, 300)
            eulerRotation.x: -30
        }

        Model {
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            x: -100
            source: "#Cone"
            materials: [
                CustomMaterial {
                    TextureInput {
                        id: texInput
                        enabled: true
                        texture: Texture {
                            source: "../shared/maps/oulu_2.jpeg"
                        }
                    }

                    shadingMode: CustomMaterial.Unshaded
                    vertexShader: "customunshadedtexture.vert"
                    fragmentShader: "customunshadedtexture.frag"
                    property real coordXFactor: 1.0
                    property real coordYFactor: 1.0
                    property TextureInput tex: texInput
                }
            ]
        }

        Model {
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            x: 100
            source: "#Cone"
            materials: [
                CustomMaterial {
                    shadingMode: CustomMaterial.Unshaded
                    vertexShader: "customunshadedtexture.vert"
                    fragmentShader: "customunshadedtexture.frag"
                    property real coordXFactor: 0.5
                    property real coordYFactor: 0.8
                    property TextureInput tex: texInput
                }
            ]
        }
    }
}
