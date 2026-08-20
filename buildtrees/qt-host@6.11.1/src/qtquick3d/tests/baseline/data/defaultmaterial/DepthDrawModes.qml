// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D

Rectangle {
    width: 700
    height: 400
    color: "lightgray"

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 200
        color: "green"
    }

    View3D {
        anchors.fill: parent

        Texture {
            id: diffuseMap
            source: "../shared/maps/texture_withAlpha.png"
            generateMipmaps: true
            mipFilter: Texture.Linear
        }

        Node {

            Model {
                id: car1
                x: -200
                y: 100
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.y: 45
                source: "../shared/models/carCombined.mesh"
                materials: DefaultMaterial {
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                    //roughness: 0.240909
                    //alphaMode: PrincipledMaterial.Blend
                    depthDrawMode: Material.OpaqueOnlyDepthDraw
                }
            }
            Model {
                id: car2
                x: 200
                y: 100
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.y: 45
                source: "../shared/models/carCombined.mesh"
                materials: DefaultMaterial {
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                    //roughness: 0.240909
                    //alphaMode: PrincipledMaterial.Blend
                    depthDrawMode: Material.AlwaysDepthDraw
                }
            }
            Model {
                id: car3
                x: -200
                y: -100
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.y: 45
                source: "../shared/models/carCombined.mesh"
                materials: DefaultMaterial {
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                    //roughness: 0.240909
                    //alphaMode: PrincipledMaterial.Blend
                    depthDrawMode: Material.NeverDepthDraw
                }
            }
            Model {
                id: car4
                x: 200
                y: -100
                scale: Qt.vector3d(50, 50, 50)
                eulerRotation.y: 45
                source: "../shared/models/carCombined.mesh"
                materials: DefaultMaterial {
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                    //roughness: 0.240909
                    //alphaMode: PrincipledMaterial.Blend
                    depthDrawMode: Material.OpaquePrePassDepthDraw
                }
            }
            Model {
                id: cube1
                scale: Qt.vector3d(2, 2, 2)
                x: 600
                y: 200
                source: "#Cube"
                materials: DefaultMaterial {
                    depthDrawMode: Material.AlwaysDepthDraw
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                }
            }
            Model {
                id: cube2
                scale: Qt.vector3d(2, 2, 2)
                x: 600
                y: -200
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseMap: diffuseMap
                    opacityChannel: Material.A
                    depthDrawMode: Material.OpaquePrePassDepthDraw
                }
            }
        }

        DirectionalLight {
            eulerRotation.x: -20
        }

        PerspectiveCamera {
            id: camera1
            z: 600
            y: 250
            x: 200
            eulerRotation.x: -20
        }
    }
}
