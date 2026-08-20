// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick3D
import QtQuick

Rectangle {
    width: 400
    height: 400
    color: "grey"

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
                            // no source (force dummy texture path
                        }
                    }

                    fragmentShader: "depthdraw.frag"
                    property TextureInput baseColor: texInput
                }
            ]
        }

        Model {
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            x: 100
            source: "#Cone"
            materials: [
                CustomMaterial {
                    TextureInput {
                        id: texInput2
                        enabled: true
                        texture: Texture {
                            // no source (force dummy texture path
                        }
                    }

                    fragmentShader: "depthdraw.frag"
                    property TextureInput baseColor: texInput2
                    depthDrawMode: Material.OpaquePrePassDepthDraw
                }
            ]
        }

        Model {
            source: "#Rectangle"
            eulerRotation.x: -90
            materials: PrincipledMaterial {

            }
           scale: Qt.vector3d(10, 10, 10)
        }

        DirectionalLight {
            eulerRotation.y: -90
            eulerRotation.x: -30
            castsShadow: true
        }
    }
}
